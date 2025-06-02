#include "preprocessor.h"
#include <stdlib.h>

TLPP_State tlppL_init(void) {
    TLPP_State state;
    tlpp_lexer_init(state.lexer, NULL, NULL, NULL, 0);
    return state;
}

void tlppL_destroy(TLPP_State* state) {
    if (state->lexer) {
        free(state->lexer->string_storage);
        free(state->lexer);
    }
}

