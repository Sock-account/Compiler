#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.c"

int main(){
FILE *file;
file = fopen("test.txt" , "r");

Token *tokens = lexer(file);

	for(size_t i = 0; tokens[i].type != END_OF_TOKENS){

	}
