#ifndef pe_as_rd_pp_h
#define pe_as_rd_pp_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/* XH: pe_as_rd.h */
/* XH: stdio.h */
/* XH: stddef.h */

struct pe_as_rd_pp {
	struct pe_as_rd *reader;
	FILE *output;
	size_t indentation;
};

void
pe_as_rd_pp_open(struct pe_as_rd_pp *, struct pe_as_rd *);

void
pe_as_rd_pp_output(struct pe_as_rd_pp *, FILE *);

void
pe_as_rd_pp_indentation(struct pe_as_rd_pp *, size_t);

int
pe_as_rd_pp_display(struct pe_as_rd_pp *, pe_as_idx);

void
pe_as_rd_pp_close(struct pe_as_rd_pp *);

#endif
