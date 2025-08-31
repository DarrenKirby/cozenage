#!/usr/bin/env python3
import sys
import re

def parse_builtins(filename):
    with open(filename, "r") as f:
        lines = f.readlines()

    in_function = False
    categories = []
    current_category = None

    for line in lines:
        stripped = line.strip()

        if "void lex_add_builtins" in stripped:
            in_function = True
            continue
        if in_function and stripped.startswith("}"):
            break

        if in_function:
            # Match category comments that are the only thing on the line
            cat_match = re.match(r"/\*\s*(.*?)\s*\*/\s*$", stripped)
            if cat_match:
                current_category = cat_match.group(1)
                categories.append((current_category, []))
                continue

            # Match builtin lines: lex_add_builtin(e, "foo", ...)
            # ignore any trailing comments
            builtin_match = re.search(r'lex_add_builtin\([^,]+,\s*"([^"]+)"', stripped)
            if builtin_match:
                func_name = builtin_match.group(1)
                if not categories:
                    categories.append(("Uncategorized", []))
                categories[-1][1].append(func_name)

    return categories

def print_markdown(categories):
    for category, funcs in categories:
        print(f"### {category}")
        for f in funcs:
            print(f"- `{f}`")
        print("")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <source_file>")
        sys.exit(1)

    filename = sys.argv[1]
    cats = parse_builtins(filename)
    print_markdown(cats)
