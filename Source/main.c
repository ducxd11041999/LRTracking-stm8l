#include "stm8l15x.h"
#include "stm8l15x_conf.h"

#include "Radio.h"

#define IS_MASTER 0U

#define RF_FREQUENCY                                2400000000UL// Hz
#define TX_OUTPUT_POWER                             0U// dBm
#define RX_TIMEOUT_TICK_SIZE                        RADIO_TICK_SIZE_1000_US
#define RX_TIMEOUT_VALUE                            1000U // ms
#define TX_TIMEOUT_VALUE                            1000U // ms
#define BUFFER_SIZE                                 255U

uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";
#define PINGPONGSIZE                                4U



typedef enum
{
  APP_LOWPOWER,
  APP_RX,
  APP_RX_TIMEOUT,
  APP_RX_ERROR,
  APP_TX,
  APP_TX_TIMEOUT,
} AppStates_t;

void txDoneIRQ( void );
void rxDoneIRQ( void );
void rxSyncWordDoneIRQ( void );
void rxHeaderDoneIRQ( void );
void txTimeoutIRQ( void );
void rxTimeoutIRQ( void );
void rxErrorIRQ( IrqErrorCode_t errCode );
void rangingDoneIRQ( IrqRangingCode_t val );
void cadDoneIRQ( bool cadFlag );
void getRegIRQ (uint32_t reg);
void LoRa( void );

RadioCallbacks_t Callbacks = {
  txDoneIRQ,
  rxDoneIRQ,
  rxSyncWordDoneIRQ,
  rxHeaderDoneIRQ,
  txTimeoutIRQ,
  rxTimeoutIRQ,
  rxErrorIRQ,
  rangingDoneIRQ,
  cadDoneIRQ,
  getRegIRQ
};

extern const Radio_t Radio;

uint16_t RxIrqMask = IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT;
uint16_t TxIrqMask = IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT;


PacketParams_t packetParams;
PacketStatus_t packetStatus;
ModulationParams_t modulationParams;

AppStates_t AppState = APP_LOWPOWER;
uint8_t Buffer[BUFFER_SIZE];
uint8_t BufferSize = BUFFER_SIZE;
uint8_t counter = 0;

void main() {
  Radio.Init(&Callbacks);
  Radio.SetRegulatorMode( USE_DCDC );  // Can also be set in LDO mode but consume more power

  modulationParams.PacketType = PACKET_TYPE_LORA;
  modulationParams.Params.LoRa.SpreadingFactor = LORA_SF7;
  modulationParams.Params.LoRa.Bandwidth = LORA_BW_1600;
  modulationParams.Params.LoRa.CodingRate = LORA_CR_4_5;

  packetParams.PacketType = PACKET_TYPE_LORA;
  packetParams.Params.LoRa.PreambleLength = 12;
  packetParams.Params.LoRa.HeaderType = LORA_PACKET_VARIABLE_LENGTH;
  packetParams.Params.LoRa.PayloadLength = 1;
  packetParams.Params.LoRa.Crc = LORA_CRC_ON;
  packetParams.Params.LoRa.InvertIQ = LORA_IQ_NORMAL;

  Radio.SetStandby( STDBY_RC );
  Radio.SetPacketType( modulationParams.PacketType );
  Radio.SetModulationParams( &modulationParams );
  Radio.SetPacketParams( &packetParams );
  Radio.SetRfFrequency( RF_FREQUENCY );
  Radio.SetBufferBaseAddresses( 0x00, 0x00 );
  Radio.SetTxParams( TX_OUTPUT_POWER, RADIO_RAMP_20_US );

  uint8_t syncWord[] = {0xDD, 0xA0, 0x96, 0x69, 0xDD};
  Radio.SetSyncWord( 1, syncWord);


  if (IS_MASTER)
  {
    Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
  }
  else
  {
    Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
    Radio.SetRx( ( TickTime_t ) {
      RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE
    }  );
  }
  //AppState = APP_LOWPOWER;
  // uint8_t dataIn;
//  asm("rim\n");
  enableInterrupts();
  while(1)
  {
    // dataIn = SPI_ReceiveData(SPI1);
    
    // Uart_SendNumber(dataIn);
    delay(20000);
    LoRa();
  }
}

void LoRa() {
  if (IS_MASTER)
  {
    Radio.SendPayload( &counter, 10, ( TickTime_t ) {
      RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE
    }, 0 );
    
    if (++counter > 100) counter = 0;
    //Uart_SendNumber(counter);
    delay(100000);
  }
  else
  {
    switch (AppState)
    {
      case APP_LOWPOWER:
        break;
      case APP_RX:
        AppState = APP_LOWPOWER;

        Radio.GetPayload( Buffer, &BufferSize, BUFFER_SIZE );
        if (BufferSize > 0)
        {
          Uart_SendData8String(" RX \n");
         // Uart_SendData8String(" Buffer size :");
          Uart_SendNumber(BufferSize);
          //Serial.println(" bytes:");

          for (int i = 0; i < BufferSize; i++)
          {
            //Serial.println(Buffer[i]);
            Uart_SendNumber(Buffer[i]);
          }
        }

        Radio.SetRx( ( TickTime_t ) {
          RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE
        }  );
        break;
      case APP_RX_TIMEOUT:
        AppState = APP_LOWPOWER;

        Uart_SendData8String("Timeout");
      //  Uart_SendNumber(0);
        Radio.SetRx( ( TickTime_t ) {
          RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE
        }  );

        break;
      case APP_RX_ERROR:
        AppState = APP_LOWPOWER;
        break;
      case APP_TX:
        AppState = APP_LOWPOWER;
        break;
      case APP_TX_TIMEOUT:
        AppState = APP_LOWPOWER;
        break;
      default:
        AppState = APP_LOWPOWER;
        break;
    }
  }
}

void txDoneIRQ( void )
{
  AppState = APP_TX;
  Uart_SendData8String("Sent");
}

void rxDoneIRQ( void )
{             
  GPIO_ToggleBits(GPIOA, GPIO_Pin_4);
  
  AppState = APP_RX;
}

void rxSyncWordDoneIRQ( void )
{
}

void rxHeaderDoneIRQ( void )
{
}

void txTimeoutIRQ( void )
{
  GPIO_ToggleBits(GPIOA, GPIO_Pin_4);
  AppState = APP_TX_TIMEOUT;
}

void rxTimeoutIRQ( void )
{
  AppState = APP_RX_TIMEOUT;
}

void rxErrorIRQ( IrqErrorCode_t errCode )
{
  AppState = APP_RX_ERROR;
}

void rangingDoneIRQ( IrqRangingCode_t val )
{
}

void cadDoneIRQ( bool cadFlag )
{
}
void getRegIRQ (uint32_t reg)
{
  Uart_SendNumber(reg);
}