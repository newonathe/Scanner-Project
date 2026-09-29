/* SimpCalc scanner: each call to gettoken() returns one token. */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "scan.h"

static FILE *source;
static FILE *scanoutput;
static int linenum = 1;
static int tokenline = 1;
static int lexicalerrors;
static int pushback;
static int charread;

int openfile(const char *filename)
{
    closefile();
    source = fopen(filename, "r");
    linenum = tokenline = 1;
    lexicalerrors = 0;
    pushback = 0;
    return source == NULL ? -1 : 0;
}

void closefile(void)
{
    if (source != NULL)
        fclose(source);
    source = NULL;
    pushback = 0;
}

void setscanoutput(FILE *output)
{
    scanoutput = output;
}

int getlinenumber(void)
{
    return tokenline;
}

int getlexicalerrors(void)
{
    return lexicalerrors;
}

/* The single-character pushback follows the scanner lab example. */
static int mygetchar(void)
{
    int c;
    if (pushback) {
        c = charread;
        pushback = 0;
    } else {
        c = fgetc(source);
    }
    if (c == '\n')
        linenum++;
    return c;
}

static void putback(int c)
{
    if (c == EOF)
        return;
    charread = c;
    pushback = 1;
    if (c == '\n')
        linenum--;
}

static struct token newtoken(int id)
{
    struct token t;
    t.id = id;
    t.lexeme[0] = '\0';
    return t;
}

static int addchar(struct token *t, int *length, int c)
{
    if (*length >= (int)sizeof(t->lexeme) - 1)
        return 0;
    t->lexeme[(*length)++] = (char)c;
    t->lexeme[*length] = '\0';
    return 1;
}

static struct token lexicalerror(const char *message)
{
    FILE *out = scanoutput == NULL ? stdout : scanoutput;
    lexicalerrors++;
    fprintf(out, "Lexical Error: %s (line #%d)\n", message, tokenline);
    return newtoken(TokenError);
}

static int keyword(const char *word)
{
    static const struct {
        const char *name;
        int id;
    } words[] = {
        {"PRINT", TokenPrint}, {"IF", TokenIf}, {"ELSE", TokenElse},
        {"ENDIF", TokenEndif}, {"SQRT", TokenSqrt}, {"AND", TokenAnd},
        {"OR", TokenOr}, {"NOT", TokenNot}
    };
    int i;
    for (i = 0; i < (int)(sizeof(words) / sizeof(words[0])); i++)
        if (strcmp(word, words[i].name) == 0)
            return words[i].id;
    return TokenIdentifier;
}

static struct token identifier(int first)
{
    struct token t = newtoken(TokenIdentifier);
    int length = 0, too_long = 0, c = first;
    do {
        if (!addchar(&t, &length, c))
            too_long = 1;
        c = mygetchar();
    } while (isalnum((unsigned char)c) || c == '_');
    putback(c);
    if (too_long)
        return lexicalerror("Illegal character/character sequence");
    t.id = keyword(t.lexeme);
    return t;
}

static struct token number(int first)
{
    struct token t = newtoken(TokenNumber);
    int length = 0, too_long = 0, c = first;
    do {
        if (!addchar(&t, &length, c))
            too_long = 1;
        c = mygetchar();
    } while (isdigit((unsigned char)c));

    if (c == '.') {
        if (!addchar(&t, &length, c))
            too_long = 1;
        c = mygetchar();
        if (!isdigit((unsigned char)c)) {
            if (!isalpha((unsigned char)c) && c != '_')
                putback(c);
            return lexicalerror("Invalid number format");
        }
        do {
            if (!addchar(&t, &length, c))
                too_long = 1;
            c = mygetchar();
        } while (isdigit((unsigned char)c));
    }

    if (c == 'e' || c == 'E') {
        if (!addchar(&t, &length, c))
            too_long = 1;
        c = mygetchar();
        if (c == '+' || c == '-') {
            if (!addchar(&t, &length, c))
                too_long = 1;
            c = mygetchar();
        }
        if (!isdigit((unsigned char)c)) {
            if (!isalpha((unsigned char)c) && c != '_')
                putback(c);
            return lexicalerror("Invalid number format");
        }
        do {
            if (!addchar(&t, &length, c))
                too_long = 1;
            c = mygetchar();
        } while (isdigit((unsigned char)c));
    }
    putback(c);
    if (too_long)
        return lexicalerror("Illegal character/character sequence");
    return t;
}

static struct token stringtoken(void)
{
    struct token t = newtoken(TokenString);
    int length = 0, too_long = 0, bad_character = 0, c;
    addchar(&t, &length, '"');
    while ((c = mygetchar()) != EOF && c != '\n') {
        if (!isprint((unsigned char)c))
            bad_character = 1;
        if (!addchar(&t, &length, c))
            too_long = 1;
        if (c == '"') {
            if (too_long || bad_character)
                return lexicalerror("Illegal character/character sequence");
            return t;
        }
    }
    return lexicalerror("Unterminated string");
}

static struct token symbol(int id, int first, int second)
{
    struct token t = newtoken(id);
    t.lexeme[0] = (char)first;
    if (second != 0) {
        t.lexeme[1] = (char)second;
        t.lexeme[2] = '\0';
    } else {
        t.lexeme[1] = '\0';
    }
    return t;
}

struct token gettoken(void)
{
    int c, next;
    if (source == NULL) {
        tokenline = linenum;
        return newtoken(TokenEndOfFile);
    }
    for (;;) {
        c = mygetchar();
        tokenline = linenum;
        if (c == EOF)
            return newtoken(TokenEndOfFile);
        if (isspace((unsigned char)c))
            continue;
        if (c == '/') {
            next = mygetchar();
            if (next == '/') {
                while ((c = mygetchar()) != EOF && c != '\n')
                    ;
                continue;
            }
            putback(next);
            return symbol(TokenDivide, '/', 0);
        }
        break;
    }

    if (isalpha((unsigned char)c) || c == '_')
        return identifier(c);
    if (isdigit((unsigned char)c))
        return number(c);
    if (c == '"')
        return stringtoken();

    switch (c) {
        case ';': return symbol(TokenSemicolon, c, 0);
        case ':':
            next = mygetchar();
            if (next == '=') return symbol(TokenAssign, c, next);
            putback(next);
            return symbol(TokenColon, c, 0);
        case ',': return symbol(TokenComma, c, 0);
        case '(': return symbol(TokenLeftParen, c, 0);
        case ')': return symbol(TokenRightParen, c, 0);
        case '+': return symbol(TokenPlus, c, 0);
        case '-': return symbol(TokenMinus, c, 0);
        case '*':
            next = mygetchar();
            if (next == '*') return symbol(TokenRaise, c, next);
            putback(next);
            return symbol(TokenMultiply, c, 0);
        case '<':
            next = mygetchar();
            if (next == '=') return symbol(TokenLTEqual, c, next);
            putback(next);
            return symbol(TokenLessThan, c, 0);
        case '=': return symbol(TokenEqual, c, 0);
        case '>':
            next = mygetchar();
            if (next == '=') return symbol(TokenGTEqual, c, next);
            putback(next);
            return symbol(TokenGreaterThan, c, 0);
        case '!':
            next = mygetchar();
            if (next == '=') return symbol(TokenNotEqual, c, next);
            /* The sample recovery consumes one letter after a lone '!'. */
            if (!isalpha((unsigned char)next) && next != '_')
                putback(next);
            return lexicalerror("Illegal character/character sequence");
        default:
            return lexicalerror("Illegal character/character sequence");
    }
}
