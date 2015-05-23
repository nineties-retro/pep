#ifndef pe_as_lwr_h
#define pe_as_lwr_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stdio.h>

/* XH: stddef.h */
/* XH: pe_as_id.h */
/* XH: pe_as_kw.h */
/* XH: pe_as_str.h */
/* XH: pe_as_int.h */
/* XH: pe_as_line.h */

#include "pe_as_nt.h"
#include "pe_as_lidx.h"
#include "pe_as_lnode.h"

struct pe_as_lwr_idx {
	pe_as_lidx p;
	short on_disk;
};

struct pe_as_lwr_idx_map {
	struct pe_as_lwr_idx *buffer;
	size_t size;
};

struct pe_as_lwr {
	struct pe_as_lnode node;
	struct pe_as_lnode *buffer;
	FILE *sink;
	size_t next_segment;
	size_t nesting_level;
	size_t n_nodes;
	struct pe_as_lwr_idx_map last_at_level;
	size_t buffer_size;
	size_t buffer_segment_size;
};

int  pe_as_lwr_close(struct pe_as_lwr *);
int  pe_as_lwr_open(struct pe_as_lwr *, char const *);
int  pe_as_lwr_eos(struct pe_as_lwr *);
int  pe_as_lwr_id(struct pe_as_lwr *, pe_as_id);
int  pe_as_lwr_int(struct pe_as_lwr *, pe_as_int);
void pe_as_lwr_line(struct pe_as_lwr *, pe_as_line);
int  pe_as_lwr_kw(struct pe_as_lwr *, pe_as_kw);
int  pe_as_lwr_str(struct pe_as_lwr *, pe_as_str);
void pe_as_lwr_set_buffers(struct pe_as_lwr *, size_t, size_t);

#endif
