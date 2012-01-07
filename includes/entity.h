#ifndef ENTITY_H
#define ENTITY_H

#include <actypes.h>
#include <time.h>

typedef struct _user user;
typedef struct _chan chan;

typedef struct _usernode {
    user *U;
    struct _usernode *next;
} usernode;
typedef struct _channode {
    chan *C;
    struct _channode *next;
} channode;
typedef struct _statusnode {
    user *U;
    /* same space as char modes[8], but this is so much more logical and consistent */
    unsigned int modeMinor;
    unsigned int modeMajor;
    struct _statusnode *next;
} statusnode;

struct _user {
    char *uid;
    char *nick;
    char *ident;/* I fail to see a use for a vident field */
    char *host;
    char *vhost;
    char *ip;
    char *gecos;
    //account *acc;
    unsigned int modeMinor;
    unsigned int modeMajor;
    channode *chans;
    metanode *metadata;
};

struct _chan {
    char *name;
    char *topic;
    //chanaccount *acc;
    /* banlist? */
    unsigned int modeMinor;
    unsigned int modeMajor;
    statusnode *users;
    metanode *metadata;
};

#endif /* ENTITY_H */
