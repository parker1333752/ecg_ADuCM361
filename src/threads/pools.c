/* author Sijun Li */
#include "pools.h"

void initQ(MqueueDef* const th,int size,uint8_t* data){
	th->size = size;
	th->data = data;
	th->front = th->rear = 0;
}
void pushQ(MqueueDef* const th, uint8_t data){
	th->data[th->rear++] = data;
	if(th->rear >= th->size)th->rear = 0;
	// Discards oldest data;
	if(th->rear == th->front)th->front = (th->front+1)%th->size;
}
uint8_t popQ(MqueueDef* const th){
	uint8_t rt = th->data[th->front++];
	if(th->front >= th->size)th->front = 0;
	return rt;
}
int emptyQ(MqueueDef* const th){
	return (th->front == th->rear);
}
int sizeQ(MqueueDef* const th){
	int tmp = th->rear - th->front;
	if(tmp<0)tmp += th->size;
	return tmp;
}
