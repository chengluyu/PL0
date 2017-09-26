/* A minimal interpreter of PL/0 language which is written in pure C */
/* Author: Luyu Cheng (GitHub: chengluyu) */

#include <limits.h>
#include <ctype.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum interpreter_error_t {
    INTERNAL_ERROR,
    LEXICAL_ERROR,
    SYNTAX_ERROR,
    EXECUTION_ERROR,
};

enum token_type_t {
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_INTERNAL,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_PUNCTUATOR
};

#define TOKEN_LIST(declare)                                                     \
    declare(ILLEGAL, NULL, TOKEN_TYPE_INTERNAL)                                 \
    declare(EOS, NULL, TOKEN_TYPE_INTERNAL)                                     \
    declare(INTEGER, NULL, TOKEN_TYPE_INTERNAL)                                 \
    declare(IDENTIFIER, NULL, TOKEN_TYPE_INTERNAL)                              \
    declare(LPAREN, "(", TOKEN_TYPE_PUNCTUATOR)                                 \
    declare(RPAREN, ")", TOKEN_TYPE_PUNCTUATOR)                                 \
    declare(SEMICOLON, ";", TOKEN_TYPE_PUNCTUATOR)                              \
    declare(PERIOD, ".", TOKEN_TYPE_PUNCTUATOR)                                 \
    declare(COMMA, ",", TOKEN_TYPE_PUNCTUATOR)                                  \
    declare(ADD, "+", TOKEN_TYPE_OPERATOR)                                      \
    declare(SUB, "-", TOKEN_TYPE_OPERATOR)                                      \
    declare(MUL, "*", TOKEN_TYPE_OPERATOR)                                      \
    declare(DIV, "/", TOKEN_TYPE_OPERATOR)                                      \
    declare(ASSIGN, ":=", TOKEN_TYPE_OPERATOR)                                  \
    declare(EQ, "=", TOKEN_TYPE_OPERATOR)                                       \
    declare(NEQ, "#", TOKEN_TYPE_OPERATOR)                                      \
    declare(LE, "<", TOKEN_TYPE_OPERATOR)                                       \
    declare(LEQ, "<=", TOKEN_TYPE_OPERATOR)                                     \
    declare(GE, ">", TOKEN_TYPE_OPERATOR)                                       \
    declare(GEQ, ">=", TOKEN_TYPE_OPERATOR)                                     \
    declare(CONST, "const", TOKEN_TYPE_KEYWORD)                                 \
    declare(VAR, "var", TOKEN_TYPE_KEYWORD)                                     \
    declare(BEGIN, "begin", TOKEN_TYPE_KEYWORD)                                 \
    declare(END, "end", TOKEN_TYPE_KEYWORD)                                     \
    declare(IF, "if", TOKEN_TYPE_KEYWORD)                                       \
    declare(THEN, "then", TOKEN_TYPE_KEYWORD)                                   \
    declare(ELSE, "else", TOKEN_TYPE_KEYWORD)                                   \
    declare(CALL, "call", TOKEN_TYPE_KEYWORD)                                   \
    declare(WHILE, "while", TOKEN_TYPE_KEYWORD)                                 \
    declare(DO, "do", TOKEN_TYPE_KEYWORD)                                       \
    declare(READ, "read", TOKEN_TYPE_KEYWORD)                                   \
    declare(WRITE, "write", TOKEN_TYPE_KEYWORD)                                 \
    declare(PROCEDURE, "procedure", TOKEN_TYPE_KEYWORD)

/* the token enumerator */
enum token_t {
#define DEFINE_ENUM_ITEM(name, literal, type) TOKEN_##name,
    TOKEN_LIST(DEFINE_ENUM_ITEM) TOKEN_COUNT
#undef DEFINE_ENUM_ITEM
};

/* name of each token */
static const char *token_names[TOKEN_COUNT] = {
    #define DEFINE_TOKEN_NAME(name, literal, type) #name,
    TOKEN_LIST(DEFINE_TOKEN_NAME)
#undef DEFINE_TOKEN_NAME
};

/* string literals of each token */
static const char *literals[TOKEN_COUNT] = {
#define DEFINE_TOKEN_LITERAL(name, literal, type) literal,
    TOKEN_LIST(DEFINE_TOKEN_LITERAL)
#undef DEFINE_TOKEN_LITERAL
};

/* types of each token */
static const int token_type[TOKEN_COUNT] = {
#define DEFINE_TOKEN_TYPE(name, literal, type) type,
    TOKEN_LIST(DEFINE_TOKEN_TYPE)
#undef DEFINE_TOKEN_TYPE
};

#define ALPHABET_SIZE (1 << 7)

