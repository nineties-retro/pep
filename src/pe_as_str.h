#ifndef pe_as_str_h
#define pe_as_str_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

typedef struct { size_t str; } pe_as_str;

static inline pe_as_str
size_t2pe_as_str(size_t s)
{
	pe_as_str str;
	str.str= s;
	return str;
}



static inline pe_as_str
pe_as_str_bottom(void)
{
	return size_t2pe_as_str(0);
}



static inline int
pe_as_str_eq(pe_as_str a, pe_as_str b)
{
	return a.str == b.str;
}



static inline size_t
pe_as_str2size_t(pe_as_str str)
{
	return str.str;
}

#endif
