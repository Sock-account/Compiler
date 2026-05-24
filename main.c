#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
void lexer(FILE *file) {
    char current = fgetc(file);
    char previous;
    if (file == NULL) {
        printf("Error reading file\n");
        return;
    }
    while (current != EOF) {
        if (current == ';') {
            // Each if branch excluding the one for detecting if current is a digit
            // has to check if previous is a digit so that multi-digit numbers display correctly on the same line.
            if (isdigit(previous)) {
                printf("\n");
            }
            printf("FOUND SEMICOLON\n");
        }
        else if (current == '(') {
            if (isdigit(previous)) {
                printf("\n");
            }
            printf("FOUND OPEN PAREN\n");
        }
        else if (current == ')') {
            if (isdigit(previous)) {
                printf("\n");
            }
            printf("FOUND CLOSE PAREN\n");

        }
        else if (isdigit(current)) {
            if (isdigit(previous)) {
                printf("%d", current - 48);
            }
            else {
                printf("FOUND DIGIT: %d", current - 48);
            }
            }

        //printf("%c", current);
        previous = current;
        current = fgetc(file);
    }
}
int main() {
    FILE *file;
    file = fopen("C:\\Users\\Owner\\CLionProjects\\Compiler\\test.txt", "r");
    lexer(file);


}