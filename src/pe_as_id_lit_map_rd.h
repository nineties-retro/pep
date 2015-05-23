#ifndef pe_as_id_lit_map_rd_h
#define pe_as_id_lit_map_rd_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/* XH: stddef.h */
/* XH: pe_as_id.h */

struct pe_as_id_lit_map_rd {
	char *raw_ids;
	char const **ids;
	size_t n_ids;
};

int
pe_as_id_lit_map_rd_open(struct pe_as_id_lit_map_rd *, char const *);

const char *
pe_as_id_lit_map_rd_id(const struct pe_as_id_lit_map_rd *, pe_as_id, size_t *len);

size_t
pe_as_id_lit_map_rd_n_ids(const struct pe_as_id_lit_map_rd *);

void
pe_as_id_lit_map_rd_close(struct pe_as_id_lit_map_rd *);

#endif
