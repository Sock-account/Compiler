#include <stdio.h>

typedef enum {
    SEMI,
    OPEN_PAREN,
    CLOSE_PAREN,
} TokenTypeSeparator;

typedef enum{
    EXIT ,
   } TokenTypeKeyword;

typedef enum {
    INT ,
    } TokenTypeLiteral;


typedef struct {
    TokenTypeKeyword type;
} TokenKeyword;

typedef struct {
    TokenTypeLiteral type;
    int value;
} TokenLiteral;

typedef struct {
    TokenTypeSeparator type;
} TokenSeparator;

int main() {
    FILE *file;
    file = fopen("C:\\Users\\Owner\\CLionProjects\\Compiler\\test.txt", "r");
    char current = fgetc(file);

    if (file == NULL) {
        printf("Error reading file\n");
        return 1;
    }
    while (current != EOF) {
        if (current == ';') {
            printf("FOUND SEMICOLON\n");
        }
        //printf("%c", current);
        current = fgetc(file);
    }

}