#include <services.h>
#include <stdlib.h>
#include <string.h>
#include "nickserv.h"

void ns_set_email(char *uid, char *target, char *msg){
    char *email, *spaces;
    nickaccount *acc;
    nickgroup *grp;
    email = strtok_r(msg, " ", &spaces);
    acc = getNickAccountByNick(target);
    if(!hasNickServPermission(uid, acc, 2, "SETEMAIL", "SET")){
        ns_message(uid, "Access denied.");
        return;
    }
    if(!valid_email(email)){
        ns_message(uid, "\x02%s\x02 is an invalid email.", email);
        return;
    }
    if(getNickGroupByEmail(email)){
        ns_message(uid, "\x02%s\x02 is already in use.", email);
        return;
    }
    grp = acc->group;
    free(grp->email);
    safenmalloc(grp->email, char, strlen(email)+1, );
    strcpy(grp->email, email);
    ns_message(uid, "\x02%s\x02 is your new email.", email);
}

void ns_sethelp_email(char *uid, char *msg){
    ns_message(uid, "Syntax: SET EMAIL\x02 email\x02");/* XXX: HAX here. How do you get \x02 next to an 'e'? (turns into \x02e by default)s */
}


void INIT_MOD(){
    addNickServSetOption("EMAIL", "Sets an email for the group", ns_sethelp_email, ns_set_email);
}
