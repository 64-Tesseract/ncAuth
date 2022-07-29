#pragma once

#include "sha1.c"
#include "str.c"

void hmac(
    void *dest,
    const void *key,
    uint32_t key_len,
    const void *msg,
    uint32_t msg_len,
    void (*hash)(char *dest, const char *str, int len),
    uint16_t block_size,
    uint16_t output_size
);