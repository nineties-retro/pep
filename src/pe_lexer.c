/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pe_hash.h"
#include "pe_as_kw.h"
#include "pe_as_lit_kw_map.h"
#include "pe_lexer.h"


#define pe_lexer_buffer_start 0

#ifndef pe_lexer_buffer_size
#define pe_lexer_buffer_size 8192
#endif

#define pe_lexer_out_ids_name   "id"
#define pe_lexer_out_strs_name  "str"
#define pe_lexer_out_nodes_name "as"


/*
 * Refill the lexer buffer from the source, taking into 
 * account the `slop' characters at the end of the buffer that
 * make up the current (unfinished) lexeme.  If the number
 * of slop characters is a large percentage of the maximum
 * buffer size then the buffer is extended.
 *
 * Returns the number of characters in the new buffer
 * (consists the number of characters read and the 
 * number of slop characters).  Returns a negative
 * value if there is any error.
 */
static int
pe_lexer_refill(struct pe_lexer *in, size_t slop)
{
	char           * buff=   in->buffer;
	pe_lexer_input   src=    in->file;
	size_t           size=   in->buffer_size;
	size_t           nitems= size - slop;
	size_t           nread;
	
	if (nitems < size/2) {
		char * new_buff;
		size *= 2;
		assert(size < INT_MAX);
		new_buff= (char *)malloc(size);
		if (new_buff == (char *)0) {
			return -1;
		}
		if (slop != 0) {
			(void)strncpy(new_buff, buff+nitems, slop);
		}
		free(buff);
		in->buffer_size= size;
		nitems= size-slop;
		buff= new_buff;
		in->buffer= buff;
	} else if (slop != 0) {
		(void)strncpy(buff, buff+nitems, slop);
	}
	nread= pe_lexer_input_read(src, nitems, buff+slop);
	nread += slop;
	in->buffer_end=   nread;
	in->lexeme_start= pe_lexer_buffer_start;
	in->i=     slop;
	return nread;
}


#define pe_lexer_char_ws           (1<<0)
#define pe_lexer_char_illegal      (1<<1)
#define pe_lexer_char_string_delim (1<<2)
#define pe_lexer_char_string_body  (1<<3)
#define pe_lexer_char_id_start     (1<<4)
#define pe_lexer_char_id_body      (1<<5)
#define pe_lexer_char_kw_start     (1<<6)
#define pe_lexer_char_kw_end       (1<<7)
#define pe_lexer_char_int_start    (1<<8)
#define pe_lexer_char_int_body     (1<<9)
#define pe_lexer_char_newline      (1<<10)
#define pe_lexer_char_delim        (1<<11)

typedef unsigned short pe_lexer_char_type;

