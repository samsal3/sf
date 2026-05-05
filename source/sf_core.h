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

#define SF_ASSERT(e) sf_assert(e, SF_STRING(__FILE__), SF_STRING(__func__), SF_STRING(__LINE__), #e)

#ifndef NULL
#define NULL ((void *)0)
#endif

#define SF_ARRAY_INIT(a, value)                                                                                                            \
	do                                                                                                                                 \
	{                                                                                                                                  \
		size_t i_;                                                                                                                 \
		for (i_ = 0; i_ < SF_SIZE(a); ++i_)                                                                                        \
			(a)[i_] = value;                                                                                                   \
	} while (0)

#define SF_EXTERNAL
#define SF_INTERNAL static
#define SF_INLINE inline
#define SF_LOCAL_PERSIST static

#define sf_public
#define sf_private static
#define sf_local_persist static

typedef unsigned char sf_byte;
typedef uint32_t      u32;
typedef int32_t	      i32;
typedef uint64_t      u64;

typedef int32_t sf_bool;

typedef struct sf_arena
{
	char *data;
	u64   position;
	u64   alignment;
	u64   capacity;
} sf_arena;

typedef struct sf_string
{
	u64   size;
	char *data;
} sf_string;

#define SF_STRING(literal)                                                                                                                 \
	(sf_string)                                                                                                                        \
	{ SF_SIZE(literal), literal }

sf_public void *
sf_arena_allocate(sf_arena *arena, u64 size);

sf_public void
sf_arena_scratch(sf_arena *arena, u64 capacity, sf_arena *scratch);

sf_public void
sf_arena_clear(sf_arena *arena);

#define sf_memory_copy(destination, source, size)                                                                                          \
	do                                                                                                                                 \
	{                                                                                                                                  \
		u64 i;                                                                                                                     \
		for (i = 0; i < count; ++i)                                                                                                \
			((sf_byte *)destination)[i] = ((sf_byte const *)source)[i];                                                        \
	} while (0)

#define sf_memoty_set(destination, source, size)                                                                                           \
	do                                                                                                                                 \
	{                                                                                                                                  \
		u64 i;                                                                                                                     \
		for (i = 0; i < count; ++i)                                                                                                \
			((sf_byte *)destination)[i] = ((sf_byte)value);                                                                    \
	} while (0)

sf_public void
sf_string_from_non_literal(char const *non_literal, u64 max_size, sf_string *str);

sf_public sf_bool
sf_string_compare(sf_string const *lhs, sf_string const *rhs, u64 max_size);

sf_public void
sf_string_clone(sf_arena *arena, sf_string const *source, sf_string *destination);

sf_public void
sf_string_null_terminate(sf_arena *arena, sf_string const *source, sf_string *destination);

sf_public void
sf_assert(sf_bool test, sf_string file, sf_string function, int line, sf_string expresion);

#ifdef SF_CORE_IMPLEMENTATION

#include <stdio.h>

sf_private u64
sf_u64_align(u64 value, u64 alignment)
{ return (value + alignment - 1) & ~(alignment - 1); }

sf_public void *
sf_arena_allocate(sf_arena *arena, u64 size)
{
	sf_byte *memory = NULL;
	u64	 i = 0, required_size = 0;

	if (!arena || !size)
		return NULL;

	required_size = arena->position + size;
	if (required_size > arena->capacity)
		return NULL;

	memory		= &arena->data[arena->position];
	arena->position = sf_u64_align(required_size, arena->alignment);

	for (i = 0; i < size; ++i)
		memory[i] = 0x0;

	return memory;
}

sf_public void
sf_arena_scratch(sf_arena *arena, u64 capacity, sf_arena *scratch)
{
	scratch->position = 0;

	scratch->data = sf_arena_allocate(arena, capacity);
	if (scratch->data)
	{
		scratch->alignment = arena->alignment;
		scratch->capacity  = capacity;
	}
	else
	{
		scratch->alignment = 0;
		scratch->capacity  = 0;
	}
}

sf_public void
sf_arena_clear(sf_arena *arena)
{ arena->position = 0; }

sf_private u64
sf_non_literal_string_size(char const *non_literal, u64 max_size)
{
	u64 i = 0;

	for (i = 0; i < max_size; ++i)
		if ('\0' == non_literal[i])
			return i;

	return max_size;
}

sf_publlic void
sf_string_from_non_literal(char const *non_literal, u64 max_size, sf_string *destination)
{
	destination->data = non_literal;
	destination->size = sf_non_literal_string_size(non_literal, max_size);
}

sf_public sf_bool
sf_string_compare(sf_string const *lhs, sf_string const *rhs, u64 max_size)
{
	u64 i = 0;

	if (lhs->size != rhs->size)
		return SF_FALSE;

	for (i = 0; i < SF_MIN(lhs->size, max_size); ++i)
		if (lhs.data[i] != rhs.data[i])
			return SF_FALSE;

	return SF_TRUE;
}

sf_public void
sf_string_clone(sf_arena *arena, sf_string const *source, sf_string *destination)
{
	destination->size = 0;
	destination->data = sf_arena_allocate(arena, source->size);

	if (destination->data)
	{
		destination->size = source.size;
		sf_memory_copy(destination->data, source->data, destination->size);
	}
}

sf_public void
sf_string_null_terminate(sf_arena *arena, sf_string const *source, sf_string *destination)
{
	destination->size = 0;
	destination->data = sf_arena_allocate(arena, source->size + 1);

	if (destination->data)
	{
		destination->size = source->size + 1;
		sf_memory_copy(destination->data, source->data, source->size);
		destination->data[source->size] = '\0';
	}
}

sf_public void
sf_assert(sf_bool test, sf_string file, sf_string function, int line, sf_string expression)
{
	if (!test)
	{
		fprintf(
		    stderr,
		    "%.*s:%i - %.*s - SF_ASSERT(%.*s)\n",
		    (unsigned int)file.size,
		    file.data,
		    line,
		    (unsigned int)function.size,
		    function.data,
		    (unsigned int)expression.size,
		    expression.data
		);
		abort();
	}
}

#endif // SF_IMPLEMENTATION

#endif // SF_H
