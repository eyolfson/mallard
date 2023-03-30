#include "token.h"

#include <string.h>

bool token_equals_c_str(struct token* token, const char* c_str) {
    size_t len = strlen(c_str);
    if (len != token->str.size) {
        return false;
    }
    return strncmp((char *) token->str.data, c_str, len) == 0;
}
