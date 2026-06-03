# Sensor Systems Labs — STM32F4 Discovery

A collection of embedded-systems lab projects for the **STM32F407VGT6** (STM32F4 Discovery board). Each project is an independent STM32CubeIDE workspace that demonstrates a specific peripheral or sensor interface.

---

## Hardware Platform

| Item | Details |
|------|---------|
| MCU | STM32F407VGT6 (Cortex-M4, 84 MHz) |
| Board | STM32F4 Discovery |
| UART debug | USART2 → ST-Link VCP, 115200 8N1 |
| LCD shield | PMDB16 (16×2 HD44780, 4-bit parallel) |

### Common Pin Assignments

| Function | Pin |
|----------|-----|
| User LED (LD2) | PA5 |
| User button (B1) | PC13 |
| USART2 TX/RX | PA2 / PA3 |
| I2C1 SCL/SDA | PB6 / PB9 |
| LCD RS / E | PB2 / PB1 |
| LCD D4–D7 | PB12–PB15 |
| LCD backlight | PA4 |

---

## Projects

### 1. UART — Basic Serial Transmit

**Path:** `UART/UART-name-surname/`

Sends a fixed identification string (`"Name Surname 2001\r\n"`) via USART2 DMA every second.

**Key concepts:** UART TX with DMA, `HAL_Delay`-based timing.

---

### 2. UART — LCD Display

**Path:** `UART/UART-LCD/`

Displays a sorted list of names on the PMDB16 LCD. The list is sorted at startup with insertion sort, then a timer cycles through the entries every second.

**Key concepts:** LCD driver, insertion sort, TIM2 periodic interrupt.

---

### 3. LED Toggle with Microphone Input

**Path:** `LED toggle with microphone and PWM/led-micrphone/`

A button press on PA8 (external microphone click signal) toggles the green LED on PA5 via GPIO EXTI interrupt.

**Key concepts:** EXTI interrupt, GPIO toggle.

---

### 4. PWM Output

**Path:** `LED toggle with microphone and PWM/PWM/`

Generates a 1 Hz, 50 % duty-cycle PWM signal on PA5 (TIM2 CH1).

> **Note:** The STM32CubeMX procedure documented in the lab sheet was incorrect. The correct configuration is PA5 → TIM2_CH1, prescaler = 8399, period = 9999 (gives 1 Hz at 84 MHz). See [`IMPORTANT.md`](LED%20toggle%20with%20microphone%20and%20PWM/IMPORTANT.md) for details.

**Key concepts:** TIM PWM mode, `HAL_TIM_PWM_Start`.

---

### 5. Play a Song

**Path:** `Play a song/play_song_TIM/`

Plays "London Bridge Is Falling Down" through a piezo speaker on TIM1 CH2 (PA9). Press PA8 to start playback. TIM2 drives note duration; each note's frequency is set dynamically by reconfiguring TIM1.

**Key concepts:** PWM frequency reconfiguration at runtime, EXTI start trigger, song data as a struct array.

**Note definitions** (timer period at prescaler = 99, 84 MHz clock):

| Note | Period |
|------|--------|
| DO4  | 1603   |
| RE4  | 1429   |
| MI4  | 1273   |
| FA4  | 1203   |
| SOL4 | 1071   |
| LA4  | 955    |
| SI4  | 850    |

---

### 6. ADC — 3-Channel Scan

**Path:** `ADC and LDR/3 channel ADC/`

Reads three ADC channels via DMA (triggered by TIM2 TRGO at 1 Hz) and prints results over UART:

| Channel | Source | Output |
|---------|--------|--------|
| ADC1_IN1 | Potentiometer (PA1) | Voltage (V) |
| TEMPSENSOR | Internal sensor | Temperature (°C) |
| VREFINT | Internal reference | VRef (V) |

**Key concepts:** Multi-channel ADC scan mode, DMA circular transfer, ADC external trigger.

---

### 7. ADC — Light-Dependent Resistor (LDR)

**Path:** `ADC and LDR/Light Dependant Resistor/`

Reads an LDR on PA0 at ~10 kHz (TIM2 TRGO), accumulates samples, and prints the computed lux value once per second (TIM3 interrupt).

**Lux formula:**  
`lux = 10 × (100 kΩ / R_ldr)^1.25`  
where `R_ldr` is calculated from the ADC voltage with a 100 kΩ pull-up.

