#ifndef pe_as_lit_kw_map_wr_h
#define pe_as_lit_kw_map_wr_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stdio.h>
/* XH: pe_as_kw.h */
/* XH: pe_hash.h */
#include "pe_as_lit_kw_map.h"

struct pe_as_lit_kw_map_wr {
	struct pe_as_lit_kw_map   map;
	FILE * sink;
	unsigned long data_size;
};

int
pe_as_lit_kw_map_wr_open(struct pe_as_lit_kw_map_wr *, char const *);

int
pe_as_lit_kw_map_wr_kw(struct pe_as_lit_kw_map_wr *, size_t,
		       char const *, int, pe_hash, pe_as_kw *);

int
pe_as_lit_kw_map_wr_close(struct pe_as_lit_kw_map_wr *);

#endif