static pe_lexer_char_type
pe_lexer_char_table[]=
{ /*   0 */ pe_lexer_char_illegal
, /*   1 */ pe_lexer_char_illegal
, /*   2 */ pe_lexer_char_illegal
, /*   3 */ pe_lexer_char_illegal
, /*   4 */ pe_lexer_char_illegal
, /*   5 */ pe_lexer_char_illegal
, /*   6 */ pe_lexer_char_illegal
, /*   7 */ pe_lexer_char_illegal
, /*   8 */ pe_lexer_char_illegal
, /*   9 */ pe_lexer_char_ws | pe_lexer_char_string_body | pe_lexer_char_delim
, /*  10 */ pe_lexer_char_newline| pe_lexer_char_ws | pe_lexer_char_string_body | pe_lexer_char_delim
, /*  11 */ pe_lexer_char_illegal
, /*  12 */ pe_lexer_char_illegal
, /*  13 */ pe_lexer_char_ws | pe_lexer_char_string_body | pe_lexer_char_delim
, /*  14 */ pe_lexer_char_illegal
, /*  15 */ pe_lexer_char_illegal
, /*  16 */ pe_lexer_char_illegal
, /*  17 */ pe_lexer_char_illegal
, /*  18 */ pe_lexer_char_illegal
, /*  19 */ pe_lexer_char_illegal
, /*  20 */ pe_lexer_char_illegal
, /*  21 */ pe_lexer_char_illegal
, /*  22 */ pe_lexer_char_illegal
, /*  23 */ pe_lexer_char_illegal
, /*  24 */ pe_lexer_char_illegal
, /*  25 */ pe_lexer_char_illegal
, /*  26 */ pe_lexer_char_illegal
, /*  27 */ pe_lexer_char_illegal
, /*  28 */ pe_lexer_char_illegal
, /*  29 */ pe_lexer_char_illegal
, /*  30 */ pe_lexer_char_illegal
, /*  31 */ pe_lexer_char_illegal
, /*  32 */ pe_lexer_char_ws | pe_lexer_char_string_body | pe_lexer_char_delim
, /*  33 */ pe_lexer_char_string_body
, /*  34 */ pe_lexer_char_string_delim | pe_lexer_char_delim
, /*  35 */ pe_lexer_char_string_body
, /*  36 */ pe_lexer_char_string_body
, /*  37 */ pe_lexer_char_string_body
, /*  38 */ pe_lexer_char_id_start | pe_lexer_char_string_body
, /*  39 */ pe_lexer_char_string_body
, /*  40 */ pe_lexer_char_kw_start | pe_lexer_char_string_body | pe_lexer_char_delim
, /*  41 */ pe_lexer_char_kw_end | pe_lexer_char_string_body | pe_lexer_char_delim
, /*  42 */ pe_lexer_char_string_body
, /*  43 */ pe_lexer_char_int_start | pe_lexer_char_string_body
, /*  44 */ pe_lexer_char_string_body
, /*  45 */ pe_lexer_char_int_start | pe_lexer_char_string_body
, /*  46 */ pe_lexer_char_string_body
, /*  47 */ pe_lexer_char_string_body
, /*  48 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  49 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  50 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  51 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  52 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  53 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  54 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  55 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  56 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  57 */ pe_lexer_char_int_start | pe_lexer_char_int_body | pe_lexer_char_string_body | pe_lexer_char_id_body
, /*  58 */ pe_lexer_char_string_body
, /*  59 */ pe_lexer_char_string_body
, /*  60 */ pe_lexer_char_string_body
, /*  61 */ pe_lexer_char_string_body
, /*  62 */ pe_lexer_char_string_body
, /*  63 */ pe_lexer_char_string_body
, /*  64 */ pe_lexer_char_string_body
, /*  65 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  66 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  67 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  68 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  69 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  70 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  71 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  72 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  73 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  74 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  75 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  76 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  77 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  78 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  79 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  80 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  81 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  82 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  83 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  84 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  85 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  86 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  87 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  88 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  89 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  90 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  91 */ pe_lexer_char_string_body
, /*  92 */ pe_lexer_char_string_body
, /*  93 */ pe_lexer_char_string_body
, /*  94 */ pe_lexer_char_string_body
, /*  95 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  96 */ pe_lexer_char_string_body
, /*  97 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  98 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /*  99 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 100 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 101 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 102 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 103 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 104 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 105 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 106 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 107 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 108 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 109 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 110 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 111 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 112 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 113 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 114 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 115 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 116 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 117 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 118 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 119 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 120 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 121 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 122 */ pe_lexer_char_id_start | pe_lexer_char_id_body | pe_lexer_char_string_body
, /* 123 */ pe_lexer_char_string_body
, /* 124 */ pe_lexer_char_string_body
, /* 125 */ pe_lexer_char_string_body
, /* 126 */ pe_lexer_char_string_body
, /* 127 */ pe_lexer_char_illegal
, /* 128 */ pe_lexer_char_illegal
, /* 129 */ pe_lexer_char_illegal
, /* 130 */ pe_lexer_char_illegal
, /* 131 */ pe_lexer_char_illegal
, /* 132 */ pe_lexer_char_illegal
, /* 133 */ pe_lexer_char_illegal
, /* 134 */ pe_lexer_char_illegal
, /* 135 */ pe_lexer_char_illegal
, /* 136 */ pe_lexer_char_illegal
, /* 137 */ pe_lexer_char_illegal
, /* 138 */ pe_lexer_char_illegal
, /* 139 */ pe_lexer_char_illegal
, /* 140 */ pe_lexer_char_illegal
, /* 141 */ pe_lexer_char_illegal
, /* 142 */ pe_lexer_char_illegal
, /* 143 */ pe_lexer_char_illegal
, /* 144 */ pe_lexer_char_illegal
, /* 145 */ pe_lexer_char_illegal
, /* 146 */ pe_lexer_char_illegal
, /* 147 */ pe_lexer_char_illegal
, /* 148 */ pe_lexer_char_illegal
, /* 149 */ pe_lexer_char_illegal
, /* 150 */ pe_lexer_char_illegal
, /* 151 */ pe_lexer_char_illegal
, /* 152 */ pe_lexer_char_illegal
, /* 153 */ pe_lexer_char_illegal
, /* 154 */ pe_lexer_char_illegal
, /* 155 */ pe_lexer_char_illegal
, /* 156 */ pe_lexer_char_illegal
, /* 157 */ pe_lexer_char_illegal
, /* 158 */ pe_lexer_char_illegal
, /* 159 */ pe_lexer_char_illegal
, /* 160 */ pe_lexer_char_illegal
, /* 161 */ pe_lexer_char_illegal
, /* 162 */ pe_lexer_char_illegal
, /* 163 */ pe_lexer_char_illegal
, /* 164 */ pe_lexer_char_illegal
, /* 165 */ pe_lexer_char_illegal
, /* 166 */ pe_lexer_char_illegal
, /* 167 */ pe_lexer_char_illegal
, /* 168 */ pe_lexer_char_illegal
, /* 169 */ pe_lexer_char_illegal
, /* 170 */ pe_lexer_char_illegal
, /* 171 */ pe_lexer_char_illegal
, /* 172 */ pe_lexer_char_illegal
, /* 173 */ pe_lexer_char_illegal
, /* 174 */ pe_lexer_char_illegal
, /* 175 */ pe_lexer_char_illegal
, /* 176 */ pe_lexer_char_illegal
, /* 177 */ pe_lexer_char_illegal
, /* 178 */ pe_lexer_char_illegal
, /* 179 */ pe_lexer_char_illegal
, /* 180 */ pe_lexer_char_illegal
, /* 181 */ pe_lexer_char_illegal
, /* 182 */ pe_lexer_char_illegal
, /* 183 */ pe_lexer_char_illegal
, /* 184 */ pe_lexer_char_illegal
, /* 185 */ pe_lexer_char_illegal
, /* 186 */ pe_lexer_char_illegal
, /* 187 */ pe_lexer_char_illegal
, /* 188 */ pe_lexer_char_illegal
, /* 189 */ pe_lexer_char_illegal
, /* 190 */ pe_lexer_char_illegal
, /* 191 */ pe_lexer_char_illegal
, /* 192 */ pe_lexer_char_illegal
, /* 193 */ pe_lexer_char_illegal
, /* 194 */ pe_lexer_char_illegal
, /* 195 */ pe_lexer_char_illegal
, /* 196 */ pe_lexer_char_illegal
, /* 197 */ pe_lexer_char_illegal
, /* 198 */ pe_lexer_char_illegal
, /* 199 */ pe_lexer_char_illegal
, /* 200 */ pe_lexer_char_illegal
, /* 201 */ pe_lexer_char_illegal
, /* 202 */ pe_lexer_char_illegal
, /* 203 */ pe_lexer_char_illegal
, /* 204 */ pe_lexer_char_illegal
, /* 205 */ pe_lexer_char_illegal
, /* 206 */ pe_lexer_char_illegal
, /* 207 */ pe_lexer_char_illegal
, /* 208 */ pe_lexer_char_illegal
, /* 209 */ pe_lexer_char_illegal
, /* 210 */ pe_lexer_char_illegal
, /* 211 */ pe_lexer_char_illegal
, /* 212 */ pe_lexer_char_illegal
, /* 213 */ pe_lexer_char_illegal
, /* 214 */ pe_lexer_char_illegal
, /* 215 */ pe_lexer_char_illegal
, /* 216 */ pe_lexer_char_illegal
, /* 217 */ pe_lexer_char_illegal
, /* 218 */ pe_lexer_char_illegal
, /* 219 */ pe_lexer_char_illegal
, /* 220 */ pe_lexer_char_illegal
, /* 221 */ pe_lexer_char_illegal
, /* 222 */ pe_lexer_char_illegal
, /* 223 */ pe_lexer_char_illegal
, /* 224 */ pe_lexer_char_illegal
, /* 225 */ pe_lexer_char_illegal
, /* 226 */ pe_lexer_char_illegal
, /* 227 */ pe_lexer_char_illegal
, /* 228 */ pe_lexer_char_illegal
, /* 229 */ pe_lexer_char_illegal
, /* 230 */ pe_lexer_char_illegal
, /* 231 */ pe_lexer_char_illegal
, /* 232 */ pe_lexer_char_illegal
, /* 233 */ pe_lexer_char_illegal
, /* 234 */ pe_lexer_char_illegal
, /* 235 */ pe_lexer_char_illegal
, /* 236 */ pe_lexer_char_illegal
, /* 237 */ pe_lexer_char_illegal
, /* 238 */ pe_lexer_char_illegal
, /* 239 */ pe_lexer_char_illegal
, /* 240 */ pe_lexer_char_illegal
, /* 241 */ pe_lexer_char_illegal
, /* 242 */ pe_lexer_char_illegal
, /* 243 */ pe_lexer_char_illegal
, /* 244 */ pe_lexer_char_illegal
, /* 245 */ pe_lexer_char_illegal
, /* 246 */ pe_lexer_char_illegal
, /* 247 */ pe_lexer_char_illegal
, /* 248 */ pe_lexer_char_illegal
, /* 249 */ pe_lexer_char_illegal
, /* 250 */ pe_lexer_char_illegal
, /* 251 */ pe_lexer_char_illegal
, /* 252 */ pe_lexer_char_illegal
, /* 253 */ pe_lexer_char_illegal
, /* 254 */ pe_lexer_char_illegal
, /* 255 */ pe_lexer_char_illegal
};