**Key concepts:** ADC single-channel, accumulation for averaging, `powf`.

---

### 8. Temperature Sensor — LM75 (I2C)

**Path:** `Temperature sensor/Temperature sensor/`

Reads temperature from an **LM75** sensor over I2C1 at 1 Hz. Supports negative temperatures via two's complement decoding of the 9-bit result.

**I2C address:** `0x48` (A0=A1=A2=GND → write `0x90`, read `0x91`)

**Wiring:** SDA → PB9, SCL → PB6

**Output format:** `Temp: xx.xx C\r\n`

**Key concepts:** I2C DMA master receive, two's complement temperature conversion.

---

### 9. Accelerometer — LIS2DE/LIS2DE12 (I2C, Direct)

**Path:** `Accellerometer I2C/Accellerometer "normal"/`

Configures the LIS2DE accelerometer over I2C1 (blocking writes at startup), then reads X/Y/Z axis data at 1 Hz using DMA. Results are printed in g over UART.

**I2C addresses:**

| Variant | Write | Read |
|---------|-------|------|
| LIS2DE  | `0x50` | `0x51` |
| LIS2DE12 | `0x30` | `0x32` |

**Output:** `X: +0.25 g Y: -0.12 g Z: +1.00 g`

**Key concepts:** I2C DMA chained TX→RX, accelerometer register configuration.

---

### 10. Accelerometer — LIS2DE (I2C, State Machine)

**Path:** `Accellerometer I2C/Accellerometer state machine/`

Same hardware as project 9, but all I2C operations (register init + periodic reads) are driven by a state machine through DMA callbacks — no blocking calls anywhere.

**State machine:**

| State | Action |
|-------|--------|
| 0 | Write CTRL_REG1 (enable X/Y/Z, ODR) |
| 1 | Write CTRL_REG2 (disable high-pass) |
| 2 | Write CTRL_REG4 (set FSR) |
| 3 | Start TIM2 periodic interrupt |
| 4 | Send multi-read address (0xA9) |
| 5 | Read 5 bytes → back to state 4 |

**Key concepts:** Non-blocking I2C with state machine, `HAL_I2C_MasterTxCpltCallback` chaining.

---

### 11. LED Matrix (SPI)

**Path:** `LED matrix and SPI/LED matrix/`

Drives a 5×8 LED matrix connected to a shift register via SPI1 DMA. Displays the characters **"C"** and **"P"** alternating every 2 seconds (TIM3). Row multiplex runs at ~2 kHz (TIM2).

**SPI strobe pin:** PB6 (latches shift register on falling + rising edge)

**Key concepts:** SPI DMA TX, GPIO strobe, multi-timer coordination, character bitmap lookup.

---

### 12. Keyboard (4×4 Matrix)

**Path:** `Keyboard and Encoder/Keyboard/`

Scans a 4×4 hex keyboard wired as rows (PC8–PC11) and columns (PC12, PC13, PC2, PC3). Uses software debouncing (3 samples per key state) driven by TIM2 at ~5 ms intervals. Detected keystrokes are sent over UART.

**Keymap:**

```
0  4  8  C
1  5  9  D
2  6  A  E
3  7  B  F
```

**Key concepts:** GPIO matrix scan, debounce via sample counter, state machine (`p_matrix` pending/active/released).

---

### 13. Rotary Encoder

**Path:** `Keyboard and Encoder/Encoder/`

Reads a quadrature rotary encoder using TIM3 in encoder mode (TIM_ENCODERMODE_TI12). TIM2 samples the counter every second and computes angular speed in RPM (assumes 48 pulses per revolution).

**Output:** `Speed: x.xx rpm (old -> new)`

**Key concepts:** Hardware encoder interface, counter overflow handling, angular velocity calculation.

---

### 14. Potentiometer — LCD Display

**Path:** `Potentiometer with LCD UART and variable string/Potentiometer-LCD/`

Reads a potentiometer on PA1 via ADC1 (TIM2-triggered at 1 Hz). Displays voltage on LCD row 0 and a proportional bargraph on row 1.

**Key concepts:** ADC interrupt mode, LCD bargraph, `lcd_drawBar`.

---

### 15. Potentiometer — Remote Terminal

**Path:** `Potentiometer with LCD UART and variable string/Potentiometer-Remote-Terminal/`

