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

void ns_drophelp(char *uid, char *msg){
    ns_message(uid,
        "Syntax: DROP nick\n"
        " \n"
        "Drops a nick. This deletes the entire account associated with it. You\n"
        "will want to be very cautious with this command!");
}

void INIT_MOD(){
    registerNickServCommand("drop", ns_drop);
    addNickServHelp("DROP", "Drops a nick from a group", ns_drophelp);
}
