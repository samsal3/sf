#ifndef SF_H
#define SF_H

#include <stdint.h>

#define SF_OFFSET_OF(type, name) ((intptr_t)&(((type *)NULL)->name))
#define SF_SIZE(a) (sizeof(a) / sizeof(0 [a]))
#define SF_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SF_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SF_CLAMP(v, l, h) SF_MAX(SF_MIN(v, h), l);
#define SF_FALSE 0
#define SF_TRUE 1

#define SF_ASSERT(e) sf_assert(e, __FILE__, __func__, __LINE__, #e)

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SF_ARRAY_INIT(a, value)                                                                             \
	do {                                                                                                \
		size_t i_;                                                                                  \
		for (i_ = 0; i_ < SF_SIZE(a); ++i_)                                                         \
			(a)[i_] = value;                                                                    \
	} while (0)

#ifdef SF_IMPLEMENTATION

#define SF_EXTERNAL
#define SF_INTERNAL static

#else

#define SF_EXTERNAL
#define SF_INTERNAL static

#endif // SF_IMPLEMENTATION

typedef int32_t sf_bool;

struct sf_arena {
	char	*data;
	uint64_t position;
	uint64_t alignment;
	uint64_t capacity;
};

struct sf_string {
	uint64_t    size;
	char const *data;
};

struct sf_queue {
	struct sf_queue *next;
	struct sf_queue *previous;
} sf_queue;

SF_EXTERNAL void *sf_allocate(struct sf_arena *arena, uint64_t size);
SF_EXTERNAL void *sf_allocate_struct(struct sf_arena *arena, uint64_t n, uint64_t struct_size);
SF_EXTERNAL void sf_scratch(struct sf_arena *arena, uint64_t capacity, struct sf_arena *scratch);
SF_EXTERNAL void sf_clear_arena(struct sf_arena *arena);

#define SF_S8_FROM_LITERAL(s, literal)                                                                      \
	do {                                                                                                \
		(s)->data = &(literal)[0];                                                                  \
		(s)->size = SF_SIZE(literal);                                                               \
	} while (0)

#define SF_QUEUE_IS_EMPTY(queue) ((queue) == (queue)->next)

#define SF_QUEUE_NEXT(queue) ((queue)->next)

#define SF_QUEUE_NEXT_NO_SOURCE_NODE(source, queue)                                                         \
	((queue)->next != (source) ? (queue)->next : (queue)->next->next)

#define SF_QUEUE_HEAD(queue) ((queue)->next)

#define SF_QUEUE_PREVIOUS(queue) ((queue)->previous)

#define SF_QUEUE_FOR_EACH(it, head) for ((it) = (head)->next; (it) != (head); (it) = (it)->next)

#define SF_QUEUE_DATA(queue, type, node_name) (type *)((intptr_t)(queue) - SF_OFFSET_OF(type, node_name))

#define SF_QUEUE_INIT(queue)                                                                                \
	do {                                                                                                \
		(queue)->previous = (queue);                                                                \
		(queue)->next	  = (queue);                                                                \
	} while (0)

#define SF_QUEUE_INSERT_HEAD(head, queue)                                                                   \
	do {                                                                                                \
		(queue)->next		= (head)->next;                                                     \
		(queue)->previous	= (head);                                                           \
		(queue)->next->previous = (queue);                                                          \
		(head)->next		= (queue);                                                          \
	} while (0)

#define SF_QUEUE_REMOVE(queue)                                                                              \
	do {                                                                                                \
		(queue)->next->previous = (queue)->previous;                                                \
		(queue)->previous->next = (queue)->next;                                                    \
	} while (0)

#define SF_QUEUE_ADD(head, node)                                                                            \
	do {                                                                                                \
		(head)->previous->next = (node)->next;                                                      \
		(head)->next->previous = (node)->previous;                                                  \
		(head)->previous       = (node)->previous;                                                  \
		(head)->previous->next = (node);                                                            \
	} while (0)

