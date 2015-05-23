#ifndef pe_hash_h
#define pe_hash_h

/*
 * Copyright (c) 1995-2015 Nineties-Retro
 */

typedef unsigned int pe_hash;

#define pe_hash_vars(h) \
  pe_hash pe_hash_value= h, pe_hash_g


#define pe_hash_char(c) \
  do { \
    pe_hash_value = (pe_hash_value << 4) + (c & ~('a' - 'A')); \
    if (pe_hash_g = pe_hash_value&0xf0000000, pe_hash_g != 0) { \
      pe_hash_value = pe_hash_value^(pe_hash_g>>24); \
      pe_hash_value = pe_hash_value^pe_hash_g; \
    } \
  } while (0)


pe_hash
pe_hash_lit(size_t, char const *);

#endif
