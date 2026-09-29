#include <stdio.h>
#include "token.h"

int openfile(const char *filename);
void closefile(void);
void setscanoutput(FILE *output);
struct token gettoken(void);
int getlinenumber(void);
int getlexicalerrors(void);
