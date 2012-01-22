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
    safenmallocvoid(grp->email, char, strlen(email)+1);
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

void ns_set_nick(char *uid, char *target, char *msg){
    /* changes NiCk caSiNg */
    char *newnick, *spaces;
    nickaccount *acc;
    newnick = strtok_r(msg, " ", &spaces);
    acc = getNickAccountByNick(target);
    if(!hasNickServPermission(uid, acc, 2, "SETNICK", "SET")){
        ns_message(uid, "Access denied.");
        return;
    }
    if(irccasecmp(acc->nick, newnick)){
        ns_message(uid, "Nicks need to match.");
        return;
    }
    free(acc->nick);
    safenmallocvoid(acc->nick, char, strlen(newnick)+1);
    strcpy(acc->nick, newnick);
    ns_message(uid, "\x02%s\x02 is your new nick.", newnick);
}

void ns_sethelp_nick(char *uid, char *msg){
    ns_message(uid, "Syntax: SET NICK \x02nIcK\x02");
}


void INIT_MOD(){
    addNickServSetOption("EMAIL", "Sets an email for the group", ns_sethelp_email, ns_set_email);
    addNickServSetOption("PASSWORD", "Sets a password for the group", ns_sethelp_password, ns_set_password);
    addNickServSetOption("NICK", "Changes casing for a nick", ns_sethelp_nick, ns_set_nick);
}
