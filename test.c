//
// Created by Owner on 6/28/2026.
//
// So the guide on I have been using decided to start a new file entirely. Because the lexer wasn't reading the parentheses.
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file;
    char *buffer = 0;
    long length;

    file = fopen("C:\\Users\\Owner\\CLionProjects\\Compiler\\test.txt", "r");

    if (!file) {
        printf("Error opening file\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    printf("Length: %d\n", length);
    fseek(file, 0, SEEK_SET);
    buffer = malloc(length + 1);

    fread(buffer, 1, length, file);
    if (!fread) {
        printf("ERROR: Fread failed");
        exit(1);
    }

    fclose(file);
    buffer[length] = '\0';

    char *current = malloc(sizeof(char) * length + 1);
    current = buffer;
    int current_index = 0;
    while (current[current_index] != '\0') {
    printf("%c\n", current[current_index]);
        current_index++;
    }
    return 0;
}
