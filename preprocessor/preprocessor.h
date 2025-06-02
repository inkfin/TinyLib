/// Tinylib Preprocessor
///
/// references:
/// - <https://jorenjoestar.github.io/post/writing_a_simple_code_generator/>
/// - <https://craftinginterpreters.com/contents.html>
///

#ifndef TINYLIB_PREPROCESSOR_H
#define TINYLIB_PREPROCESSOR_H

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

#include <stddef.h>
#include "c_lexer.h"

#define TObject(...)
#define TEnum(...)
#define TString(...)

typedef struct TLPP_State {
    TLPP_lexer *lexer;
} TLPP_State;

extern TLPP_State tlppL_init(void);

extern void tlppL_destroy(TLPP_State* state);

#endif // TINYLIB_PREPROCESSOR_H

