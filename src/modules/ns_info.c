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
    /* TODO: hooks */
}

void ns_help_info(char *uid, char *msg){
    ns_message(uid, "Syntax: INFO \x02nick\x02");
}

void INIT_MOD(){
    registerNickServCommand("info",ns_info);
    addNickServHelp("INFO", "Displays information about a nick", ns_help_info);
}