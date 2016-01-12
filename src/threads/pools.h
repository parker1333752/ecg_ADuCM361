#ifndef __POOLS_H__
#define __POOLS_H__
#include <stdint.h>
#include <stdlib.h>

#define _PoolDef(name, count, type) type name[count];int name##_point = 0;const int name##_MAX = count;
#define _PoolExternDeclaration(name, type) extern type name[];extern int name##_point;extern const int name##_MAX;
#define _PoolAlloc(name) ( &name[name##_point = (name##_point+1)%name##_MAX] )
//#define _PoolGetTop(name) ( &name[name##_point] )
//#define _PoolInc(name) ( name##_point = (name##_point+1)%name##_MAX )

typedef struct {
	uint8_t* data;
	int front;
	int rear;
	int size;
}MqueueDef;

void initQ(MqueueDef* const th,int size,uint8_t*);
void pushQ(MqueueDef* const th, uint8_t data);
uint8_t popQ(MqueueDef* const th);
int emptyQ(MqueueDef* const th);
int sizeQ(MqueueDef* const th);

#endif
