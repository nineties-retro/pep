#ifndef pe_as_lidx_h
#define pe_as_lidx_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

typedef struct { long l; } pe_as_lidx;

static inline int
pe_as_lidx_eq(pe_as_lidx a, pe_as_lidx b)
{
	return a.l == b.l;
}



static inline int
pe_as_lidx_le(pe_as_lidx a, pe_as_lidx b)
{
	return a.l <= b.l;
}



static inline int
pe_as_lidx_lt(pe_as_lidx a, pe_as_lidx b)
{
	return a.l < b.l;
}


static inline pe_as_lidx
ulong2pe_as_lidx(unsigned long l)
{
	pe_as_lidx idx;
	idx.l= l;
	return idx;
}



static inline unsigned long
pe_as_lidx2ulong(pe_as_lidx idx)
{
	return idx.l;
}



static inline pe_as_lidx
pe_as_lidx_bottom(void)
{
	return ulong2pe_as_lidx(0L);
}


static inline int
pe_as_lidx_valid(pe_as_lidx idx)
{
	return idx.l != 0L;
}



static inline pe_as_lidx
pe_as_lidx_succ(pe_as_lidx idx)
{
	pe_as_lidx new_idx;
	new_idx.l = idx.l+1;
	return new_idx;
}

#endif
