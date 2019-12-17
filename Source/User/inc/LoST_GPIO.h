#include "stm8l15x_gpio.h"

typedef enum level{
  LOW = 0,
  HIGH = 1
} status;

void GPIO_Init_NSS_BUSY_NRESET(void);
void GPIO_digitalWrite(GPIO_Pin_TypeDef Pin, status value);
status GPIO_digitalRead(GPIO_Pin_TypeDef Pin);