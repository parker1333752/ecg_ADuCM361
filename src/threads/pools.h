#ifndef __POOLS_H__
#define __POOLS_H__

#define _PoolDef(name, count, type) type name[count];int name##_point = 0;const int name##_MAX = count;
#define _PoolExternDeclaration(name, type) extern type name[];extern int name##_point;extern const int name##_MAX;
#define _PoolAlloc(name) ( &name[name##_point = (name##_point+1)%name##_MAX] )
//#define _PoolGetTop(name) ( &name[name##_point] )
//#define _PoolInc(name) ( name##_point = (name##_point+1)%name##_MAX )

#endif