static void
pe_lexer_int(struct pe_lexer *in, unsigned long v, int negative, struct pe_lexeme *t)
{
	pe_lexer_char_type   char_type;
	size_t               i=     in->i + 1;
	char const         * b=     in->buffer;
	size_t               e=     in->buffer_end;
	
	t->error_type= pe_lexeme_int;
loop:
	if (i == e) {
		size_t slop;
		int    nread;
		
		if (e != 0 && e != in->buffer_size)
			goto valid_int;
		
		slop= e - in->lexeme_start; 
		if ((nread= pe_lexer_refill(in, slop)) < 0) {
			t->error = pe_lexeme_application_error;
			in->producing_output= 0;
			goto the_end;
		}
		i= in->i;
		if (nread == slop)
			goto valid_int;
		e= nread;
		b= in->buffer;
	}
	
	char_type= pe_lexer_char_table[(size_t)b[i]];
	if (char_type & pe_lexer_char_int_body) {
		int c;
		if (v > pe_as_int_max/10) {
			if (t->error == pe_lexeme_no_error) {
				t->error= pe_lexeme_int_overflow;
			}
			in->producing_output= 0;
			i += 1;
			goto loop;
		}
		c= b[i] - '0';
		if ((v == pe_as_int_max/10)
		    &&  (c > (pe_as_int_max - v*10))) {
			if (t->error == pe_lexeme_no_error) {
				t->error= pe_lexeme_int_overflow;
			}
			in->producing_output= 0;
			i += 1;
			goto loop;
		}
		v= v*10 + c;
		i += 1;
		goto loop;
	} else if (!(char_type & pe_lexer_char_delim)) {
		if (t->error == pe_lexeme_no_error) {
			t->v.bad_char= b[i];
		}
		t->error= pe_lexeme_bad_char;
		in->producing_output= 0;
		i += 1;
		goto loop;
	}

valid_int:
	if (t->error == pe_lexeme_no_error) {
		t->v.i = long2pe_as_int(negative ? -(long)v : (long)v);
		if (in->producing_output
		    &&  pe_as_lwr_int(&in->nodes, t->v.i)) {
			t->error = pe_lexeme_application_error;
			in->producing_output= 0;
		} else {
			t->type = pe_lexeme_int;
		}
	}
the_end:
	in->i= i;
	t->len= i - in->lexeme_start;
	t->str= &in->buffer[in->lexeme_start];
}



