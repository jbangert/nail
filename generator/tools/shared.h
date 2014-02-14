#include <sstream>
#include <list>
struct Token {
        int type;
        std::string data;
};
std::list<Token *>* lexer(FILE *);
extern const char *tokennames[];

void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  std::stringstream * yyminor,       /* The value for the token */
  std::string *output
);
void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
);
void ParseTrace(FILE *TraceFILE, char *zTracePrompt);
