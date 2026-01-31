#include "test_meta.h"
#include <criterion/criterion.h>


TestSuite(end_to_end_strings);

Test(end_to_end_strings, test_string, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII Construction */
    cr_assert_str_eq(t_eval("(string #\\a #\\b #\\c)"), "\"abc\"");

    /* 2. Pure UTF-8 Construction (Multiple Byte Widths) */
    /* mu (2b), alpha (2b), omega (2b), beta (2b) */
    cr_assert_str_eq(t_eval("(string #\\mu #\\alpha #\\omega #\\beta)"), "\"μαωβ\"");
    /* euro (3b) */
    cr_assert_str_eq(t_eval("(string #\\euro #\\euro)"), "\"€€\"");

    /* 3. Mixed Width Construction */
    /* 'A' (1b), 'λ' (2b), '€' (3b) */
    cr_assert_str_eq(t_eval("(string #\\A #\\lambda #\\euro)"), "\"Aλ€\"");

    /* 4. Corner Case: Empty String */
    cr_assert_str_eq(t_eval("(string)"), "\"\"");

    /* 5. Edge Case: Non-printable/Control Characters */
    /* Space (1b), Tab (1b), and a UTF-8 char */
    cr_assert_str_eq(t_eval("(string #\\space #\\tab #\\pi)"), "\" \\tπ\"");

    /* 6. Metadata Check: string-length vs. byte count */
    /* Construction should result in char_count = 3, though bytes = 6 */
    cr_assert_str_eq(t_eval("(number->string (string-length (string #\\mu #\\alpha #\\omega)))"), "\"3\"");

    /* 7. Edge Case: Multiple instances of the same multibyte char */
    cr_assert_str_eq(t_eval("(string #\\pi #\\pi #\\pi)"), "\"πππ\"");
}


Test(end_to_end_strings, test_string_length, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII Literal */
    cr_assert_str_eq(t_eval("(number->string (string-length \"abc\"))"), "\"3\"");

    /* 2. UTF-8 Literal (Mixed widths) */
    /* "λ" (2b), "€" (3b), "A" (1b) = 3 chars, 6 bytes */
    cr_assert_str_eq(t_eval("(number->string (string-length \"λ€A\"))"), "\"3\"");

    /* 3. Empty String */
    cr_assert_str_eq(t_eval("(number->string (string-length \"\"))"), "\"0\"");

    /* 4. Constructed via (string ...) */
    cr_assert_str_eq(t_eval("(number->string (string-length (string #\\mu #\\space #\\euro)))"), "\"3\"");

    /* 5. Long string with Multi-byte chars */
    cr_assert_str_eq(t_eval("(number->string (string-length \"ππππππππππ\"))"), "\"10\"");

    /* 6. Result of string-append */
    cr_assert_str_eq(t_eval("(begin (define s (string-append \"α\" \"β\" \"γ\")) (number->string (string-length s)))"), "\"3\"");

    /* 7. Result of substring */
    /* Take 2 chars out of a 3-char UTF-8 string */
    cr_assert_str_eq(t_eval("(begin (define s (substring \"μαω\" 0 2)) (number->string (string-length s)))"), "\"2\"");
}


Test(end_to_end_strings, test_string_equals, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic Arity: 0 and 1 arguments (R7RS requirements) */
    cr_assert_str_eq(t_eval("(string=?)"), "#true");
    cr_assert_str_eq(t_eval("(string=? \"foo\")"), "#true");

    /* 2. Simple ASCII Equality and Inequality */
    cr_assert_str_eq(t_eval("(string=? \"abc\" \"abc\")"), "#true");
    cr_assert_str_eq(t_eval("(string=? \"abc\" \"abd\")"), "#false");

    /* 3. UTF-8 Equality (Same Codepoints) */
    cr_assert_str_eq(t_eval("(string=? \"μαω\" \"μαω\")"), "#true");
    cr_assert_str_eq(t_eval("(string=? \"€€\" \"€€\")"), "#true");

    /* 4. Length Short-Circuiting (Fast Path) */
    /* "α" is 2 bytes, "a" is 1 byte. Should fail immediately on length. */
    cr_assert_str_eq(t_eval("(string=? \"α\" \"a\")"), "#false");
    /* "abc" vs "abcd" */
    cr_assert_str_eq(t_eval("(string=? \"abc\" \"abcd\")"), "#false");

    /* 5. Variadic Arity (3+ arguments) */
    cr_assert_str_eq(t_eval("(string=? \"apple\" \"apple\" \"apple\")"), "#true");
    cr_assert_str_eq(t_eval("(string=? \"apple\" \"apple\" \"orange\")"), "#false");

    /* 6. Mixed Construction Equality */
    /* Literal vs. (string ...) vs. (make-string ...) */
    cr_assert_str_eq(t_eval("(string=? \"ππ\" (string #\\pi #\\pi) (make-string 2 #\\pi))"), "#true");

    /* 7. Case Sensitivity (Standard string=? is case-sensitive) */
    cr_assert_str_eq(t_eval("(string=? \"Alpha\" \"alpha\")"), "#false");
    cr_assert_str_eq(t_eval("(string=? \"Π\" \"π\")"), "#false");
}


