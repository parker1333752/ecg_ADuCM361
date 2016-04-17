#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>

// Define signal value.
#define SIG_I2C_TXRX_COMPLETE 0x03

#define RX_BUFFER_NEED_TO_READ 0x7fff

#define I2C_RXTXBUFFER_SIZE 100
typedef struct{
	uint8_t data[I2C_RXTXBUFFER_SIZE];
	int len;
	int index;
	int addr;
}I2cRxTxBufferDef;

void I2C_init(void);

uint8_t I2C_read(int addr, int reg);
uint8_t I2C_readMulti(uint8_t addr,uint8_t reg,uint8_t bytesCount,uint8_t *data);
uint8_t I2C_writeMulti(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);
void I2C_write(int addr, int reg, int data);
int32_t getTime(void);

#endif
