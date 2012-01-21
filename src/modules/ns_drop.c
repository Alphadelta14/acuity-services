#include <services.h>
#include "nickserv.h"

void ns_drop(char *uid, char *msg){
    char *target, *spaces;
    nickaccount *acc;
    nickgroup *group;
    strtok_r(msg, " ", &spaces);/* DROP */
    target = strtok_r(NULL, " ", &spaces);
    acc = getNickAccountByNick(target);
    if(!acc){
        ns_message(uid, "\x02%s\x02 is not registered", target);
        return;
    }
    /* TODO: insert permissions here */
    group = acc->group;
    removeNickFromGroup(acc, group);
    deleteNickAccount(acc);
}

void ns_drop_info(char *uid, char *msg){
    ns_message(uid, "Syntax: DROP \x02nick\x02");
}

void INIT_MOD(){
    registerNickServCommand("drop", ns_drop);
    addNickServHelp("DROP", "Drops a nick from a group", ns_help_drop);
}
