#include "thread_i2c.h"
#include "mpu6050.h"
#include "I2cLib.h"
#include "thread_uart.h"
#include "timer.h"
#include "stdlib.h"

extern volatile uint8_t i2c_rx_data;
extern I2cRxTxBufferDef txbuffer;
extern I2cRxTxBufferDef rxbuffer;
osThreadId tid_Thread_i2c;

volatile int init_result;
osEvent os_result;
TM_MPU6050_t data;
MpuDataDef mpu_data;
int32_t time;

int MPUcpy(MpuDataDef *source ,MpuDataDef *dest){
	dest->Accelerometer_X = source->Accelerometer_X;
	dest->Accelerometer_Y = source->Accelerometer_Y;
	dest->Accelerometer_Z = source->Accelerometer_Z;
	dest->Gyroscope_X = source->Gyroscope_X;
	dest->Gyroscope_Y = source->Gyroscope_Y;
	dest->Gyroscope_Z = source->Gyroscope_Z;
	dest->date = source->date;
	return 0;
}

typedef struct {
	MpuDataDef* data;
	int front;
	int rear;
	int size;
}MPUqueueDef;
MPUqueueDef MpuDataQueue;
MPUqueueDef* const PMpuDataQueue = &MpuDataQueue;
void initQMPU(MPUqueueDef* const th,int size){
	th->size = size;
	th->data = malloc(size*sizeof(MPUqueueDef));
	th->front = th->rear = 0;
}
void pushQMPU(MPUqueueDef* const th, MpuDataDef* data){
	MPUcpy(data, (th->data + th->rear));
	th->rear++;
	if(th->rear >= th->size)th->rear = 0;
	// Discards oldest data;
	if(th->rear == th->front)th->front = (th->front+1)%th->size;
}
MpuDataDef* popQMPU(MPUqueueDef* const th){
	MpuDataDef* rt = (th->data + th->front);
	th->front++;
	if(th->front >= th->size)th->front = 0;
	return rt;
}
int emptyQMPU(MPUqueueDef* const th){
	return (th->front == th->rear);
}
MpuDataDef* popNewestMPU(MPUqueueDef* const th){
	int tmp;
	if(emptyQMPU(th)){
		tmp = th->rear-1;
		if(tmp<0)tmp = th->size-1;
		return (th->data + tmp);
	}else{
		return popQMPU(th);
	}
}
MpuDataDef* popNewestMPUforExt(void){
	return popNewestMPU(PMpuDataQueue);
}

void Thread_i2c (void const *argument) {
	/* initialize */
	if(TM_MPU6050_Result_Ok != TM_MPU6050_Init(&data,TM_MPU6050_Device_0,TM_MPU6050_Accelerometer_8G,TM_MPU6050_Gyroscope_250s)){
		// TODO: Change to periodically check device if device not found.
		return;
	}
	initQMPU(PMpuDataQueue,3);
//	Timer0_init(400);
	
	while(1) {
//		if(TM_MPU6050_ReadSta(&data, DATA_RDY_INT)){
			osDelay(2);
			TM_MPU6050_ReadAll(&data);
			mpu_data.Accelerometer_X = data.Accelerometer_X;
			mpu_data.Accelerometer_Y = data.Accelerometer_Y;
			mpu_data.Accelerometer_Z = data.Accelerometer_Z;
			mpu_data.Gyroscope_X = data.Gyroscope_X;
			mpu_data.Gyroscope_Y = data.Gyroscope_Y;
			mpu_data.Gyroscope_Z = data.Gyroscope_Z;
			mpu_data.date = data.Time & 0x7fff; 
			pushQMPU(PMpuDataQueue, &mpu_data);
//			UART_Write_Frame(0x02, sizeof(MpuDataDef), &mpu_data);
//		}
	}
}

void GP_Tmr0_Int_Handler(){
	volatile int flag = GptSta(pADI_TM0);
	MpuDataDef* tmp;
	if(flag & TSTA_TMOUT){
		GptClrInt(pADI_TM0, TCLRI_TMOUT);
		// mpu_data.date = getTime() & 0x7fff;
		tmp = popNewestMPU(PMpuDataQueue);
		UART_Write_Frame(0x02, sizeof(MpuDataDef), tmp);
//		UART_Write_Frame(0x02, sizeof(MpuDataDef), &mpu_data);
	} else if(flag & TSTA_CAP){
		GptClrInt(pADI_TM0, TCLRI_CAP);
	}
}
void I2C0_Master_Int_Handler(void)
{
	volatile int uiStatus = I2cSta(0);
	volatile uint8_t data;
	if((uiStatus & I2CMSTA_RXREQ) == I2CMSTA_RXREQ) {/* i2c receive interrupt */
		data = I2cRx(0)&0xff;
		if(rxbuffer.index < rxbuffer.len){
			rxbuffer.data[rxbuffer.index++] = data;
			if(rxbuffer.index == rxbuffer.len){
				osSignalSet(tid_Thread_i2c, SIG_I2C_RX_COMPLETE);
			}
		}
	}
	if((uiStatus & I2CMSTA_TXREQ) == I2CMSTA_TXREQ) {// Master Transmit IRQ 
		if (txbuffer.index < txbuffer.len ) {
			I2cTx(0, txbuffer.data[txbuffer.index++]);
		} else { // TXREQ disabled to avoid multiple unecessary interrupts 
			I2cMCfg(I2CMCON_TXDMA_DIS|I2CMCON_RXDMA_DIS, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX_DIS, I2CMCON_MAS_EN);
		}
	}
	if((uiStatus & I2CMSTA_TCOMP_SET) == I2CMSTA_TCOMP_SET) {// communication complete	
		I2cMCfg(I2CMCON_TXDMA_DIS|I2CMCON_RXDMA_DIS, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX_EN, I2CMCON_MAS_EN);   // TXREQ enabled for future master transmissions    
		if(rxbuffer.index == 0 && rxbuffer.index < rxbuffer.len){
			I2cMRdCfg(rxbuffer.addr, rxbuffer.len, I2CMRXCNT_EXTEND_DIS);
		} else {
			osSignalSet(tid_Thread_i2c, SIG_I2C_TX_COMPLETE);
		}
	}
}
