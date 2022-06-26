
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

struct TableEntry {
    struct TableEntry* next;
    char* key;
    void* value;

    uint32_t keyLength;
};

struct Table {
    uint32_t lanes;
    struct TableEntry** entries;
};

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

struct Table* tableInit(uint32_t lanes) {
    struct Table* table = (struct Table*) malloc(sizeof (struct Table));
    table->lanes = lanes;
    table->entries = (struct TableEntry**) calloc(lanes, sizeof (struct TableEntry*));
    return table;
}

struct TableEntry* tableLookup(struct Table* table, char* key, uint32_t keyLength) {
    uint32_t hashValue = hash(key, keyLength);
    struct TableEntry* entry = table->entries[hashValue % table->lanes];

    for (; entry != NULL; entry = entry->next) {
        if (keyLength == entry->keyLength && !memcmp(key, entry->key, keyLength)) {
            return entry;
        }
    }

    return NULL;
}

int tableInsert(struct Table* table, char* key, uint32_t keyLength, void* value) {
    struct TableEntry* entry = tableLookup(table, key, keyLength);

    if (!entry) {
        entry = (struct TableEntry*) malloc(sizeof (struct TableEntry));
        entry->key = key;
        entry->keyLength = keyLength;
        entry->value = value;

        uint32_t hashValue = hash(key, keyLength);
        entry->next = table->entries[hashValue % table->lanes];
        table->entries[hashValue] = entry;

        return 0;

    } else {
        free(entry->value);
        entry->value = value;
        return 1;
    }
}

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

static struct Table* keywordTable;

bool isKeyword(char* buffer, char** cursor) {
    return false;
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
    char* minifiedBuffer = (char*) malloc(size + 1);
    int miniIndex = 0;

    char* cursor = buffer;
    char c = *cursor;

    bool prevTokenIdentifier = false;
    bool prevTokenWhitespace = true;

    // walk the original buffer, copying data to output (minified) buffer
    while (c != '\0') {
        switch (c) {
            case '\0':
                prevTokenWhitespace = false;
                prevTokenIdentifier = false;
                goto write;

            // might have a comment...
            case '/': {
                prevTokenWhitespace = false;
                prevTokenIdentifier = false;
                char nc = *(cursor + 1);
                if (nc == '/') {
                    while (c != '\0') {
                        if (c == '\n') {
                            c = *++cursor;
                            break;
                        }

                        c = *++cursor;
                    }
                } else if (nc == '*') {
                    bool prevTokenAsterisk = false;
                    while (c != '\0') {
                        if (c == '*') {
                            prevTokenAsterisk = true;

                        } else if (c == '/' && prevTokenAsterisk) {
                            c = *++cursor;
                            break;

                        } else {
                            prevTokenAsterisk = false;
                        }

                        c = *++cursor;
                    }
                }
            } break;

            // these characters can never be minified.
            case '+':
            case '-':
            case '%':
            case '=':
            case '?':
            case '.':
            case ',':
            case '&':
            case '*':
            case '!':
            case '~':
            case '|':
            case ':':
            case ';':
            case '<':
            case '>':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
                prevTokenWhitespace = false;
                prevTokenIdentifier = false;
                break;

            case '"':
            case '\'':
            case '`': {
                prevTokenWhitespace = false;
                prevTokenIdentifier = false;
                minifiedBuffer[miniIndex++] = c;
                char quote = c;
                bool escape = false;

                // we have to faithfully copy all bytes that are part of a string.
                while (c != '\0') {
                    c = *++cursor;

                    if (c == '\\') {
                        escape = true;

                    } else {
                        if (!escape && c == quote) {
                            break;
                        }

                        escape = false;
                    }

                    minifiedBuffer[miniIndex++] = c;
                }
            } break;

            // carriage return never means anything.
            case '\r': continue;

            case '\n':
                // I don't know what to do here
                prevTokenWhitespace = true;
                prevTokenIdentifier = false;
                break;

            // relevant whitespace...
            case '\t': case ' ':
                // if we just parsed some whitespace, we can skip this whitespace, as whitespace sequences larger than one can always be reduced to a sequence of 0 or 1
                // IF, the
                if (prevTokenWhitespace) {
                    c = *++cursor;
                    continue;
                }

                // check if we just saw an identifier. if we didn't we can skip whitespace.
                // unless we're seeing a keyword like let, const, var, function, as, in, typeof, instanceof, which require a space (or parens) after
                if (!prevTokenIdentifier) {
                    c = *++cursor;
                    continue;
                }

                prevTokenWhitespace = true;
                prevTokenIdentifier = false;
                break;

            case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':case 'g':case 'h':case 'i':case 'j':case 'k':case 'l':case 'm':
            case 'n':case 'o':case 'p':case 'q':case 'r':case 's':case 't':case 'u':case 'v':case 'w':case 'x':case 'y':case 'z':
            case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':case 'G':case 'H':case 'I':case 'J':case 'K':case 'L':case 'M':
            case 'N':case 'O':case 'P':case 'Q':case 'R':case 'S':case 'T':case 'U':case 'V':case 'W':case 'X':case 'Y':case 'Z':
            case '_':case '$':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                prevTokenWhitespace = false;
                prevTokenIdentifier = true; // @HACK lazy, not necessarily true.
                break;

            default:
                printf("weird code point: |%hhx|, panicking...\n", c);
                exit(1);
                break;
        }

        minifiedBuffer[miniIndex++] = c;
        c = *++cursor;
    }

write:
    printf("%s\n", minifiedBuffer);
    printf("\ninitial size (bytes): %u, minified: %d, a %.2f%% reduction.\n", size, miniIndex, (1 - ((float)miniIndex)/((float)size))*100);

    // open a file for writing.
    FILE* outFp = fopen("test.min.js", "wb");
    fwrite(minifiedBuffer, 1, miniIndex, outFp);
    fclose(outFp);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            printf("minifying |%s|...\n", argv[i]);
            clock_t begin = clock();
            minify(argv[i]);
            clock_t end = clock();
            double timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;
            printf("Total time including printfs: %f seconds.\n", timeSpent);
        }
    }
}

