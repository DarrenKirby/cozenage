/* main.c - The main REPL */

#include "compat_readline.h"
#include "main.h"
#include "parser.h"
#include "printer.h"
#include "types.h"
#include "environment.h"
#include "eval.h"
#include "coz_ext_lib.h"
#include "file_lib.h"
#include "process_context_lib.h"
#include "inexact_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>


/* set up readline history file loading and saving */
//char history_file[] = HIST_FILE;

void read_history_from_file() {
    char *hf = tilde_expand(HIST_FILE);
    if (access(hf, R_OK) == -1) {
        /* create empty file if it does not exist */
        FILE* f = fopen(hf, "w");

        if (f == NULL) {
            printf("Error: Could not read, open, or create history file `~/cozenage_history`.\n");
        }
        fclose(f);
    }
    read_history(hf);
    free(hf);
}

void save_history_to_file() {
    char *hf = tilde_expand(HIST_FILE);
    write_history(hf);
    free(hf);
}

struct lib_load {
    unsigned int coz_ext:1;
    unsigned int case_lambda:1;
    unsigned int char_lib:1;
    unsigned int complex:1;
    unsigned int cxr:1;
    unsigned int eval:1;
    unsigned int file:1;
    unsigned int inexact:1;
    unsigned int lazy:1;
    unsigned int load:1;
    unsigned int process_context:1;
    unsigned int read:1;
    unsigned int repl:1;
    unsigned int time:1;
    unsigned int write:1;
} load_libs = {0,0,0,0,
    0,0,0, 0,
    0,0,0,
    0,0,0,0};

static volatile sig_atomic_t got_sigint = 0;
#ifdef __linux__
static volatile sig_atomic_t discard_line = 0;
#endif

static void sigint_handler(const int sig) {
    (void)sig;
    got_sigint = 1;
    printf("\n");
}

#ifdef __linux__
/* Callback for Ctrl-G binding */
int discard_continuation(const int count, const int key) {
    (void)count; (void)key;
    discard_line = 1;
    rl_replace_line("", 0);  /* clear current line buffer */
    rl_done = 1;             /* break out of readline() */
    return 0;
}
#endif

/* Allow multi-line input in the REPL */
static char* read_multiline(const char* prompt, const char* cont_prompt) {
    char *line = NULL;
    char *input = NULL;
    size_t total_len = 0;
    int balance = 0;
    int in_string = 0;   /* track string literal state across lines */

    line = readline(prompt);
    if (!line) return NULL;
    if (got_sigint) {
        free(line);
        got_sigint = 0;
        return strdup("");  /* return empty input so REPL just re-prompts */
    }

    balance += paren_balance(line, &in_string);

    input = strdup(line);
    total_len = strlen(line);
    free(line);

    while (balance > 0 || in_string) {
        line = readline(cont_prompt);
        if (!line) break;
        if (got_sigint) {
            free(line);
            free(input);
            got_sigint = 0;
            return strdup("");  /* abort multiline and reset prompt */
        }

        balance += paren_balance(line, &in_string);

        const size_t line_len = strlen(line);
        char *tmp = realloc(input, total_len + line_len + 2);
        if (!tmp) { fprintf(stderr, "ENOMEM: malloc failed\n"); exit(EXIT_FAILURE); }
        input = tmp;
        input[total_len] = '\n';
        memcpy(input + total_len + 1, line, line_len + 1);

        total_len += line_len + 1;
        free(line);
    }
    return input;
}

/* read()
 * Take a line of input from a prompt and pass it
 * through a 2 step lexer/parser stream, and convert
 * the value to a Cell struct.
 * */
Cell* coz_read(Lex* e) {
    char *input = read_multiline(PS1_PROMPT, PS2_PROMPT);
    /* reset bold input */
    printf("%s", ANSI_RESET);
    if (!input || strcmp(input, "exit") == 0) {
        printf("\n");
        save_history_to_file();
        lex_delete(e);
        exit(0);
    }

    Parser *p = parse_str(input);
    if (!p) { free(input); return NULL; }

    Cell *v = parse_tokens(p);

    free_tokens(p->array, p->size);
    free(p);
    if (v) add_history(input);
    free(input);

    return v;
}

/* print()
 * Take the Cell produced by eval and print it in a
 * context-specific, meaningful way.
 * */
