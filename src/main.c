/*
 * 'src/main.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025  Darren Kirby <darren@dragonbyte.ca>
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

#include "compat_readline.h"
#include "main.h"
#include "parser.h"
#include "config.h"
#include "repl.h"
#include "runner.h"

#include <gc/gc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>


/* Initialize load_libs struct to zeros. */
lib_load_config load_libs = {0};

/* 'Global' argc and argv for use by (command-line). */
int g_argc;
char** g_argv;

static void show_help(void)
{
    printf("Usage: cozenage [option ...] [file] \n\n\
A Scheme-derived REPL and code runner\n\n\
Options:\n\
    -l, --library\t preload Cozenage libraries at startup\n\
    -h, --help\t\t display this help\n\
    -V, --version\t display version information\n\n\
\n\
    '-l' and '--library' accept a required comma-delimited list of\n\
    libraries to pre-load. Accepted values are:\n\
    'bits' 'cxr' 'file' 'math' 'random' 'system' and 'time' \n\n\
Report bugs to <darren@dragonbyte.ca>\n");
}


static void process_library_arg(struct lib_load *l, const char *arg)
{
    char *arg_copy = GC_strdup(arg);
    if (!arg_copy) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    char *token = strtok(arg_copy, ",");

    while (token != NULL) {

        if (strcmp(token, "cxr") == 0) {
            l->cxr = 1;
        } else if (strcmp(token, "file") == 0) {
            l->file = 1;
        } else if (strcmp(token, "time") == 0) {
            l->time = 1;
        } else if (strcmp(token, "math") == 0) {
            l->math = 1;
        } else if (strcmp(token, "system") == 0) {
            l->system = 1;
        } else if (strcmp(token, "bits") == 0) {
            l->bits = 1;
        } else if (strcmp(token, "random") == 0) {
            l->random = 1;
        } else {
            fprintf(stderr, "Error: Unknown library name '%s' specified.\n", token);
            fprintf(stderr, "Run with -h for a list of valid library names.\n");
            exit(EXIT_FAILURE);
        }
        /* Get the next token */
        token = strtok(nullptr, ",");
    }
}


int main(const int argc, char** argv)
{
    /* GC docs say this probably isn't necessary,
     * but to do it to be portable with older versions. */
    GC_INIT();

    const struct option long_opts[] = {
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'V'},
        {"r5rs", no_argument, nullptr, '5'},
        {"library", required_argument, nullptr, 'l'},
        {nullptr,0,nullptr,0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "Vh5l:", long_opts, nullptr)) != -1) {
        switch(opt) {
            case 'V':
                printf("%s%s%s version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
                printf(" Compiled on %s at %s\n", __DATE__, __TIME__);
                exit(EXIT_SUCCESS);
            case 'h':
                show_help();
                exit(EXIT_SUCCESS);
                /* TODO: implement r5rs mode, maybe. */
            case '5':
                printf("--r5rs not implemented yet\n\n");
                break;
            case 'l':
                process_library_arg(&load_libs, optarg);
                break;
            default:
                ;
        }
    }

    const int non_option_args = argc - optind;

    if (non_option_args > 0) {
        /* Grab the number of args, and the args themselves starting from the file arg
         * to construct (command-line) later if needed. */
        if (non_option_args > 1) {
            g_argc = argc - optind;
            g_argv = argv + optind;
        }
        /* File-Runner Mode */
        const char *file_path = argv[optind];

        /* Pass the file path (argv[optind]) and the load_libs struct. */
        return run_file_script(file_path, load_libs);
    }

    /* REPL Mode (no non-option arguments were provided). */
    run_repl(load_libs);
    return EXIT_SUCCESS;
}
