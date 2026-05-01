#ifndef SF_CORE_H
#define SF_CORE_H

#include <stdint.h>

#define SF_OFFSET_OF(type, name) ((intptr_t)&(((type *)NULL)->name))
#define SF_SIZE(a) (sizeof(a) / sizeof(0 [a]))
#define SF_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SF_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SF_CLAMP(v, l, h) SF_MAX(SF_MIN(v, h), l);
#define SF_FALSE 0
#define SF_TRUE 1

#define SF_ASSERT(e) sfAssert(e, SF_STRING(__FILE__), SF_STRING(__func__), SF_STRING(__LINE__), #e)

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SF_ARRAY_INIT(a, value)                                                                                       \
	do {                                                                                                          \
		size_t i_;                                                                                            \
		for (i_ = 0; i_ < SF_SIZE(a); ++i_)                                                                   \
			(a)[i_] = value;                                                                              \
	} while (0)

#ifdef SF_IMPLEMENTATION

#define SF_EXTERNAL
#define SF_INTERNAL static

#else

#define SF_EXTERNAL
#define SF_INTERNAL static

#define SF_INLINE inline

#endif // SF_IMPLEMENTATION

typedef unsigned char SFByte;
typedef uint32_t      U32;
typedef uint64_t U64;

typedef int32_t SFBool;

typedef struct SFArena {
	char	*data;
	U64 position;
   U64 alignment;
   U64    capacity;
} SFArena;

typedef struct SFString {
	U64   size;
	char *data;
} SFString;

typedef struct SFQueue {
	struct SFQueue *next;
	struct SFQueue *previous;
} SFQueue;

#define SF_STRING(literal)                                                                                            \
   (SFString) { SF_SIZE(literal), literal }	

SF_EXTERNAL void *sfArenaAllocate(SFArena *arena, U64 size);
SF_EXTERNAL SFArena sfArenaScratch(SFArena *arena, U64 capacity);
SF_EXTERNAL void    sfArenaClear(SFArena *arena);

SF_INLINE SFBool sfQueueIsEmpty(SFQueue *queue) { 
	return (queue) == (queue)->next; 
}

SF_INLINE SFQueue *sfQueueNext(SFQueue *queue) { 
	return (queue)->next; 
}

SF_INLINE SFQueue *sfQueueNextNoSourceNode(SFQueue *source, SFQueue *queue) { 
	return (queue)->next != (source) ? (queue)->next : (queue)->next->next; 
}

SF_INLINE SFQueue *sfQueueHead(SFQueue *queue) { 
	return (queue)->next; 
}

SF_INLINE SFQueue *sfQueuePrevious(SFQueue *queue) { 
	return (queue)->previous; 
}

#define SF_QUEUE_FOR_EACH(it, head) for ((it) = (head)->next; (it) != (head); (it) = (it)->next)
#define SF_QUEUE_DATA(queue, type, name) (type *)((intptr_t)(queue) - SF_OFFSET_OF(type, name))

SF_INLINE void sfQueueInit(SFQueue *queue) {
	queue->previous = queue;
	queue->next = queue;
}

SF_INLINE void sfQueueInsertHead(SFQueue *head, SFQueue *queue) {
	queue->next = head->next;
	queue->previous = head;
	queue->next->previous = queue;
	head->next = queue;
}

SF_INLINE void sfQueueRemove(SFQueue *queue) {
	queue->next->previous = queue->previous;
	queue->previous->next = queue->next;
}

SF_INLINE void sfMemoryCopy(void *destination, void const *source, U64 count) {
   U64 i;
	for (i = 0; i < count; ++i)
		((SFByte *)destination)[i] = ((SFByte const *)source)[i];
}

SF_INLINE void sfMemorySet(void *destination, int value, U64 count) {
	U64 i;
	for (i = 0; i < count; ++i)
		((SFByte *)destination)[i] = (SFByte)value;
}

SF_EXTERNAL SFString sfStringFromNonLiteral(char const *non_literal, U64 maxSize);
SF_EXTERNAL SFBool sfStringCompare(SFString lhs, SFString rhs, U64 maxSize);
SF_EXTERNAL SFString sfStringClone(SFArena *arena, SFString source);
SF_EXTERNAL SFString sfStringNullTerminate(SFArena *arena, SFString source);
SF_EXTERNAL void sfAssert(SFBool test, SFString file, SFString function, int line, SFString expresion);

#define SF_IMPLEMENTATION
#ifdef SF_IMPLEMENTATION

#include <stdio.h>

SF_INTERNAL U64 sfU64Align(U64 value, U64 alignment) {
	return (value + alignment - 1) & ~(alignment - 1);
}

SF_EXTERNAL void *sfArenaAllocate(SFArena *arena, U64 size) {
   if (!arena || !size)
      return NULL;

   U64 reqSize = arena->position + size;
	if (reqSize > arena->capacity)
		return NULL;

	SFByte *memory = &arena->data[arena->position];
	arena->position = sfU64Align(reqSize, arena->alignment);

	for (U64 i = 0; i < size; ++i)
      memory[i] = 0x0;

   return memory;
}

SF_EXTERNAL SFArena sfArenaScratch(SFArena *arena, U64 capacity) {
   SFArena result = {0};

	result.data = sfArenaAllocate(arena, capacity);
	if (result.data)
		result.capacity = capacity;

	return result;
}

SF_EXTERNAL void sfArenaClear(SFArena *arena) { arena->position = 0; }

SF_INTERNAL U64 sfNonLiteralStringSize(char const *nonLiteral, U64 maxSize) {
	for (U64 i = 0; i < maxSize; ++i)
		if ('\0' == nonLiteral[i])
			return i;

	return maxSize;
}

SF_EXTERNAL SFString sfStringFromNonLitral(char const *nonLiteral) {
   SFString result = {0};
	
	result.data = nonLiteral;
	result.size = sfNonLiteralSize(nonLiteral, 1024);

	return result;
}

SF_EXTERNAL SFBool sfStringCompare(SFString lhs, SFString rhs, U64 maxSize) {
	if (lhs.size != rhs.size)
		return SF_FALSE;

	for (U64 i = 0; i < SF_MIN(lhs.size, maxSize); ++i)
		if (lhs.data[i] != rhs.data[i])
			return SF_FALSE;

	return SF_TRUE;
}

SF_EXTERNAL SFString sfStringClone(SFArena *arena, SFString source) {
   SFString result = {0};

	result.data = sfArenaAllocate(arena, source.size);
	if (result.data) {
      result.size = source.size;
      sfMemoryCopy(result.data, source.data, result.size);
	}

	return result;
}

SF_EXTERNAL SFString sfStringNullTerminate(SFArena *arena, SFString source) {
   SFString result = {0};

   result.data = sfArenaAllocate(arena, source.size + 1);
   if (result.data) {
      result.size = source.size + 1;
      sfMemoryCopy(result.data, source.data, source.size);
      result.data[source.size] = '\0';
   }

   return result;
}

SF_EXTERNAL void sfAssert(SFBool test, SFString file, SFString function, int line, SFString expression) {
	if (!test) {
		fprintf(stderr, "%.*s:%i - %.*s - SF_ASSERT(%.*s)\n", file.size, file.data, line, function.size, function.data, expression.size, expression.data);
		abort();
	}
}

#endif // SF_IMPLEMENTATION

#endif // SF_H