#define SF_MEMORY_COPY(destination, source, count)                                                          \
	do {                                                                                                \
		uintptr_t i_;                                                                               \
		for (i_ = 0; i_ < (count); ++i_)                                                            \
			((char *)(destination))[i_] = ((char const *)(source))[i_];                         \
	} while (0)

SF_EXTERNAL void sf_string_create_from_non_literal(char const *non_literal, struct sf_string *result);

SF_EXTERNAL sf_bool sf_string_compare(struct sf_string *lhs, struct sf_string *rhs, uint64_t max_size);

SF_EXTERNAL void sf_string_copy(struct sf_arena *arena, struct sf_string *source, struct sf_string *result);

SF_EXTERNAL void
sf_string_null_terminate(struct sf_arena *arena, struct sf_string *source, struct sf_string *result);

SF_EXTERNAL void sf_assert(sf_bool test, char const *file, char const *fn, int line, char const *expr);

#define SF_IMPLEMENTATION
#ifdef SF_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

SF_INTERNAL uint64_t u64align(uint64_t value, uint64_t alignment) {
	return (value + alignment - 1) & ~(alignment - 1);
}

SF_EXTERNAL void *sf_allocate(struct sf_arena *arena, uint64_t size) {
	char	*memory = NULL;
	uint64_t i, new_size = 0;

	if (!arena || !size)
		return NULL;

	new_size	= arena->position + size;
	memory		= &arena->data[arena->position];
	arena->position = u64align(new_size, arena->alignment);

	for (i = 0; i < size; ++i)
		memory[i] = 0x0;

	return memory;
}

SF_EXTERNAL void *sf_allocate_struct(struct sf_arena *arena, uint64_t n, uint64_t struct_size) {
	return sf_allocate(arena, n * struct_size);
}

SF_EXTERNAL void sf_scratch(struct sf_arena *arena, uint64_t capacity, struct sf_arena *scratch) {
	scratch->data = sf_allocate(arena, capacity);
	if (scratch->data)
		scratch->capacity = capacity;
	else
		scratch->capacity = 0;
}

SF_EXTERNAL void sf_clear_arena(struct sf_arena *arena) { arena->position = 0; }

SF_INTERNAL uint64_t sf_string_find_non_literal_size(char const *non_literal, uint64_t max_size) {
	uint64_t i;

	for (i = 0; i < max_size; ++i)
		if ('\0' == non_literal[i])
			return i;

	return max_size;
}

SF_EXTERNAL void sf_string_create_from_non_literal(char const *non_literal, struct sf_string *result) {
	result->data = non_literal;
	result->size = sf_string_find_non_literal_size(non_literal, 1024);
}

SF_EXTERNAL sf_bool sf_string_compare(struct sf_string *lhs, struct sf_string *rhs, uint64_t max_size) {
	uint64_t i;

	if (lhs->size != rhs->size)
		return SF_FALSE;

	for (i = 0; i < SF_MIN(lhs->size, max_size); ++i)
		if (lhs->data[i] != rhs->data[i])
			return SF_FALSE;

	return SF_TRUE;
}

SF_EXTERNAL void sf_string_copy(struct sf_arena *arena, struct sf_string *source, struct sf_string *result) {
	char *data = sf_allocate(arena, source->size);
	if (data) {
		SF_MEMORY_COPY(data, source->data, source->size);
		result->data = data;
		result->size = source->size;
	} else {
		result->data = NULL;
		result->size = source->size;
	}
}

SF_EXTERNAL void
sf_string_null_terminate(struct sf_arena *arena, struct sf_string *source, struct sf_string *result) {
	char *data = sf_allocate(arena, source->size + 1);
	if (data) {
		SF_MEMORY_COPY(data, source->data, source->size);
		data[source->size] = '\0';
		result->data	   = data;
		result->size	   = source->size;
	} else {
		result->data = NULL;
		result->size = source->size;
	}
}

SF_EXTERNAL void sf_assert(sf_bool test, char const *file, char const *fn, int line, char const *expr) {
	if (!test) {
		fprintf(stderr, "%s:%i - %s - SF_ASSERT(%s)\n", file, line, fn, expr);
		abort();
	}
}

#endif // SF_IMPLEMENTATION

#endif // SF_H
