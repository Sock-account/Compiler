//
// Created by Owner on 6/28/2026.
//
// So the guide on I have been using decided to start a new file entirely. Because the lexer wasn't reading the parentheses.
#include <stdio.h>

int main() {
    FILE *file;
    char buffer[100];

    file = fopen("test.txt", "r");

    fseek(file, 0, SEEK_SET);

    fread(buffer, 1, 1, file);
    printf("%c\n", buffer);
    fclose(file);

    return 0;
}