void coz_print(const Cell* v) {
    println_cell(v);
}

/* repl()
 * Read-Evaluate-Print loop
 * */
void repl() {
    /* load history */
    read_history_from_file();
    /* Initialize global environment */
    Lex* e = lex_initialize();
    /* Load (scheme base) procedures */
    lex_add_builtins(e);

    /* Load additional library procedures as specified by -l args */
    if (load_libs.coz_ext) {
        lex_add_coz_ext(e);
    }
    if (load_libs.file) {
        lex_add_file_lib(e);
    }
    if (load_libs.process_context) {
        lex_add_proc_con_lib(e);
    }
    if (load_libs.inexact) {
        lex_add_inexact_lib(e);
    }

    for (;;) {
        Cell *val = coz_read(e);
        if (!val) {
            continue;
        }
        Cell *result = coz_eval(e, val);
        if (!result) {
            continue;
        }
        coz_print(result);
        cell_delete(result);
    }
}

static void show_help(void) {
    printf("Usage: %s [<options>]\n\n\
A (not yet) R7RS-compliant Scheme REPL\n\n\
Options:\n\
    -l, --library\t preload scheme libraries at startup\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
\n\
    '-l' and '--library' accept a required comma-delimited list of\n\
    libraries to pre-load. Accepted values are:\n\
    coz-ext, case-lambda, char, complex, cxr, eval, file, inexact\n\
    lazy, load, process-context, read, repl, time, write\n\n\
Report bugs to <bulliver@gmail.com>\n", APP_NAME);
}

void process_library_arg(struct lib_load *l, const char *arg) {
    char *arg_copy = strdup(arg);
    if (!arg_copy) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    char *token = strtok(arg_copy, ",");

    while (token != NULL) {
        if (strcmp(token, "coz-ext") == 0) {
            l->coz_ext = 1;
        } else if (strcmp(token, "case-lambda") == 0) {
            l->case_lambda = 1;
        } else if (strcmp(token, "char") == 0) {
            l->char_lib = 1;
        } else if (strcmp(token, "complex") == 0) {
            l->complex = 1;
        } else if (strcmp(token, "cxr") == 0) {
            l->cxr = 1;
        } else if (strcmp(token, "eval") == 0) {
            l->eval = 1;
        } else if (strcmp(token, "file") == 0) {
            l->file = 1;
        } else if (strcmp(token, "inexact") == 0) {
            l->inexact = 1;
        } else if (strcmp(token, "lazy") == 0) {
            l->lazy = 1;
        } else if (strcmp(token, "load") == 0) {
            l->load = 1;
        } else if (strcmp(token, "process-context") == 0) {
            l->process_context = 1;
        } else if (strcmp(token, "read") == 0) {
            l->read = 1;
        } else if (strcmp(token, "repl") == 0) {
            l->repl = 1;
        } else if (strcmp(token, "time") == 0) {
            l->time = 1;
        } else if (strcmp(token, "write") == 0) {
            l->write = 1;
        } else {
            fprintf(stderr, "Error: Unknown library name '%s' specified.\n", token);
            fprintf(stderr, "Run with -h for a list of valid library names.\n");
            free(arg_copy); /* Clean up before exiting */
            exit(EXIT_FAILURE);
        }
        /* Get the next token */
        token = strtok(NULL, ",");
    }
    free(arg_copy);
}

int main(const int argc, char** argv) {
    int opt;

    const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'},
        {"library", required_argument, NULL, 'l'},
        {NULL,0,NULL,0}
    };

    while ((opt = getopt_long(argc, argv, "Vhl:", long_opts, NULL)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s%s%s version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
                printf(" Compiled on %s at %s\n", __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
            case 'l':
                process_library_arg(&load_libs, optarg);
                break;
            default:
                ;
        }
    }

    /* Print Version and Exit Information */
    printf("  %s%s%s version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
    printf("  Press <Ctrl+d> or type 'exit' to quit\n\n");

    /* Set up keybinding and signal for Ctrl-G and CTRL-C */
#ifdef __linux__
    rl_bind_key('\007', discard_continuation);  /* 7 = Ctrl-G */
#endif
    signal(SIGINT, sigint_handler);

    /* Run until we don't */
    repl();
    return 0;
}