Test(end_to_end_strings, test_string_comparison, .init = setup_each_test, .fini = teardown_each_test) {
    /* --- string<? and string<=? --- */

    /* 1. Basic ASCII Lexicographical Order */
    cr_assert_str_eq(t_eval("(string<? \"a\" \"b\")"), "#true");
    cr_assert_str_eq(t_eval("(string<? \"abc\" \"abd\")"), "#true");
    cr_assert_str_eq(t_eval("(string<? \"abc\" \"abc\")"), "#false");
    cr_assert_str_eq(t_eval("(string<=? \"abc\" \"abc\")"), "#true");

    /* 2. UTF-8 Codepoint Ordering */
    /* Alpha (U+03B1) comes before Omega (U+03C9) */
    cr_assert_str_eq(t_eval("(string<? \"α\" \"ω\")"), "#true");
    /* ASCII 'z' (U+007A) comes before any non-ASCII char like 'α' (U+03B1) */
    cr_assert_str_eq(t_eval("(string<? \"z\" \"α\")"), "#true");

    /* 3. Prefix comparison (Shorter string is 'less') */
    cr_assert_str_eq(t_eval("(string<? \"abc\" \"abcd\")"), "#true");
    cr_assert_str_eq(t_eval("(string<=? \"abc\" \"abc\")"), "#true");

    /* 4. Variadic Arity */
    cr_assert_str_eq(t_eval("(string<? \"a\" \"b\" \"c\")"), "#true");
    cr_assert_str_eq(t_eval("(string<? \"a\" \"c\" \"b\")"), "#false");

    /* --- string>? and string>=? --- */

    /* 5. Basic ASCII Lexicographical Order */
    cr_assert_str_eq(t_eval("(string>? \"b\" \"a\")"), "#true");
    cr_assert_str_eq(t_eval("(string>? \"abc\" \"abc\")"), "#false");
    cr_assert_str_eq(t_eval("(string>=? \"abc\" \"abc\")"), "#true");

    /* 6. UTF-8 Codepoint Ordering */
    /* Omega (U+03C9) is greater than Alpha (U+03B1) */
    cr_assert_str_eq(t_eval("(string>? \"ω\" \"α\")"), "#true");
    /* Euro (U+20AC) is greater than Dollar (U+0024) */
    cr_assert_str_eq(t_eval("(string>? \"€\" \"$\")"), "#true");

    /* 7. Variadic Arity */
    cr_assert_str_eq(t_eval("(string>? \"z\" \"y\" \"x\")"), "#true");
    cr_assert_str_eq(t_eval("(string>=? \"z\" \"z\" \"a\")"), "#true");

    /* 8. Arity 0 and 1 (R7RS requires #true) */
    cr_assert_str_eq(t_eval("(string<?)"), "#true");
    cr_assert_str_eq(t_eval("(string>?)"), "#true");
    cr_assert_str_eq(t_eval("(string<? \"anything\")"), "#true");
}


Test(end_to_end_strings, test_string_append, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII Append */
    cr_assert_str_eq(t_eval("(string-append \"foo\" \"bar\")"), "\"foobar\"");

    /* 2. UTF-8 Append (Mixed widths) */
    /* "μ" (2b) + "€" (3b) + "!" (1b) */
    cr_assert_str_eq(t_eval("(string-append \"μ\" \"€\" \"!\")"), "\"μ€!\"");

    /* 3. Empty String Edge Cases (R7RS) */
    cr_assert_str_eq(t_eval("(string-append)"), "\"\"");
    cr_assert_str_eq(t_eval("(string-append \"\")"), "\"\"");
    cr_assert_str_eq(t_eval("(string-append \"foo\" \"\" \"bar\")"), "\"foobar\"");

    /* 4. Single String (Newly allocated copy) */
    cr_assert_str_eq(t_eval("(begin (define s \"original\") (define a (string-append s)) (string-set! a 0 #\\O) s)"), "\"original\"");
    cr_assert_str_eq(t_eval("(begin (define s \"original\") (define a (string-append s)) a)"), "\"original\"");

    /* 5. Metadata Propagation: string-length */
    /* α (2b) + β (2b) + γ (2b) = 3 chars, 6 bytes */
    cr_assert_str_eq(t_eval("(number->string (string-length (string-append \"α\" \"β\" \"γ\")))"), "\"3\"");

    /* 6. Metadata Propagation: ascii flag */
    /* Appending a UTF-8 string to an ASCII string should result in a non-ascii string */
    cr_assert_str_eq(t_eval("(begin (define s (string-append \"abc\" \"ω\")) (if (string=? s \"abcω\") \"ok\" \"fail\"))"), "\"ok\"");

    /* 7. Large number of arguments */
    cr_assert_str_eq(t_eval("(string-append \"a\" \"b\" \"c\" \"d\" \"e\" \"f\")"), "\"abcdef\"");
}


