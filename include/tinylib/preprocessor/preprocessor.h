/// Tinylib Preprocessor
///
/// references:
/// - <https://jorenjoestar.github.io/post/writing_a_simple_code_generator/>
/// - <https://craftinginterpreters.com/contents.html>
///

///
/// Configurations:
/// ================
///
/// TLPP_DEBUG
/// TLPP_VERBOSE # For debugging purposes
/// TLPP_RULES_FILE
/// TLPP_OUTPUT_DIR
///
#ifndef TLPP_RULES_FILE
#error "TLPP_RULES_FILE is not defined. Please define it in your build system."
#endif
#ifndef TLPP_OUTPUT_DIR
#error "TLPP_OUTPUT_DIR is not defined. Please define it in your build system."
#endif

#ifdef TINYLIB_PREPROCESSOR_IMPLEMENTATION
// If you want to customize:
#ifndef TINYLIB_PREPROCESSOR_CONFIGURATION
#define TLPP_WHITE_SPACE " \t\n\r"
#define TLPP_COMMENT_START "/\*"
#define TLPP_COMMENT_END "*\/"
#define TLPP_SINGLE_LINE_COMMENT "//"
#define TLPP_STRING_START "\""
#define TLPP_STRING_END "\""
#endif // TINYLIB_PREPROCESSOR_CONFIGURATION
#endif // TINYLIB_PREPROCESSOR_IMPLEMENTATION

#ifndef TINYLIB_PREPROCESSOR_H
#define TINYLIB_PREPROCESSOR_H

#include <stddef.h>
// #include "stb/stb_c_lexer.h"

#define TLPP_MAX_LINE_LENGTH 1024

#define TObject(...)
#define TEnum(...)
#define TString(...)

enum
{
   CLEX_eof = 256,
   CLEX_parse_error,
   CLEX_intlit        ,
   CLEX_floatlit      ,
   CLEX_id            ,
   CLEX_dqstring      ,
   CLEX_sqstring      ,
   CLEX_charlit       ,
   CLEX_eq            ,
   CLEX_noteq         ,
   CLEX_lesseq        ,
   CLEX_greatereq     ,
   CLEX_andand        ,
   CLEX_oror          ,
   CLEX_shl           ,
   CLEX_shr           ,
   CLEX_plusplus      ,
   CLEX_minusminus    ,
   CLEX_pluseq        ,
   CLEX_minuseq       ,
   CLEX_muleq         ,
   CLEX_diveq         ,
   CLEX_modeq         ,
   CLEX_andeq         ,
   CLEX_oreq          ,
   CLEX_xoreq         ,
   CLEX_arrow         ,
   CLEX_eqarrow       ,
   CLEX_shleq, CLEX_shreq,

   CLEX_first_unused_token

};

#endif // TINYLIB_PREPROCESSOR_H

#ifdef TINYLIB_PREPROCESSOR_IMPLEMENTATION

/// definitions for internal use
#define EQLSTRCHAR(a, b) L


#endif // TINYLIB_PREPROCESSOR_IMPLEMENTATION
