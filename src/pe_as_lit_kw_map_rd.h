#ifndef pe_as_lit_kw_map_rd_h
#define pe_as_lit_kw_map_rd_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/* XH: stddef.h */
/* XH: pe_as_kw.h */
/* XH: pe_error.h */

#include "pe_hash.h"
#include "pe_as_lit_kw_map.h"
#include "pe_as_kw_lit_map_rd.h"

struct pe_as_lit_kw_map_rd {
	struct pe_as_lit_kw_map map;
	struct pe_as_kw_lit_map_rd kws;
};

int
pe_as_lit_kw_map_rd_open(struct pe_as_lit_kw_map_rd *, char const *);

pe_as_kw
pe_as_lit_kw_map_rd_kw(struct pe_as_lit_kw_map_rd *, size_t, char const *);

#define pe_as_lit_kw_map_rd_lit(m, kw, len) \
  pe_as_kw_lit_map_rd_kw(&(m)->kws, kw, len)

void
pe_as_lit_kw_map_rd_close(struct pe_as_lit_kw_map_rd *);

#endif