Test(end_to_end_strings, test_string_ref, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. ASCII Fast Path */
    cr_assert_str_eq(t_eval("(string-ref \"abc\" 0)"), "#\\a");
    cr_assert_str_eq(t_eval("(string-ref \"abc\" 2)"), "#\\c");

    /* 2. UTF-8 Mid-string access */
    /* "λ" (2b), "€" (3b), "A" (1b) */
    cr_assert_str_eq(t_eval("(string-ref \"λ€A\" 0)"), "#\\λ");
    cr_assert_str_eq(t_eval("(string-ref \"λ€A\" 1)"), "#\\€");
    cr_assert_str_eq(t_eval("(string-ref \"λ€A\" 2)"), "#\\A");

    /* 3. Accessing the end of a UTF-8 string */
    /* Ensure the scan doesn't stop early or overrun */
    cr_assert_str_eq(t_eval("(string-ref \"μαω\" 2)"), "#\\ω");

    /* 4. Mixed ASCII and Multi-byte */
    cr_assert_str_eq(t_eval("(string-ref \"123π56\" 3)"), "#\\π");

    /* 5. Edge Case: Result of string-append */
    cr_assert_str_eq(t_eval("(begin (define s (string-append \"α\" \"β\")) (string-ref s 1))"), "#\\β");

    /* 6. High-order Unicode (4-byte chars if supported) */
    /* Testing a character outside the Basic Multilingual Plane if your map/parser supports it,
       otherwise another 2-byte or 3-byte char is fine. */
    cr_assert_str_eq(t_eval("(string-ref \"§\" 0)"), "#\\§");
}


Test(end_to_end_strings, test_make_string, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. ASCII Fill (memset fast path) */
    cr_assert_str_eq(t_eval("(make-string 3 #\\a)"), "\"aaa\"");

    /* 2. Default Fill (should be Space U+0020) */
    cr_assert_str_eq(t_eval("(make-string 2)"), "\"  \"");

    /* 3. UTF-8 Fill (2-byte character) */
    /* Checks if byte_idx correctly jumps by char_len (2) each iteration */
    cr_assert_str_eq(t_eval("(make-string 3 #\\π)"), "\"πππ\"");

    /* 4. UTF-8 Fill (3-byte character) */
    /* Checks if byte_idx correctly jumps by char_len (3) each iteration */
    cr_assert_str_eq(t_eval("(make-string 2 #\\€)"), "\"€€\"");

    /* 5. Zero Length */
    cr_assert_str_eq(t_eval("(make-string 0 #\\π)"), "\"\"");

    /* 6. Metadata Verification: Length */
    cr_assert_str_eq(t_eval("(number->string (string-length (make-string 5 #\\λ)))"), "\"5\"");

    /* 7. Metadata Verification: Mutability & ASCII status */
    /* If we create a 10-char ASCII string, we should be able to ref it in O(1) */
    cr_assert_str_eq(t_eval("(begin (define s (make-string 10 #\\A)) (string-ref s 9))"), "#\\A");
}


Test(end_to_end_strings, test_string_to_list, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII Conversion */
    cr_assert_str_eq(t_eval("(string->list \"abc\")"), "(#\\a #\\b #\\c)");

    /* 2. UTF-8 Conversion (Mixed Widths) */
    /* π (2b), € (3b), Z (1b) */
    cr_assert_str_eq(t_eval("(string->list \"π€Z\")"), "(#\\π #\\€ #\\Z)");

    /* 3. With Start Index */
    /* Skip the first 2 characters of "μαωβ" */
    cr_assert_str_eq(t_eval("(string->list \"μαωβ\" 2)"), "(#\\ω #\\β)");

    /* 4. With Start and End Indices */
    /* Extract middle character '€' from "λ€A" */
    cr_assert_str_eq(t_eval("(string->list \"λ€A\" 1 2)"), "(#\\€)");

    /* 5. Edge Case: Start equals End (Empty List) */
    cr_assert_str_eq(t_eval("(string->list \"μαω\" 1 1)"), "()");

    /* 6. Edge Case: Full indices on UTF-8 string */
    cr_assert_str_eq(t_eval("(string->list \"μαω\" 0 3)"), "(#\\μ #\\α #\\ω)");

    /* 7. Empty String */
    cr_assert_str_eq(t_eval("(string->list \"\")"), "()");
}


