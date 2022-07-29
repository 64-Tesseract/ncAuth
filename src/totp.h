#pragma once

#include "base32.c"
#include "hmac.c"

void get_2fa_code(char *secret, char code[7], uint64_t hotp);
