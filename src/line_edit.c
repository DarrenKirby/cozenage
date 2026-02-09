/*
 * 'src/line_edit.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2026 Darren Kirby <darren@dragonbyte.ca>
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


#include "line_edit.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <wchar.h>
#include <gc/gc.h>

#ifdef __linux__
#include <ctype.h>
#endif


/* ANSI escape codes */
#define ESC "\033"
#define CURSOR_LEFT  ESC "[D"
#define CURSOR_RIGHT ESC "[C"
#define CLEAR_TO_EOL ESC "[K"
#define CURSOR_UP    ESC "[A"
#define CURSOR_DOWN  ESC "[B"

/* Special key codes */
#define CTRL_A 1
#define CTRL_B 2
#define CTRL_C 3
#define CTRL_D 4
#define CTRL_E 5
#define CTRL_F 6
#define CTRL_G 7
#define CTRL_H 8
#define TAB    9
#define ENTER  13
#define CTRL_K 11
#define CTRL_L 12
#define CTRL_U 21
#define CTRL_W 23
#define ESC_CODE 27
#define BACKSPACE 127

/* History settings */
#define DEFAULT_HISTORY_SIZE 500


char **scheme_procedures = nullptr;

/* Terminal state */
static struct termios orig_termios;
static int raw_mode_enabled = 0;
static volatile sig_atomic_t got_interrupt = 0;

/* History management */
typedef struct {
    char** entries;
    size_t count;
    size_t capacity;
    size_t current;
} History;

static History history = {nullptr, 0, 0, 0};


/* Line editing state */
typedef struct {
    char* buffer;
    size_t buffer_size;
    size_t length;
    size_t cursor;
    const char* prompt;
    size_t prompt_len;
    int last_was_tab;
} LineState;


/* Signal handler */
static void sigint_handler(const int sig) {
    (void)sig;
    got_interrupt = 1;
}


/* Terminal management */
static void disable_raw_mode(void) {
    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_enabled = 0;
    }
}


static int enable_raw_mode(void) {
    if (!isatty(STDIN_FILENO)) return -1;
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) return -1;

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) return -1;
    raw_mode_enabled = 1;
    return 0;
}


/* UTF-8 handling */
static size_t utf8_char_len(unsigned char c) {
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}


static size_t utf8_strlen(const char* str) {
    size_t len = 0;
    while (*str) {
        const size_t char_len = utf8_char_len((unsigned char)*str);
        str += char_len;
        len++;
    }
    return len;
}


static size_t utf8_prev_char(const char* str, size_t pos) {
    if (pos == 0) return 0;
    pos--;
    while (pos > 0 && (str[pos] & 0xC0) == 0x80) pos--;
    return pos;
}


static size_t utf8_next_char(const char* str, size_t pos, size_t len) {
    if (pos >= len) return len;
    const size_t char_len = utf8_char_len((unsigned char)str[pos]);
    pos += char_len;
    if (pos > len) return len;
    return pos;
}


/* Get terminal width */
static int get_terminal_width(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) return 80;
    return ws.ws_col;
}


/* Line display functions */
static void refresh_line(LineState* ls) {
    /* Move cursor to beginning of line */
    printf("\r");

    /* Write prompt */
    printf("%s", ls->prompt);

    /* Write current buffer */
    printf("%s", ls->buffer);

    /* Clear to end of line */
    printf(CLEAR_TO_EOL);

    /* Move cursor to correct position */
    const size_t display_pos = ls->prompt_len + utf8_strlen(ls->buffer);
    size_t cursor_display = ls->prompt_len;

    /* Calculate cursor display position */
    char temp[ls->cursor + 1];
    memcpy(temp, ls->buffer, ls->cursor);
    temp[ls->cursor] = '\0';
    cursor_display += utf8_strlen(temp);

    /* Move cursor back from end */
    const size_t moves = display_pos - cursor_display;
    for (size_t i = 0; i < moves; i++) {
        printf(CURSOR_LEFT);
    }

    fflush(stdout);
}


