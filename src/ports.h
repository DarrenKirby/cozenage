/*
 * 'ports.h'
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

#ifndef COZENAGE_PORTS_H
#define COZENAGE_PORTS_H

#include "cell.h"

/* Input/output and ports */
Cell* builtin_current_input_port(const Lex* e, const Cell* a);
Cell* builtin_current_output_port(const Lex* e, const Cell* a);
Cell* builtin_current_error_port(const Lex* e, const Cell* a);
Cell* builtin_input_port_pred(const Lex* e, const Cell* a);
Cell* builtin_output_port_pred(const Lex* e, const Cell* a);
Cell* builtin_text_port_pred(const Lex* e, const Cell* a);
Cell* builtin_binary_port_pred(const Lex* e, const Cell* a);
Cell* builtin_input_port_open(const Lex* e, const Cell* a);
Cell* builtin_output_port_open(const Lex* e, const Cell* a);
Cell* builtin_close_port(const Lex* e, const Cell* a);
Cell* builtin_read_line(const Lex* e, const Cell* a);
Cell* builtin_read_char(const Lex* e, const Cell* a);
Cell* builtin_read_string(const Lex* e, const Cell* a);
Cell* builtin_peek_char(const Lex* e, const Cell* a);
Cell* builtin_char_ready(const Lex* e, const Cell* a);
Cell* builtin_u8_ready(const Lex* e, const Cell* a);
Cell* builtin_write_char(const Lex* e, const Cell* a);
Cell* builtin_write_string(const Lex* e, const Cell* a);
Cell* builtin_write_u8(const Lex* e, const Cell* a);
Cell* builtin_write_bytevector(const Lex* e, const Cell* a);
Cell* builtin_newline(const Lex* e, const Cell* a);
Cell* builtin_eof(const Lex* e, const Cell* a);
Cell* builtin_read_error(const Lex* e, const Cell* a);
Cell* builtin_file_error(const Lex* e, const Cell* a);
Cell* builtin_flush_output_port(const Lex* e, const Cell* a);

#endif //COZENAGE_PORTS_H
