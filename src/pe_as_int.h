#ifndef pe_as_int_h
#define pe_as_int_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

typedef struct { long i; } pe_as_int;

#include <limits.h>
#define pe_as_int_max LONG_MAX

static inline int
pe_as_int_eq(pe_as_int a, pe_as_int b)
{
	return a.i == b.i;
}


static inline int
pe_as_int_le(pe_as_int a, pe_as_int b)
{
	return a.i <= b.i;
}



static inline int
pe_as_int_lt(pe_as_int a, pe_as_int b)
{
	return a.i < b.i;
}


static inline long
pe_as_int2long(pe_as_int i)
{
	return i.i;
}



static inline pe_as_int
long2pe_as_int(long l)
{
	pe_as_int i;
	i.i= l;
	return i;
}

#endif
