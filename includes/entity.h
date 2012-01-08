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
    int modeMinor;
    int modeMajor;
    user *U;
    struct _statusnode *next;
} statusnode;

struct _user {
    /* see note below (chan) on modes */
    int modeMinor;
    int modeMajor;
    char *uid;
    char *nick;
    char *ident;/* I fail to see a use for a vident field */
    char *host;
    char *vhost;
    char *ip;
    char *gecos;
    //account *acc;
    channode *chans;
    metanode *metadata;
};

struct _chan {
    /* modeMinor and modeMajor go first in the node so that a channel can be cast to a user when setting modes.
        U = (user*)somechan; U->modeMinor will exist for both */
    int modeMinor;
    int modeMajor;
    char *name;
    char *topic;
    //chanaccount *acc;
    /* banlist? */
    statusnode *users;
    metanode *metadata;
};

#endif /* ENTITY_H */
