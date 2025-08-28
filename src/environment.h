#ifndef COZENAGE_ENVIRONMENT_H
#define COZENAGE_ENVIRONMENT_H


struct Cell;  // forward declaration
typedef struct Cell Cell;

typedef struct Lex {
    int count;
    char** syms;   /* symbol names */
    Cell** vals;   /* values (can be VAL_PROC, VAL_INT, etc.) */
} Lex;

/* Environment management */
Lex* lex_initialize(void);
void lex_delete(Lex* e);

/* Environment operations */
Cell* lex_get(const Lex* e, const Cell* k);
void lex_put(Lex* e, const Cell* k, const Cell* v);

/* Builtin helpers */
Cell* lex_register_builtin(const char* name, Cell* (*func)(Lex*, Cell*));
void lex_add_builtin(Lex* e, const char* name, Cell* (*func)(Lex*, Cell*));
void lex_add_builtins(Lex* e);


#endif //COZENAGE_ENVIRONMENT_H
