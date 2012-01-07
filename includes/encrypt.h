#ifndef ENC_H
#define ENC_H

#include "actypes.h"

#define ENC_SHA256 0x1

typedef struct {
    unsigned long int total[2];
    unsigned long int state[8];
    unsigned char buffer[64];
} sha256_context;

void sha256_starts( sha256_context *ctx );
void sha256_update( sha256_context *ctx, unsigned char *input, unsigned long int length );
void sha256_finish( sha256_context *ctx, unsigned char digest[32] );

#endif /* ENC_H */