static void
pe_lexer_str(struct pe_lexer *in, struct pe_lexeme *t)
{
	pe_lexer_char_type char_type;
	size_t i = in->i + 1;
	size_t e = in->buffer_end;
	char const * b = in->buffer;
	
	in->lexeme_start = i;		/* skip the " */
	t->error_type = pe_lexeme_str;
	
loop:
	if (i == e) {
		size_t slop;
		int nread;

		if (e != 0 && e != in->buffer_size) {
			t->error= pe_lexeme_str_unterminated;
			in->producing_output = 0;
			goto the_end;
		}
		slop = e - in->lexeme_start;  
		if ((nread= pe_lexer_refill(in, slop)) < 0) {
			t->error = pe_lexeme_application_error;
			in->producing_output= 0;
			goto the_end;
		}
		i = in->i;
		if (nread == slop) {
			t->error= pe_lexeme_str_unterminated;
			in->producing_output= 0;
			goto the_end;
		}
		e = nread;
		b= in->buffer;
	}

	char_type = pe_lexer_char_table[(size_t)b[i]];
	if (char_type & pe_lexer_char_string_body) {
		if (char_type & pe_lexer_char_newline) {
			in->line += 1;
		}
		i += 1;
		goto loop;
	} else if (char_type & pe_lexer_char_string_delim) {
		t->len = i - in->lexeme_start;
		t->str = &in->buffer[in->lexeme_start];
		in->i = i+1;
		if (in->producing_output
		    &&  (pe_as_lit_str_map_wr_str(&in->strings, t->len, t->str, &t->v.str)
			 || pe_as_lwr_str(&in->nodes, t->v.str))) {
			t->error = pe_lexeme_application_error;
			in->producing_output = 0;
		} else {
			t->type= pe_lexeme_str;
		}
		return;

	} else {
		if (t->error == pe_lexeme_no_error) {
			t->v.bad_char= b[i];
		}
		t->error = pe_lexeme_bad_char;
		in->producing_output = 0;
		i += 1;
		goto loop;
	}

the_end:
	in->i = i;
	t->len = i - in->lexeme_start;
	t->str = &in->buffer[in->lexeme_start];
}



