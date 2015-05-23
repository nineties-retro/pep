#ifndef pe_as_line_h
#define pe_as_line_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

typedef struct { unsigned long l; } pe_as_line;

static inline pe_as_line
ulong2pe_as_line(unsigned long l)
{
	pe_as_line line;
	line.l= l;
	return line;
}


static inline pe_as_line
pe_as_line_bottom(void)
{
	return ulong2pe_as_line(0L);
}


static inline int
pe_as_line_eq(pe_as_line a, pe_as_line b)
{
	return a.l == b.l;
}



static inline int
pe_as_line_le(pe_as_line a, pe_as_line b)
{
	return a.l <= b.l;
}



static inline int
pe_as_line_lt(pe_as_line a, pe_as_line b)
{
	return a.l < b.l;
}



static inline unsigned long
pe_as_line2ulong(pe_as_line l)
{
	return l.l;
}

#endif