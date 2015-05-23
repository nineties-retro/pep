/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "pe_as_mrd.h"

static char const *program_name;
static char const *language= "";
static char const *input_file_name;


static void
usage(void)
{
	fprintf(stderr, "%s: [ -l language ] input-file-prefix\n", program_name);
	exit(EXIT_FAILURE);
}



static void
process_arguments(int argc, char **argv)
{
	while (++argv, --argc != 0 && **argv == '-' && (*argv)[2] == '\0') {
		switch((*argv)[1]) {
		case 'l':
			language= argv[1];
			argc -= 1;  argv += 1;
			break;
		case 'h':
		default:
			usage();
		}
	}
	if (argc != 1) usage();
	input_file_name= *argv;
}


static int
dump(struct pe_as_rd *as)
{
	size_t n_nodes= pe_as_rd_n_node(as);
	pe_as_idx i= pe_as_rd_root(as);
	printf("(pe_as_mrd_dump\n");
	printf("  (nodes %lu)\n", (unsigned long)n_nodes);
	printf("  (identifiers %lu)\n", (unsigned long)pe_as_rd_n_id(as));
	printf("  (keywords %lu)\n", (unsigned long)pe_as_rd_n_kw(as));
	printf("  (strings %lu)\n", (unsigned long)pe_as_rd_n_str(as));
	for (; n_nodes != 0; n_nodes -= 1, i= pe_as_lidx_succ(i)) {
		printf("  (node %lu (line %lu) (parent %lu) (next %lu) (child %lu) ",
		       pe_as_lidx2ulong(i),
		       pe_as_line2ulong(pe_as_rd_line(as, i)),
		       pe_as_lidx2ulong(pe_as_rd_parent(as, i)),
		       pe_as_lidx2ulong(pe_as_rd_next(as, i)),
		       pe_as_lidx2ulong(pe_as_rd_child(as, i)));
		switch(pe_as_rd_type(as, i)) {
		case pe_as_nt_kw:
		{
			pe_as_kw kw= pe_as_rd_kw(as, i);
			size_t   len;
			printf("(keyword %d %s)",
			       pe_as_kw2int(kw),
			       pe_as_rd_kw_lit(as, kw, &len));
		}
		break;
		case pe_as_nt_id:
		{
			pe_as_id id= pe_as_rd_id(as, i);
			size_t   len;
			printf("(identifier %lu %s)",
			       (unsigned long)pe_as_id_code(id),
			       pe_as_rd_id_lit(as, id, &len));
		}
		break;
		case pe_as_nt_str:
		{
			pe_as_str str= pe_as_rd_str(as, i);
			size_t    len;
			printf("(string %lu \"%s\")", 
			       (unsigned long)pe_as_str2size_t(str),
			       pe_as_rd_str_lit(as, str, &len));
		}
		break;
		case pe_as_nt_int:
			printf("(integer %ld)", pe_as_int2long(pe_as_rd_int(as, i)));
			break;
		default:
			abort();
		}
		printf(")\n");
	}
	printf(")\n");
	return 1;
}


int main(int argc, char **argv)
{
	struct pe_as_rd as;

	if (argv[0] != (char *)0) {
		program_name = argv[0];
	} else {
		program_name= "pep";
	}
	process_arguments(argc, argv);
	if (!pe_as_rd_open(&as, language, input_file_name))
		return EXIT_FAILURE;
	if (!dump(&as))
		return EXIT_FAILURE;
	if (!pe_as_rd_close(&as))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