static void
pe_lexer_generic_id(struct pe_lexer *in, pe_hash *h, struct pe_lexeme *t)
{
	pe_hash_vars(*h);
	pe_lexer_char_type char_type;
	size_t i = in->i + 1;
	size_t e = in->buffer_end;
	char const *b = in->buffer;
	
loop:
	if (i == e) {
		int    nread;
		size_t slop;
		
		if (e != 0 && e != in->buffer_size)
			goto valid_id;
		
		slop = e - in->lexeme_start;
		if ((nread = pe_lexer_refill(in, slop)) < 0) {
			t->error = pe_lexeme_application_error;
			in->producing_output = 0;
			goto the_end;
		}
		i = in->i;
		if (nread == slop)
			goto valid_id;
		e = nread;
		b = in->buffer;
	}

	char_type = pe_lexer_char_table[(size_t)b[i]];
	if (char_type & pe_lexer_char_id_body) {
		pe_hash_char(b[i]);
		i += 1;
		goto loop;
	} else if (!(char_type & pe_lexer_char_delim)) {
		if (t->error == pe_lexeme_no_error) {
			t->v.bad_char= b[i];
		}
		t->error = pe_lexeme_bad_char;
		in->producing_output= 0;
		i += 1;
		goto loop;
	}

valid_id:
the_end:
	in->i = i;
	t->len = i - in->lexeme_start;
	t->str = &in->buffer[in->lexeme_start];
	*h = pe_hash_value;
}



