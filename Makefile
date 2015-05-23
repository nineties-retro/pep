TEXI2INFO=makeinfo
TEXI2PDF=texi2pdf
TEXI2HTML=texi2html
NROFF=nroff

CFLAGS = -Wall -Werror -O3

pe_as_rd_src = doc/pe_as_rd.texi
pe_as_rd_info = $(pe_as_rd_src:%.texi=%.info)
pe_as_rd_pdf = $(pe_as_rd_src:%.texi=%.pdf)

pe_as_html_obj = \
	pe_as_rd_toc.html \
	pe_as_rd.html \
	pe_as_rd_fot.html \
	pe_as_rd_abt.html

pe_as_pdf_obj = \
	pe_as_rd.aux \
	pe_as_rd.cp \
	pe_as_rd.fn \
	pe_as_rd.fns \
	pe_as_rd.ky \
	pe_as_rd.log \
	pe_as_rd.pg \
	pe_as_rd.toc \
	pe_as_rd.tp \
	pe_as_rd.tps \
	pe_as_rd.vr

peg_kws_src = \
	src/pe_hash.c \
	src/pe_as_lit_kw_map.c \
	src/pe_as_lit_kw_map_wr.c \
	src/peg_kws.c

peg_kws_obj = $(peg_kws_src:%.c=%.o)

pel_src = \
	src/pe_hash.c \
	src/pe_as_kw_lit_map_rd.c \
	src/pe_as_lit_kw_map.c \
	src/pe_as_lit_kw_map_rd.c \
	src/pe_as_lit_id_map.c \
	src/pe_as_lit_id_map_wr.c \
	src/pe_as_lit_str_map_wr.c \
	src/pe_as_lwr.c \
	src/pe_lexer.c \
	src/pe_lexer_cl.c

pel_obj = $(pel_src:%.c=%.o)

pe_as_rd_pp_src = \
	src/pe_as_id_lit_map_rd.c \
	src/pe_as_kw_lit_map_rd.c \
	src/pe_as_rd.c \
	src/pe_as_rd_pp.c \
	src/pe_as_rd_pp_cl.c \
	src/pe_as_str_lit_map_rd.c

pe_as_rd_pp_obj = $(pe_as_rd_pp_src:%.c=%.o)

pe_as_rd_dump_src = \
	src/pe_as_id_lit_map_rd.c \
	src/pe_as_kw_lit_map_rd.c \
	src/pe_as_rd.c \
	src/pe_as_rd_dump.c \
	src/pe_as_str_lit_map_rd.c

pe_as_rd_dump_obj = $(pe_as_rd_dump_src:%.c=%.o)

pegc_src = \
	src/pe_as_id_lit_map_rd.c \
	src/pe_as_lit_kw_map.c \
	src/pe_as_lit_kw_map_wr.c \
	src/pe_as_kw_lit_map_rd.c \
	src/pe_as_rd.c \
	src/pe_hash.c \
	src/pegc.c \
	src/pe_as_str_lit_map_rd.c

pegc_obj = $(pegc_src:%.c=%.o)

pep_src = \
	src/pe_hash.c \
	src/pe_as_id_lit_map_rd.c \
	src/pe_as_kw_lit_map_rd.c \
	src/pe_as_lit_kw_map.c \
	src/pe_as_lit_kw_map_rd.c \
	src/pe_as_lit_id_map.c \
	src/pe_as_lit_id_map_wr.c \
	src/pe_as_lit_str_map_wr.c \
	src/pe_as_lwr.c \
	src/pe_as_rd.c \
	src/pe_as_str_lit_map_rd.c \
	src/pe_grammar.c \
	src/pe_lexer.c \
	src/pe_parser.c \
	src/pe_parser_cl.c \
	src/pe_rule_set.c

pep_obj = $(pep_src:%.c=%.o)


.PHONEY: all 

all: \
	grammars/peg/as \
	grammars/peg/pp \
	grammars/edif/200/kw \
	grammars/edif/200/pp \
	grammars/edif/300/kw \
	grammars/edif/300/pp \
	grammars/edif/400/kw \
	grammars/edif/400/pp \
	src/pep \
	doc/peg2h.txt \
	doc/peg2prolog.txt \
	doc/peg2scheme.txt \
	doc/peg_make.txt \
	doc/pegc.txt \
	doc/pep.txt \
	doc/pe_as_rd.info \
	doc/pe_as_rd.pdf \
	doc/pe_as_rd.html

