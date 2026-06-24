#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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

TokenKeyword *generate_keyword(char current, FILE *file) {
    TokenKeyword *token = malloc(sizeof(TokenKeyword));
    char *keyword = malloc(sizeof(char) * 8);
    int keyword_index = 0;
    if (keyword == NULL) {
        free(token);
        return NULL;
    }
    while (isalpha(current) && current != EOF) {
        keyword[keyword_index] = current;
        current = fgetc(file);
        keyword_index++;
    }
    if (keyword_index > 0) {
        keyword[keyword_index] = '\0';
    } else {
        keyword[0] = '\0';  // Handle empty string case
    }
    //printf("TOKEN %s\n", keyword);
    if (strcmp(keyword, "exit") == 0) {
        token->type = EXIT;
    }
    return token;
}

void lexer(FILE *file) {
    char current = fgetc(file);
    //char previous;

    if (file == NULL) {
        printf("Error reading file\n");
        return;
    }
    while (current != EOF) {
        printf("curr: %c\n", current);
        if (current == ';') {
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
            TokenKeyword *test_keyword = generate_keyword(current, file);
            printf("Alpha %c\n", test_keyword->type);
            //printf("FOUND CHARACTER: %c\n", current)
            if (test_keyword->type == EXIT) {
                printf("EXIT\n");
            }
        }
        current = fgetc(file);
    }
}
int main() {
    FILE *file;
    file = fopen("C:\\Users\\Owner\\CLionProjects\\Compiler\\test.txt", "r");
    lexer(file);


}