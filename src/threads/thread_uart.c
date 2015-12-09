#include "thread_uart.h"
#include "uart.h"
#include "utils.h"
#include <stdio.h>
#include "pools.h"

/* ========== Define Signal Numbers ========================================= */
#define SIG_UART_SEND_COMPLETE 0x55


/* ========== Define Message Queues ========================================= */
/* Define uart write message queue.
 * For storing data that need to send from MCU. */
#define Se_UartWrite_Count 1 // only one uart peripherals.
osSemaphoreId Se_UartWrite;
osSemaphoreDef (Se_UartWrite);
#define UartWriteQueueSize 2000
typedef struct {
	uint8_t data[UartWriteQueueSize];
	int front;
	int rear;
}MqueueDef;
MqueueDef __UartWriteQueue;
MqueueDef* const UartWriteQueue = &__UartWriteQueue;
void initQ(MqueueDef* const th){
	th->front = th->rear = 0;
}
void pushQ(MqueueDef* const th, uint8_t data){
	th->data[th->rear++] = data;
	if(th->rear >= UartWriteQueueSize)th->rear = 0;
	// Discards oldest data;
	if(th->rear == th->front)th->front = (th->front+1)%UartWriteQueueSize;
}
uint8_t popQ(MqueueDef* const th){
	uint8_t rt = th->data[th->front++];
	if(th->front >= UartWriteQueueSize)th->front = 0;
	return rt;
}
int emptyQ(MqueueDef* const th){
	return (th->front == th->rear);
}

/* Define uart read message queue. Used for transform MPU data. */
#define Q_UartRead_Size 30
osMessageQId Q_UartRead;
osMessageQDef (Q_UartRead, Q_UartRead_Size, uint32_t);

/* ========== Define variable pools ========================================= */
/* Define Pool for usage of MPU data queue. */
_PoolDef(P_MPUData, Q_UartRead_Size, MPUDataDef);

/* ========== Define thread ids ============================================= */
osThreadId tid_Thread_uart;
osThreadId tid_Thread_uart_send;

/* Definition of Uart send thread. This thread will create in Thread_uart. */
void Thread_uart_send (void const*);
osThreadDef_t ThreadDef_uart_send = {Thread_uart_send, osPriorityNormal, 1, 0};

/* ========== Define Global variables ======================================= */
/* For test if uart init complete,
 * prevent calling uart send before initialization completed. */
char flagUartInitComplete = 0;

/* ========== Define Global Functions ======================================= */
// Declare status machine for uart read data process.
int status_machine(int status, uint8_t data, MPUDataDef **pmpu);

/* Thread funcion for uart data reading and processing.
 * Use message for getting data from uart read (after status machine),
 * and send this data to PC.
 */
void Thread_uart (void const *argument) {
	osEvent os_result;
	MPUDataDef *pmpu = NULL;
	int32_t tickcount_start;

	// Configure uart peripheral.
	UART_init();

	// Create message queue
	Q_UartRead = osMessageCreate(osMessageQ(Q_UartRead), NULL);
	
	// Create semaphoreCreate for uart write queue
	Se_UartWrite = osSemaphoreCreate(osSemaphore(Se_UartWrite), Se_UartWrite_Count);
	initQ(UartWriteQueue);

	// create thread uart send
	tid_Thread_uart_send = osThreadCreate(&ThreadDef_uart_send, NULL);

	// Tell other threads that initialization complete.
	flagUartInitComplete = 1;

	tickcount_start = getCurrentCount_Timer1();
	while(1) {
		os_result = osMessageGet(Q_UartRead, osWaitForever);
		if(os_result.status == osEventMessage){
			pmpu = (MPUDataDef*)os_result.value.v;
			pmpu->date -= tickcount_start;
			UART_Write_Frame(0x02, sizeof(MPUDataDef), pmpu);
		}
	}
}

/* Status machine for receiving MPU6050 module's data.
 * If return value is 0xff, means that a complete frame received.
 */
