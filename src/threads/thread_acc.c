#include "thread_acc.h"
#include "adxl346.h"

/**
 * Id pointer.
 * If value is not NULL, means thread has been created.
 * Also used to send signal into thread from other threads or IRQs.
 */
osThreadId tid_Thread_acc;
extern osThreadId tid_Thread_uart;
char flagSuspend_acc = 0;

#define SIG_THREAD_ACC_SUSPENDED 0x71
#define SIG_THREAD_ACC_RESUME 0x91

#pragma pack(push, 1)
typedef struct{
	int16_t datax;
	int16_t datay;
	int16_t dataz;
	int32_t date;
}AccDataDef;
#pragma pack(pop)

extern void UART_Write_Frame(uint8_t, uint16_t, void*);

void Thread_acc (void const *argument)
{
	AccDataDef acc_data;
	
	// Periperals init.
	ADXL346_init();
	
	while(1) {
        if(flagSuspend_acc){
            // TODO :May need a delay
            osSignalSet(tid_Thread_uart, SIG_THREAD_ACC_SUSPENDED);
            osSignalWait(SIG_THREAD_ACC_RESUME, osWaitForever);
        }
		//acc_data.datax = Spix_read_reg(DEVID);
		acc_data.datax = ADXL_read_datax();
		acc_data.datay = ADXL_read_datay();
		acc_data.dataz = ADXL_read_dataz();
		acc_data.date = 1445428299;
		UART_Write_Frame(0x01, sizeof(acc_data), (void*)&acc_data);
		// Delay ms
		osDelay(100);
	}
}

volatile uint16_t Spi0ReadData = 0;
void SPI0_Int_Handler(void)
{
	const uint8_t flag = SpiSta(pADI_SPI0);
    if((flag & SPI0STA_TX_MSK) == SPI0STA_TX){	// SPI0 Tx IRQ
		Spi0ReadData = SpiRx(pADI_SPI0);
		Spi0ReadData = (Spi0ReadData << 8);
		Spi0ReadData |= SpiRx(pADI_SPI0);
		osSignalSet(tid_Thread_acc,0x01);
	}
}

uint8_t Spix_read_reg(uint8_t reg)
{
   uint8_t data;
	
	// Flush fifo
    SpiFifoFlush(SPIx, SPICON_TFLUSH_EN, SPICON_RFLUSH_DIS);

    SpiTx(SPIx, reg | 0xc0);
    SpiTx(SPIx, 0x00);
	osSignalWait(0x01,osWaitForever);

    data = (uint8_t)(Spi0ReadData & 0xff);

    return data;
}

void Spix_write_reg(uint8_t reg, uint8_t data)
{
	// Flush fifo
    SpiFifoFlush(SPIx, SPICON_TFLUSH_EN, SPICON_RFLUSH_EN);
  
    SpiTx(SPIx, reg | 0x40);
    SpiTx(SPIx, data);
	osSignalWait(0x01,osWaitForever);
}
