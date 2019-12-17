#include "LoST_GPIO.h"
void GPIO_Init_NSS_BUSY_NRESET(void)
{
  //Clear GPIO
  //GPIO_DeInit(GPIOB);
  GPIO_Init(GPIOA, GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow);
  //NRESET - OUTPUT
  GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Slow);
  
  //NSS - OUTPUT
  GPIO_Init(GPIOB, GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow);
  
  //BUSY - INPUT
  GPIO_Init(GPIOB, GPIO_Pin_2, GPIO_Mode_In_FL_No_IT);
  
  //EXTI - DIO1
  GPIO_Init(GPIOB, GPIO_Pin_1, GPIO_Mode_In_FL_IT);
}

void GPIO_digitalWrite(GPIO_Pin_TypeDef Pin, status Value)
{
  GPIO_WriteBit(GPIOB, Pin, Value);
}

status GPIO_digitalRead(GPIO_Pin_TypeDef Pin)
{
  if(GPIO_ReadInputDataBit(GPIOB, Pin) == RESET)
    return LOW;
  else
    return HIGH;
}