static void
pe_lexer_kw(struct pe_lexer *in, struct pe_lexeme *t)
{
	pe_lexer_char_type char_type;
	size_t e = in->buffer_end;
	size_t i = in->lexeme_start= in->i + 1;
	char const *b = in->buffer;
	pe_hash_vars(0);
	
	t->error_type = pe_lexeme_kw;
loop:
	if (i == e) {
		size_t slop;
		int nread;

		if (e != 0 && e != in->buffer_size) {
			t->error = pe_lexeme_kw_unknown;
			in->producing_output = 0;
			goto error;
		}
		slop = e - in->lexeme_start;
		if ((nread = pe_lexer_refill(in, slop)) < 0) {
			t->error = pe_lexeme_application_error;
			in->producing_output = 0;
			goto error;
		}
		i = in->i;
		if (nread == slop) {
			t->error = pe_lexeme_kw_unknown;
			in->producing_output = 0;
			goto error;
		}
		e = nread;
		b = in->buffer;
	}
	
	char_type = pe_lexer_char_table[(size_t)b[i]];
	if (char_type & pe_lexer_char_id_start) {
		int fix;
		if (char_type & pe_lexer_char_id_body) {
			fix= 0;  pe_hash_char(b[i]);  
		} else {
			fix= 1;
		}
		in->i = i;
		in->lexeme_start = i;
		pe_lexer_generic_id(in, &pe_hash_value, t);
		if (t->error != pe_lexeme_no_error) {
			return;
		} else {
			struct pe_as_lit_kw_map_node **n;
			n = pe_as_lit_kw_map_find(in->keywords,
						  in->i - in->lexeme_start,
						  &in->buffer[in->lexeme_start],
						  fix,
						  pe_hash_value);
			if (*n == 0) {
				t->error = pe_lexeme_kw_unknown;
				in->producing_output = 0;
				return;
			} else {
				t->v.kw = (*n)->code;
				if (in->producing_output
				    &&  pe_as_lwr_kw(&in->nodes, t->v.kw)) {
					t->error = pe_lexeme_application_error;
					in->producing_output= 0;
					return;
				} else {
					t->type = pe_lexeme_kw;
					return;
				}
			}
		}
	} else if (char_type & pe_lexer_char_ws) {
		if (char_type & pe_lexer_char_newline) { in->line += 1; }
		i += 1;
		goto loop;
	} else {
		in->producing_output= 0;
	}

error:
	in->i = i;
	t->len = i - in->lexeme_start;
	t->str = &in->buffer[in->lexeme_start];
}



void
pe_lexer_lexeme(struct pe_lexer *in, struct pe_lexeme *t)
{
	pe_lexer_char_type char_type;
	char const *b = in->buffer;
	size_t e = in->buffer_end;
	size_t i = in->i;
	
	t->type = pe_lexeme_undefined;
	t->error_type = pe_lexeme_undefined;
	t->error = pe_lexeme_no_error;
next:
	if (i == e) {
		int nread;
		if (e != 0 && e != in->buffer_size) {
			in->i = i;
			t->type = pe_lexeme_eoi;
			return;
		}
		if ((nread = pe_lexer_refill(in, 0)) < 0) {
			t->error = pe_lexeme_application_error;
			in->producing_output = 0;
			return;
		} else if (nread == 0) {
			t->type = pe_lexeme_eoi;
			return;
		}
		i = in->i;
		b = in->buffer;
		e = nread;
	}

	char_type = pe_lexer_char_table[(size_t)b[i]];
	
	if (char_type & pe_lexer_char_ws) {
		if (char_type & pe_lexer_char_newline) {
			in->line += 1;
		}
		i += 1;
		goto next;
		
	} else {
		in->lexeme_start = in->i= i;
		t->line = in->line;
		pe_as_lwr_line(&in->nodes, ulong2pe_as_line(t->line));
		
		if (char_type & pe_lexer_char_kw_start) {
			pe_lexer_kw(in, t);
		} else if (char_type & pe_lexer_char_id_start) {
			int fix;
			pe_hash_vars(0);
			
			if (char_type & pe_lexer_char_id_body) {
				pe_hash_char(b[i]);  fix= 0;
			} else {
				fix= 1;
			}
			t->error_type = pe_lexeme_id;
			pe_lexer_generic_id(in, &pe_hash_value, t);
			if (t->error == pe_lexeme_no_error) {
				if (in->producing_output
				    &&  (pe_as_lit_id_map_wr_id(&in->ids, t->len, t->str, fix, pe_hash_value, &t->v.id)
					 || pe_as_lwr_id(&in->nodes, t->v.id))) {
					t->error = pe_lexeme_application_error;
					in->producing_output = 0;
				} else {
					t->type = pe_lexeme_id;
				}
			}
		} else if (char_type & pe_lexer_char_kw_end) {
			i += 1;
			in->i = i;
			t->len = i - in->lexeme_start;
			t->str = &in->buffer[in->lexeme_start];
			t->error_type = pe_lexeme_eos;
			if (in->producing_output 
			    &&  pe_as_lwr_eos(&in->nodes)) {
				in->producing_output = 0;
			} else {
				t->error = pe_lexeme_no_error;
				t->type = pe_lexeme_eos;
			}
			
		} else if (char_type & pe_lexer_char_int_start) {
			unsigned long v;
			int negative;
			if (char_type & pe_lexer_char_int_body) {
				v= b[i] - '0';   negative= 0;
			} else {
				v= 0;   negative= (b[i] == '-');  
			}
			pe_lexer_int(in, v, negative, t);
			
		} else if (char_type & pe_lexer_char_string_delim) {
			pe_lexer_str(in, t);

		} else {
			in->i = i+1;
			t->len = in->i - in->lexeme_start;
			t->str = &in->buffer[in->lexeme_start];
			t->v.bad_char = in->buffer[in->lexeme_start];
			t->error = pe_lexeme_bad_char;
			in->producing_output = 0;
		}
	}
}