src/pel: $(pel_obj)
	$(CC) -o $@ $(pel_obj)

src/peg_kws: $(peg_kws_obj)
	$(CC) -o $@ $(peg_kws_obj)

src/pe_as_rd_pp: $(pe_as_rd_pp_obj)
	$(CC) -o $@ $(pe_as_rd_pp_obj)

src/pe_as_rd_dump: $(pe_as_rd_dump_obj)
	$(CC) -o $@ $(pe_as_rd_dump_obj)

src/pegc: $(pegc_obj)
	$(CC) -o $@ $(pegc_obj)

src/pep: $(pep_obj)
	$(CC) -o $@ $(pep_obj)

grammars/peg/kw:	src/peg_kws
	src/peg_kws -o grammars/peg/

grammars/peg/as grammars/peg/id:	src/pel grammars/peg/kw
	src/pel -o grammars/peg/ -k grammars/peg/kw grammars/peg/peg

grammars/peg/pp:	src/pe_as_rd_pp
	src/pe_as_rd_pp -lgrammars/peg/ grammars/peg/ > $@

grammars/edif/200/as grammars/edif/200/id: src/pel grammars/peg/kw grammars/edif/200/peg
	src/pel -o grammars/edif/200/ -k grammars/peg/kw grammars/edif/200/peg

grammars/edif/200/kw grammars/edif/200/kws: src/pegc grammars/edif/200/as
	src/pegc grammars/edif/200/

grammars/edif/300/as grammars/edif/300/id: src/pel grammars/peg/kw grammars/edif/300/peg
	src/pel -o grammars/edif/300/ -k grammars/peg/kw grammars/edif/300/peg

grammars/edif/300/kw grammars/edif/300/kws: src/pegc grammars/edif/300/as
	src/pegc grammars/edif/300/

grammars/edif/400/as grammars/edif/400/id: src/pel grammars/peg/kw grammars/edif/400/peg
	src/pel -o grammars/edif/400/ -k grammars/peg/kw grammars/edif/400/peg

grammars/edif/400/kw grammars/edif/400/kws: src/pegc grammars/edif/400/as
	src/pegc grammars/edif/400/

grammars/edif/200/pp: src/pe_as_rd_pp
	src/pe_as_rd_pp -lgrammars/peg/ grammars/edif/200/ > $@

grammars/edif/300/pp: src/pe_as_rd_pp
	src/pe_as_rd_pp -lgrammars/peg/ grammars/edif/300/ > $@

grammars/edif/400/pp: src/pe_as_rd_pp
	src/pe_as_rd_pp -lgrammars/peg/ grammars/edif/400/ > $@

%.info:	 %.texi
	$(TEXI2INFO) -o $@ $<

%.pdf:	 %.texi
	$(TEXI2PDF) -o $@ $<

%.html:	 %.texi
	$(TEXI2HTML) $<

%.txt:	%.1
	$(NROFF) -man $< | col -b > $@

clean:
	$(RM) $(peg_kws_obj) $(pel_obj) $(pe_as_pdf_obj) $(pegc_obj) $(pep_obj) $(pe_as_rd_pp_obj) grammars/peg/pp grammars/edif/200/pp grammars/edif/300/pp grammars/edif/400/pp

realclean: clean
	$(RM) $(pe_as_rd_info) doc/pe_as_rd.pdf $(pe_as_rd_html_obj) doc/*.txt \
	grammars/peg/as \
	grammars/peg/id \
	grammars/peg/kw \
	grammars/peg/str \
	grammars/edif/200/as \
	grammars/edif/200/id \
	grammars/edif/200/kw \
	grammars/edif/200/kws \
	grammars/edif/200/str \
	grammars/edif/300/as \
	grammars/edif/300/id \
	grammars/edif/300/kw \
	grammars/edif/300/kws \
	grammars/edif/300/str \
	grammars/edif/400/as \
	grammars/edif/400/id \
	grammars/edif/400/kw \
	grammars/edif/400/kws \
	grammars/edif/400/str \
	src/pe_as_dump \
	src/peg_kws \
	src/pegc \
	src/pel \
	src/pep
