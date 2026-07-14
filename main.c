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
    if (keyword_index > 0) {
        keyword[keyword_index] = '\0';
    } else {
        keyword[0] = '\0';  // Handle empty string case
    }
    if (strcmp(keyword, "exit") == 0) {
        printf("TYPE EXIT\n");
        token->type = KEYWORD;
        token->value = "EXIT";
    }
    return token;
}

void lexer(FILE *file) {
    int length;
    char *buffer = 0;
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = malloc(sizeof(char) * length);
    fread(buffer, 1, length, file);
    fclose(file);
    buffer[length + 1] = '\0';
    char *current = malloc(sizeof (char) * length + 1);
    current = buffer;
    int current_index = 0;

    if (file == NULL) {
        printf("Error reading file\n");
        return;
    }
    while (current[current_index] != '\0') {
        //printf("curr: %c\n", current[current_index]);
        if (current[current_index] == ';') {
            printf("FOUND SEMICOLON\n");
        }else if (current[current_index] == '(') {
            printf("FOUND OPEN PAREN\n");
        }else if (current[current_index] == ')') {
            printf("FOUND CLOSE PAREN\n");
        }else if (isdigit(current[current_index])) {
        Token test_token = generate_number(current, current_index);
            printf("TEST TOKEN VALUE: %s\n", test_token.value);
            int token_value = atoi(test_token.value);
            while (token_value >= 10) {
                token_value = token_value / 10;
                current_index++;
            }
        }else if (isalpha(current[current_index])) {
            Token *token_keyword = generate_keyword(current, current_index);
            //printf("Alpha %c\n", test_keyword->type);
            //printf("FOUND CHARACTER: %c\n", current[current_index]);

        }
        current_index++;
        //current = fgetc(file);
    }
}
int main() {
    FILE *file;
    file = fopen("C:\\Users\\Owner\\CLionProjects\\Compiler\\test.txt", "r");
    lexer(file);


}