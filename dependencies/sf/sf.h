#ifndef SF_H
#define SF_H

#include <stdint.h>

#define SF_OFFSET_OF(type, name) ((intptr_t)&(((type *)NULL)->name))
#define SF_SIZE(a) (sizeof(a) / sizeof(0 [a]))
#define SF_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SF_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SF_FALSE 0
#define SF_TRUE 1

#define SF_ASSERT(e) sfAssert(e, __FILE__, __func__, __LINE__, #e)

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SF_ARRAY_INIT(a, value)           \
   do {                                   \
      size_t i_;                          \
      for (i_ = 0; i_ < SF_SIZE(a); ++i_) \
         (a)[i_] = value;                 \
   } while (0)

#ifdef SF_IMPLEMENTATION

#define SF_EXTERNAL
#define SF_INTERNAL static

#else

#define SF_EXTERNAL
#define SF_INTERNAL static

#endif // SF_IMPLEMENTATION


typedef int32_t SfBool;

typedef struct SfArena {
   char *data;
   uint64_t position;
   uint64_t alignment;
   uint64_t capacity;
} SfArena;

typedef struct SfS8 {
   uint64_t size;
   char *data;
} SfS8;

typedef struct SfQueue {
   struct SfQueue *next;
   struct SfQueue *previous;
} SfQueue;

SF_EXTERNAL void *sfAllocate(SfArena *arena, uint64_t size);
SF_EXTERNAL SfArena sfScratch(SfArena *arena, uint64_t capacity);
SF_EXTERNAL void sfClearArena(SfArena *arena);

#define SF_S8_FROM_LITERAL(s, literal) \
   do {                                \
      (s)->data = &(literal)[0];       \
      (s)->size = SF_SIZE(literal);    \
   } while (0)

#define SF_QUEUE_IS_EMPTY(queue) ((queue) == (queue)->next)
#define SF_QUEUE_NEXT(queue) ((queue)->next)
#define SF_QUEUE_NEXT_NO_SOURCE_NODE(source, queue) ((queue)->next != (source) ? (queue)->next : (queue)->next->next)
#define SF_QUEUE_HEAD(queue) ((queue)->next)
#define SF_QUEUE_PREVIOUS(queue) ((queue)->previous)
#define SF_QUEUE_FOR_EACH(it, head) for ((it) = (head)->next; (it) != (head); (it) = (it)->next)
#define SF_QUEUE_DATA(queue, type, node_name) (type *)((intptr_t)(queue) - SF_OFFSET_OF(type, node_name))

#define SF_QUEUE_INIT(queue)       \
   do {                            \
      (queue)->previous = (queue); \
      (queue)->next = (queue);     \
   } while (0)

#define SF_QUEUE_INSERT_HEAD(head, queue) \
   do {                                   \
      (queue)->next = (head)->next;       \
      (queue)->previous = (head);         \
      (queue)->next->previous = (queue);  \
      (head)->next = (queue);             \
   } while (0)

#define SF_QUEUE_REMOVE(queue)                     \
   do {                                            \
      (queue)->next->previous = (queue)->previous; \
      (queue)->previous->next = (queue)->next;     \
   } while (0)

#define SF_QUEUE_ADD(head, node)                 \
   do {                                          \
      (head)->previous->next = (node)->next;     \
      (head)->next->previous = (node)->previous; \
      (head)->previous = (node)->previous;       \
      (head)->previous->next = (node);           \
   } while (0)

#define SF_MEMORY_COPY(destination, source, count)                         \
   do {                                                                    \
      uintptr_t i_;                                                        \
      for (i_ = 0; i_ < (count); ++i_)                                     \
         ((sf_byte *)(destination))[i_] = ((char const *)(source))[i_]; \
   } while (0)

SF_EXTERNAL SfS8 sfCreateS8FromNonLiteral(char const *nonLiteral);
SF_EXTERNAL SfBool sfCompareS8(SfS8 lhs, SfS8 rhs, uint64_t max_size);
SF_EXTERNAL SfS8 sfCopyS8(SfArena *arena, SfS8 source);
SF_EXTERNAL SfS8 sfNullTerminateS8(SfArena *arena, SfS8 source);
SF_EXTERNAL void sfAssert(SfBool test, char const *file, char const *fn, int line, char const *expr);

#define SF_IMPLEMENTATION
#ifdef SF_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

SF_INTERNAL uint64_t sfAlignU64(uint64_t value, uint64_t alignment) {
   return (value + alignment - 1) & ~(alignment - 1);
}

SF_EXTERNAL void *sfAllocate(SfArena *arena, uint64_t size) {
   char *memory = NULL;

   if (arena && !size) {
      uint64_t newSize = arena->position + size;

      if (newSize < arena->capacity) {
         memory = &arena->data[arena->position];

         arena->position = sfAlignU64(newSize, arena->alignment);

         for (uint64_t i = 0; i < size; ++i)
            memory[i] = 0x0;
      }
   }

   return memory;
}

SF_EXTERNAL SfArena sfScratch(SfArena *arena, uint64_t capacity) {
   SfArena result = {0};

   result.data = sf_allocate(arena, capacity);
   if (result.data)
      result.capacity = capacity;

   return result;
}

SF_EXTERNAL void sfClearArena(SfArena *arena) {
   arena->position = 0;
}

SF_INTERNAL uint64_t sfFindNonLiteralSize(char const *nonLiteral, uint64_t maxSize) {
   uint64_t result = maxSize;

   for (uint64_t i = 0; i < maxSize && result == maxSize; ++i)
      if ('\0' == nonLiteral[i])
         result = i;

   return result;
}

SF_EXTERNAL SfS8 sfCreateS8FromNonLiteral(char const *nonLiteral) {
   SfS8 result = {
      .size = sfFindNonLiteralSize(nonLiteral, 1024),
      .data = nonLiteral,
   };
   return result;
}

SF_EXTERNAL SfBool sfCompareS8(SfS8 lhs, SfS8 rhs, uint64_t max_size) {
   SfBool result = SF_FALSE;

   if (lhs.size == rhs.size) {
      result = SF_TRUE;

      for (uint64_t i = 0; i < SF_MIN(lhs.size, max_size) && result; ++i)
         if (lhs.data[i] != rhs.data[i])
            result = SF_FALSE;
   }

   return result;
}

SF_EXTERNAL SfS8 sfCopyS8(SfArena *arena, SfS8 source) {
   SfS8 result = {0};

   result.data = sfAllocate(arena, source.size);
   if (result.data) {
      result.size = source.size;
      SF_MEMORY_COPY(result.data, source.data, source.size);
   }

   return result;
}

SF_EXTERNAL SfS8 sfNullTerminateS8(SfArena *arena, SfS8 source) {
   SfS8 result = {0};

   result.data = sf_allocate(arena, source.size + 1);
   if (result.data) {
      SF_MEMORY_COPY(result.data, source.data, source.size);
      result.data[source.size] = '\0';
      result.size = source.size;
   }

   return result;
}

SF_EXTERNAL void sfAssert(SfBool test, char const *file, char const *fn, int line, char const *expr) {
   if (!test) {
      fprintf(stderr, "%s:%i - %s - SF_ASSERT(%s)\n", file, line, fn, expr);
      abort();
   }
}

#endif // SF_IMPLEMENTATION

#endif // SF_H
