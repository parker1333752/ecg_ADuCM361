#include "thread_i2c.h"
#include "mpu6050.h"
#include "I2cLib.h"
#include "thread_uart.h"
#include "timer.h"
#include "stdlib.h"
#include "string.h"

extern volatile uint8_t i2c_rx_data;
extern I2cRxTxBufferDef txbuffer;
extern I2cRxTxBufferDef rxbuffer;
osThreadId tid_Thread_i2c;

volatile int init_result;
osEvent os_result;
TM_MPU6050_t data;
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

typedef struct{
	int locked;
}QueueLock;
QueueLock mpu_queue_lock;
char takeQueueLock(QueueLock* lock){
	int temp;
	do{
		temp = __ldrex(&lock->locked);
	}while((temp == 1) || __strex(1,&lock->locked) != 0);
	return temp;
}
void releaseQueueLock(QueueLock* lock){
	lock->locked = 0;
}
void initQueueLock(QueueLock* lock){
	releaseQueueLock(lock);
}

#define MpuDataDefQueueSize 5
typedef struct {
	MpuDataDef data[MpuDataDefQueueSize];
	int front;
	int rear;
	int size;
}MPUqueueDef;
MPUqueueDef MpuDataQueue;
MPUqueueDef* const PMpuDataQueue = &MpuDataQueue;
void initQMPU(MPUqueueDef* const th){
	th->size = MpuDataDefQueueSize;
	th->front = th->rear = 0;
}
void pushQMPU(MPUqueueDef* const th, MpuDataDef* data){
	int last;
	takeQueueLock(&mpu_queue_lock);
	last = (th->rear-1)>0?((th->rear-1)):(th->rear+th->size-1);
	if(memcmp(&th->data[last], data, 12) == 0){
		//discard repetitive data;
	}else{
		MPUcpy(data, &th->data[th->rear]);
		th->rear++;
		if(th->rear >= th->size)th->rear = 0;
		// Discards oldest data;
		if(th->rear == th->front)th->front = (th->front+1)%th->size;
	}
	releaseQueueLock(&mpu_queue_lock);
}
MpuDataDef* popQMPU(MPUqueueDef* const th){
	MpuDataDef* rt;
	takeQueueLock(&mpu_queue_lock);
	rt = &th->data[th->front++];
	if(th->front >= th->size)th->front = 0;
	releaseQueueLock(&mpu_queue_lock);
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
		return &th->data[tmp];
	}else{
		return popQMPU(th);
	}
}
MpuDataDef* popNewestMPUforExt(void){
	return popNewestMPU(PMpuDataQueue);
}

void Thread_i2c (void const *argument) {
	MpuDataDef mpu_data;

	if(TM_MPU6050_Result_Ok != TM_MPU6050_Init(&data,TM_MPU6050_Device_0,TM_MPU6050_Accelerometer_8G,TM_MPU6050_Gyroscope_250s)){
		return;
	}
	
	initQMPU(PMpuDataQueue);
	initQueueLock(&mpu_queue_lock);
	
	while(1) {
//		if(TM_MPU6050_ReadSta(&data, DATA_RDY_INT)){
			osDelay(1);
			TM_MPU6050_ReadAll(&data, &mpu_data);
			pushQMPU(PMpuDataQueue, &mpu_data);
//			UART_Write_Frame(0x02, sizeof(MpuDataDef), &mpu_data);
//		}
	}
}

void I2C0_Master_Int_Handler(void)
{
	int uiStatus = I2cSta(0);
	uint8_t data;
	if((uiStatus & I2CMSTA_RXREQ) == I2CMSTA_RXREQ) {/* i2c receive interrupt */
		data = I2cRx(0);
		if(rxbuffer.index < rxbuffer.len){
			rxbuffer.data[rxbuffer.index++] = data;
		}
	}
	if((uiStatus & I2CMSTA_TXREQ) == I2CMSTA_TXREQ) {// Master Transmit IRQ 
		if (txbuffer.index < txbuffer.len ) {
			I2cTx(0, txbuffer.data[txbuffer.index++]);
		} else { // TXREQ disabled to avoid multiple unecessary interrupts 
			I2cMCfg(0, I2CMCON_IENCMP|I2CMCON_IENRX, I2CMCON_MAS_EN);
			if(rxbuffer.index == RX_BUFFER_NEED_TO_READ){
				rxbuffer.index = 0;
				I2cMRdCfg(rxbuffer.addr, rxbuffer.len, I2CMRXCNT_EXTEND_DIS);
			}
		}
	}
	if((uiStatus & I2CMSTA_TCOMP_SET) == I2CMSTA_TCOMP_SET) {// communication complete	
		// TXREQ enabled for future master transmissions    
		I2cMCfg(0, I2CMCON_IENCMP|I2CMCON_IENRX|I2CMCON_IENTX, I2CMCON_MAS_EN);
//   		if(rxbuffer.index == RX_BUFFER_NEED_TO_READ){
//			rxbuffer.index = 0;
//			I2cMRdCfg(rxbuffer.addr, rxbuffer.len, I2CMRXCNT_EXTEND_DIS);
//		if(rxbuffer.index < rxbuffer.len){
//			
//		}else{
			osSignalSet(tid_Thread_i2c, SIG_I2C_TXRX_COMPLETE);
//		}
	}
}
