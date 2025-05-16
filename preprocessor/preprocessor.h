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
#ifndef TLPP_RULES_FILE
#error "TLPP_RULES_FILE is not defined. Please define it in your build system."
#endif
#ifndef TLPP_OUTPUT_DIR
#error "TLPP_OUTPUT_DIR is not defined. Please define it in your build system."
#endif

#include <stddef.h>

#endif // TINYLIB_PREPROCESSOR_H