/* History functions */
void add_history_entry(const char* line) {
    if (!line || !*line) return;

    /* Don't add duplicates */
    if (history.count > 0 && strcmp(history.entries[history.count - 1], line) == 0) {
        return;
    }

    if (history.count >= history.capacity) {
        if (history.capacity == 0) {
            history.capacity = DEFAULT_HISTORY_SIZE;
            history.entries = GC_MALLOC(sizeof(char*) * history.capacity);
        } else if (history.count >= DEFAULT_HISTORY_SIZE) {
            /* Shift everything down */
            GC_FREE(history.entries[0]);
            memmove(history.entries, history.entries + 1,
                    (history.count - 1) * sizeof(char*));
            history.count--;
        }
    }

    history.entries[history.count] = GC_strdup(line);
    history.count++;
    history.current = history.count;
}


int read_history(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return -1;

    char* line = nullptr;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        /* Remove newline */
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        add_history_entry(line);
    }

    free(line);
    fclose(fp);
    return 0;
}


int write_history(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return -1;

    for (size_t i = 0; i < history.count; i++) {
        fprintf(fp, "%s\n", history.entries[i]);
    }

    fclose(fp);
    return 0;
}


/* Tilde expansion */
char* tilde_expand(const char* path) {
    if (!path || path[0] != '~') {
        return GC_strdup(path);
    }

    const char* home;
    const char* rest;

    if (path[1] == '/' || path[1] == '\0') {
        /* ~/... or just ~ */
        home = getenv("HOME");
        rest = path + 1;
    } else {
        /* ~username/... */
        const char* slash = strchr(path + 1, '/');
        size_t username_len = slash ? (size_t)(slash - path - 1) : strlen(path + 1);
        char* username = GC_MALLOC_ATOMIC(username_len + 1);
        memcpy(username, path + 1, username_len);
        username[username_len] = '\0';

        struct passwd* pw = getpwnam(username);
        if (pw) {
            home = pw->pw_dir;
            rest = slash ? slash : "";
        } else {
            return GC_strdup(path);
        }
    }

    if (!home) {
        return GC_strdup(path);
    }

    size_t result_len = strlen(home) + strlen(rest) + 1;
    char* result = GC_MALLOC_ATOMIC(result_len);
    snprintf(result, result_len, "%s%s", home, rest);
    return result;
}


/* Filename completion */
static char* le_filename_generator(const char* text, int state) {
    static DIR* dir = nullptr;
    static char* dirname_buf = nullptr;
    static char* basename_buf = nullptr;
    static size_t basename_len = 0;

    if (state == 0) {
        /* First call - initialize */
        if (dir) {
            closedir(dir);
            dir = nullptr;
        }

        /* Expand tilde if present */
        const char* expanded = tilde_expand(text);

        /* Split into directory and basename */
        const char* last_slash = strrchr(expanded, '/');
        if (last_slash) {
            size_t dir_len = last_slash - expanded + 1;
            dirname_buf = GC_MALLOC_ATOMIC(dir_len + 1);
            memcpy(dirname_buf, expanded, dir_len);
            dirname_buf[dir_len] = '\0';

            basename_buf = GC_strdup(last_slash + 1);
            basename_len = strlen(basename_buf);

            dir = opendir(dirname_buf);
        } else {
            dirname_buf = GC_strdup("");
            basename_buf = GC_strdup(expanded);
            basename_len = strlen(basename_buf);
            dir = opendir(".");
        }

        if (!dir) return nullptr;
    }

    /* Find next matching entry */
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        /* Check if it matches the prefix */
        if (strncmp(entry->d_name, basename_buf, basename_len) == 0) {
            /* Build full path */
            size_t result_len = strlen(dirname_buf) + strlen(entry->d_name) + 2;
            char* result = GC_MALLOC_ATOMIC(result_len);
            snprintf(result, result_len, "%s%s", dirname_buf, entry->d_name);

            /* Check if it's a directory and add trailing slash */
            struct stat st;
            if (stat(result, &st) == 0 && S_ISDIR(st.st_mode)) {
                const size_t len = strlen(result);
                char* dir_result = GC_MALLOC_ATOMIC(len + 2);
                strcpy(dir_result, result);
                dir_result[len] = '/';
                dir_result[len + 1] = '\0';
                return dir_result;
            }

            return result;
        }
    }

    /* No more matches */
    closedir(dir);
    dir = nullptr;
    return nullptr;
}


