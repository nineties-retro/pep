#ifndef pe_as_kw_lit_map_rd_h
#define pe_as_kw_lit_map_rd_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/* XH: stddef.h */
/* XH: pe_as_kw.h */

struct pe_as_kw_lit_map_rd {
	char         * raw_kws;
	char const  ** kws;
	size_t         n_kws;
};

typedef struct pe_as_kw_lit_map_rd pe_as_kw_lit_map_rd;

int
pe_as_kw_lit_map_rd_open(pe_as_kw_lit_map_rd *, char const *);

char const *
pe_as_kw_lit_map_rd_kw(const pe_as_kw_lit_map_rd *, pe_as_kw, size_t *len);

size_t
pe_as_kw_lit_map_rd_n_kws(const pe_as_kw_lit_map_rd *);

void
pe_as_kw_lit_map_rd_close(pe_as_kw_lit_map_rd *);

#endif