static int
pe_lexer_open_output(struct pe_lexer *in, char const *output_prefix)
{
	int err = 0;
	int n_wanted;
	char file_name[PATH_MAX];

	n_wanted = snprintf(file_name, sizeof file_name, "%s%s", output_prefix, pe_lexer_out_ids_name);
	if (n_wanted >= sizeof file_name) {
		err = ENAMETOOLONG;
		goto could_not_create_id_name;
	}
	err = pe_as_lit_id_map_wr_open(&in->ids, file_name);
	if (err)
		goto could_not_open_id_map;

	n_wanted = snprintf(file_name, sizeof file_name, "%s%s", output_prefix, pe_lexer_out_strs_name);
	if (n_wanted >= sizeof file_name) {
		err = ENAMETOOLONG;
		goto could_not_create_str_name;
	}
	err = pe_as_lit_str_map_wr_open(&in->strings, file_name);
	if (err)
		goto could_not_open_str_map;

	n_wanted = snprintf(file_name, sizeof file_name, "%s%s", output_prefix, pe_lexer_out_nodes_name);
	if (n_wanted >= sizeof file_name) {
		err = ENAMETOOLONG;
		goto could_not_create_nodes_name;
	}
	pe_as_lwr_set_buffers(&in->nodes, 4, 256);
	err = pe_as_lwr_open(&in->nodes, file_name);
	if (err)
		goto could_not_open_nodes;
	return err;

could_not_open_nodes:
could_not_create_nodes_name:
	(void)pe_as_lit_str_map_wr_close(&in->strings);
could_not_open_str_map:
could_not_create_str_name:
	(void)pe_as_lit_id_map_wr_close(&in->ids);
could_not_open_id_map:
could_not_create_id_name:
	return err;
}



int
pe_lexer_open(struct pe_lexer *in, char const *input_file_name,
	      char const *output_prefix, 
	      struct pe_as_lit_kw_map *kws)
{
	int err = 0;

	in->keywords = kws;
	if (!pe_lexer_input_open(in->file, input_file_name)) {
		err = errno;
		goto could_not_open_input;
	}
	if ((in->buffer = malloc(pe_lexer_buffer_size)) == (char *)0) {
		err = ENOMEM;
		goto out_of_memory_failure;
	}
	
	in->producing_output = 1;
	in->some_output_produced = 1;
	err = pe_lexer_open_output(in, output_prefix);
	if (err)
		goto could_not_open_output;
	
	in->i = in->lexeme_start = in->buffer_end = pe_lexer_buffer_start;
	in->buffer_size =  pe_lexer_buffer_size;
	in->line = 1;
	return err;

could_not_open_output:
	free(in->buffer);
out_of_memory_failure:
	(void)pe_lexer_input_close(in->file);
could_not_open_input:
	return err;
}


static int
pe_lexer_close_output(struct pe_lexer *in) 
{
	int err;

	err = pe_as_lwr_close(&in->nodes);
	if (err)
		goto could_not_close_nodes;
	err = pe_as_lit_str_map_wr_close(&in->strings);
	if (err)
		goto could_not_close_strings;
	err = pe_as_lit_id_map_wr_close(&in->ids);
	return err;

could_not_close_nodes:
	(void)pe_as_lit_str_map_wr_close(&in->strings);
could_not_close_strings:
	(void)pe_as_lit_id_map_wr_close(&in->ids);
	return err;
}



int
pe_lexer_close(struct pe_lexer *in)
{
	int err;

	err = pe_lexer_close_output(in);
	free(in->buffer);
	if (!pe_lexer_input_close(in->file))
		err = errno;
	return err;
}