Test(end_to_end_strings, test_list_to_string, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII List */
    cr_assert_str_eq(t_eval("(list->string '(#\\a #\\b #\\c))"), "\"abc\"");

    /* 2. Pure UTF-8 List (Mixed Widths) */
    /* π (2b), € (3b), A (1b) */
    cr_assert_str_eq(t_eval("(list->string '(#\\π #\\€ #\\A))"), "\"π€A\"");

    /* 3. Empty List */
    cr_assert_str_eq(t_eval("(list->string '())"), "\"\"");

    /* 4. Single Character List */
    cr_assert_str_eq(t_eval("(list->string '(#\\ω))"), "\"ω\"");

    /* 5. Result of (string->list) round-trip */
    cr_assert_str_eq(t_eval("(begin (define s \"μαω\") (list->string (string->list s)))"), "\"μαω\"");

    /* 6. Metadata Verification: Length and ASCII flag */
    /* Ensuring the reconstructed string correctly identifies as non-ASCII */
    cr_assert_str_eq(t_eval("(begin (define s (list->string '(#\\λ))) (number->string (string-length s)))"), "\"1\"");

    /* 7. Large list to string */
    cr_assert_str_eq(t_eval("(list->string (list #\\x #\\y #\\z #\\!))"), "\"xyz!\"");
}


Test(end_to_end_strings, test_substring, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII Substring */
    cr_assert_str_eq(t_eval("(substring \"hello\" 1 4)"), "\"ell\"");

    /* 2. UTF-8 Substring (Start/End on multibyte boundaries) */
    /* "μαωβ" -> "αω" */
    cr_assert_str_eq(t_eval("(substring \"μαωβ\" 1 3)"), "\"αω\"");

    /* 3. Extracting a single multibyte character */
    cr_assert_str_eq(t_eval("(substring \"λ€A\" 1 2)"), "\"€\"");

    /* 4. Substring that results in ASCII from a UTF-8 parent */
    /* This tests your is_pure_ascii() re-scan logic */
    cr_assert_str_eq(t_eval("(substring \"μαωABC\" 3 6)"), "\"ABC\"");

    /* 5. Edge Case: Empty Substring (start == end) */
    cr_assert_str_eq(t_eval("(substring \"μαω\" 1 1)"), "\"\"");

    /* 6. Edge Case: Full string slice */
    cr_assert_str_eq(t_eval("(substring \"πππ\" 0 3)"), "\"πππ\"");

    /* 7. Metadata Check: Character length vs Byte length */
    /* "€€€" is 9 bytes. Substring 1 to 2 is the middle "€" (3 bytes). */
    cr_assert_str_eq(t_eval("(begin (define s (substring \"€€€\" 1 2)) (number->string (string-length s)))"), "\"1\"");
}


Test(end_to_end_strings, test_string_copy, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Full Copy (The Shortcut Path) */
    cr_assert_str_eq(t_eval("(begin (define s1 \"μαω\") (define s2 (string-copy s1)) s2)"), "\"μαω\"");

    /* 2. Full Copy Mutability (Ensures it's a new allocation) */
    cr_assert_str_eq(t_eval("(begin (define s1 \"abc\") (define s2 (string-copy s1)) (string-set! s2 0 #\\z) s1)"), "\"abc\"");
    cr_assert_str_eq(t_eval("(begin (define s1 \"abc\") (define s2 (string-copy s1)) (string-set! s2 0 #\\z) s2)"), "\"zbc\"");

    /* 3. Sub-range Copy (1 index: start to end) */
    cr_assert_str_eq(t_eval("(string-copy \"μαωβ\" 2)"), "\"ωβ\"");

    /* 4. Sub-range Copy (2 indices: start and end) */
    cr_assert_str_eq(t_eval("(string-copy \"λ€A\" 1 2)"), "\"€\"");

    /* 5. Sub-range ASCII Upgrade */
    /* Copying the ASCII part of a UTF-8 string should result in an ASCII string */
    cr_assert_str_eq(t_eval("(string-copy \"μαωABC\" 3 6)"), "\"ABC\"");

    /* 6. Edge Case: Empty String Copy */
    cr_assert_str_eq(t_eval("(string-copy \"\")"), "\"\"");

    /* 7. Metadata Verification */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"πππ\" 0 2)) (number->string (string-length s)))"), "\"2\"");
}


