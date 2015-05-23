#ifndef pe_as_lnode_h
#define pe_as_lnode_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/* XH: pe_as_nt.h */
/* XH: pe_as_line.h */
/* XH: pe_as_str.h */
/* XH: pe_as_id.h */
/* XH: pe_as_kw.h */
/* XH: pe_as_lidx.h */

struct pe_as_lnode {
	pe_as_nt   tag;
	pe_as_line line;
	pe_as_lidx  parent;
	pe_as_lidx  next;
	union {
		pe_as_int i;
		pe_as_str str;
		pe_as_id id;
		pe_as_kw kw;
	} v;
};

#define pe_as_lnode_rw_magic        "peas"
#define pe_as_lnode_rw_magic_size   (sizeof(pe_as_lnode_rw_magic) -1)
#define pe_as_lnode_rw_magic_offset 0L

#define pe_as_lnode_rw_n_nodes_offset pe_as_lnode_rw_magic_size
#define pe_as_lnode_rw_data_offset (pe_as_lnode_rw_n_nodes_offset + sizeof(pe_as_lidx))

static inline int pe_as_lnode_has_child(struct pe_as_lnode *n)
{
	return n->tag & pe_as_nt_has_child;
}

static inline int pe_as_lnode_type(struct pe_as_lnode *n)
{
	return (n->tag & ~pe_as_nt_has_child);
}

static inline void pe_as_lnode_make_parent(struct pe_as_lnode *n)
{
	(n)->tag |= pe_as_nt_has_child;
}

#endif
