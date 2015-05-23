#ifndef pe_as_rd_h
#define pe_as_rd_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include "pe_as_id.h"
#include "pe_as_int.h"
#include "pe_as_lidx.h"
#include "pe_as_kw.h"
#include "pe_as_line.h"
#include "pe_as_nt.h"
#include "pe_as_str.h"

#define pe_as_idx        pe_as_lidx
#define pe_as_idx_eq     pe_as_lidx_eq
#define pe_as_idx_le     pe_as_lidx_le
#define pe_as_idx_lt     pe_as_lidx_lt
#define pe_as_idx2ulong  pe_as_lidx2ulong
#define pe_as_idx_valid  pe_as_lidx_valid
#define pe_as_idx_bottom pe_as_lidx_bottom

#include "pe_as_lnode.h"
#include "pe_as_id_lit_map_rd.h"
#include "pe_as_kw_lit_map_rd.h"
#include "pe_as_str_lit_map_rd.h"

struct pe_as_rd {
	struct pe_as_lnode * nodes;
	size_t n_nodes;
	struct pe_as_kw_lit_map_rd kws;
	struct pe_as_id_lit_map_rd ids;
	struct pe_as_str_lit_map_rd strs;
	char const           * language;
};

pe_as_lidx
pe_as_rd_child(const struct pe_as_rd *, pe_as_lidx);

void
pe_as_rd_close(struct pe_as_rd *);

size_t
pe_as_rd_n_node(const struct pe_as_rd *);

pe_as_id
pe_as_rd_id(const struct pe_as_rd *, pe_as_lidx);

char const *
pe_as_rd_id_lit(const struct pe_as_rd *, pe_as_id);

char const *
pe_as_rd_id_lit_len(const struct pe_as_rd *, pe_as_id, size_t *);

pe_as_int
pe_as_rd_int(const struct pe_as_rd *, pe_as_lidx);

pe_as_kw
pe_as_rd_kw(const struct pe_as_rd *, pe_as_lidx);

char const *
pe_as_rd_kw_lit(const struct pe_as_rd *, pe_as_kw);

char const *
pe_as_rd_kw_lit_len(const struct pe_as_rd *, pe_as_kw, size_t *);

pe_as_line
pe_as_rd_line(const struct pe_as_rd *, pe_as_lidx);

size_t
pe_as_rd_n_id(const struct pe_as_rd *);

size_t
pe_as_rd_n_kw(const struct pe_as_rd *);

size_t
pe_as_rd_n_str(const struct pe_as_rd *);

pe_as_lidx
pe_as_rd_next(const struct pe_as_rd *, pe_as_lidx);

pe_as_lidx
pe_as_rd_on_line(const struct pe_as_rd *, unsigned long);

void
pe_as_rd_open(struct pe_as_rd *);

int
pe_as_rd_language(struct pe_as_rd *, char const *);

int
pe_as_rd_data(struct pe_as_rd *, char const *);

pe_as_lidx
pe_as_rd_parent(const struct pe_as_rd *, pe_as_lidx);

pe_as_lidx
pe_as_rd_root(const struct pe_as_rd *);

pe_as_str
pe_as_rd_str(const struct pe_as_rd *, pe_as_lidx);

const char *
pe_as_rd_str_lit(const struct pe_as_rd *, pe_as_str);

const char *
pe_as_rd_str_lit_len(const struct pe_as_rd *, pe_as_str, size_t *);

pe_as_nt
pe_as_rd_type(const struct pe_as_rd *, pe_as_lidx);

#endif
