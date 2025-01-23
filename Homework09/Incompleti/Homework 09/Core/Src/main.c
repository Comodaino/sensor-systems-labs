
#define SIZE 2


struct symbol_s{
	uint8_t c;
	uint8_t r;
};

struct symbol_s sym_c[8] = {
		{1,34},
		{2,65},
		{4,65},
		{8,65},
		{16,62}};

struct symbol_s sym_p[8] = {
		{1,48},
		{2,72},
		{4,72},
		{8,72},
		{16,127}
};


uint8_t data[SIZE];
int counter = 0;
int flag = 0;

HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim2){
		if(flag == 0){
			data[0] = sym_c[counter].r;
			data[1] = sym_c[counter].c;
		}else{
			data[0] = sym_p[counter].r;
			data[1] = sym_p[counter].c;
		}
		HAL_SPI_Transmit_DMA(&hspi1, &data, SIZE);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
		counter = (counter+1)%5;
	}
	if(htim == &htim3){
		flag = !flag;
	}
}
