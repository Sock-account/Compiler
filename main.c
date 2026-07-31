#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    INT,
    KEYWORD,
    SEPARATOR,

} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

void print_token(Token token) {
    printf("TOKEN VALUE: ");
    for (int i = 0; token.value[i] != '\0'; i++){
        printf("%c", token.value[i]);
    }
    printf("\n");
    if (token.type == INT) {
        printf("TOKEN TYPE: INT\n");
    }
    if (token.type == KEYWORD) {
        printf("TOKEN TYPE: KEYWORD\n");
    }
    if (token.type == SEPARATOR) {
        printf("TOKEN TYPE: SEPARATOR\n");
    }
}

Token generate_number(char *current, int current_index) {
    Token *token = malloc(sizeof(Token));
    token->type = INT;
    char *value = malloc(sizeof(char) * 8);
    int value_index = 0;
    while (isdigit(current[current_index]) && current[current_index] != '\0') {
        if (!isdigit(current[current_index])) {
            break;
        }
        value[value_index] = current[current_index];
        value_index++;
        current_index++;
    }
    value[value_index] = '\0';
    //printf("%c", token->value);
    token->value = value;
    return *token;
}

Token *generate_keyword(char *current, int current_index) {
    Token *token = malloc(sizeof(Token));
    char *keyword = malloc(sizeof(char) * 8);
    int keyword_index = 0;
    while (current[current_index] != '\0' && isalpha(current[current_index])) {
        keyword[keyword_index] = current[current_index];
        //printf("%c", current[current_index]);
        keyword_index++;
        current_index++;
    }
    keyword[keyword_index] = '\0';
    token->type = KEYWORD;
    token->value = keyword;
    if (strcmp(keyword, "exit") == 0) {
        //printf("TYPE EXIT\n");
    }
    return token;
}

Token *lexer(FILE *file) {
    int length;
    char *buffer = 0;

    if (file == NULL) {
        printf("Error reading file\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = malloc(sizeof(char) * length + 1);
    length = fread(buffer, 1, length, file);
    fclose(file);
    buffer[length] = '\0';
    char *current = buffer;
    int current_index = 0;

    Token *tokens = malloc(sizeof(Token) * 12);
    size_t tokens_index = 0;

    while (current[current_index] != '\0') {
        //printf("curr: %c\n", current[current_index]);
        if (current[current_index] == ';') {
            printf("FOUND SEMICOLON\n");
            Token *semi_token = malloc(sizeof(Token));
            semi_token->type = SEPARATOR;
            semi_token->value = malloc(sizeof(char) * 2);
            semi_token->value[0] = current[current_index];
            semi_token->value[1] = '\0';
            print_token(*semi_token);
            current_index++;

        }else if (current[current_index] == '(') {
            printf("FOUND OPEN PAREN\n");
            Token *opar_token = malloc(sizeof(Token));
            opar_token->type = SEPARATOR;
            opar_token->value = malloc(sizeof(char) * 2);
            opar_token->value[0] = current[current_index];
            opar_token->value[1] = '\0';
            print_token(*opar_token);
            current_index++;

        }else if (current[current_index] == ')') {
            printf("FOUND CLOSE PAREN\n");
            Token *clopar_token = malloc(sizeof(Token));
            clopar_token->type = SEPARATOR;
            clopar_token->value = malloc(sizeof(char) * 2);
            clopar_token->value[0] = current[current_index];
            clopar_token->value[1] = '\0';
            print_token(*clopar_token);
            current_index++;

        }else if (isdigit(current[current_index])) {
            Token digit_token = generate_number(current, current_index);
            print_token(digit_token);
            current_index += strlen(digit_token.value);

        }else if (isalpha(current[current_index])) {
            Token *token_keyword = generate_keyword(current, current_index);
            print_token(*token_keyword);
            current_index += strlen(token_keyword->value);

        }else {
            printf("ERROR: UNRECOGNIZED CHARACTER\n");
            exit(-1);
        }
    }
}
int main() {
    FILE *file;
    file = fopen("test.txt", "r");
    lexer(file);


}

