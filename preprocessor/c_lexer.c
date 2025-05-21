#include "c_lexer.h"

#include <stdlib.h>
#include "tinylib/macrohelper.h"

static inline bool tlpp_is_whitespace(char c) {
    return TL_FOREACH(c ==, ||, TLPP_WHITE_SPACE_LIST);
}

static inline bool tlpp_is_eol(char c) {
    return TL_FOREACH(c ==, ||, TLPP_EOL_LIST);
}

static inline bool tlpp_is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool tlpp_is_digit(char c) {
    return (c >= '0' && c <= '9');
}

static inline bool tlpp_is_line_comment(const char *s) {
    return ((s)[0] == '/' && (s)[1] == '/');
}

static inline bool tlpp_is_block_comment_start(const char *s) {
    return ((s)[0] == '/' && (s)[1] == '*');
}

static inline bool tlpp_is_block_comment_end(const char *s) {
    return ((s)[0] == '*' && (s)[1] == '/');
}

// helper function to set the token and its location, returns the token
static inline int tlpp__set_token(TLPP_lexer *lexer, int token, char *start, char *end) {
    lexer->token = token;
    lexer->where_firstchar = start;
    lexer->where_lastchar = end;
    lexer->parse_point = end + 1;
    return token;
}

// helper function to parse next char
static inline int tlpp__parse_char(char *p, char **const q)
{
    if (*p == '\\') {
        *q = p+2; // tentatively guess we'll parse two characters
        switch(p[1]) {
            case '\\': return '\\';
            case '\'': return '\'';
            case '"':  return '"';
            case 't':  return '\t';
            case 'f':  return '\f';
            case 'n':  return '\n';
            case 'r':  return '\r';
            case '0':  return '\0'; // TODO: ocatal constants

            case 'x': case 'X': return -1; // TODO: hex constants
            case 'u': return -1; // TODO: unicode constants
        }
    }
    *q = p+1;
    return (unsigned char) *p;
}

static inline int tlpp__parse_string(TLPP_lexer *lexer, char* p) {
    char* start = p;
    char  delim = *p; ++p; // grab the " or ' for later matching
    char* outbeg = lexer->string_storage;
    char* outend = lexer->string_storage + lexer->string_storage_len;
    while (*p != delim) {
        int n;
        if (*p == '\\') {
            char* q;
            n = tlpp__parse_char(p, &q);
            if (n < 0)
                return tlpp__set_token(lexer, TLPP_PARSE_ERROR, start, q);
            p = q;
        } else {
            // OPTIMIZE: could speed this up by looping-while-not-backslash
            n = (unsigned char)*p; ++p;
        }
        if (outbeg >= outend)
            return tlpp__set_token(lexer, TLPP_OUT_OF_MEMORY, start, p);
        // TODO: expand unicode escapes to UTF8
        *outbeg++ = (char)n;
    }
    *outbeg = 0;
    lexer->string = lexer->string_storage;
    lexer->string_len = (int)(outbeg - lexer->string_storage);
    return tlpp__set_token(lexer, TLPP_DQSTRING, start, p);
}


// API functions

void tlpp_lexer_init(TLPP_lexer* lexer, const char* input_stream, const char* input_stream_end, char* string_store, int store_length)
{
    lexer->input_stream = (char*)input_stream;
    lexer->eof = (char*)input_stream_end;
    lexer->parse_point = (char*)input_stream;
    lexer->string_storage = string_store;
    lexer->string_storage_len = store_length;
}

