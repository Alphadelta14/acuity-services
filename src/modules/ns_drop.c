#include <services.h>
#include "nickserv.h"
#include <string.h>

void ns_drop(char *uid, char *msg){
    char *target, *spaces;
    nickaccount *acc;
    strtok_r(msg, " ", &spaces);/* DROP */
    target = strtok_r(NULL, " ", &spaces);
    acc = getNickAccountByNick(target);
    if(!acc){
        ns_message(uid, "\x02%s\x02 is not registered", target);
        return;
    }
    /* TODO: insert permissions here */
    deleteNickAccount(acc);
    ns_message(uid, "\x02%s\x02 has been dropped", target);
}

void ns_help_drop(char *uid, char *msg){
    ns_message(uid, "Syntax: DROP \x02nick\x02");
}

void INIT_MOD(){
    registerNickServCommand("drop", ns_drop);
    addNickServHelp("DROP", "Drops a nick from a group", ns_help_drop);
}
