#include <services.h>
#include <encrypt.h>
#include <stdlib.h>
#include <string.h>
#include "nickserv.h"
#include <encrypt.h>

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
    ns_message(uid, "Syntax: SET EMAIL \x02\x65mail\x02");
}

void ns_set_password(char *uid, char *target, char *msg){
    char *pass, *spaces, *minpassconf;
    nickaccount *acc;
    nickgroup *grp;
    sha256_context ctx;
    pass = strtok_r(msg, " ", &spaces);
    acc = getNickAccountByNick(target);
    if(!hasNickServPermission(uid, acc, 2, "SETPASSWORD", "SET")){
        ns_message(uid, "Access denied.");
        return;
    }
    minpassconf = getConfigValue("NickServMinPasslen");
    if(minpassconf){
        if(strlen(pass)<atoi(minpassconf)){
            ns_message(uid, "Password must be at least %s characters long.", minpassconf);
            return;
        }
    }
    grp = acc->group;
    if(grp->passmethod[3] == ENC_SHA256){
        sha256_starts(&ctx);
        sha256_update(&ctx,(unsigned char*)pass,strlen(pass));
        sha256_update(&ctx,grp->passmethod,3);
        sha256_finish(&ctx,grp->passwd);
    }else{
        aclog(LOG_ERROR, "Passmethod for %s is not SHA256", acc->nick);
    }
    ns_message(uid, "Your new password is \x02%s\x02. Please remember this for later use.", pass);
}

void ns_sethelp_password(char *uid, char *msg){
    ns_message(uid, "Syntax: SET PASSWORD \x02new password\x02");
}


void INIT_MOD(){
    addNickServSetOption("EMAIL", "Sets an email for the group", ns_sethelp_email, ns_set_email);
    addNickServSetOption("PASSWORD", "Sets a password for the group", ns_sethelp_password, ns_set_password);
}
