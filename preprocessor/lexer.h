/// TinyLib lexer
///
///   This is the lexer for the TinyLib Preprocessor. It is based on stb_lexer.h.
///
/// references:
/// - <https://github.com/nothings/stb/blob/802cd454f25469d3123e678af41364153c132c2a/stb_lexer.h>
///

#ifndef TINYLIB_PREPROCESSOR_LEXER_H
#define TINYLIB_PREPROCESSOR_LEXER_H

///
/// Configurations:
/// ================
///

#define TLPP_EOL_LIST '\n','\r'
#define TLPP_WHITE_SPACE_LIST ' ','\t', '\v', '\f','\n','\r'
// #define TLPP_IS_LINE_COMMENT ... // Default: "//"
// #define TLPP_IS_BLOCK_COMMENT_START ... // Default: "/*"
// #define TLPP_IS_BLOCK_COMMENT_END ... // Default: "*/"
#define TLPP_STRING_START "\""
#define TLPP_STRING_END "\""

typedef struct {
   // lexer variables
   char *input_stream;
   char *eof;
   char *parse_point;
   char *string_storage;
   int   string_storage_len;

   // lexer parse location for error messages
   char *where_firstchar;
   char *where_lastchar;

   // lexer token variables
   long   token;
   double real_number;
   long   int_number;
   int    string_len;
   char   *string;
} TLPP_lexer;

enum {
    TLPP_EOF = 256,
    TLPP_INTLIT,
    TLPP_FLOATLIT,
    TLPP_ID,
    TLPP_DQSTRING,
    TLPP_SQSTRING,
    TLPP_CHARLIT, // Char literal
    TLPP_EQ,
    TLPP_NOTEQ,
    TLPP_LESSEQ,
    TLPP_GREATEREQ,
    TLPP_ANDAND,
    TLPP_OROR,
    TLPP_SHL,
    TLPP_SHR,
    TLPP_PLUSPLUS,
    TLPP_MINUSMINUS,
    TLPP_PLUSEQ,
    TLPP_MINUSEQ,
    TLPP_MULEQ,
    TLPP_DIVEQ,
    TLPP_MODEQ,
    TLPP_ANDEQ,
    TLPP_OREQ,
    TLPP_XOREQ,
    TLPP_ARROW,
    TLPP_EQARROW,
    TLPP_DECLARE,
    TLPP_SHLEQ,
    TLPP_SHREQ,

    TLPP_FIRST_UNUSED_TOKEN,

    // error tokens
    TLPP_PARSE_ERROR,
    TLPP_UNEXPECTED_EOF,
    TLPP_OUT_OF_MEMORY,
};

#ifdef __cplusplus
extern "C" {
#endif

/// Initilize the lexer struct
extern void tlpp_lexer_init(
    TLPP_lexer *lexer, //< lexer object
    const char *input_stream, //< input stream
    const char *input_stream_end, //< end of the input stream
    char       *string_store, //> string storage
    int         store_length //< length of the string storage
);

/// Parse the next valid token from the input stream, return 0 on EOF
extern int tlpp_parse_next_token(TLPP_lexer *lexer);

#ifdef __cplusplus
}
#endif

#ifdef TLPP_DEBUG
#include <stdio.h>

static void print_token(TLPP_lexer *lexer)
{
   switch (lexer->token) {
      case TLPP_ID        : printf("_%s", lexer->string); break;
      case TLPP_EQ        : printf("=="); break;
      case TLPP_NOTEQ     : printf("!="); break;
      case TLPP_LESSEQ    : printf("<="); break;
      case TLPP_GREATEREQ : printf(">="); break;
      case TLPP_ANDAND    : printf("&&"); break;
      case TLPP_OROR      : printf("||"); break;
      case TLPP_SHL       : printf("<<"); break;
      case TLPP_SHR       : printf(">>"); break;
      case TLPP_PLUSPLUS  : printf("++"); break;
      case TLPP_MINUSMINUS: printf("--"); break;
      case TLPP_ARROW     : printf("->"); break;
      case TLPP_ANDEQ     : printf("&="); break;
      case TLPP_OREQ      : printf("|="); break;
      case TLPP_XOREQ     : printf("^="); break;
      case TLPP_PLUSEQ    : printf("+="); break;
      case TLPP_MINUSEQ   : printf("-="); break;
      case TLPP_MULEQ     : printf("*="); break;
      case TLPP_DIVEQ     : printf("/="); break;
      case TLPP_MODEQ     : printf("%%="); break;
      case TLPP_SHLEQ     : printf("<<="); break;
      case TLPP_SHREQ     : printf(">>="); break;
      case TLPP_EQARROW   : printf("=>"); break;
      case TLPP_DECLARE   : printf(":="); break;
      case TLPP_DQSTRING  : printf("\"%s\"", lexer->string); break;
      case TLPP_SQSTRING  : printf("'\"%s\"'", lexer->string); break;
      case TLPP_CHARLIT   : printf("'%s'", lexer->string); break;
      case TLPP_INTLIT    : printf("#%ld", lexer->int_number); break;
      case TLPP_FLOATLIT  : printf("%g", lexer->real_number); break;
      default:
         if (lexer->token >= 0 && lexer->token < 256)
            printf("%c", (int) lexer->token);
         else {
            printf("<<<UNKNOWN TOKEN %ld >>>\n", lexer->token);
         }
         break;
   }
}

#else

static void print_token(TLPP_lexer *lexer)
{
   // No-op
}

#endif // TLPP_DEBUG

#endif // TINYLIB_PREPROCESSOR_LEXER_H
