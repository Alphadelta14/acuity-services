#ifndef _ENC_H
#define _ENC_H

#include <services.h>

#define ENC_SHA256 0x1

typedef struct {
    unsigned long int total[2];
    unsigned long int state[8];
    unsigned char buffer[64];
} sha256_context;

typedef union context_u {
    sha256_context sha256;
} context_t;

/* Module developers: This is the only relevant function for you here, unless for whatever
 * reason you may have, you intend on generating your own passwords for some kind of
 * secondary auth (OperServ passwords, anyone?). */
bool matchPassword(char *pass, unsigned char passwd[], unsigned char passmethod[]);

/* Usage of the sha256_* functions:
 * - Create a sha256_context (can be uninitialized), then call sha256_stats on it.
 * - Call sha256_update with the context, the password or whatever, and the length of
 *   that string (i.e., strlen(password) or something).
 * - If you do salting, call sha256_update again, this time with the salt and its length.
 * - Call sha256_finish with the context and the unsigned char[32] you want the sha256
 *   hash to be written to.
 */
void sha256_starts( sha256_context *ctx );
void sha256_update( sha256_context *ctx, unsigned char *input, unsigned long int length );
void sha256_finish( sha256_context *ctx, unsigned char digest[32] );

#endif /* _ENC_H */
