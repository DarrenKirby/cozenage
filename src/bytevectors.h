/*
 * 'src/bytevectors.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COZENAGE_BYTEVECTORS_H
#define COZENAGE_BYTEVECTORS_H

#include "cell.h"
#include "buffer.h"

#include <gc/gc.h>


typedef struct bv_ops_t {
    int64_t (*get)(const Cell*, int index);
    void (*set)(Cell*, int index, int64_t value);
    void (*repr)(const Cell*, str_buf_t*);
    void (*append)(Cell*, int64_t value);
    size_t elem_size;
} bv_ops_t;


extern const bv_ops_t BV_OPS[];

/* This big ugly-ass macro makes a constructor, getter, setter, and repr for
 * each type of bytevector: u8, s8, u16, s16, u32, s32, u64, and s64 */

#define DEFINE_BV_TYPE(suffix, ctype, fmt)                                  \
static int64_t get_##suffix(const Cell* bv, int i) {                        \
return ((ctype*)bv->bv->data)[i];                                           \
}                                                                           \
static void set_##suffix(Cell* bv, int i, int64_t val) {                    \
((ctype*)bv->bv->data)[i] = (ctype)val;                                     \
}                                                                           \
static void append_##suffix(Cell* bv, int64_t val) {                        \
if (bv->count == bv->bv->capacity) {                                        \
bv->bv->capacity *= 2;                                                      \
bv->bv->data = GC_REALLOC(bv->bv->data, bv->bv->capacity * sizeof(ctype));  \
}                                                                           \
((ctype*)bv->bv->data)[bv->count++] = (ctype)val;                           \
}                                                                           \
static void repr_##suffix(const Cell* bv, str_buf_t *sb) {                  \
sb_append_fmt(sb, "#%s(", #suffix);                                         \
for (int i = 0; i < bv->count; i++) {                                       \
sb_append_fmt(sb, fmt, ((ctype*)bv->bv->data)[i]);                          \
if (i != bv->count - 1) sb_append_char(sb, ' ');                            \
}                                                                           \
sb_append_char(sb, ')');                                                    \
}


/* Bytevector constructors, selectors,and procedures */
Cell* builtin_bytevector(const Lex* e, const Cell* a);
Cell* builtin_bytevector_length(const Lex* e, const Cell* a);
Cell* builtin_bytevector_ref(const Lex* e, const Cell* a);
Cell* builtin_bytevector_set_bang(const Lex* e, const Cell* a);
Cell* builtin_make_bytevector(const Lex* e, const Cell* a);
Cell* builtin_bytevector_copy(const Lex* e, const Cell* a);
Cell* builtin_bytevector_copy_bang(const Lex* e, const Cell* a);
Cell* builtin_bytevector_append(const Lex* e, const Cell* a);
Cell* builtin_utf8_string(const Lex* e, const Cell* a);
Cell* builtin_string_utf8(const Lex* e, const Cell* a);

#endif //COZENAGE_BYTEVECTORS_H
