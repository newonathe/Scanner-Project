/* Recursive descent parser for the SimpCalc grammar in the project handout. */

#include <stdio.h>
#include <string.h>
#include "scan.h"

static struct token currenttoken;
static FILE *parseoutput;
static int failed;

static void advance(void)
{
    do {
        currenttoken = gettoken();
    } while (currenttoken.id == TokenError);
}

static void parseerror(const char *message)
{
    if (!failed)
        fprintf(parseoutput, "Parse Error: %s (line #%d)\n",
                message, getlinenumber());
    failed = 1;
}

static void match(int tokenid)
{
    if (failed)
        return;
    if (currenttoken.id != tokenid) {
        fprintf(parseoutput, "Parse Error: %-12sexpected. (line #%d)\n",
                tokennames[tokenid], getlinenumber());
        failed = 1;
        return;
    }
    advance();
}

static void Exp(void);
static void Blk(void);

/* Val -> Identifier | Number | SQRT ( Exp ) | ( Exp ) */
static void Val(void)
{
    if (currenttoken.id == TokenIdentifier)
        match(TokenIdentifier);
    else if (currenttoken.id == TokenNumber)
        match(TokenNumber);
    else if (currenttoken.id == TokenSqrt) {
        match(TokenSqrt);
        match(TokenLeftParen);
        Exp();
        match(TokenRightParen);
    } else if (currenttoken.id == TokenLeftParen) {
        match(TokenLeftParen);
        Exp();
        match(TokenRightParen);
    } else
        parseerror("Invalid Statement");
}

/* Lit -> - Val | Val */
static void Lit(void)
{
    if (currenttoken.id == TokenMinus)
        match(TokenMinus);
    Val();
}

/* Litfollow -> ** Lit Litfollow | epsilon */
static void Litfollow(void)
{
    if (!failed && currenttoken.id == TokenRaise) {
        match(TokenRaise);
        Lit();
        Litfollow();
    }
}

/* Fac -> Lit Litfollow */
static void Fac(void)
{
    Lit();
    Litfollow();
}

/* Facfollow -> * Fac Facfollow | / Fac Facfollow | epsilon */
static void Facfollow(void)
{
    if (!failed && (currenttoken.id == TokenMultiply ||
                    currenttoken.id == TokenDivide)) {
        int op = currenttoken.id;
        match(op);
        Fac();
        Facfollow();
    }
}

/* Trm -> Fac Facfollow */
static void Trm(void)
{
    Fac();
    Facfollow();
}

/* Trmfollow -> + Trm Trmfollow | - Trm Trmfollow | epsilon */
static void Trmfollow(void)
{
    if (!failed && (currenttoken.id == TokenPlus ||
                    currenttoken.id == TokenMinus)) {
        int op = currenttoken.id;
        match(op);
        Trm();
        Trmfollow();
    }
}

/* Exp -> Trm Trmfollow */
static void Exp(void)
{
    Trm();
    Trmfollow();
}

static void Rel(void)
{
    switch (currenttoken.id) {
        case TokenLessThan: case TokenEqual: case TokenGreaterThan:
        case TokenLTEqual: case TokenNotEqual: case TokenGTEqual:
            match(currenttoken.id);
            break;
        default:
            parseerror("Missing relational operator");
    }
}

static void Cnd(void)
{
    Exp();
    if (failed) return;
    Rel();
    if (failed) return;
    Exp();
}

/* Arg -> String | Exp; Argfollow -> , Arg Argfollow | epsilon */
static void Arg(void)
{
    if (currenttoken.id == TokenString)
        match(TokenString);
    else
        Exp();
}

static void Argfollow(void)
{
    if (!failed && currenttoken.id == TokenComma) {
        match(TokenComma);
        Arg();
        Argfollow();
    }
}

/* Iffollow -> ENDIF ; | ELSE Blk ENDIF ; */
static void Iffollow(void)
{
    if (currenttoken.id == TokenElse) {
        match(TokenElse);
        Blk();
        if (failed) return;
        if (currenttoken.id != TokenEndif) {
            parseerror("Incomplete if Statement");
            return;
        }
    } else if (currenttoken.id != TokenEndif) {
        parseerror("Incomplete if Statement");
        return;
    }
    match(TokenEndif);
    match(TokenSemicolon);
}

/* Stm -> assignment | PRINT statement | IF statement */
static void Stm(void)
{
    if (currenttoken.id == TokenIdentifier) {
        match(TokenIdentifier);
        match(TokenAssign);
        Exp();
        match(TokenSemicolon);
        if (!failed)
            fprintf(parseoutput, "Assignment Statement Recognized\n");
    } else if (currenttoken.id == TokenPrint) {
        match(TokenPrint);
        match(TokenLeftParen);
        Arg();
        Argfollow();
        match(TokenRightParen);
        match(TokenSemicolon);
        if (!failed)
            fprintf(parseoutput, "Print Statement Recognized\n");
    } else if (currenttoken.id == TokenIf) {
        fprintf(parseoutput, "If Statement Begins\n");
        match(TokenIf);
        Cnd();
        match(TokenColon);
        if (failed) return;
        Blk();
        if (failed) return;
        Iffollow();
        if (!failed)
            fprintf(parseoutput, "If Statement Ends\n");
    } else
        parseerror("Invalid Statement");
}

/* Blk -> Stm Blk | epsilon. A non-statement token selects epsilon. */
static void Blk(void)
{
    if (!failed && (currenttoken.id == TokenIdentifier ||
                    currenttoken.id == TokenPrint ||
                    currenttoken.id == TokenIf)) {
        Stm();
        Blk();
    }
}

/* Prg -> Blk EndOfFile */
static void Prg(void)
{
    Blk();
    match(TokenEndOfFile);
}

static const char *basenameof(const char *path)
{
    const char *name = path;
    while (*path != '\0') {
        if (*path == '/' || *path == '\\')
            name = path + 1;
        path++;
    }
    return name;
}

static int outputname(char *out, int size, const char *input)
{
    int n = strlen(input);
    const char *suffix = "_output_parse.txt";
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

static int parsefile(const char *input)
{
    char output[4096];
    int result;
    if (outputname(output, sizeof(output), input) != 0) {
        fprintf(stderr, "Path too long: %s\n", input);
        return 1;
    }
    if (openfile(input) != 0) {
        fprintf(stderr, "Cannot open %s\n", input);
        return 1;
    }
    parseoutput = fopen(output, "w");
    if (parseoutput == NULL) {
        fprintf(stderr, "Cannot write %s\n", output);
        closefile();
        return 1;
    }
    setscanoutput(parseoutput);
    failed = 0;
    advance();
    Prg();
    if (!failed && getlexicalerrors() == 0)
        fprintf(parseoutput, "%s is a valid SimpCalc program\n", basenameof(input));
    result = failed ? 2 : (getlexicalerrors() != 0 ? 1 : 0);
    closefile();
    fclose(parseoutput);
    return result;
}

int main(int argc, char **argv)
{
    int i, result = 0;
    if (argc < 2) {
        printf("Usage: compute input1.txt [input2.txt ...]\n");
        return 1;
    }
    for (i = 1; i < argc; i++) {
        int status = parsefile(argv[i]);
        if (status == 2)
            return 1;
        if (status != 0)
            result = 1;
    }
    return result;
}
