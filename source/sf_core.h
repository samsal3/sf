#ifndef SF_CORE_H
#define SF_CORE_H

#include <stdint.h>
#include <assert.h>

#define SF_OFFSET_OF(type, name) ((intptr_t)&(((type *)NULL)->name))
#define SF_SIZE(a) (sizeof(a) / sizeof(0 [a]))
#define SF_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SF_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SF_CLAMP(v, l, h) SF_MAX(SF_MIN(v, h), l);
#define SF_FALSE 0
#define SF_TRUE 1
#define SF_KB(n) ((n) * 1024)
#define SF_MB(n) ((n) * 1024 * 1024)
#define SF_GB(n) ((n) * 1024 * 1024 * 1024)
#define SF_UNUSED(x) (void)(x)

#define SF_STATIC_ASSERT(e, msg) typedef char sf_static_assert_##msg[(e) ? 1 : -1]
#define SF_ASSERT assert
#define SF_STRING(str) sf_string_from_literal(str, SF_SIZE(str) - 1)

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SF_ARRAY_INIT(a, value)                     \
	do {                                        \
		size_t i_;                          \
		for (i_ = 0; i_ < SF_SIZE(a); ++i_) \
			(a)[i_] = value;            \
	} while (0)

#define SF_MEMORY_COPY(destination, source, size)                                     \
	do {                                                                          \
		u64 i_;                                                               \
		for (i_ = 0; i_ < size; ++i_)                                         \
			((sf_byte *)destination)[i_] = ((sf_byte const *)source)[i_]; \
	} while (0)

#define SF_MEMORY_SET(destination, value, size)                          \
	do {                                                             \
		u64 i_;                                                  \
		for (i_ = 0; i_ < size; ++i_)                            \
			((sf_byte *)destination)[i_] = ((sf_byte)value); \
	} while (0)

#define SF_STRING_LITERAL(source, s)            \
	do {                                    \
		(s)->size = sizeof(source) - 1; \
		(s)->data = source;             \
	} while (0)

#define sf_public
#define sf_private static
#define sf_local_persist static

typedef unsigned char sf_byte;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;

typedef int32_t sf_bool;

struct sf_arena {
	sf_byte *data;
	u64 position;
	u64 alignment;
	u64 capacity;
};

struct sf_string {
	u64 size;
	char const *data;
};

sf_public void *sf_arena_allocate(struct sf_arena *arena, u64 size);

sf_public void sf_arena_scratch(struct sf_arena *arena, u64 capacity, struct sf_arena *out);

sf_public void sf_arena_clear(struct sf_arena *arena);

sf_public void sf_string_from_non_literal(char const *non_literal, u64 max_size, struct sf_string *out);

sf_public sf_bool sf_string_compare(struct sf_string const *lhs, struct sf_string const *rhs, u64 max_size);

sf_public void sf_string_clone(struct sf_arena *arena, struct sf_string const *in, struct sf_string *out);

sf_public void sf_string_null_terminate(struct sf_arena *arena, struct sf_string const *in, struct sf_string *out);


sf_public struct sf_string sf_string_from_literal(char const *str, u64 size);

#ifdef SF_CORE_IMPLEMENTATION

#include <stdio.h>

sf_private u64 sf_u64_align(u64 value, u64 alignment) {
	return (value + alignment - 1) & ~(alignment - 1);
}

sf_public void *sf_arena_allocate(struct sf_arena *arena, u64 size) {
	sf_byte *memory = NULL;
	u64 i = 0;
	u64 required_size = 0;

	if (!arena || !size)
		return NULL;

	required_size = arena->position + size;
	if (required_size > arena->capacity)
		return NULL;

	memory = &arena->data[arena->position];
	arena->position = sf_u64_align(required_size, arena->alignment);

	for (i = 0; i < size; ++i)
		memory[i] = 0x0;

	return memory;
}

sf_public void sf_arena_scratch(struct sf_arena *arena, u64 capacity, struct sf_arena *out) {
	out->position = 0;

	out->data = sf_arena_allocate(arena, capacity);
	if (out->data) {
		out->alignment = arena->alignment;
		out->capacity = capacity;
	} else {
		out->alignment = 0;
		out->capacity = 0;
	}
}

sf_public void sf_arena_clear(struct sf_arena *arena) {
	arena->position = 0;
}

sf_private u64 sf_non_literal_string_size(char const *non_literal, u64 max_size) {
	u64 i = 0;

	if (!non_literal)
		return 0;

	for (i = 0; i < max_size; ++i)
		if ('\0' == non_literal[i])
			return i;

	return max_size;
}

sf_public void sf_string_from_non_literal(char const *non_literal, u64 max_size, struct sf_string *out) {
	out->data = non_literal;
	out->size = sf_non_literal_string_size(non_literal, max_size);
}

sf_public sf_bool sf_string_compare(struct sf_string const *lhs, struct sf_string const *rhs, u64 max_size) {
	u64 i = 0;

	if (lhs->size != rhs->size)
		return SF_FALSE;

	for (i = 0; i < SF_MIN(lhs->size, max_size); ++i)
		if (lhs->data[i] != rhs->data[i])
			return SF_FALSE;

	return SF_TRUE;
}

sf_public void sf_string_clone(struct sf_arena *arena, struct sf_string const *in, struct sf_string *out) {
	char *data = NULL;

	if (!arena || !in || !out)
		return;

	out->size = 0;
	out->data = NULL;

	data = sf_arena_allocate(arena, in->size);
	if (!data)
		return;

	SF_MEMORY_COPY(data, in->data, in->size);

	out->size = in->size;
	out->data = data;
}

sf_public void sf_string_null_terminate(struct sf_arena *arena, struct sf_string const *in, struct sf_string *out) {
	char *data = NULL;

	if (!arena || !in || !out)
		return;

	out->size = 0;
	out->data = NULL;

	data = sf_arena_allocate(arena, in->size + 1);
	if (!data)
		return;

	SF_MEMORY_COPY(data, in->data, in->size);
	data[in->size] = '\0';

	out->size = in->size;
	out->data = data;
}

sf_public struct sf_string sf_string_from_literal(char const *str, u64 size) {
	struct sf_string result = {0};

	result.data = str;
	result.size = size;

	return result;
}

#endif // SF_IMPLEMENTATION

#endif // SF_H
