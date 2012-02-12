#include <services.h>
#include "nickserv.h"
#include <string.h>
#include <time.h>

void ns_info(char *uid, char *msg){
    char *target, *spaces;
    nickaccount *acc;
    nickgroup *group;
    strtok_r(msg, " ", &spaces);/* INFO */
    target = strtok_r(NULL, " ", &spaces);
    acc = getNickAccountByNick(target);
    if(!acc){
        ns_message(uid, "\x02%s\x02 is not registered", target);
        return;
    }
    /* TODO: hasNickServPermission(uid, acc, 1, "INFO")), etc */
    group = acc->group;
    ns_message(uid, "Information about \x02%s\x02:", acc->nick);
    ns_message(uid, "Email:      \x02%s\x02", group->email);
    ns_message(uid, "Group:      \x02%s\x02", group->main->nick);
    ns_message(uid, "Registered: \x02%s\x02", getLocalTimeString(uid, acc->regtime));
    /* TODO: hooks */
}

void ns_infohelp(char *uid, char *msg){
    ns_message(uid,
        "Syntax: INFO nick\n"
        " \n"
        "The INFO command allows you to see whether a nick is registered, its\n"
        "e-mail address and to what group it belongs.");
}

void ns_ginfo(char *uid, char *msg){
    char *target, *spaces;
    nickaccount *acc;
    nickgroup *group;
    nicklist *nicks;
    int count = 0;
    strtok_r(msg, " ", &spaces);/* GINFO */
    target = strtok_r(NULL, " ", &spaces);
    acc = getNickAccountByNick(target);
    if(!acc){
        ns_message(uid, "\x02%s\x02 is not registered", target);
        return;
    }
    /* TODO: hasNickServPermission(uid, acc, 2, "GINFO", "INFO")), etc */
    group = acc->group;
    ns_message(uid, "Main nick:  \x02%s\x02", group->main->nick);
    ns_message(uid, "Email:      \x02%s\x02", group->email);
    ns_message(uid, "Members: ");
    nicks = group->nicks;
    while(nicks){
        count++;
        ns_message(uid, "     %d:     \x02%s\x02", count, nicks->acc->nick);
        nicks = nicks->next;
    }
}

void ns_ginfohelp(char *uid, char *msg){
    ns_message(uid,
        "Syntax: GINFO group\n"
        " \n"
        "The GINFO command allows you to see detailed information about a\n"
        "nick group, such as its registered nicks and e-mail address.");
}

void INIT_MOD(){
    registerNickServCommand("info",ns_info);
    addNickServHelp("INFO", "Displays information about a nick", ns_infohelp);
    registerNickServCommand("ginfo",ns_ginfo);
    addNickServHelp("GINFO", "Displays information about a group", ns_ginfohelp);
}
