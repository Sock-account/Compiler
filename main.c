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

TokenLiteral generate_number(char current, FILE *file) {
    TokenLiteral *token = malloc(sizeof(TokenLiteral));
    token->type = INT;
    char *value = malloc(sizeof(char) * 8);
    int value_index = 0;
    while (isdigit(current) && current != EOF) {
        if (!isdigit(current)) {
            break;
        }
        value[value_index] = current;
        value_index++;
        //printf("%c", current);
        current = fgetc(file);
    }
    //printf("%c", token->value);
    token->value = atoi(value);
    return *token;
}

void lexer(FILE *file) {
    char current = fgetc(file);
    //char previous;

    if (file == NULL) {
        printf("Error reading file\n");
        return;
    }
    while (current != EOF) {
        if (current == ';') {
            // Each if branch excluding the one for detecting if current is a digit
            // has to check if previous is a digit so that multi-digit numbers display correctly on the same line.
            // This approach is not what is needed
            printf("FOUND SEMICOLON\n");
        }else if (current == '(') {
            printf("FOUND OPEN PAREN\n");
        }else if (current == ')') {
            printf("FOUND CLOSE PAREN\n");
        }else if (isdigit(current)) {
        TokenLiteral test_token = generate_number(current, file);
            //printf("FOUND DIGIT: %d", current);
            printf("TEST TOKEN VALUE: %d\n", test_token.value);
        }else if (isalpha(current)) {
            printf("FOUND CHARACTER: %c\n", current);
        }
        /*while (isdigit(current) && current != EOF) {
            while (isdigit(current) && current != EOF) {
                if (!isdigit(current)) {
                    break;
                }
                printf("%c", current);
                current = fgetc(file);
            }

        }*/
        //previous = current;
        //printf("%c", current);
        current = fgetc(file);
    }
}
int main() {
    FILE *file;
    file = fopen("C:\\Users\\Owner\\CLionProjects\\Compiler\\test.txt", "r");
    lexer(file);


}