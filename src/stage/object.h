#ifndef PSXF_GUARD_OBJECT_H
#define PSXF_GUARD_OBJECT_H

#include "../psx/psx.h"

//Object types
typedef struct Object
{
	//Object linked list
	struct Object *prev, *next;
	
	//Object functions
	boolean (*tick)(struct Object*);
	void (*free)(struct Object*);
} Object;

typedef Object* ObjectList;

//Object functions
void ObjectList_Add(ObjectList *list, Object *obj);
void ObjectList_Remove(ObjectList *list, Object *obj);
void ObjectList_Tick(ObjectList *list);
void ObjectList_Free(ObjectList *list);

#endif