Same ADC reading as project 14 but sends voltage over UART instead of LCD.

**Output:** `Voltage: x.xxx V\r\n`

**Key concepts:** ADC + UART DMA, no LCD.

---

### 16. UART → LCD (Variable String)

**Path:** `Potentiometer with LCD UART and variable string/Variable-string/`

Receives characters from UART2 (DMA, one byte at a time) and displays them on the LCD. A carriage return (`\r` or `\n`) or a full 32-character buffer flushes the display: first 16 chars on row 0, next 16 on row 1.

**Key concepts:** UART RX DMA in character-at-a-time mode, `HAL_UART_RxCpltCallback` re-arming.

---

### 17. IR Transmitter (untested)

**Path:** `IR NOT TESTED/IR TX/`

Transmits bytes as UART-framed IR pulses at 32 kHz carrier (TIM2 CH3 PWM). TIM10 provides baud-rate timing. See [`project.md`](IR%20NOT%20TESTED/project.md) for the implementation notes.

> ⚠️ **Not tested on hardware.**

---

### 18. IR Receiver (untested)

**Path:** `IR NOT TESTED/IR RX/`

Receives IR frames via USART1 (PA10). Decoded bytes switch between LED-matrix symbols. See [`project.md`](IR%20NOT%20TESTED/project.md).

> ⚠️ **Not tested on hardware.**

---

## Shared Driver — PMDB16 LCD

**Files:** `<project>/Core/Inc/PMDB16_LCD.h` · `<project>/Core/Src/PMDB16_LCD.c`

Used in: projects 2, 14, 16, and UART-LCD.

### Public API

```c
void lcd_initialize(void);          // Must be called once before any other LCD function
void lcd_clear(void);               // Clear display (2 ms delay required by HD44780)
void setCursor(uint8_t col, uint8_t row);
void lcd_print(char *string);       // Print from current cursor position
void lcd_println(char *string, uint8_t row);  // Print padded to 16 chars on given row
void lcd_drawBar(int value);        // Draw bargraph on row 1, value 0–80
void lcd_backlight_ON(void);
void lcd_backlight_OFF(void);
```

The driver uses the **DWT cycle counter** for microsecond-precision delays (required by the HD44780 initialization sequence) and stores five custom bargraph bitmaps in CGRAM.

---

## Building

All projects are **STM32CubeIDE** workspaces. Open each project folder individually in STM32CubeIDE (File → Open Projects from File System). Each project contains its own copy of the STM32F4xx HAL drivers; no external dependencies are needed.

Flash with the built-in ST-Link programmer (`Run → Debug`). Connect a serial terminal (115200 8N1) to observe UART output on the ST-Link VCP.

---

## Notes on Code Quality

All projects were refactored from the original lab submissions with the following fixes applied:

- **PMDB16_LCD driver:** added include guards; moved private pin/command macros and internal helpers (`static`) into the `.c` file; removed duplicate definitions between `.h` and `.c`.
- **Missing `void` return types** on HAL callback functions (`HAL_TIM_PeriodElapsedCallback`, `HAL_I2C_MasterRxCpltCallback`, etc.) — in C89/C90 an untyped function defaults to `int`, causing undefined behaviour at link time with the HAL weak symbols.
- **Temperature sensor:** fixed two `=` vs `==` comparison bugs that caused callbacks to always execute regardless of which peripheral fired; fixed a `uint8_t *` parameter being passed an integer constant (`LM75_TEMP_ADDRESS`); removed duplicate DMA receive calls in the timer callback; simplified to remove the `times_called` workaround.
- **Buffer sizes:** `snprintf` calls now use `sizeof(buffer)` instead of hardcoded values that exceeded buffer lengths.
- **Unused variables** removed: `start`, `count` (accelerometer state machine); `members_index` (UART-LCD); `reset` (Variable-string); `h`, `len` globals (Keyboard); `matrix_string` (Keyboard).
- **`HAL_UART_Transmit_DMA` type casts:** the API requires `uint8_t *`; `char *` buffers now carry the explicit cast.
- **`const` correctness:** string literals and read-only lookup tables marked `const`.
- **Dead code:** commented-out `HAL_GPIO_TogglePin` in PWM callback removed; unreachable `HAL_ADC_Stop_IT` after infinite loop removed.