void populate_dynamic_completions(const Lex* e)
{
    int symbol_count = 0;
    /* Iterate once to get number of symbols. */
    hti it = ht_iterator(e->global);
    while (ht_next(&it)) {
        symbol_count++;
    }

    /* Special forms have to be added manually. */
    char* special_forms[] = { "quote", "define", "lambda", "let", "let*", "letrec", "set!", "if",
        "when", "unless", "cond", "else", "begin", "import", "and", "or", "do", "case", "letrec*",
        "defmacro", "quasiquote", "unquote", "unquote-splicing", "with_gc_stats"};
    /* Why tho, does CLion always think this is C++ code? */
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int num_sfs = sizeof(special_forms) / sizeof(special_forms[0]);

    /* Allocate space for 'symbol_count' pointers to strings, plus one for NULL, plus num_sfs for the SF. */
    scheme_procedures = GC_MALLOC(sizeof(char*) * (symbol_count + 1 + num_sfs));
    if (!scheme_procedures) {
        perror("ENOMEM: malloc failed");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    for (int j = 0; j < num_sfs; j++) {
        scheme_procedures[i] = GC_strdup(special_forms[j]);
        i++;
    }
    /* Iterate second time to copy symbol names. */
    it = ht_iterator(e->global);
    while (ht_next(&it)) {
        scheme_procedures[i] = GC_strdup(it.key);
        i++;
    }

    /* The list must be NULL-terminated for the generator to know when to stop. */
    scheme_procedures[i] = nullptr;
}


static char* le_symbol_generator(const char *text, const int state)
{
    static int list_index, len;
    char *name;

    /* If this is the first call for this completion, reset the state. */
    if (!state) {
        list_index = 0;
        len = (int)strlen(text);
    }

    /* Iterate through the procedure list and return the next match. */
    while ((name = scheme_procedures[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    /* No more matches found. */
    return nullptr;
}

/* Completion matching */
static char** le_completion_matches(const char* text, char* (*generator)(const char*, int)) {
    size_t matches_size = 16;
    char** matches = GC_MALLOC(sizeof(char*) * matches_size);
    size_t match_count = 0;

    /* Get all matches */
    int state = 0;
    char* match;
    while ((match = generator(text, state++)) != NULL) {
        if (match_count + 2 >= matches_size) {
            matches_size *= 2;
            matches = GC_REALLOC(matches, sizeof(char*) * matches_size);
        }
        matches[match_count++] = match;
    }

    if (match_count == 0) {
        return nullptr;
    }

    /* NULL terminate */
    matches[match_count] = nullptr;

    /* If more than one match, add common prefix as first element */
    if (match_count > 1) {
        /* Find common prefix */
        size_t prefix_len = strlen(matches[0]);
        for (size_t i = 1; i < match_count; i++) {
            size_t j = 0;
            while (j < prefix_len && matches[0][j] == matches[i][j]) {
                j++;
            }
            prefix_len = j;
        }

        /* Shift array and add prefix */
        char** new_matches = GC_MALLOC(sizeof(char*) * (match_count + 2));
        new_matches[0] = GC_MALLOC_ATOMIC(prefix_len + 1);
        memcpy(new_matches[0], matches[0], prefix_len);
        new_matches[0][prefix_len] = '\0';

        memcpy(new_matches + 1, matches, sizeof(char*) * match_count);
        new_matches[match_count + 1] = nullptr;

        return new_matches;
    }

    return matches;
}


static bool is_filename_char(const char c) {
    return c != '"' && !isspace((unsigned char)c);
}


static bool is_symbol_char(const char c) {
    return isalnum((unsigned char)c) ||
           c == '-' || c == '_' ||
           c == '!' || c == '?' ||
           c == '*' || c == '/' ||
           c == '<' || c == '>' ||
           c == '+' || c == '.' ||
           c == '=';
}


/* Extract word at cursor for completion */
static void extract_word_at_cursor(const LineState* ls, size_t* start, size_t* end, const bool filename_mode) {
    *start = ls->cursor;
    *end   = ls->cursor;

    if (filename_mode) {
        while (*start > 0 && is_filename_char(ls->buffer[*start - 1])) {
            (*start)--;
        }
        while (*end < ls->length && is_filename_char(ls->buffer[*end])) {
            (*end)++;
        }
    } else {
        while (*start > 0 && is_symbol_char(ls->buffer[*start - 1])) {
            (*start)--;
        }
        while (*end < ls->length && is_symbol_char(ls->buffer[*end])) {
            (*end)++;
        }
    }
}


static bool cursor_is_inside_string(const LineState *ls) {
    bool in_string = false;

    for (size_t i = 0; i < ls->cursor; i++) {
        if (ls->buffer[i] == '"' &&
            (i == 0 || ls->buffer[i - 1] != '\\')) {
            in_string = !in_string;
            }
    }
    return in_string;
}


/* Handle tab completion */
static void handle_completion(LineState* ls) {
    bool filename_mode = cursor_is_inside_string(ls);

    /* Extract word at cursor */
    size_t word_start, word_end;
    extract_word_at_cursor(ls, &word_start, &word_end, filename_mode);

    /* Extract text to complete */
    size_t text_len = ls->cursor - word_start;
    char* text = GC_MALLOC_ATOMIC(text_len + 1);
    memcpy(text, ls->buffer + word_start, text_len);
    text[text_len] = '\0';

    /* Get completions */
    char** completions;
    if (filename_mode) {
        completions = le_completion_matches(text, le_filename_generator);
    } else {
        completions = le_completion_matches(text, le_symbol_generator);
    }

    if (!completions || !completions[0]) {
        /* No completions */
        if (ls->last_was_tab) {
            /* Second tab - beep or do nothing */
            printf("\a");
            fflush(stdout);
        }
        ls->last_was_tab = 1;
        return;
    }

    if (completions[1] == NULL) {
        /* Single completion */
        const char* completion = completions[0];
        size_t comp_len = strlen(completion);

        /* Replace word with completion */
        size_t new_len = ls->length - text_len + comp_len;
        if (new_len >= ls->buffer_size) {
            ls->buffer_size = new_len + 256;
            ls->buffer = GC_REALLOC(ls->buffer, ls->buffer_size);
        }

        /* Move rest of buffer */
        memmove(ls->buffer + word_start + comp_len,
                ls->buffer + ls->cursor,
                ls->length - ls->cursor);

        /* Insert completion */
        memcpy(ls->buffer + word_start, completion, comp_len);

        ls->length = new_len;
        ls->cursor = word_start + comp_len;
        ls->buffer[ls->length] = '\0';

        /* Add space if completing a file/command */
        struct stat st;
        if (stat(completion, &st) != 0 || !S_ISDIR(st.st_mode)) {
            if (ls->length + 1 < ls->buffer_size) {
                memmove(ls->buffer + ls->cursor + 1,
                        ls->buffer + ls->cursor,
                        ls->length - ls->cursor);
                ls->buffer[ls->cursor] = ' ';
                ls->cursor++;
                ls->length++;
                ls->buffer[ls->length] = '\0';
            }
        }

        refresh_line(ls);
        ls->last_was_tab = 0;
    } else {
        /* Multiple completions */
        if (ls->last_was_tab) {
            /* Second tab - show completions */

            /* Move to beginning of line and clear */
            printf("\r" CLEAR_TO_EOL);

            /* Count completions */
            size_t count = 0;
            while (completions[count + 1]) count++;

            /* Check if too many */
            if (count > 100) {
                printf("Display all %zu possibilities? (y or n) ", count);
                fflush(stdout);

                char response;
                if (read(STDIN_FILENO, &response, 1) != 1 || (response != 'y' && response != 'Y')) {
                    printf("\n");
                    refresh_line(ls);
                    ls->last_was_tab = 0;
                    return;
                }
                printf("\n");
            }

            /* Find maximum length for display */
            size_t max_len = 0;
            for (size_t i = 1; completions[i]; i++) {
                size_t len = utf8_strlen(completions[i]);
                if (len > max_len) max_len = len;
            }

            /* Calculate how many columns we can fit */
            int term_width = get_terminal_width();
            size_t col_width = max_len + 2;  /* Add 2 for spacing */
            int cols = term_width / (int)col_width;
            if (cols < 1) cols = 1;

            /* Display completions in columns */
            size_t col = 0;
            for (size_t i = 1; completions[i]; i++) {
                const size_t display_len = utf8_strlen(completions[i]);
                printf("%s", completions[i]);

                /* Add padding */
                for (size_t pad = display_len; pad < col_width; pad++) {
                    printf(" ");
                }

                col++;
                if (col >= (size_t)cols) {
                    printf("\n");
                    col = 0;
                }
            }
            if (col > 0) printf("\n");

            /* Redraw the prompt and current line */
            refresh_line(ls);
        } else {
            /* First tab - complete common prefix */
            const char* common = completions[0];
            const size_t common_len = strlen(common);

            if (common_len > text_len) {
                /* There's a common prefix to ad */
                size_t add_len = common_len - text_len;
                const size_t new_len = ls->length + add_len;

                if (new_len >= ls->buffer_size) {
                    ls->buffer_size = new_len + 256;
                    ls->buffer = GC_REALLOC(ls->buffer, ls->buffer_size);
                }

                /* Move rest of buffer */
                memmove(ls->buffer + ls->cursor + add_len,
                        ls->buffer + ls->cursor,
                        ls->length - ls->cursor);

                /* Insert additional characters */
                memcpy(ls->buffer + ls->cursor, common + text_len, add_len);

                ls->length = new_len;
                ls->cursor += add_len;
                ls->buffer[ls->length] = '\0';

                refresh_line(ls);
            }
        }
        ls->last_was_tab = 1;
    }
}

/* Main readline function */
char* readline(const char* prompt) {
    if (!isatty(STDIN_FILENO)) {
        /* Not a TTY - just read a line normally */
        char* line = nullptr;
        size_t len = 0;
        ssize_t read = getline(&line, &len, stdin);
        if (read == -1) {
            free(line);
            return nullptr;
        }
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        char* result = GC_strdup(line);
        free(line);
        return result;
    }

    /* Initialize line state */
    LineState ls = {0};
    ls.buffer_size = 256;
    ls.buffer = malloc(ls.buffer_size);
    ls.buffer[0] = '\0';
    ls.prompt = prompt ? prompt : "";
    ls.prompt_len = strlen(ls.prompt);
    history.current = history.count;

    /* Setup signal handling */
    struct sigaction sa, old_sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, &old_sa);
    got_interrupt = 0;

    /* Enable raw mode */
    if (enable_raw_mode() == -1) {
        return nullptr;
    }

    /* Print prompt */
    printf("%s", prompt);
    fflush(stdout);

    /* Main input loop */
    while (1) {
        unsigned char c;
        int nread = (int)read(STDIN_FILENO, &c, 1);

        if (nread <= 0) {
            if (errno == EINTR && got_interrupt) {
                /* Ctrl-C pressed */
                printf("\r");  /* Reset to left margin */
                printf("^C\n");
                disable_raw_mode();
                sigaction(SIGINT, &old_sa, nullptr);
                return GC_strdup("");
            }
            continue;
        }

        /* Reset tab state if not tab */
        if (c != TAB) {
            ls.last_was_tab = 0;
        }

        /* Handle special keys */
        switch (c) {
            case ENTER:
                printf("\r");  /* Reset to left margin */
                printf("\n");
                disable_raw_mode();
                if (ls.length > 0) {
                    add_history_entry(ls.buffer);
                }
                sigaction(SIGINT, &old_sa, nullptr);
                return ls.buffer;

            case CTRL_C:
                printf("\r");  /* Reset to left margin */
                printf("^C\n");
                disable_raw_mode();
                sigaction(SIGINT, &old_sa, nullptr);
                return GC_strdup("");

            case CTRL_D:
                if (ls.length == 0) {
                    printf("\r");  /* Reset to left margin */
                    printf("\n");
                    disable_raw_mode();
                    sigaction(SIGINT, &old_sa, nullptr);
                    return nullptr;
                }
                break;

            case CTRL_G:
                printf("\r");  /* Reset to left margin */
                printf("^G\n");
                disable_raw_mode();
                sigaction(SIGINT, &old_sa, nullptr);
                return GC_strdup("");

            case BACKSPACE:
            case CTRL_H:
                if (ls.cursor > 0) {
                    size_t prev = utf8_prev_char(ls.buffer, ls.cursor);
                    size_t char_len = ls.cursor - prev;
                    memmove(ls.buffer + prev,
                            ls.buffer + ls.cursor,
                            ls.length - ls.cursor + 1);
                    ls.cursor = prev;
                    ls.length -= char_len;
                    refresh_line(&ls);
                }
                break;

            case CTRL_U:
                /* Delete to beginning of line */
                memmove(ls.buffer, ls.buffer + ls.cursor, ls.length - ls.cursor + 1);
                ls.length -= ls.cursor;
                ls.cursor = 0;
                refresh_line(&ls);
                break;

            case CTRL_K:
                /* Delete to end of line */
                ls.buffer[ls.cursor] = '\0';
                ls.length = ls.cursor;
                refresh_line(&ls);
                break;

            case CTRL_A:
                /* Move to beginning */
                ls.cursor = 0;
                refresh_line(&ls);
                break;

            case CTRL_E:
                /* Move to end */
                ls.cursor = ls.length;
                refresh_line(&ls);
                break;

            case TAB:
                handle_completion(&ls);
                break;

            case ESC_CODE: {
                /* Handle escape sequences */
                unsigned char seq[3];
                if (read(STDIN_FILENO, &seq[0], 1) != 1) break;
                if (seq[0] != '[') break;
                if (read(STDIN_FILENO, &seq[1], 1) != 1) break;

                if (seq[1] >= '0' && seq[1] <= '9') {
                    /* Extended escape sequence */
                    if (read(STDIN_FILENO, &seq[2], 1) != 1) break;
                    if (seq[1] == '3' && seq[2] == '~') {
                        /* Delete key */
                        if (ls.cursor < ls.length) {
                            size_t next = utf8_next_char(ls.buffer, ls.cursor, ls.length);
                            size_t char_len = next - ls.cursor;
                            memmove(ls.buffer + ls.cursor,
                                    ls.buffer + next,
                                    ls.length - next + 1);
                            ls.length -= char_len;
                            refresh_line(&ls);
                        }
                    }
                } else {
                    switch (seq[1]) {
                        case 'A': /* Up arrow */
                            if (history.current > 0) {
                                history.current--;
                                size_t hist_len = strlen(history.entries[history.current]);
                                if (hist_len >= ls.buffer_size) {
                                    ls.buffer_size = hist_len + 256;
                                    ls.buffer = GC_REALLOC(ls.buffer, ls.buffer_size);
                                }
                                strcpy(ls.buffer, history.entries[history.current]);
                                ls.length = ls.cursor = hist_len;
                                refresh_line(&ls);
                            }
                            break;

                        case 'B': /* Down arrow */
                            if (history.current < history.count) {
                                history.current++;
                                if (history.current == history.count) {
                                    ls.buffer[0] = '\0';
                                    ls.length = ls.cursor = 0;
                                } else {
                                    size_t hist_len = strlen(history.entries[history.current]);
                                    if (hist_len >= ls.buffer_size) {
                                        ls.buffer_size = hist_len + 256;
                                        ls.buffer = GC_REALLOC(ls.buffer, ls.buffer_size);
                                    }
                                    strcpy(ls.buffer, history.entries[history.current]);
                                    ls.length = ls.cursor = hist_len;
                                }
                                refresh_line(&ls);
                            }
                            break;

                        case 'C': /* Right arrow */
                            if (ls.cursor < ls.length) {
                                ls.cursor = utf8_next_char(ls.buffer, ls.cursor, ls.length);
                                refresh_line(&ls);
                            }
                            break;

                        case 'D': /* Left arrow */
                            if (ls.cursor > 0) {
                                ls.cursor = utf8_prev_char(ls.buffer, ls.cursor);
                                refresh_line(&ls);
                            }
                            break;

                        case 'H': /* Home */
                            ls.cursor = 0;
                            refresh_line(&ls);
                            break;

                        case 'F': /* End */
                            ls.cursor = ls.length;
                            refresh_line(&ls);
                            break;
                        default: ;
                    }
                }
                break;
            }

            default:
                if (c >= 32) {  /* Printable character */
                    /* Check if we need to handle UTF-8 */
                    unsigned char utf8_buf[5] = {c, 0, 0, 0, 0};
                    size_t char_len = utf8_char_len(c);

                    /* Read rest of UTF-8 character if needed */
                    for (size_t i = 1; i < char_len; i++) {
                        if (read(STDIN_FILENO, &utf8_buf[i], 1) != 1) {
                            break;
                        }
                    }

                    /* Check buffer size */
                    if (ls.length + char_len >= ls.buffer_size) {
                        ls.buffer_size = ls.length + char_len + 256;
                        ls.buffer = GC_REALLOC(ls.buffer, ls.buffer_size);
                    }

                    /* Insert character(s) */
                    if (ls.cursor < ls.length) {
                        memmove(ls.buffer + ls.cursor + char_len,
                                ls.buffer + ls.cursor,
                                ls.length - ls.cursor);
                    }
                    memcpy(ls.buffer + ls.cursor, utf8_buf, char_len);
                    ls.cursor += char_len;
                    ls.length += char_len;
                    ls.buffer[ls.length] = '\0';
                    refresh_line(&ls);
                }
                break;
        }
    }
}
