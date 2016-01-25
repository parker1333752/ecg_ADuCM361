#include "i2c.h"
#include "I2cLib.h"
#include "DioLib.h"
#include "cmsis_os.h"
#include "timer.h"

#define I2C_READ_BIT 0x01
#define I2C_WRITE_BIT 0x00

volatile uint8_t i2c_rx_data;

I2cRxTxBufferDef txbuffer;
I2cRxTxBufferDef rxbuffer;

void I2C_init(void)
{
	I2cMCfg(0, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX, I2CMCON_MAS_EN);
	I2cStretch(MASTER, STRETCH_EN);
	I2cBaud(0x12, 0x13); // 400kHz clockx
	
	// GPIO init
	DioCfgPin(pADI_GP0, PIN1, 2);
	DioCfgPin(pADI_GP0, PIN2, 2);
//	DioPul(pADI_GP0, 0xF9);     // External pull ups required externally

	NVIC_EnableIRQ(I2CM_IRQn); 
	
	rxbuffer.index = rxbuffer.len = 0;
}

uint8_t I2C_read(int addr, int reg)
{
	uint8_t data;
	I2C_readMulti(addr,reg,&data,1);
	return data;
}

void I2C_readMulti(int addr, int reg, uint8_t* data, int bytesCount)
{
	volatile int status;
	int i;
	I2cFifoFlush(0,1);
	txbuffer.data[0] = reg;
	txbuffer.len = 1;txbuffer.index = 0;
	
	rxbuffer.len = bytesCount;
	rxbuffer.index = RX_BUFFER_NEED_TO_READ;
	rxbuffer.addr = addr;
	
	I2cTx(0, txbuffer.data[txbuffer.index++]);
	I2cMWrCfg(addr);
	osSignalWait(SIG_I2C_TXRX_COMPLETE, osWaitForever);
	for(i = 0;i<bytesCount;++i){
		data[i] = rxbuffer.data[i];
	}
}

// This function can't run concurrently with I2C_read;
void I2C_write(int addr, int reg, int data)
{
	volatile int status;
	txbuffer.data[0] = reg;
	txbuffer.data[1] = data;
	txbuffer.len = 2;txbuffer.index = 0;
	I2cTx(0, txbuffer.data[txbuffer.index++]);
	I2cMWrCfg(addr);
	osSignalWait(SIG_I2C_TXRX_COMPLETE, osWaitForever);
}

//int32_t getTime(){
//	return getCurrentCount_Timer1();
//}
