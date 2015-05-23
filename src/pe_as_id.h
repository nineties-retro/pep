#ifndef pe_as_id_h
#define pe_as_id_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

typedef struct { size_t id; } pe_as_id;

static inline pe_as_id
size_t2pe_as_id(size_t i)
{
	pe_as_id id;
	id.id= i;
	return id;
}

static inline pe_as_id
pe_as_id_bottom(void)
{
	return size_t2pe_as_id(0);
}

static inline int
pe_as_id_eq(pe_as_id a, pe_as_id b)
{
	return a.id == b.id;
}

static inline int
pe_as_id_le(pe_as_id a, pe_as_id b)
{
	return a.id <= b.id;
}

static inline int
pe_as_id_lt(pe_as_id a, pe_as_id b)
{
	return a.id < b.id;
}

static inline pe_as_id
pe_as_id_succ(pe_as_id id)
{
	return size_t2pe_as_id(id.id+1);
}

static inline size_t
pe_as_id2size_t(pe_as_id id)
{
	return id.id;
}

static inline size_t
pe_as_id_code(pe_as_id id)
{
	return id.id;
}

#endif