typedef struct trie_node {
    struct trie_node *indices[ALPHABET_SIZE];
    int token;
} trie_node_t;

trie_node_t* create_trie_node() {
    trie_node_t *t = (trie_node_t*) malloc(sizeof (trie_node_t));
    // clear
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        t->indices[i] = NULL;
    }
    t->token = TOKEN_ILLEGAL;
    return t;
}

void destroy_trie_node(trie_node_t *t) {
    for (int i = 0; i < 1 << 7; i++) {
        if (t->indices[i]) {
            destroy_trie_node(t->indices[i]);
        }
    }
    free(t);
}

void print_trie_node(trie_node_t *t, int indent_size) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (t->indices[i]) {
            for (int j = 0; j < indent_size; j++) {
                printf("    ");
            }
            printf("%c\n", (char) i);
            print_trie_node(t->indices[i], indent_size + 1);
        }
        
    }
}

void insert_trie_node(trie_node_t *t, const char *s, int token) {
    while (*s) {
        if (t->indices[(int) *s] == NULL) {
            t->indices[(int) *s] = create_trie_node();
        }
        t = t->indices[(int) *s];
        s++;
    }
    if (*s == '\0') {
        t->token = token;
    }
}

trie_node_t* make_trie_tree() {
    trie_node_t *root = create_trie_node();
    for (int i = 0; i < TOKEN_COUNT; i++) {
        if (literals[i]) {
            insert_trie_node(root, literals[i], i);
        }
    }
    return root;
}

int lookup_trie_tree(trie_node_t *t, const char *s) {
    while (*s && t) {
        if (t->indices[(int) *s]) {
            t = t->indices[(int) *s];
        }
        s++;
    }
    if (!t || t->token == TOKEN_ILLEGAL) {
        return TOKEN_ILLEGAL;
    }
    return t->token;
}

trie_node_t *lex_trie_tree = NULL;

FILE *input;
int peek;

void ignore() {
    peek = fgetc(input);
}

int next() {
    int backup = peek;
    peek = fgetc(input);
    return backup;
}

#define TOKEN_BUFFER_SIZE (1 << 10)

int current_token;
char current_token_buffer[TOKEN_BUFFER_SIZE];

/* advance the next token */
void advance() {
    /* skip leading whitespaces */
    while (isspace(peek)) {
        ignore();
    }

    /* check if reach the end */
    if (peek == EOF) {
        current_token = TOKEN_EOS;
        return;
    }

    /* identifiers or declares */
    if (isalpha(peek) || peek == '_') {
        bool starts_with_underscore = peek == '_';
        int i = 0;
        do {
            current_token_buffer[i++] = next();
        } while (isalnum(peek) || peek == '_');
        /* important: terminate the buffer with a zero */
        current_token_buffer[i] = '\0';
        /* an extra check */
        if (starts_with_underscore) {
            current_token = TOKEN_IDENTIFIER;
            return;
        }
        /* check if is a keyword */
        int result = lookup_trie_tree(lex_trie_tree, current_token_buffer);
        current_token = result == TOKEN_ILLEGAL ? TOKEN_IDENTIFIER : result;
        return;
    }

    /* integer literal */
    if (isdigit(peek)) {
        int i = 0;
        do {
            current_token_buffer[i++] = next();
        } while (isdigit(peek));
        current_token_buffer[i] = '\0';
        current_token = TOKEN_INTEGER;
        return;
    }

    trie_node_t *t = lex_trie_tree;
    if (t) {
        /* operators or punctuators */
        /* greedy matching along the tree */
        while (t->indices[peek]) {
            t = t->indices[peek];
            ignore();
        }
        current_token = t->token;
    } else {
        sprintf(current_token_buffer, "unrecognized characters: %c (code point = %d)", peek, peek);
        /* unrecognized characters are considered to be illegal */
        current_token = TOKEN_ILLEGAL;
    }
}

bool match(int token) {
    if (current_token == token) {
        advance();
        return true;
    }
    return false;
}

/* a test subroutine of lexer */
void test_lex() {
    input = fopen("example.pl0", "r");
    if (input == NULL) {
        puts("Cannot open test input file.");
        return;
    }
    ignore();
    advance();
    while (current_token != TOKEN_EOS && current_token != TOKEN_ILLEGAL) {
        printf("token = %s, literal = \"%s\"\n", token_names[current_token],
                literals[current_token] ? literals[current_token] : current_token_buffer);
        advance();
    }
}

void init() {
    lex_trie_tree = make_trie_tree();
}

void clean() {
    destroy_trie_node(lex_trie_tree);
}

int main(int argc, const char *argv[]) {
    init();
    test_lex();
    clean();
    return 0;
}