/**
 * Notice : all function begin with ADXL_read../ADXL_write../ADXL_set../ADXL_get/ must
 *           running in Thread_acc() (in main.c file).
 */

#include "adxl346.h"
#include "thread_acc.h"

typedef union{
  int16_t data;
  uint8_t data_[2];
}byte2int16;

#define SCLK_PORT   pADI_GP1
#define SCLK_PIN    PIN5

#define SI_PORT     pADI_GP1
#define SI_PIN      PIN6

#define SO_PORT     pADI_GP1
#define SO_PIN      PIN4

#define CSn_PORT    pADI_GP1
#define CSn_PIN     PIN7

#define INT2_PORT   pADI_GP0
#define INT2_PIN    PIN5

void ADXL346_init(void){
	// pin config
	DioCfgPin(SCLK_PORT,SCLK_PIN,2);
	DioCfgPin(SI_PORT,SI_PIN,2);
	DioCfgPin(SO_PORT,SO_PIN,2);
	DioCfgPin(CSn_PORT,CSn_PIN,2);
	
	// SPI config
	SpiBaud(SPIx,10,SPIDIV_BCRST_DIS); 
	SpiCfg(SPIx, SPICON_MOD_TX2RX2, SPICON_MASEN_EN, \
			SPICON_CON_EN | SPICON_SOEN_EN | SPICON_RXOF_EN | SPICON_ZEN_EN | \
			SPICON_TIM_TXWR | SPICON_CPOL_HIGH | SPICON_CPHA_SAMPLETRAILING | SPICON_ENABLE_EN);
	
	NVIC_EnableIRQ(SPI0_IRQn);	// config spi0 nvic enable
}

int16_t ADXL_read_datax(void){
	byte2int16 data;
    data.data_[0] = Spix_read_reg(DATAX0);
    data.data_[1] = Spix_read_reg(DATAX1);
    return data.data;
}

int16_t ADXL_read_datay(void){
    byte2int16 data;
    data.data_[0] = Spix_read_reg(DATAY0);
    data.data_[1] = Spix_read_reg(DATAY1);
    return data.data;
}

int16_t ADXL_read_dataz(void){
    byte2int16 data;
    data.data_[0] = Spix_read_reg(DATAZ0);
    data.data_[1] = Spix_read_reg(DATAZ1);
    return data.data;
}

/**
 * ADXL_set_fifo, set fifo configuration
 * @param  conf = {FIFO_MODE|Trigger|Samples}
 *    FIFO_MODE:
 *        0x00 , fifo is bypass
 *        0x40 , fifo collect new data when fifo is not full 
 *        0x80 , fifo collect all new data and throw oldest data when filo is full
 *        0xc0 , fifo collect all new data and throw oldest data when filo is full
 *        0x40 , fifo collect new data when fifo is not full, 
 *                and generate a trigger event in INT pin when fifo is full
 *    Trigger:
 *        0 , trigger link to INT1 pin
 *        0x20 , trigger link to INT2 pin
 *    Samples: 
 *        0 ~ 32 ,when fifo data count > Samples , trigger a watermark interrupt
 *                or generate a trigger event (INT1 or INT2)
 * @return     0
 */
uint8_t ADXL_set_fifo(uint8_t conf)
{
    Spix_write_reg(FIFO_CTL, conf);
    return 0;
}

/**
 * ADXL_fifo_count
 * @return  how many data values in FIFO
 *    0~32
 */
uint8_t ADXL_get_fifocount(void)
{
    uint8_t data;
    data = Spix_read_reg(FIFO_STATUS);
    data &= 0x3f;
    return data;
}

/**
 * ADXL_get_fifotrig 
 * @return  check if fifo event is occurred
 *    0 , trigger event has not occurred
 *    1 , trigger event is occurring
 */
uint8_t ADXL_get_fifotrig(void)
{
    uint8_t data;
    data = Spix_read_reg(FIFO_STATUS);
    data &= 0x80;
    return (data == 0x80);
}
