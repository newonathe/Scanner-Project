/* Scanner tester: write the tokens for each input file. */

#include <stdio.h>
#include <string.h>
#include "scan.h"
#include "inputs.h"

static int outputname(char *out, size_t size, const char *input)
{
    size_t n = strlen(input);
    const char *suffix = "_output_scan.txt";
    if (n >= 4 && strcmp(input + n - 4, ".txt") == 0)
        n -= 4;
    if (n >= 6 && strncmp(input + n - 6, "_input", 6) == 0)
        n -= 6;
    if (n + strlen(suffix) + 1 > size)
        return -1;
    memcpy(out, input, n);
    strcpy(out + n, suffix);
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
    return runinputs(argc, argv, scanfile);
}
