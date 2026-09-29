/* One token returned by gettoken(). ID zero represents a lexical error. */
struct token {
    int id;
    char lexeme[1000];
};

extern const char *tokennames[];

#define TokenError          0
#define TokenIdentifier     1
#define TokenNumber         2
#define TokenString         3
#define TokenAssign         4
#define TokenSemicolon      5
#define TokenColon          6
#define TokenComma          7
#define TokenLeftParen      8
#define TokenRightParen     9
#define TokenPlus          10
#define TokenMinus         11
#define TokenMultiply      12
#define TokenDivide        13
#define TokenRaise         14
#define TokenLessThan      15
#define TokenEqual         16
#define TokenGreaterThan   17
#define TokenLTEqual       18
#define TokenGTEqual       19
#define TokenNotEqual      20
#define TokenEndOfFile     21
#define TokenPrint         22
#define TokenIf            23
#define TokenElse          24
#define TokenEndif         25
#define TokenSqrt          26
#define TokenAnd           27
#define TokenOr            28
#define TokenNot           29
