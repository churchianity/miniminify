
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "table.h"


const char* keywords[] = {
    "of",      "for",        "from",      "break",      "return",  "if",
    "try",     "true",       "false",     "finally",    "new",     "enum",
    "const",   "in",         "continue",  "case",       "catch",   "export",
    "extends", "set",        "interface", "instanceof", "as",      "function",
    "void",    "async",      "static",    "var",        "null",    "yield",
    "typeof",  "private",    "get",       "protected",  "package", "let",
    "else",    "super",      "do",        "with",       "await",   "delete",
    "default", "debugger",   "class",     "public",     "this",    "throw",
    "import",  "implements", "switch",    "while"
};

Table<const char, int>* keywordTable;

// jenkins one-at-a-time string hash
uint32_t hash(char* s, int len) {
    uint32_t h = 0;

    int i = 0;
    while (i < len) {
        h += *s;
        h += (h << 10);
        h ^= (h >> 6);
        s++;
        i++;
    }

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);

    return h;
}

int isKeyword(char* buffer, char** cursor) {
    return 0;
    char* cur = *cursor;
    int len = 1;
    while (1) {
        switch (*cur++) {
            case '_': case '$':
            case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':case 'h':case 'i':case 'j':case 'k':case 'l':case 'm':
            case 'n':case 'o':case 'p':case 'q':case 'r':case 's':case 't':case 'u':case 'v':case 'w':case 'x':case 'y':case 'z':
            case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':case 'H':case 'I':case 'J':case 'K':case 'L':case 'M':
            case 'N':case 'O':case 'P':case 'Q':case 'R':case 'S':case 'T':case 'U':case 'V':case 'W':case 'X':case 'Y':case 'Z':
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                len++;
                break;

            default:
                return 0;
        }
    }

    return 0;
}

void minify(char* filepath) {
    // open the file, read its contents
    FILE* fp = fopen(filepath, "rb");

    if (fp == NULL) {
        printf("Failed to open file: |%s|\n", filepath);
        return;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char* buffer = (char*) malloc(size + 1);
    if (buffer == NULL) {
        printf("Failed to alloc buffer for file: |%s|. Probably out of memory.\n", filepath);
        return;
    }
    fread(buffer, sizeof (char), size + 1, fp);
    buffer[size] = '\0';

    fclose(fp);

    // alloc a parallel buffer. a minified file size must be <= size of the original file
    // at the end we'll write this buffer to a file from 0 -> miniCursor
    char* minifiedBuffer = (char*) malloc(size + 1);
    int miniIndex = 0;

    // we need to keep a two arrays here, one of the identifiers in the program, and another of the minified identifer
    char** identifiers;
    char** minifiedIdentifiers;

    char* cursor = buffer;
    char c = *cursor;

    int prevTokenIdent = 0;
    // walk the original buffer, copying data to output buffer
    while (c != '\0') {
        c = *++cursor;

        switch (c) {
            // might have a comment...
            case '/':
                if (*(cursor + 1) == '/') {
                    while (c != '\0') {
                        if (c == '\n') {
                            break;
                        }

                        c = *++cursor;
                    }
                } else if (c == '*') {
                    int prevTokenAsterisk = 0;
                    while (c != '\0') {
                        if (c == '*') {
                            prevTokenAsterisk = 1;

                        } else if (c == '/' && prevTokenAsterisk) {
                            break;

                        } else {
                            prevTokenAsterisk = 0;
                        }

                        c = *++cursor;
                    }
                }
                break;

            case ';':
                // @HACK, all semicolons can be replaced with newlines, not all newlines can be replaced with semicolons
                c = '\n';
                break;

            case '"':
            case '\'':
            case '`': {
                minifiedBuffer[miniIndex++] = c;
                char quote = c;
                // we have to faithfully copy all bytes that are part of a string
                while (c != '\0') {
                    c = *++cursor;
                    if (c == quote) {
                        break;
                    }

                    minifiedBuffer[miniIndex++] = c;
                }
            } break;

            // these characters can never be minified.
            case '+':
            case '-':
            case '%':
            case '=':
            case '?':

                break;

            case ' ': case '\t': case '\r': case '\n':
                // TODO we need to check if the previous token is a keyword. certain keywords require whitespace after them, such as:
                // let, const, var, function, new, etc.
                // otherwise, we can skip it.
                continue;
                break;

            case '_': case '$':

            case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':case 'h':case 'i':case 'j':case 'k':case 'l':case 'm':
            case 'n':case 'o':case 'p':case 'q':case 'r':case 's':case 't':case 'u':case 'v':case 'w':case 'x':case 'y':case 'z':
            case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':case 'H':case 'I':case 'J':case 'K':case 'L':case 'M':
            case 'N':case 'O':case 'P':case 'Q':case 'R':case 'S':case 'T':case 'U':case 'V':case 'W':case 'X':case 'Y':case 'Z':
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                break;
        }

        minifiedBuffer[miniIndex++] = c;
    }

    minifiedBuffer[miniIndex + 1] = '\0';

    printf("%s\n", minifiedBuffer);
    printf("initial size (bytes): %u, minified: %d\n", size, miniIndex);

    // open a file for writing.
    FILE* outFp = fopen("test.min.js", "wb");
    fwrite(minifiedBuffer, 1, miniIndex, outFp);
    fclose(outFp);
}

int main(int argc, char* argv[]) {
    keywordTable = new Table<const char, int>();
    for (int i = 0; i < (sizeof(keywords) / sizeof(char*)); i++) {
        keywordTable->insert(keywords[i], sizeof(keywords[i]), 1);
    }

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            printf("minifying |%s|...\n", argv[i]);
            minify(argv[i]);
        }
    }
}

