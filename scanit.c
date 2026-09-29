/* Scanner tester: write the tokens for each input file. */

#include <stdio.h>
#include <string.h>
#include "scan.h"

static int outputname(char *out, int size, const char *input)
{
    int n = strlen(input);
    const char *suffix = "_output_scan.txt";
    if (n + 1 > size)
        return -1;
    strcpy(out, input);
    if (n >= 4 && strcmp(out + n - 4, ".txt") == 0) {
        n -= 4;
        out[n] = '\0';
    }
    if (n >= 6 && strcmp(out + n - 6, "_input") == 0) {
        n -= 6;
        out[n] = '\0';
    }
    if (n + (int)strlen(suffix) + 1 > size)
        return -1;
    strcat(out, suffix);
    return 0;
}

static int scanfile(const char *input)
{
    char output[4096];
    FILE *out;
    struct token t;
    int errors;
    if (outputname(output, sizeof(output), input) != 0) {
        fprintf(stderr, "Path too long: %s\n", input);
        return 1;
    }
    if (openfile(input) != 0) {
        fprintf(stderr, "Cannot open %s\n", input);
        return 1;
    }
    out = fopen(output, "w");
    if (out == NULL) {
        fprintf(stderr, "Cannot write %s\n", output);
        closefile();
        return 1;
    }
    setscanoutput(out);
    do {
        t = gettoken();
        if (t.id != TokenEndOfFile)
            fprintf(out, "%-12s%s\n", tokennames[t.id], t.lexeme);
    } while (t.id != TokenEndOfFile);
    errors = getlexicalerrors();
    closefile();
    fclose(out);
    return errors != 0;
}

int main(int argc, char **argv)
{
    int i, result = 0;
    if (argc < 2) {
        printf("Usage: scanit input1.txt [input2.txt ...]\n");
        return 1;
    }
    for (i = 1; i < argc; i++)
        if (scanfile(argv[i]) != 0)
            result = 1;
    return result;
}
