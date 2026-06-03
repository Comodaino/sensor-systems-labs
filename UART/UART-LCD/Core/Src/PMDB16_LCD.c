#include "string.h"
#include "stm32f4xx_hal.h"
#include "PMDB16_LCD.h"

// PMDB16 16x2 LCD driver — v1.2
// 4-bit parallel interface for WH1602C on STM32F4 Discovery
// v1.1: fix drawBar overflow at value=80; remove HAL_Delay from callbacks
// v1.2: add minimum delay after lcd_clear

// Custom bargraph character slots (CGRAM addresses 1-5)
#define CHAR_1_5 0x01
#define CHAR_2_5 0x02
#define CHAR_3_5 0x03
#define CHAR_4_5 0x04
#define CHAR_5_5 0x05

// Bargraph bitmaps: 1/5 to 5/5 columns filled
static const uint8_t CUSTOM_1_5[] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
static const uint8_t CUSTOM_2_5[] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
static const uint8_t CUSTOM_3_5[] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
static const uint8_t CUSTOM_4_5[] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
static const uint8_t CUSTOM_5_5[] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};

// LCD GPIO pins (STM32F4 Discovery / PMDB16 shield)
#define LCD_RS    GPIOB, GPIO_PIN_2
#define LCD_E     GPIOB, GPIO_PIN_1
#define LCD_D4    GPIOB, GPIO_PIN_12
#define LCD_D5    GPIOB, GPIO_PIN_13
#define LCD_D6    GPIOB, GPIO_PIN_14
#define LCD_D7    GPIOB, GPIO_PIN_15
#define LCD_BL_ON GPIOA, GPIO_PIN_4

// LCD controller commands
#define LCD_CMD_CLEAR    0x01
#define LCD_CMD_DISPLAY  0x08
#define LCD_CMD_DISPLAY_ON 0x04
#define LCD_CMD_SETDDRAM 0x80

// Microsecond delay using DWT cycle counter
// Credit: https://deepbluembedded.com/stm32-delay-microsecond-millisecond-utility-dwt-delay-timer-delay/
static void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
}

static void DWT_Delay_us(volatile uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = (HAL_RCC_GetHCLKFreq() / 1000000) * us;
    while ((DWT->CYCCNT - start) < ticks - (HAL_RCC_GetHCLKFreq() / 1000000));
}

static void lcd_enable(void)
{
    HAL_GPIO_WritePin(LCD_E, GPIO_PIN_SET);
    DWT_Delay_us(50);
    HAL_GPIO_WritePin(LCD_E, GPIO_PIN_RESET);
    DWT_Delay_us(50);
}

static void lcd_write4(uint8_t nibble)
{
    HAL_GPIO_WritePin(LCD_D4, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    lcd_enable();
}

static void lcd_write(uint8_t byte)
{
    lcd_write4(byte >> 4);
    lcd_write4(byte);
}

static void lcd_command(uint8_t byte)
{
    HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_RESET);
    lcd_write(byte);
}

static void lcd_data(uint8_t byte)
{
    HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_SET);
    lcd_write(byte);
}

static void writeCustomChar(uint8_t slot, const uint8_t *map)
{
    slot &= 0x07;
    lcd_command(0x40 | (slot << 3));
    for (int i = 0; i < 8; i++)
        lcd_data(map[i]);
}

static void loadCustomChars(void)
{
    writeCustomChar(CHAR_1_5, CUSTOM_1_5);
    writeCustomChar(CHAR_2_5, CUSTOM_2_5);
    writeCustomChar(CHAR_3_5, CUSTOM_3_5);
    writeCustomChar(CHAR_4_5, CUSTOM_4_5);
    writeCustomChar(CHAR_5_5, CUSTOM_5_5);
}

// --- Public API ---

void lcd_clear(void)
{
    lcd_command(LCD_CMD_CLEAR);
    DWT_Delay_us(2000);
}

void setCursor(uint8_t col, uint8_t row)
{
    if ((col + 1) * (row + 1) < 80)
        lcd_command(LCD_CMD_SETDDRAM | (col + 40 * row));
}

void lcd_print(char *string)
{
    while (*string)
        lcd_data(*string++);
}

void lcd_println(char *string, uint8_t row)
{
    char line[17] = "                ";
    int size = strlen(string);
    if (size > 16)
        size = 16;
    for (int i = 0; i < size; i++)
        line[i] = string[i];
    setCursor(0, row);
    lcd_print(line);
}

void lcd_drawBar(int value)
{
    char bar[16];
    if (value > 80) value = 80;
    int full  = value / 5;
    int frac  = value % 5;
    int i = 0;
    while (i < full)
        bar[i++] = CHAR_5_5;
    if (i < 16)
        bar[i++] = (frac == 0) ? ' ' : (CHAR_1_5 + frac - 1);
    while (i < 16)
        bar[i++] = ' ';
    setCursor(0, 1);
    lcd_print(bar);
}

void lcd_initialize(void)
{
    HAL_Delay(50);
    HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_E,  GPIO_PIN_RESET);
    DWT_Delay_Init();
    // HD44780 reset sequence (4-bit mode entry)
    lcd_write4(0x03); HAL_Delay(5);
    lcd_write4(0x03); HAL_Delay(5);
    lcd_write4(0x03); HAL_Delay(5);
    lcd_write4(0x02);
    lcd_write(0x28);           // 4-bit, 2 lines, 5x8 font
    HAL_Delay(5);
    lcd_write(0x08);           // display off
    lcd_write(LCD_CMD_CLEAR);  // clear display
    HAL_Delay(5);
    lcd_write(0x06);           // entry mode: increment, no shift
    lcd_write(LCD_CMD_DISPLAY | LCD_CMD_DISPLAY_ON);
    lcd_write(0x02);           // return home
    HAL_Delay(2);
    loadCustomChars();
    HAL_GPIO_WritePin(LCD_BL_ON, GPIO_PIN_SET);
}

void lcd_backlight_ON(void)
{
    HAL_GPIO_WritePin(LCD_BL_ON, GPIO_PIN_SET);
}

void lcd_backlight_OFF(void)
{
    HAL_GPIO_WritePin(LCD_BL_ON, GPIO_PIN_RESET);
}
