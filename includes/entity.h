#ifndef _ENTITY_H
#define _ENTITY_H

#include "services.h"

typedef struct user_s user_t;
typedef struct chan_s chan_t;

typedef struct usernode_s {
    user_t *user;
    struct usernode_s *next;
} usernode_t;

typedef struct channode_s {
    chan_t *chan;
    struct channode_s *next;
} channode_t;

typedef struct modes_s {
    int minor;
    int major;
} modes_t;

typedef struct statusnode_s {
    modes_t mode;
    user_t *user;
    struct statusnode_s *next;
} statusnode_t;

struct _user {
    char *uid;/* unique identifier */
    char *nick;
    char *ident;
    char *host;
    char *vhost;
    char *ip;
    char *gecos;
    modes_t mode;
    channode_t *chanlist;
    metanode_t *metadata;
};

struct _chan {
    /* char *cid; unique identifierd, but not used...
        Maybe for & channels? Not sure what allows multiple channels with
        same names */
    char *name;
    char *topic;
    modes_t mode;
    /* banlist? */
    statusnode_t *userlist;
    metanode_t *metadata;
};

#endif /* _ENTITY_H */
