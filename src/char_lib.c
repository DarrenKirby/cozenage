#include "char_lib.h"
#include "environment.h"
#include "types.h"
#include <ctype.h>
#include <unicode/uchar.h>

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
    return make_val_bool(u_isalpha(a->cell[0]->c_val));
}

Cell* builtin_char_whitespace(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-whitespace?: arg 1 must be a char");
    }
    return make_val_bool(u_isspace(a->cell[0]->c_val));
}

Cell* builtin_char_numeric(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-numeric?: arg 1 must be a char");
    }
    return make_val_bool(u_isdigit(a->cell[0]->c_val));
}

Cell* builtin_char_upper_case(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-upper-case?: arg 1 must be a char");
    }
    return make_val_bool(u_isupper(a->cell[0]->c_val));
}

Cell* builtin_char_lower_case(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-lower-case?: arg 1 must be a char");
    }
    return make_val_bool(u_islower(a->cell[0]->c_val));
}

Cell* builtin_char_upcase(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-upcase: arg 1 must be a char");
    }
    return make_val_char(u_toupper(a->cell[0]->c_val));
}

Cell* builtin_char_downcase(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-downcase: arg 1 must be a char");
    }
    return make_val_char(u_tolower(a->cell[0]->c_val));
}

Cell* builtin_char_foldcase(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("char-foldcase: arg 1 must be a char");
    }
    const unsigned char c = a->cell[0]->c_val;
    return make_val_char(u_foldCase(c, U_FOLD_CASE_DEFAULT));
}

Cell* builtin_digit_value(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_CHAR) {
        return make_val_err("digit-value: arg 1 must be a char");
    }

    const int32_t value = u_charDigitValue(a->cell[0]->c_val);

    if (value == -1) {
        return make_val_bool(0);
    }
    return make_val_int(value);
}

void lex_add_char_lib(Lex* e) {
    lex_add_builtin(e, "char-alphabetic?", builtin_char_alphabetic);
    lex_add_builtin(e, "char-whitespace?", builtin_char_whitespace);
    lex_add_builtin(e, "char-numeric?", builtin_char_numeric);
    lex_add_builtin(e, "char-upper-case?", builtin_char_upper_case);
    lex_add_builtin(e, "char-lower-case?", builtin_char_lower_case);
    lex_add_builtin(e, "char-upcase", builtin_char_upcase);
    lex_add_builtin(e, "char-downcase", builtin_char_downcase);
    lex_add_builtin(e, "char-foldcase", builtin_char_foldcase);
    lex_add_builtin(e, "digit-value", builtin_digit_value);
}