int tlpp_parse_next_token(TLPP_lexer *lexer) {
   char *p = lexer->parse_point;

   while(true) {
        while(p != lexer->eof && tlpp_is_whitespace(*p)) {
            ++p;
        }

        // skip comments
        if (p != lexer->eof && tlpp_is_line_comment(p)) {
            while (p != lexer->eof && *p != '\r' && *p != '\n')
                ++p;
            continue;
        }

        if (p != lexer->eof && tlpp_is_block_comment_start(p)) {
           char *start = p;
           p += 2;
           while (p != lexer->eof && !tlpp_is_block_comment_end(p))
              ++p;
           if (p == lexer->eof)
              return tlpp__set_token(lexer, TLPP_UNEXPECTED_EOF, start, p-1);
           p += 2;
           continue;
        }

        // TODO: check for multi-line #define

        break;
    }

    if (p == lexer->eof) {
        lexer->token = TLPP_EOF;
        return 0;
    }

    switch (*p) {
    default:
        if (tlpp_is_alpha(*p) || *p == '_'
            || (unsigned char)*p >= 128 // >= 128 is UTF8 char
            || *p == '$') {
            int i = 0;
            lexer->string = lexer->string_storage;
            do {
                if (i + 1 >= lexer->string_storage_len)
                    return tlpp__set_token(lexer, TLPP_OUT_OF_MEMORY, p, p + i);
                lexer->string[i] = p[i];
                ++i;
            } while (tlpp_is_alpha(p[i]) || p[i] == '_'
                || (unsigned char)p[i] >= 128 // >= 128 is UTF8 char
                || p[i] == '$'
                || tlpp_is_digit(p[i])// allow digits in middle of identifie
            );

            lexer->string[i] = 0; // \0 end of string
            lexer->string_len = i;
            return tlpp__set_token(lexer, TLPP_ID, p, p + i - 1);
        }

        // check for EOF ('\0')
        if (*p == 0) {
            lexer->token = TLPP_EOF;
            return 0;
        }

    single_char:
        // not an identifier, return the character as itself
        return tlpp__set_token(lexer, *p, p, p);

    case '+':
        if (p + 1 != lexer->eof) {
            if (p[1] == '+') return tlpp__set_token(lexer, TLPP_PLUSPLUS, p, p + 1);
            if (p[1] == '=') return tlpp__set_token(lexer, TLPP_PLUSEQ, p, p + 1);
        }
        goto single_char;
    case '-':
        if (p + 1 != lexer->eof) {
            if (p[1] == '-') return tlpp__set_token(lexer, TLPP_MINUSMINUS, p, p + 1);
            if (p[1] == '=') return tlpp__set_token(lexer, TLPP_MINUSEQ, p, p + 1);
            // STB_C_LEX_C_ARROW(if (p[1] == '>') return tlpp_set_token(lexer, TLPP_ARROW, p, p + 1);)
        }
        goto single_char;
    case '&':
        if (p + 1 != lexer->eof) {
            if (p[1] == '&') return tlpp__set_token(lexer, TLPP_ANDAND, p, p + 1);
            // if (p[1] == '=') return tlpp_set_token(lexer, TLPP_ANDEQ, p, p + 1);
        }
        goto single_char;
    case '|':
        if (p + 1 != lexer->eof) {
            if (p[1] == '|') return tlpp__set_token(lexer, TLPP_OROR, p, p + 1);
            // if (p[1] == '=') return tlpp_set_token(lexer, TLPP_OREQ, p, p + 1);
        }
        goto single_char;
    case '=':
        if (p + 1 != lexer->eof) {
            if (p[1] == '=') return tlpp__set_token(lexer, TLPP_EQ, p, p + 1);
            // if (p[1] == '>') return tlpp_set_token(lexer, TLPP_EQARROW, p, p + 1); // =>
        }
        goto single_char;
    case '!':
        if (p + 1 != lexer->eof && p[1] == '=') return tlpp__set_token(lexer, TLPP_NOTEQ, p, p + 1);
        goto single_char;
    case '^':
        // if (p + 1 != lexer->eof && p[1] == '=') return tlpp_set_token(lexer, TLPP_XOREQ, p, p + 1));
        goto single_char;
    case '%':
        if (p + 1 != lexer->eof && p[1] == '=') return tlpp__set_token(lexer, TLPP_MODEQ, p, p + 1);
        goto single_char;
    case '*':
        if (p + 1 != lexer->eof && p[1] == '=') return tlpp__set_token(lexer, TLPP_MULEQ, p, p + 1);
        goto single_char;
    case '/':
        if (p + 1 != lexer->eof && p[1] == '=') return tlpp__set_token(lexer, TLPP_DIVEQ, p, p + 1);
        goto single_char;
    case '<':
        if (p + 1 != lexer->eof) {
            if (p[1] == '=') return tlpp__set_token(lexer, TLPP_LESSEQ, p, p + 1);
            // if (p[1] == '<') {
            //     if (p + 2 != lexer->eof && p[2] == '=') return tlpp_set_token(lexer, TLPP_SHLEQ, p, p + 2);
            //     return tlpp_set_token(lexer, TLPP_SHL, p, p + 1);
            // }
        }
        goto single_char;
    case '>':
        if (p + 1 != lexer->eof) {
            if (p[1] == '=') return tlpp__set_token(lexer, TLPP_GREATEREQ, p, p + 1);
            // if (p[1] == '>') {
            //     if (p + 2 != lexer->eof && p[2] == '=') return tlpp_set_token(lexer, TLPP_SHREQ, p, p + 2);
            //     return tlpp_set_token(lexer, TLPP_SHR, p, p + 1);
            // }
        }
        goto single_char;

    case '"':
        return tlpp__parse_string(lexer, p);

    case '\'':
        {
            char* start = p;
            lexer->int_number = tlpp__parse_char(p + 1, &p);
            if (lexer->int_number < 0)
                return tlpp__set_token(lexer, TLPP_PARSE_ERROR, start, start);
            if (p == lexer->eof || *p != '\'')
                return tlpp__set_token(lexer, TLPP_PARSE_ERROR, start, p);
            return tlpp__set_token(lexer, TLPP_CHARLIT, start, p + 1);
        }
        goto single_char;

    /* FALL THROUGH */
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            char* q = p;
            while (q != lexer->eof && (*q >= '0' && *q <= '9'))
                ++q;
            if (q != lexer->eof) {
                if (*q == '.' || *q == 'e' || *q == 'E') {
                    lexer->real_number = strtod((char*)p, (char**)&q);
                    return tlpp__set_token(lexer, TLPP_FLOATLIT, p, q-1);
                }
            }
        }
        {
            char* q = p;
            lexer->int_number = strtol((char*)p, (char**)&q, 10);
            return tlpp__set_token(lexer, TLPP_INTLIT, p, q-1);
        }
        goto single_char;
    }
}


