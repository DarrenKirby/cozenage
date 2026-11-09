#include <criterion/internal/test.h>
#include <gc/gc.h>
#include "../src/cell.h"
#include "../src/polymorph.h"
#include "../src/types.h"
#include "../src/sets.h"


static void setup(void) {
    GC_INIT();
    init_global_singletons();
}

TestSuite(set_builtins);

Test(set_builtins, set_constructor, .init = setup) {
    Cell* c = make_cell_set(nullptr);
    cr_assert_not_null(c);
    cr_assert_eq(c->type, CELL_SET);
    /* Empty constructor should still initialize the hash */
    cr_assert_not_null(c->table);

    Cell* a1 = make_cell_integer(1);
    Cell* a2 = make_cell_integer(2);
    Cell* a3 = make_cell_integer(3);
    Cell* a4 = make_cell_integer(4);
    Cell* s = make_sexpr_len4(a1, a2, a3, a4);

    c = make_cell_set(s);
    cr_assert_not_null(c);
    cr_assert_eq(c->type, CELL_SET);
    cr_assert_not_null(c->table);

    /* Test non-hashable type raises error. */
    //Cell* a5 = make_list_from_sexpr(s);
    //cr_assert_eq(c->type, CELL_PAIR);
    //Cell* a6 = make_sexpr_len1(a5);
    //c = make_cell_set(a6);
    //cr_assert_not_null(c);
    //cr_assert_eq(c->type, CELL_ERROR);
}
