#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <gc/gc.h>

#include "buffer.h"


static void setup(void) {
    GC_INIT();
}

Test(string_builtins, test_buffer_new, .init = setup) {
    str_buf_t *sb = sb_new();
    cr_assert_not_null(sb);
    cr_assert_not_null(sb->buffer);
    cr_assert_eq('\0', sb->buffer[0]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 0);
}

Test(string_builtins, test_sb_append_char, .init = setup) {
    // Allocate empty buffer
    str_buf_t *sb = sb_new();
    cr_assert_not_null(sb);
    cr_assert_not_null(sb->buffer);
    cr_assert_eq('\0', sb->buffer[0]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 0);

    // Add a char
    sb_append_char(sb, 'A');
    cr_assert_eq('A', sb->buffer[0]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 1);

    // And another
    sb_append_char(sb, 'B');
    cr_assert_eq('A', sb->buffer[0]);
    cr_assert_eq('B', sb->buffer[1]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 2);

    // And another
    sb_append_char(sb, 67);
    cr_assert_eq('A', sb->buffer[0]);
    cr_assert_eq('B', sb->buffer[1]);
    cr_assert_eq('C', sb->buffer[2]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 3);
}


Test(string_builtins, test_sb_append_string, .init = setup) {
    // Allocate empty buffer
    str_buf_t *sb = sb_new();
    cr_assert_not_null(sb);
    cr_assert_not_null(sb->buffer);
    cr_assert_eq('\0', sb->buffer[0]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 0);

    // Append a string
    sb_append_str(sb, "Hello, World!");
    cr_assert_str_eq(sb->buffer, "Hello, World!");
    cr_assert_eq('\0', sb->buffer[13]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 13);

    // Append another
    sb_append_str(sb, " Bonjour, le Monde!");
    cr_assert_str_eq(sb->buffer, "Hello, World! Bonjour, le Monde!");
    cr_assert_eq('\0', sb->buffer[32]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 32);
}

Test(string_builtins, test_sb_append_data, .init = setup) {
    // Allocate empty buffer
    str_buf_t *sb = sb_new();
    cr_assert_not_null(sb);
    cr_assert_not_null(sb->buffer);
    cr_assert_eq('\0', sb->buffer[0]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 0);

    // Append an array of bytes
    int8_t greeting[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21};
    sb_append_data(sb, greeting, sizeof(greeting));
    cr_assert_str_eq(sb->buffer, "Hello, World!");
    cr_assert_eq('\0', sb->buffer[13]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 13);

    // Append another
    int8_t greeting2[] = {32, 66, 111, 110, 106, 111, 117, 114, 44, 32, 108, 101, 32, 77, 111, 110, 100, 101, 33};
    sb_append_data(sb, greeting2, sizeof(greeting2));
    cr_assert_str_eq(sb->buffer, "Hello, World! Bonjour, le Monde!");
    cr_assert_eq('\0', sb->buffer[32]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 32);
}

Test(string_builtins, test_sb_append_fmt, .init = setup) {
    // Allocate empty buffer
    str_buf_t *sb = sb_new();
    cr_assert_not_null(sb);
    cr_assert_not_null(sb->buffer);
    cr_assert_eq('\0', sb->buffer[0]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 0);

    // Append a format string
    sb_append_fmt(sb, "One: %d, two: %d, %s %d", 1, 2, "three:", 3);
    cr_assert_str_eq(sb->buffer, "One: 1, two: 2, three: 3");
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 24);
}

Test(string_builtins, test_sb_ensure_capacity, .init = setup) {
    // Allocate empty buffer
    str_buf_t *sb = sb_new();
    cr_assert_not_null(sb);
    cr_assert_not_null(sb->buffer);
    cr_assert_eq('\0', sb->buffer[0]);
    cr_assert_eq(sb->capacity, 256);
    cr_assert_eq(sb->length, 0);

    // Append a long string, forcing a buffer realloc
    const char* exact_257 =
    "12345678901234567890123456789012345678901234567890" // 50
    "12345678901234567890123456789012345678901234567890" // 100
    "12345678901234567890123456789012345678901234567890" // 150
    "12345678901234567890123456789012345678901234567890" // 200
    "12345678901234567890123456789012345678901234567890" // 250
    "1234567";

    sb_append_str(sb, exact_257);
    // cap should have doubled
    cr_assert_eq(sb->capacity, 512);
    cr_assert_eq(sb->length, 257);
}



