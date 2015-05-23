/*
 * Copyright (c) 1994-2015 Nineties Retro
 *
 * Dump out the AS in a format that shows the contents of every node.
 * The format itself can be represented by a PEG.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "pe_as_rd.h"

static char const *program_name;
static char const *language= "peg/";
static char const *input_file_name;


static void
usage(void)
{
	fprintf(stderr, "%s: [ -p<peg> ] grammar-prefix", program_name);
}


static int
process_arguments(int argc, char **argv)
{
	program_name = argv[0];
	while (++argv, --argc != 0 && **argv == '-') {
		switch ((*argv)[1]) {
		case 'p':
			language= &(*argv)[2];
			break;
		case 'h':
		default:
			usage();
			return EINVAL;
		}
	}
	if (argc != 1) {
		usage();
		return EINVAL;
	}
	input_file_name = *argv;
	return 0;
}



static int
dump(struct pe_as_rd *as)
{
	size_t n_nodes= pe_as_rd_n_node(as);
	pe_as_idx i= pe_as_rd_root(as);
	printf("(pe_as_rd_dump\n");
	printf("  (nodes %lu)\n", (unsigned long)n_nodes);
	printf("  (identifiers %lu)\n", (unsigned long)pe_as_rd_n_id(as));
	printf("  (keywords %lu)\n", (unsigned long)pe_as_rd_n_kw(as));
	printf("  (strings %lu)\n", (unsigned long)pe_as_rd_n_str(as));
	for (; n_nodes != 0; n_nodes -= 1, i= pe_as_lidx_succ(i)) {
		printf("  (node %lu\n", pe_as_lidx2ulong(i));
		printf("    (line %lu)\n", pe_as_line2ulong(pe_as_rd_line(as, i)));
		printf("    (parent %lu)\n", pe_as_lidx2ulong(pe_as_rd_parent(as, i)));
		printf("    (next %lu)\n", pe_as_lidx2ulong(pe_as_rd_next(as, i)));
		printf("    (child %lu)\n", pe_as_lidx2ulong(pe_as_rd_child(as, i)));
		switch(pe_as_rd_type(as, i)) {
		case pe_as_nt_kw:
		{
			pe_as_kw kw = pe_as_rd_kw(as, i);
			printf("    (keyword %d %s)",
			       (int)pe_as_kw2size_t(kw),
			       pe_as_rd_kw_lit(as, kw));
		}
		break;
		case pe_as_nt_id:
		{
			pe_as_id id = pe_as_rd_id(as, i);
			printf("    (identifier %lu %s)",
			       (unsigned long)pe_as_id_code(id),
			       pe_as_rd_id_lit(as, id));
		}
		break;
		case pe_as_nt_str:
		{
			pe_as_str str = pe_as_rd_str(as, i);
			printf("    (string %lu \"%s\")", 
			       (unsigned long)pe_as_str2size_t(str),
			       pe_as_rd_str_lit(as, str));
		}
		break;
		case pe_as_nt_int:
			printf("    (integer %ld)", pe_as_int2long(pe_as_rd_int(as, i)));
			break;
		default:
			abort();
		}
		printf(")\n");
	}
	printf(")\n");
	return 1;
}



int
main(int argc, char **argv)
{
	struct pe_as_rd as;
	int err;

	err = process_arguments(argc, argv);
	if (err)
		goto could_not_process_arguments;
	pe_as_rd_open(&as);
	pe_as_rd_language(&as, language);
	err = pe_as_rd_data(&as, input_file_name);
	if (err)
		goto could_not_read_input_file;

	err = dump(&as);
	if (err)
		goto could_not_dump_input_file;
	pe_as_rd_close(&as);
	return EXIT_SUCCESS;

could_not_dump_input_file:
could_not_read_input_file:
	pe_as_rd_close(&as);
could_not_process_arguments:
	return err;
}
