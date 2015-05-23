/*
 * Copyright (c) 1994-2015 Nineties Retro
 *
 * Lex a PEG file and generate a binary version.  Primarily used as
 * part of the bootstrap process for PEP.
 */

#include <stdio.h>
#include <stdlib.h>
#include "pe_hash.h"
#include "pe_as_kw.h"
#include "pe_as_lit_kw_map.h"
#include "pe_as_lit_kw_map_rd.h"
#include "pe_lexer.h"

static struct pe_lexer lexer;
static struct pe_as_lit_kw_map_rd keywords;
static char const *program_name;
static char const *input_file_name;
static char const *output_prefix= "pel.";
static char const *keywords_file= "peg.kw";

static void
usage(void)
{
	fprintf(stderr, "%s: [ -o output ] [ -k keywords ] file\n", program_name);
	exit(EXIT_FAILURE);
}



static char const *
string_arg(int * argc, char ***argv)
{
	if (*argc < 2) {
		usage();
		return (char const *)0;
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
				return 0;
			break;
		case 'k':
			if ((keywords_file = string_arg(&argc, &argv)) == 0)
				return 0;
			break;
		default:
			usage();
		}
	}
	if (argc != 1)
		usage();
	input_file_name= argv[0];
	return 1;
}


static void
output_token(struct pe_lexeme *t)
{
	printf("%lu %d %d %d X", t->line, t->type, t->error_type, t->error);
	fwrite(t->str, 1, t->len, stdout);
	fputc('X', stdout);
}



static int
lex(struct pe_lexer *l)
{
	struct pe_lexeme t;

	for (;;) {
		pe_lexer_lexeme(l, &t);

		switch (t.type) {
		case pe_lexeme_eoi:
			return 1;
		case pe_lexeme_undefined:
			output_token(&t);
			switch(t.error_type) {
			case pe_lexeme_id:
				fputs(" id", stdout);
				break;
			case pe_lexeme_kw:
				fputs(" kw", stdout);
				if (t.error == pe_lexeme_kw_unknown)
					fputs("?", stdout);
				break;
			case pe_lexeme_eos:
				fputs(" eos", stdout);
				break;
			case pe_lexeme_int:
				fputs(" str", stdout);
				if (t.error == pe_lexeme_int_overflow) {
					fputs(" >", stdout);
				}
				break;
			case pe_lexeme_str:
				fputs(" str", stdout);
				if (t.error == pe_lexeme_str_unterminated) {
					fputs(" +", stdout);
				}
				break;
			case pe_lexeme_undefined:
				fputs(" ?", stdout);
				break;
			case pe_lexeme_eoi:
				abort();
			}
			break;
		default:
			break;
		}

		switch (t.error) {
		case pe_lexeme_bad_char:
			fprintf(stdout, " \"%c\" (ASCII %d)\n", t.v.bad_char, (int)t.v.bad_char);
			return 1;
			
		case pe_lexeme_application_error:
			return 0;
		default:
			break;
		}
	}
}



int main(int argc, char ** argv)
{
	int err;

	if (!process_arguments(argc, argv))
		goto could_not_process_arguments;

	err = pe_as_lit_kw_map_rd_open(&keywords, keywords_file);
	if (err)
		goto could_not_open_keywords;

	err = pe_lexer_open(&lexer, input_file_name, output_prefix, &keywords.map);
	if (err)
		goto could_not_open_lexer;

	if (!lex(&lexer))
		goto could_not_lex;

	err = pe_lexer_close(&lexer);
	if (err)
		goto could_not_close_lexer;

	pe_as_lit_kw_map_rd_close(&keywords);
	return EXIT_SUCCESS;

could_not_lex:
	(void)pe_lexer_close(&lexer);
could_not_close_lexer:
could_not_open_lexer:
	(void)pe_as_lit_kw_map_rd_close(&keywords);
could_not_open_keywords:
could_not_process_arguments:
	return EXIT_FAILURE;
}
