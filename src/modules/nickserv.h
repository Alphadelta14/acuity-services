#ifndef NICKSERV_H
#define NICKSERV_H

#include <entity.h>
#include <time.h>

typedef struct _nickgroup nickgroup;
typedef struct _nickaccount nickaccount;

typedef struct _nicklist {
    nickaccount *nick;
    struct _nicklist *next;
} nicklist;

typedef struct _nickgrouplist {
    nickgroup *group;
    struct _nickgrouplist *next;
} nickgrouplist;

struct _nickgroup {
    unsigned int groupid;
    unsigned char passwd[32];/* XXX: This does NOT end in a \0 */
    unsigned char passmethod[4];
    char *email;
    nickaccount *main;
    nicklist *nicks;
    metanode *metadata;
};

struct _nickaccount {
    char *nick;
    nickgroup *group;
    time_t regtime;
    metanode *metadata;
};

extern user *nickserv;
extern void registerNickServCommand(char *cmd, void (*callback)(char *uid, char *msg));
extern nickaccount *createNickAccount(char *nick);
extern nickgroup *createNickGroup(nickaccount *nick, char *pass, char *email);
extern void addNickToGroup(nickaccount *nick, nickgroup *group);
extern nickgroup *getNickGroupByEmail(char *email);
extern nickgroup *getNickGroupById(unsigned int id);
extern nickaccount *getNickAccountByNick(char *nick);
extern unsigned int generateNickGroupID(void);

#endif /* NICKSERV_H */