Test(end_to_end_strings, test_string_copy_2, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Full Copy (The Shortcut Path) */
    cr_assert_str_eq(t_eval("(begin (define s1 \"μαω\") (define s2 (string-copy s1)) s2)"), "\"μαω\"");

    /* 2. Full Copy Mutability (Ensures it's a new allocation) */
    cr_assert_str_eq(t_eval("(begin (define s1 \"abc\") (define s2 (string-copy s1)) (string-set! s2 0 #\\z) s1)"), "\"abc\"");
    cr_assert_str_eq(t_eval("(begin (define s1 \"abc\") (define s2 (string-copy s1)) (string-set! s2 0 #\\z) s2)"), "\"zbc\"");

    /* 3. Sub-range Copy (1 index: start to end) */
    cr_assert_str_eq(t_eval("(string-copy \"μαωβ\" 2)"), "\"ωβ\"");

    /* 4. Sub-range Copy (2 indices: start and end) */
    cr_assert_str_eq(t_eval("(string-copy \"λ€A\" 1 2)"), "\"€\"");

    /* 5. Sub-range ASCII Upgrade */
    /* Copying the ASCII part of a UTF-8 string should result in an ASCII string */
    cr_assert_str_eq(t_eval("(string-copy \"μαωABC\" 3 6)"), "\"ABC\"");

    /* 6. Edge Case: Empty String Copy */
    cr_assert_str_eq(t_eval("(string-copy \"\")"), "\"\"");

    /* 7. Metadata Verification */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"πππ\" 0 2)) (number->string (string-length s)))"), "\"2\"");
}


Test(end_to_end_strings, test_string_copy_bang_paths, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. ASCII to ASCII: Fast Path / memmove */
    cr_assert_str_eq(t_eval("(begin (define s1 (string-copy \"hello world\")) (string-copy! s1 0 \"J\") s1)"), "\"Jello world\"");
    cr_assert_str_eq(t_eval("(begin (define s2 (string-copy \"hello\")) (string-copy! s2 1 s2 0 4) s2)"), "\"hhell\"");

    /* 2. UTF-8 to UTF-8: Same Byte Width Optimization (2-byte vs 2-byte) */
    /* Alpha (0xCE 0x91) replaced by Beta (0xCE 0x92) */
    cr_assert_str_eq(t_eval("(begin (define s3 (string-copy \"ααα\")) (string-copy! s3 1 \"β\") s3)"), "\"αβα\"");

    /* 3. UTF-8 to UTF-8: Same Byte Width Optimization (3-byte vs 3-byte) */
    /* Euro (0xE2 0x82 0xAC) replaced by Euro Symbol (0xE2 0x82 0xA0) */
    cr_assert_str_eq(t_eval("(begin (define s4 (string-copy \"€€€\")) (string-copy! s4 1 \"₠\") s4)"), "\"€₠€\"");

    /* 4. Reconstruction: ASCII (1b) -> UTF-8 (3b) */
    cr_assert_str_eq(t_eval("(begin (define s5 (string-copy \"abc\")) (string-copy! s5 1 \"€\") s5)"), "\"a€c\"");

    /* 5. Reconstruction: UTF-8 (3b) -> ASCII (1b) */
    cr_assert_str_eq(t_eval("(begin (define s6 (string-copy \"a€c\")) (string-copy! s6 1 \"z\") s6)"), "\"azc\"");

    /* 6. Multi-char mismatch reconstruction */
    /* Replace 'αω' (4 bytes) with '!!' (2 bytes) */
    cr_assert_str_eq(t_eval("(begin (define s7 (string-copy \"μαωβ\")) (string-copy! s7 1 \"!!\") s7)"), "\"μ!!β\"");

    /* 7. Self-Overlap with Width Mismatch (Reconstruction Stress Test) */
    cr_assert_str_eq(t_eval("(begin (define s8 (string-copy \"aβc\")) (string-copy! s8 0 s8 1 2) s8)"), "\"ββc\"");

    /* 8. Metadata verification */
    cr_assert_str_eq(t_eval("(begin (define s9 (string-copy \"a€c\")) (string-copy! s9 1 \"z\") (number->string (string-length s9)))"), "\"3\"");
}


