#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

int length_word(char*, int); 
char* readline(int,char*,int*);
char* substr(char*, int, int);
void remove_blank(char*);
void remove_letter(char*, int);
int length_blank(char*, int);
char* replaceValue(char*, char*, char*);
