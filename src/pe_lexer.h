#ifndef pe_lexer_h
#define pe_lexer_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stddef.h>
#include "pe_lexer_input.h"
#include "pe_as_id.h"
#include "pe_as_int.h"
#include "pe_as_line.h"
#include "pe_as_str.h"
#include "pe_as_lit_id_map_wr.h"
#include "pe_as_lit_str_map_wr.h"
#include "pe_as_lwr.h"

/* XH: pe_as_hw.h */
/* XH: pe_as_lit_kw_map.h */
/* XH: pe_error.h */

enum pe_lexeme_type {
	pe_lexeme_undefined= 0,
	pe_lexeme_eoi,
	pe_lexeme_eos,
	pe_lexeme_kw,
	pe_lexeme_id,
	pe_lexeme_str,
	pe_lexeme_int
};

enum pe_lexeme_error_type {
	pe_lexeme_no_error= 0,
	pe_lexeme_kw_unknown,
	pe_lexeme_str_unterminated,
	pe_lexeme_int_overflow,
	pe_lexeme_bad_char,
	pe_lexeme_application_error
};


struct pe_lexeme {
	enum pe_lexeme_type type;
	enum pe_lexeme_type error_type;
	enum pe_lexeme_error_type error;
	size_t len;
	char * str;
	unsigned long line;
	union {
		pe_as_id  id;
		pe_as_int i;
		pe_as_kw  kw;
		pe_as_str str;
		char      bad_char;
	} v;
};


struct pe_lexer {
	char * buffer;
	size_t buffer_end;
	size_t buffer_size;
	size_t i;
	size_t lexeme_start;
	unsigned long line;
	pe_lexer_input file;
	int producing_output;
	int some_output_produced;
	struct pe_as_lit_id_map_wr ids;
	struct pe_as_lit_kw_map * keywords;
	struct pe_as_lit_str_map_wr strings;
	struct pe_as_lwr nodes;
};

int
pe_lexer_open(struct pe_lexer *, char const *, char const *,
	      struct pe_as_lit_kw_map *);

void
pe_lexer_lexeme(struct pe_lexer *, struct pe_lexeme *);

int
pe_lexer_close(struct pe_lexer *);

#endif
