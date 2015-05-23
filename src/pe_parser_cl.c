/*
 * Copyright (c) 1995-2015 Nineties-Retro
 *
 * Parse a PEG file and generate a binary version.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_hash.h"
#include "pe_as_kw.h"
#include "pe_as_lit_kw_map.h"
#include "pe_grammar.h"
#include "pe_lexer.h"
#include "pe_parser.h"

static char const *language_prefix= "peg.";
static char const *program_name= "pep";
static char const *input_file_name;
static char const *output_prefix= "pep.";


static void
usage(void)
{
	fprintf(stderr, "%s: [ -l language ] [ -o output ] file\n", program_name);
}


static char const *
string_arg(int *argc, char ***argv)
{
	if (*argc < 2) {
		usage();
		return 0;
	}
	*argc -= 1;  *argv += 1;
	return (*argv)[0];
}


static int
process_arguments(int argc, char **argv)
{
	program_name = argv[0];
	while (++argv, --argc != 0 && argv[0][0] == '-' && argv[0][2] == '\0') {
		switch(argv[0][1]) {
		case 'o':
			if ((output_prefix = string_arg(&argc, &argv)) == 0)
				return EINVAL;
			break;
		case 'l':
			if ((language_prefix = string_arg(&argc, &argv)) == 0)
				return EINVAL;
			break;
		default:
			usage();
			return EINVAL;
		}
	}
	if (argc != 1) {
		usage();
		return EINVAL;
	}
	input_file_name= argv[0];
	return 0;
}



static int
display_parse_error(unsigned long line_number)
{
	if ((fprintf(stdout, "%s:%lu error ", input_file_name, line_number) == EOF))
		return EIO;
	return 0;
}



int main(int argc, char **argv)
{
	struct pe_grammar grammar;
	struct pe_lexer lexer;
	struct pe_parser parser;
	int parse_result;
	int err;

	err = process_arguments(argc, argv);
	if (err)
		goto could_not_process_arguments;
	err = pe_grammar_open(&grammar, language_prefix);
	if (err)
		goto could_not_open_grammar;
	err = pe_lexer_open(&lexer, input_file_name, output_prefix, pe_grammar_kws(&grammar));
	if (err)
		goto could_not_open_lexer;
	pe_parser_open(&parser, &lexer, &grammar);
	pe_parser_error_sink(&parser, display_parse_error, stdout);
	if ((parse_result = pe_parser_parse(&parser)) < 0)
		goto internal_parse_error;
	pe_parser_close(&parser);
	err = pe_lexer_close(&lexer);
	if (err)
		goto could_not_close_lexer;
	pe_grammar_close(&grammar);
	return parse_result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;

internal_parse_error:
	pe_parser_close(&parser);
	(void)pe_lexer_close(&lexer);
could_not_close_lexer:
could_not_open_lexer:
	(void)pe_grammar_close(&grammar);
could_not_open_grammar:
could_not_process_arguments:
	return err;
}