int status_machine(int status, uint8_t data, MPUDataDef **rt_pmpu){
	static uint8_t s[10];
	static int s_top = 0;
	MPUDataDef *pmpu = *rt_pmpu;
	uint8_t sum;
	if(status == 0){ // idle
		if(data == 0x55)status = 1;
	}else if(status == 1){ // received first byte;
		if(data == 0x51){
			s_top = 0;status = 2; // acc
		}else if(data == 0x52){
			s_top = 0;status = 3; // omega
		}else if(data == 0x53){
			s_top = 0;status = 4; // angle
		}else{ status = 0; } // error status.
	}else if(status == 2 || status == 3 || status == 4){ // receive data
		if(s_top < 9){
			s[s_top++] = data;
		}else status += 100;
	}else if(status > 100){
		status = status - 101 + 0x50;
		sum = 0xff & status;
		sum += 0x55;
		sum -= s[--s_top];
		while(s_top > 0)sum += s[--s_top];
		if(sum != 0)status = 0; // sum check failed;
		else{
			if(pmpu == NULL)pmpu = _PoolAlloc(P_MPUData);
			if(status == 0x51){ // acc
				pmpu->accx = *(int16_t*)(s+0);
				pmpu->accy = *(int16_t*)(s+2);
				pmpu->accz = *(int16_t*)(s+4);
				status = 0;
			}else if(status == 0x52){ // omega
				pmpu->omegax = *(int16_t*)(s+0);
				pmpu->omegay = *(int16_t*)(s+2);
				pmpu->omegaz = *(int16_t*)(s+4);
				status = 0;
			}else if(status == 0x53){ // angle
				pmpu->anglex = *(int16_t*)(s+0);
				pmpu->angley = *(int16_t*)(s+2);
				pmpu->anglez = *(int16_t*)(s+4);
				pmpu->date = getCurrentCount_Timer1();
				status = 0xff;
			}else status = 0; // status error.
		}
	}
	*rt_pmpu = pmpu;
	return status;
}

#define USE_PACKAGE
#ifdef USE_PACKAGE
	#define PACKAGE_ESC		0xfa
	#define PACKAGE_START 	0xfb
	#define PACKAGE_END		0xfc
	#define PACKAGE_X1		0xff
	#define PACKAGE_X2		0xaa
	#define PACKAGE_E_ESC	0x01
	#define PACKAGE_E_START	0x02
	#define PACKAGE_E_END	0x03
	#define PACKAGE_E_X1	0x04
	#define PACKAGE_E_X2	0x05
#endif

void package_and_write(uint8_t ch){
#ifdef USE_PACKAGE
	if(ch == PACKAGE_ESC){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_ESC);
	}else if(ch == PACKAGE_START){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_START);
	}else if(ch == PACKAGE_END){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_END);
	}else if(ch == PACKAGE_X1){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_X1);
	}else if(ch == PACKAGE_X2){
		pushQ(UartWriteQueue, PACKAGE_ESC);
		pushQ(UartWriteQueue, PACKAGE_E_X2);
	}else{
		pushQ(UartWriteQueue, ch);
	}
#else
	push(UartWriteQueue, ch);
#endif
}

// Called by any thread, for sending TLV frame.
// If definded USE_PACAGE, data will be packaged.
void UART_Write_Frame(uint8_t tag, uint16_t length, void* value)
{
	uint8_t temp,i;
	uint8_t* p = value;
	uint8_t sumcheck = 0;

	/* discards data before uart init completed. */
	if (flagUartInitComplete == 0) return;
	
	// Wait for queue available;
	osSemaphoreWait(Se_UartWrite, osWaitForever);

	#ifdef USE_PACKAGE
		pushQ(UartWriteQueue, PACKAGE_START);
	#endif
	// Tag
	package_and_write(tag);
	// Length
	temp = length & 0xff;
	package_and_write(temp);
	temp = length >> 8;
	package_and_write(temp);
	// Value
	for(i=0;i<length;++i){
		package_and_write(*(p+i));
		sumcheck += *(p+i);
	}
	// Sum check
	package_and_write(sumcheck);
	#ifdef USE_PACKAGE
		pushQ(UartWriteQueue, PACKAGE_END);
	#endif
	
	// release Uart Queue lock;
	osSemaphoreRelease(Se_UartWrite);
}

void Thread_uart_send (void const *argument) {
	uint8_t data;

	while(1) {
		if(emptyQ(UartWriteQueue))osThreadYield();
		else {
			data = popQ(UartWriteQueue);
			UrtTx(pADI_UART, data);
			osSignalWait(SIG_UART_SEND_COMPLETE, osWaitForever);
		}
	}
}

void UART_Int_Handler(void)
{
	int flag = UrtIntSta(pADI_UART);
	uint8_t data;
	static int mpu_status = 0;
	static MPUDataDef *mpu_data = NULL;
	if((flag & COMIIR_NINT) != 0) return;
	if((flag & COMIIR_STA_MSK) == COMIIR_STA_RXBUFFULL){
		/* uart receive interrupt */
		data = UART_read();
		if ( (mpu_status=status_machine(mpu_status, data, &mpu_data)) == 0xff){
			osMessagePut(Q_UartRead, (uint32_t)mpu_data, 0);
			mpu_data = NULL;
			mpu_status = 0;
		}
	} else if((flag & COMIIR_STA_MSK) == COMIIR_STA_TXBUFEMPTY){
		/* uart send complete interrupt */
		osSignalSet(tid_Thread_uart_send,SIG_UART_SEND_COMPLETE);
	}
}
