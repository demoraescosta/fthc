#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define MAX_WORD_SIZE 32

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

struct token {
    int type;
    union {
        float f;
        i32   i;
        u32   u;
        char* s;
    } value;
};

struct token_list {
    struct token* list;
    size_t count;
    size_t capacity;
};

enum {
    TOK_EOF,
    TOK_EOL,

    TOK_INTEGER,
    TOK_WORD,

    TOK_DOT,
    TOK_ADD,
    TOK_SUB,

    TOK_COLON,

    TOK_LPAREN,
    TOK_RPAREN,
};

struct token_list token_list_init(void);
void token_list_deinit(struct token_list* list); 

int token_list_append(struct token_list* list, struct token tk);
struct token token_list_pop(struct token_list* list);
struct token token_list_peek(struct token_list* list);

void usage(void);

void tokenize(char* str, size_t length, struct token_list* buffer);
int interpret(struct token_list* buffer);

struct token_list 
token_list_init(void)
{
    return (struct token_list) {
        .list = malloc(sizeof(struct token_list) * 32),
        .count = 0,
        .capacity = 32
    };
}

void
token_list_deinit(struct token_list* list)
{
    for (size_t i = 0; i < list->count; ++i) {
        if (list->list[i].type == TOK_WORD) { 
            free(list->list[i].value.s); 
        }
    }

    free(list->list);
}

int
token_list_append(struct token_list* list, struct token tk)
{
    list->list[list->count++] = tk;
    if (list->count + 1 > list->capacity) {
        struct token* np = realloc(list->list, list->capacity * 2);
        if (np == NULL) return 0;
        list->capacity *= 2;
    }
    return 1;
}

struct token
token_list_pop(struct token_list* list)
{
    if (list->count == 0) return (struct token) { 0 };
    struct token tk = list->list[--list->count];
    return tk;
}

struct token
token_list_peek(struct token_list* list)
{
    if (list->count == 0) return (struct token) { 0 };
    struct token tk = list->list[list->count - 1];
    return tk;
}

void
tokenize(char* str, size_t length, struct token_list* buffer)
{
    char* name_buff = malloc(MAX_WORD_SIZE * sizeof(char)); 
    size_t name_buff_iter = 0;

    if (length < 1) {
        token_list_append(buffer, (struct token) {
            .type = TOK_EOF,
        });
    }

    for (size_t i = 0; i < length; ++i) {
        int tok_type = 0;
        u32 tok_value = 0;
        char c0 = str[i];
        
        switch (c0) {
        case '\n': tok_type = TOK_EOL;    break;
        case '.':  tok_type = TOK_DOT;    break;
        case '+':  tok_type = TOK_ADD; break;
        case '-':  tok_type = TOK_SUB; break;
        case ':':  tok_type = TOK_COLON; break;
        case '(':  tok_type = TOK_LPAREN; break;
        case ')':  tok_type = TOK_RPAREN; break;
        case ' ':  break;
        default:
            /* TODO: to support strings in the future i should probably implement a string builder but this will do */
            name_buff[name_buff_iter] = c0;
            name_buff_iter = (name_buff_iter + 1) % MAX_WORD_SIZE;
            name_buff[name_buff_iter] = '\0';
            goto continue_parsing; /* break out of switch */
        }

        if (name_buff_iter > 0) {
            char* endp;
            int i = strtol(name_buff, &endp, 10);
            if (*endp != 0) {
                char* str = malloc(name_buff_iter * sizeof(char));
                memcpy(str, name_buff, name_buff_iter);
                token_list_append(buffer, (struct token) {
                    .type    = TOK_WORD,
                    .value.s = str 
                });
            } else {
                token_list_append(buffer, (struct token) {
                    .type    = TOK_INTEGER,
                    .value.i = i 
                });
            }
            name_buff_iter = 0;
        }

        if (tok_type != 0) {
            token_list_append(buffer, (struct token) {
                .type    = tok_type,
                .value.u = tok_value
            });
        }
continue_parsing: 
        ;
    }

    free(name_buff);    
}

int
interpret(struct token_list* buffer)
{
    int ret = 0;

    struct token_list stack = token_list_init();
    for (size_t i = 0; i < buffer->count; ++i) {
        int type = buffer->list[i].type;
        switch (type) {
        case TOK_INTEGER:
            token_list_append(&stack, buffer->list[i]);
            break;

        case TOK_ADD: 
            ; /* weird that i have to do this but it seems like c does not allow for empty statements after labels */
            int sum = 0;

            do {
                struct token x = token_list_pop(&stack);
                sum += x.value.i;
            } while(token_list_peek(&stack).type == TOK_INTEGER);

            token_list_append(&stack, (struct token) {
                .type = TOK_INTEGER,
                .value.i = sum
            });
            break;
            
        case TOK_DOT: 
            ;
            struct token top = token_list_pop(&stack);
            if (top.type == TOK_INTEGER) {
                fprintf(stdout, "%d", top.value.i);
            }
            break;
        default:
            break;
        }
    }
    
    token_list_deinit(&stack);
    return ret;
}

void
usage(void)
{
    fprintf(stdout, "Usage: fthc [OPTIONS] ... FILE\n");   
    fprintf(stdout, "Options:\n");   
    fprintf(stdout, "  -i      Interpret file\n");   
}

int 
main(int argc, char** argv)
{
    int flag_interpret = 0;

    opterr = 0;
    int opt;
    while ((opt = getopt(argc, argv, "i")) != -1) {
        switch (opt) {
        case 'i':
            flag_interpret = 1;
            break;

        default: 
            fprintf(stderr, "fthc: unknown option %s\n", argv[optind - 1]);
            usage();
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "fthc: no input files provided\n");
        usage();
        exit(EXIT_FAILURE);
    }

    char* filename = argv[optind];
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "fthc: %s: %s\n", filename, strerror(errno));
        exit(errno);
    }

    int ret = 0;

    struct token_list tklist = token_list_init();

    if (flag_interpret) {
        fseek(file, 0, SEEK_END);
        size_t length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char* fstr = malloc(length);

        if (fstr == NULL) {
            fprintf(stderr, "fthc: out of memory\n");
            exit(EXIT_FAILURE);
        }

        fread(fstr, 1, length, file);

        tokenize(fstr, length, &tklist);
        ret = interpret(&tklist);
    }

    fclose (file);
    token_list_deinit(&tklist);

    exit(EXIT_SUCCESS);
    return ret;
}
