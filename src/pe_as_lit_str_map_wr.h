#ifndef pe_as_lit_str_map_wr_h
#define pe_as_lit_str_map_wr_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stdio.h>

/* XH: stddef.h */
/* XH: pe_as_str.h */

struct pe_as_lit_str_map_wr {
	size_t      n_strs;
	size_t      total_size;
	FILE      * sink;
	char      * sink_file_name;
};


typedef struct pe_as_lit_str_map_wr pe_as_lit_str_map_wr;


int
pe_as_lit_str_map_wr_open(pe_as_lit_str_map_wr *, char const *);

int
pe_as_lit_str_map_wr_str(pe_as_lit_str_map_wr *, size_t, char const *, pe_as_str *);

int
pe_as_lit_str_map_wr_close(pe_as_lit_str_map_wr *);

#endif
