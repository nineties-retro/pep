#ifndef pe_as_str_lit_map_rd_h
#define pe_as_str_lit_map_rd_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/* XH: stddef.h */
/* XH: pe_as_str.h */
/* XH: pe_error.h */

struct pe_as_str_lit_map_rd {
	char *strs;
	size_t total_size;
	size_t n_strs;
};

int
pe_as_str_lit_map_rd_open(struct pe_as_str_lit_map_rd *, char const *);

char const *
pe_as_str_lit_map_rd_str(const struct pe_as_str_lit_map_rd *, pe_as_str, size_t *len);

size_t
pe_as_str_lit_map_rd_n_strs(const struct pe_as_str_lit_map_rd *);

size_t
pe_as_str_lit_map_rd_lit_space(struct pe_as_str_lit_map_rd *);

void
pe_as_str_lit_map_rd_close(struct pe_as_str_lit_map_rd *);

#endif