Test(end_to_end_strings, test_string_set_bang, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. ASCII Fast Path (In-place) */
    cr_assert_str_eq(t_eval("(begin (define s \"abc\") (string-set! s 1 #\\z) s)"), "\"azc\"");

    /* 2. UTF-8 In-place (Same byte-width: 2-byte to 2-byte) */
    /* Replace Alpha (α) with Beta (β) */
    cr_assert_str_eq(t_eval("(begin (define s \"ααα\") (string-set! s 1 #\\β) s)"), "\"αβα\"");

    /* 3. UTF-8 Reconstruction: Expansion (1-byte to 3-byte) */
    /* Replace 'b' with Euro symbol '€' */
    cr_assert_str_eq(t_eval("(begin (define s \"abc\") (string-set! s 1 #\\€) s)"), "\"a€c\"");

    /* 4. UTF-8 Reconstruction: Contraction (3-byte to 1-byte) */
    /* Replace Euro symbol '€' with '!' */
    cr_assert_str_eq(t_eval("(begin (define s \"a€c\") (string-set! s 1 #\\!) s)"), "\"a!c\"");

    /* 5. Edge Case: Setting the very last character */
    cr_assert_str_eq(t_eval("(begin (define s \"μαω\") (string-set! s 2 #\\!) s)"), "\"μα!\"");

    /* 6. Edge Case: Setting the first character */
    cr_assert_str_eq(t_eval("(begin (define s \"μαω\") (string-set! s 0 #\\!) s)"), "\"!αω\"");

    /* 7. Metadata Check: string-length consistency */
    /* Ensure changing byte-width doesn't confuse the character count */
    cr_assert_str_eq(t_eval("(begin (define s \"abc\") (string-set! s 1 #\\€) (number->string (string-length s)))"), "\"3\"");
}


Test(end_to_end_strings, test_string_fill_bang, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII Fill (Full string) */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"abc\")) (string-fill! s #\\z) s)"), "\"zzz\"");

    /* 2. ASCII Fill (Sub-range) */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"abcdef\")) (string-fill! s #\\* 2 4) s)"), "\"ab**ef\"");

    /* 3. UTF-8 Fill (In-place optimization: same byte-width) */
    /* Replacing three 2-byte 'α' with three 2-byte 'β' */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"ααα\")) (string-fill! s #\\β) s)"), "\"βββ\"");

    /* 4. UTF-8 Fill (Expansion: 1-byte chars to 3-byte chars) */
    /* "abc" (3 bytes) filled with "€" (3 * 3 = 9 bytes) */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"abc\")) (string-fill! s #\\€) s)"), "\"€€€\"");

    /* 5. UTF-8 Fill (Contraction: 3-byte chars to 1-byte chars) */
    /* "€€€" (9 bytes) filled with "!" (3 bytes) */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"€€€\")) (string-fill! s #\\!) s)"), "\"!!!\"");

    /* 6. Partial Range UTF-8 Fill */
    /* Target "aβc" (4 bytes). Fill index 1 (the 'β') with '€' (3 bytes).
       Result should be "a€c" (5 bytes). */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"aβc\")) (string-fill! s #\\€ 1 2) s)"), "\"a€c\"");

    /* 7. Edge Case: Fill empty range */
    cr_assert_str_eq(t_eval("(begin (define s (string-copy \"μαω\")) (string-fill! s #\\! 1 1) s)"), "\"μαω\"");

    /* 8. Metadata Check: ASCII flag downgrade */
    /* Filling an ASCII string with a UTF-8 char should flip the flag */
    cr_assert_str_eq(t_eval("(begin (define s (make-string 3 #\\a)) (string-fill! s #\\π) s)"), "\"πππ\"");
}


Test(end_to_end_strings, test_string_to_number, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic Integer Parsing */
    cr_assert_str_eq(t_eval("(number->string (string->number \"123\"))"), "\"123\"");
    cr_assert_str_eq(t_eval("(number->string (string->number \"-45\"))"), "\"-45\"");

    /* 2. Radix Prefixes (R7RS) */
    cr_assert_str_eq(t_eval("(number->string (string->number \"#x10\"))"), "\"16\"");
    cr_assert_str_eq(t_eval("(number->string (string->number \"#b1010\"))"), "\"10\"");
    cr_assert_str_eq(t_eval("(number->string (string->number \"#o10\"))"), "\"8\"");

    /* 3. Optional Radix Argument */
    cr_assert_str_eq(t_eval("(number->string (string->number \"10\" 16))"), "\"16\"");
    cr_assert_str_eq(t_eval("(number->string (string->number \"1010\" 2))"), "\"10\"");

    /* 4. Inexact/Floating Point (if supported) */
    cr_assert_str_eq(t_eval("(number->string (string->number \"3.14\"))"), "\"3.14\"");
    cr_assert_str_eq(t_eval("(number->string (string->number \"1e3\"))"), "\"1000.0\"");

    /* 5. Exactness Prefixes */
    cr_assert_str_eq(t_eval("(number->string (string->number \"#e1.0\"))"), "\"1.0\"");

    /* 6. Edge Case: Invalid Strings (Should return #false) */
    cr_assert_str_eq(t_eval("(string->number \"apple\")"), "#false");
    cr_assert_str_eq(t_eval("(string->number \"12a3\")"), "#false");

    /* 7. Edge Case: Empty or Sign-only strings */
    cr_assert_str_eq(t_eval("(string->number \"\")"), "#false");
    cr_assert_str_eq(t_eval("(string->number \"+\")"), "#false");
}


