#include "char_lib.h"
#include "environment.h"
#include "types.h"
#include "ops.h"
#include <ctype.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>
#include <gc/gc.h>


/*
string-ci<=?
string-ci=?
string-ci>?
string-ci<?
string-ci>=?
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

Cell* builtin_char_equal_ci(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_val_int(the_char_fc->c_val);
    }

    Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_eq_op(e, cell_sexpr);
}

Cell* builtin_char_lt_ci(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_val_int(the_char_fc->c_val);
    }

    Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_lt_op(e, cell_sexpr);
}

Cell* builtin_char_lte_ci(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_val_int(the_char_fc->c_val);
    }

    Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_lte_op(e, cell_sexpr);
}

Cell* builtin_char_gt_ci(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_val_int(the_char_fc->c_val);
    }

    Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_gt_op(e, cell_sexpr);
}

Cell* builtin_char_gte_ci(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_val_int(the_char_fc->c_val);
    }

    Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_gte_op(e, cell_sexpr);
}

Cell* builtin_string_downcase(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    UErrorCode status = U_ZERO_ERROR;
    UChar* src = convert_to_utf16(a->cell[0]->str);
    if (!src) return make_val_err("string-downcase: malformed UTF-8 string");

    const int32_t src_len = u_countChar32(src, -1);

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);;
    int32_t dest_len = u_strToLower(dst, src_len + 1, src, -1, NULL, &status);

    if (dest_len < src_len) {
        return make_val_err("string-downcase: some chars not copied!!!");
    }

    char* result = convert_to_utf8(dst);
    if (!result) {
        return make_val_err("string-downcase: malformed UTF-8 string");
    }
    return make_val_str(result);
}

Cell* builtin_string_upcase(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    UErrorCode status = U_ZERO_ERROR;
    UChar* src = convert_to_utf16(a->cell[0]->str);
    if (!src) return make_val_err("string-upcase: malformed UTF-8 string");

    const int32_t src_len = u_countChar32(src, -1);

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);;
    int32_t dest_len = u_strToUpper(dst, src_len + 1, src, -1, NULL, &status);

    if (dest_len < src_len) {
        return make_val_err("string-upcase: some chars not copied!!!");
    }

    char* result = convert_to_utf8(dst);
    if (!result) {
        return make_val_err("string-upcase: malformed UTF-8 string");
    }
    return make_val_str(result);
}

Cell* builtin_string_foldcase(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    UErrorCode status = U_ZERO_ERROR;
    UChar* src = convert_to_utf16(a->cell[0]->str);
    if (!src) return make_val_err("string-foldcase: malformed UTF-8 string");

    const int32_t src_len = u_countChar32(src, -1);

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);;
    int32_t dest_len = u_strFoldCase(dst, src_len + 1, src, -1, U_FOLD_CASE_DEFAULT, &status);

    if (dest_len < src_len) {
        return make_val_err("string-foldcase: some chars not copied!!!");
    }

    char* result = convert_to_utf8(dst);
    if (!result) {
        return make_val_err("string-foldcase: malformed UTF-8 string");
    }
    return make_val_str(result);
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
    lex_add_builtin(e, "char-ci=?", builtin_char_equal_ci);
    lex_add_builtin(e, "char-ci<?", builtin_char_lt_ci);
    lex_add_builtin(e, "char-ci<=?", builtin_char_lte_ci);
    lex_add_builtin(e, "char-ci>?", builtin_char_gt_ci);
    lex_add_builtin(e, "char-ci>=?", builtin_char_gte_ci);
    lex_add_builtin(e, "string-downcase", builtin_string_downcase);
    lex_add_builtin(e, "string-upcase", builtin_string_upcase);
    lex_add_builtin(e, "string-foldcase", builtin_string_foldcase);
}
