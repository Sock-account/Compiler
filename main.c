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
    file = fopen("test.unn", "r");
    char current = fgetc(file);

    while (current != EOF) {
        printf("%c", current);
        current = fgetc(file);
    }

}