Test(end_to_end_strings, test_number_to_string, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic Decimal Integers */
    cr_assert_str_eq(t_eval("(number->string 123)"), "\"123\"");
    cr_assert_str_eq(t_eval("(number->string -45)"), "\"-45\"");

    /* 2. Zero */
    cr_assert_str_eq(t_eval("(number->string 0)"), "\"0\"");

    /* 3. Optional Radix: Hexadecimal (16) */
    cr_assert_str_eq(t_eval("(number->string 255 16)"), "\"ff\"");

    /* Hex with negative value. */
    cr_assert_str_eq(t_eval("(number->string -16 16)"), "\"-10\"");

    /* 4. Optional Radix: Binary (2) */
    cr_assert_str_eq(t_eval("(number->string 10 2)"), "\"1010\"");

    /* 5. Optional Radix: Octal (8) */
    cr_assert_str_eq(t_eval("(number->string 64 8)"), "\"100\"");

    /* 6. Floating Point (if supported by your tower) */
    /* Note: formatting of floats can vary slightly between implementations. */
    cr_assert_str_eq(t_eval("(number->string 3.14)"), "\"3.14\"");

    /* 7. Large Numbers (Checking buffer limits) */
    /* If you support 64-bit integers, this tests the maximum digits. */
    cr_assert_str_eq(t_eval("(number->string 1000000)"), "\"1000000\"");
}


Test(end_to_end_strings, test_string_downcase, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII downcasing */
    cr_assert_str_eq(t_eval("(string-downcase \"HELLO WORLD\")"), "\"hello world\"");

    /* 2. Mixed case with non-alphabetic characters */
    cr_assert_str_eq(t_eval("(string-downcase \"123 ABC-#!?\")"), "\"123 abc-#!?\"");

    /* 3. Basic UTF-8 downcasing (Greek) */
    /* Lambda (Λ -> λ) */
    cr_assert_str_eq(t_eval("(string-downcase \"Λ\")"), "\"λ\"");

    /* 4. Contextual Sigma (The R7RS "Final Sigma" test) */
    /* a Sigma at the end of a word should become ς, otherwise σ. */
    cr_assert_str_eq(t_eval("(string-downcase \"Σ\")"), "\"σ\"");
    cr_assert_str_eq(t_eval("(string-downcase \"ΣΣ\")"), "\"σς\"");

    /* 5. Buffer expansion/contraction check */
    /* In some locales/mappings, case conversion can change the number of bytes. */
    cr_assert_str_eq(t_eval("(string-downcase \"ΠΥΘΑΓΟΡΑΣ\")"), "\"πυθαγορας\"");

    /* 6. Already lowercase */
    cr_assert_str_eq(t_eval("(string-copy \"μαω\")"), "\"μαω\"");

    /* 7. Metadata verification */
    cr_assert_str_eq(t_eval("(begin (define s (string-downcase \"Λ\")) (number->string (string-length s)))"), "\"1\"");
}


Test(end_to_end_strings, test_string_upcase, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII upcasing */
    cr_assert_str_eq(t_eval("(string-upcase \"hello world\")"), "\"HELLO WORLD\"");

    /* 2. Basic UTF-8 upcasing (Greek) */
    /* lambda (λ -> Λ) */
    cr_assert_str_eq(t_eval("(string-upcase \"λ\")"), "\"Λ\"");

    /* 3. The "Sigma" unification */
    /* Both σ and ς should upcase to Σ */
    cr_assert_str_eq(t_eval("(string-upcase \"σ\")"), "\"Σ\"");
    cr_assert_str_eq(t_eval("(string-upcase \"ς\")"), "\"Σ\"");
    cr_assert_str_eq(t_eval("(string-upcase \"σσς\")"), "\"ΣΣΣ\"");

    /* 4. German Eszett (The expansion test) */
    /* "ß" (U+00DF) upcases to "SS".
       Note: If your ICU setup is basic, this might remain "ß" or
       become a capital ẞ (U+1E9E). R7RS generally expects "SS". */
    cr_assert_str_eq(t_eval("(string-upcase \"straße\")"), "\"STRASSE\"");

    /* 5. Character count preservation (Standard) */
    cr_assert_str_eq(t_eval("(begin (define s (string-upcase \"μαω\")) (number->string (string-length s)))"), "\"3\"");

    /* 6. Already uppercase */
    cr_assert_str_eq(t_eval("(string-upcase \"ABC\")"), "\"ABC\"");

    /* 7. Mixing scripts and symbols */
    cr_assert_str_eq(t_eval("(string-upcase \"1a2β3!\")"), "\"1A2Β3!\"");
}


