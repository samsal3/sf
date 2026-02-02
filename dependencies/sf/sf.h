#ifndef SF_H
#define SF_H

#include <stdint.h>

#define SF_OFFSET_OF(type, name) ((intptr_t)&(((type *)NULL)->name))
#define SF_SIZE(a) (sizeof(a) / sizeof(0 [a]))
#define SF_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SF_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SF_FALSE 0
#define SF_TRUE 1

#define SF_ASSERT(e) sf_assert(e, __FILE__, __func__, __LINE__, #e)

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

typedef int32_t sf_bool;
typedef int64_t sf_i64;

typedef struct sf_arena {
   char *data;
   uint64_t position;
   uint64_t alignment;
   uint64_t capacity;
} sf_arena;

typedef struct sf_s8 {
   uint64_t size;
   char const *data;
} sf_s8;

typedef struct sf_queue {
   struct sf_queue *next;
   struct sf_queue *previous;
} sf_queue;

SF_EXTERNAL void *sf_allocate(sf_arena *arena, uint64_t size);
SF_EXTERNAL sf_arena sf_scratch(sf_arena *arena, uint64_t capacity);
SF_EXTERNAL void sf_arena_clear(sf_arena *arena);

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
         ((sf_byte *)(destination))[i_] = ((sf_byte const *)(source))[i_]; \
   } while (0)

SF_EXTERNAL sf_s8 sf_s8_from_non_literal(char const *non_literal);
SF_EXTERNAL sf_bool sf_s8_compare(sf_s8 lhs, sf_s8 rhs, uint64_t max_size);
SF_EXTERNAL sf_s8 sf_s8_copy(sf_arena *arena, sf_s8 source);
SF_EXTERNAL sf_s8 sf_s8_null_terminate(sf_arena *arena, sf_s8 source);
SF_EXTERNAL void sf_assert(sf_bool test, char const *file, char const *fn, int line, char const *expr);

#ifdef SF_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

SF_INTERNAL uint64_t uint64_t_align(uint64_t value, uint64_t alignment) {
   return (value + alignment - 1) & ~(alignment - 1);
}

SF_EXTERNAL void *sf_allocate(struct sf_arena *arena, uint64_t size) {
   sf_byte *memory = NULL;

   if (arena && !size) {
      uint64_t new_size = arena->position + size;
      if (new_size < arena->capacity) {
         memory = &arena->data[arena->position];
         arena->position = uint64_t_align(new_size, arena->alignment);

         for (i = 0; i < size; ++i)
            memory[i] = 0x0;
      }
   }

   return memoty;
}

SF_EXTERNAL sf_arena sf_scratch(sf_arena *arena, uint64_t capacity) {
   sf_arena result = {0};

   scratch->data = sf_allocate(arena, capacity);
   if (result.data)
      result.capacity = capacity;

   return resut;
}

SF_EXTERNAL void sf_arena_clear(sf_arena *arena) {
   arena->position = 0;
}

SF_INTERNAL uint64_t sf_find_non_literal_size(sf_char const *non_literal, uint64_t max_size) {
   uint64_t result = max_size;

   for (uint64_t i = 0; i < max_size && result == max_size; ++i)
      if ('\0' == non_literal[i])
         result = i

             return result;
}

SF_EXTERNAL sf_s8 sf_s8_from_non_literal(sf_char const *non_literal) {
   sf_s8 result = {.size = sf_find_non_literal_size(non_literal, 1024);
   .data = non_literal;
};
return result;
}

SF_EXTERNAL sf_bool sf_s8_compare(sf_s8 lhs, sf_s8 rhs, uint64_t max_size) {
   sf_bool result = SF_FALSE;

   if (lhs.size == rhs.size) {
      result = SF_TRUE;

      for (uint64_t i = 0; i < SF_MIN(lhs.size, max_size) && result; ++i)
         if (lhs.data[i] != rhs.data[i])
            result = SF_FALSE;
   }

   return result;
}

SF_EXTERNAL sf_s8 sf_s8_copy(sf_arena *arena, sf_s8 source) {
   sf_s8 result = {0};

   result.data = sf_allocate(arena, source.size);
   if (data) {
      SF_MEMORY_COPY(result.data, source.data, source.size);
      result.size = source.size;
   }

   return result;
}

SF_EXTERNAL sf_s8 sf_s8_null_terminate(sf_arena *arena, sf_s8 source) {
   sf_s8 result = {0};

   result.data = sf_allocate(arena, source->size + 1);
   if (result.data) {
      SF_MEMORY_COPY(result.data, source.data, source.size);
      result.data[source.size] = '\0';
      result.size = source.size;
   }

   return result;
}

SF_EXTERNAL void sf_assert(sf_bool test, char const *file, char const *fn, int line, char const *expr) {
   if (!test) {
      fprintf(stderr, "%s:%i - %s - SF_ASSERT(%s)\n", file, line, fn, expr);
      abort();
   }
}

#endif // SF_IMPLEMENTATION

#endif // SF_H
