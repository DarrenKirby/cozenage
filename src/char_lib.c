#include "char_lib.h"

#include <ctype.h>

#include "environment.h"
#include "types.h"

/*
char-ci<?
char-ci>=?
string-ci<=?
string-ci=?
string-ci>?
string-foldcase
char-ci<=?
char-ci=?
char-ci>?
char-foldcase
digit-value
string-ci<?
string-ci>=?
string-downcase
string-upcase
*/

Cell* builtin_char_alphabetic(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-alphabetic?: arg 1 must be a char");
    }
    return make_val_bool(isalpha(a->cell[0]->c_val));
}

Cell* builtin_char_whitespace(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-whitespace?: arg 1 must be a char");
    }
    return make_val_bool(isspace(a->cell[0]->c_val));
}

Cell* builtin_char_numeric(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-numeric?: arg 1 must be a char");
    }
    return make_val_bool(isdigit(a->cell[0]->c_val));
}

Cell* builtin_char_upper_case(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-upper-case?: arg 1 must be a char");
    }
    return make_val_bool(isupper(a->cell[0]->c_val));
}

Cell* builtin_char_lower_case(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-lower-case?: arg 1 must be a char");
    }
    return make_val_bool(islower(a->cell[0]->c_val));
}

Cell* builtin_char_upcase(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-upcase: arg 1 must be a char");
    }
    const unsigned char c = a->cell[0]->c_val;
    return make_val_char((char)toupper(c));
}

Cell* builtin_char_downcase(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-downcase: arg 1 must be a char");
    }
    const unsigned char c = a->cell[0]->c_val;
    return make_val_char((char)tolower(c));
}

void lex_add_char_lib(Lex* e) {
    lex_add_builtin(e, "char-alphabetic?", builtin_char_alphabetic);
    lex_add_builtin(e, "char-whitespace?", builtin_char_whitespace);
    lex_add_builtin(e, "char-numeric?", builtin_char_numeric);
    lex_add_builtin(e, "char-upper-case?", builtin_char_upper_case);
    lex_add_builtin(e, "char-lower-case?", builtin_char_lower_case);
    lex_add_builtin(e, "char-upcase", builtin_char_upcase);
    lex_add_builtin(e, "char-downcase", builtin_char_downcase);
}
