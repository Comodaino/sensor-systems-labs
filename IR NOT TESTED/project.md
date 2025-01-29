# Objective: 
IR UART
# Instruction:
First import the keyboard project
## UI:
enable TIM2 channel 3 which is connected to the IR LED
(PWM Generation CH3 32kHz prescaler: 0, counter period 2210)
## Code:
We add UART_IR_sendByte(char) to the while replacing the UART_DMA_Transmit()
Then we declare UART_IR_sendByte(char):

```
int BaudElapsedFlag = 0; // note that in TIM_PeriodElapsed_Callback, the flag is set to 1 in case the timer 10 calls it

void UART_IR_sendByte(char byte){
    HAL_TIM_Base_Start_IT(&htim10);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3); //send the start bit

    BaudElapsedFlag = 0;
    while(BaudElapsedFlag == 0){
        // just wait
    }
    
    for (int bit_ctr = 0; bit_ctr<8; bit_ctr++){
        //checks if the bit at position bit_ctr is a 0
        if((byte & (0x01<<bit_ctr)) == 0){
            HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
        } else {
            HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
        }
        BaudElapsedFlag = 0;
        while(BaudElapsedFlag == 0){
            // just wait
        }
    }

    //Send the stop bit
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
    
}

//optional string code

void UART_IR_sendString(char *payload, int size){
    for(int i = 0; i<size; i++){
        UART_IT_sendByte(payload[i]);
        HAL_Delay(1);
    }
}

```
---
# Receiver
##
### UI:
PA10 and PA9 must be enabled for USART1

### Code:
REMEMBER TO CHECK IF huart1 IS SELECTED
```
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1){

    	HAL_UART_Receive_DMA(&huart1, &RxChar, 1);
	    switch(RxChar){
	    	case '0': current_sym = &sym_p;
	    		break;
	    	case '1': current_sym = &sym_c;
	   	    	break;
	    	default: current_sym = &sym_c;
	   	    	break;
	    }
    }

}
```
