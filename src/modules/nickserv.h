#ifndef NICKSERV_H
#define NICKSERV_H

#include <entity.h>
#include <time.h>

typedef struct _nickgroup nickgroup;
typedef struct _nickaccount nickaccount;

typedef struct _nicklist {
    nickaccount *acc;
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
    permclass *class;
};

struct _nickaccount {
    unsigned int nickid;
    char *nick;
    nickgroup *group;
    time_t regtime;
    metanode *metadata;
};

extern user *nickserv;
extern nicklist *registerednicks;
extern nickgrouplist *registerednickgroups;
extern int MODE_NSREGISTER;
extern void registerNickServCommand(char *cmd, void (*callback)(char *uid, char *msg));
extern nickaccount *createNickAccount(char *nick);
extern void deleteNickAccount(nickaccount *acc);
extern nickgroup *createNickGroup(nickaccount *nick, char *pass, char *email);
extern void deleteNickGroup(nickgroup *group);
extern void addNickToGroup(nickaccount *nick, nickgroup *group);
extern void removeNickFromGroup(nickaccount *nick, nickgroup *group);
extern nickgroup *getNickGroupByEmail(char *email);
extern nickgroup *getNickGroupById(unsigned int id);
extern nickaccount *getNickAccountByNick(char *nick);
extern unsigned int generateNickGroupID(void);
extern void addNickServHelp(char *command, char *shorthelp, void (*longhelp)(char *uid, char *msg));
extern void addNickServSetOption(char *option, char *shorthelp, void (*longhelp)(char *uid, char *msg), void (*callback)(char *uid, char *target, char *msg));
char *getLocalTimeString(char *uid, time_t time);
void ns_message(char *uid, char *str, ...);
char hasNickServPermission(char *uid, nickaccount *acc, char *permname);

extern char valid_email(char *email);

#endif /* NICKSERV_H */