Test(end_to_end_strings, test_string_foldcase, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic ASCII folding (Equivalent to downcase for ASCII) */
    cr_assert_str_eq(t_eval("(string-foldcase \"Hello World\")"), "\"hello world\"");

    /* 2. The Greek Sigma Test (Crucial for Foldcase) */
    /* In foldcase, both σ (medial) and ς (final) must fold to the same thing
       (usually σ) so that "ΑΣ" and "ασ" are considered equal. */
    cr_assert_str_eq(t_eval("(string-foldcase \"Σ\")"), "\"σ\"");
    cr_assert_str_eq(t_eval("(string-foldcase \"ς\")"), "\"σ\"");
    cr_assert_str_eq(t_eval("(if (string=? (string-foldcase \"Σ\") (string-foldcase \"ς\")) #true #false)"), "#true");

    /* 3. The German Eszett (ß) */
    /* In Unicode Case Folding, "ß" often folds to "ss". */
    cr_assert_str_eq(t_eval("(string-foldcase \"ß\")"), "\"ss\"");

    /* 4. Multi-script consistency */
    cr_assert_str_eq(t_eval("(string-foldcase \"ΛλΠπ\")"), "\"λλππ\"");

    /* 5. Non-alphabetic characters (Remain unchanged) */
    cr_assert_str_eq(t_eval("(string-foldcase \"123 !@#\")"), "\"123 !@#\"");

    /* 6. Round-trip through ci=? */
    /* R7RS implies (string-ci=? s1 s2) is the same as (string=? (string-foldcase s1) (string-foldcase s2)) */
    cr_assert_str_eq(t_eval("(begin (define s \"Straße\") (if (string=? (string-foldcase s) \"strasse\") #true #false))"), "#true");
}


Test(end_to_end_strings, test_string_ci_equality, .init = setup_each_test, .fini = teardown_each_test) {
     /* 1. string-ci=? Basic ASCII */
     cr_assert_str_eq(t_eval("(string-ci=? \"Apple\" \"apple\" \"APPLE\")"), "#true");
     cr_assert_str_eq(t_eval("(string-ci=? \"a\" \"b\")"), "#false");

     /* 2. string-ci=? UTF-8 Greek (Sigma variations) */
     /* Final sigma (ς) and medial sigma (σ) must be equal to uppercase (Σ) */
     cr_assert_str_eq(t_eval("(string-ci=? \"Σ\" \"σ\" \"ς\")"), "#true");

     /* 3. string-ci=? German Eszett */
     /* "ß" should be equal to "ss" in a fold-case comparison */
     cr_assert_str_eq(t_eval("(string-ci=? \"Straße\" \"STRASSE\")"), "#true");

     /* 4. string-ci=? Arity 0 and 1 */
     cr_assert_str_eq(t_eval("(string-ci=?)"), "#true");
     cr_assert_str_eq(t_eval("(string-ci=? \"any\")"), "#true");
}


Test(end_to_end_strings, test_string_ci_ordering, .init = setup_each_test, .fini = teardown_each_test) {
    /* 5. string-ci<? and string-ci<=? */
    cr_assert_str_eq(t_eval("(string-ci<? \"apple\" \"Banana\")"), "#true");
    cr_assert_str_eq(t_eval("(string-ci<=? \"apple\" \"APPLE\")"), "#true");
    cr_assert_str_eq(t_eval("(string-ci<? \"A\" \"b\" \"C\")"), "#true");

    /* 6. string-ci>? and string-ci>=? */
    cr_assert_str_eq(t_eval("(string-ci>? \"Zebra\" \"apple\")"), "#true");
    cr_assert_str_eq(t_eval("(string-ci>=? \"banana\" \"Banana\")"), "#true");
    cr_assert_str_eq(t_eval("(string-ci>? \"z\" \"Y\" \"x\")"), "#true");

    /* 7. UTF-8 Ordering (Codepoint based after folding) */
    /* 'α' (alpha) is less than 'Ω' (omega) even with mixed case */
    cr_assert_str_eq(t_eval("(string-ci<? \"α\" \"Ω\")"), "#true");

    /* 8. Prefix rules in CI */
    cr_assert_str_eq(t_eval("(string-ci<? \"App\" \"apple\")"), "#true");
}

