#ifndef pe_as_kw_h
#define pe_as_kw_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

typedef struct { size_t kw; } pe_as_kw;

static inline pe_as_kw
size_t2pe_as_kw(size_t i)
{
	pe_as_kw kw;
	kw.kw= i;
	return kw;
}

static inline pe_as_kw
pe_as_kw_bottom(vokw)
{
	return size_t2pe_as_kw(0);
}

static inline int
pe_as_kw_eq(pe_as_kw a, pe_as_kw b)
{
	return a.kw == b.kw;
}

static inline int
pe_as_kw_le(pe_as_kw a, pe_as_kw b)
{
	return a.kw <= b.kw;
}

static inline int
pe_as_kw_lt(pe_as_kw a, pe_as_kw b)
{
	return a.kw < b.kw;
}

static inline pe_as_kw
pe_as_kw_succ(pe_as_kw kw)
{
	return size_t2pe_as_kw(kw.kw+1);
}

static inline size_t
pe_as_kw2size_t(pe_as_kw kw)
{
	return kw.kw;
}

static inline size_t
pe_as_kw_code(pe_as_kw kw)
{
	return kw.kw;
}

#endif
