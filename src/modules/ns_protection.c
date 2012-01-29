#include <services.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "nickserv.h"

int guestSeed = 0;
int guestRnd(){/* generate a 4 digit number, max 8191 */
    guestSeed += 0x527;
    guestSeed &= 0x1FFF;
    return guestSeed;
}


void ns_protection_enforce(int argc, char **argv){
    char *guestNick, defaultGuest[] = "Guest", buff[512];
    if(argc<2)
        return;
    guestNick = getConfigValue("NickServGuestNick");
    if(!guestNick)
        guestNick = defaultGuest;
    sprintf(buff, ":%s SVSNICK %s %s%d %d\r\n", getConfigValue("ServerId"), argv[0], guestNick, guestRnd(), (int)time(NULL));
    send_raw_line(buff);
}

void ns_protection_onnick(line *L){
    user *U;
    char *oldNick, *newNick, *waitStr, *tmpconf, defaultWait[] = "", *args[] = {"", ""};
    nickaccount *oldAcc, *newAcc;
    nicklist *nicks;
    int wait;
    U = getUser(L->id);/* U->nick already represents the new nick */
    if(!U)
        return;
    newNick = L->params[0];
    newAcc = getNickAccountByNick(newNick);
    if(!newAcc){
        delMetaValue(&U->metadata, "NICK");
        return;
    }
    oldNick = getMetaValue(&U->metadata, "NICK");/* already identified */
    if(oldNick&&(oldAcc = getNickAccountByNick(oldNick))){
        nicks = oldAcc->group->nicks;
        while(nicks){
            if(nicks->acc == newAcc){
                setMetaValue(&U->metadata, "NICK", newNick);
                aclog(LOG_DEBUG, "%s!%s@%s has automatically identified.", U->nick, U->ident, U->vhost);
                return;
            }
            nicks = nicks->next;
        }
    }
    ns_message(L->id, "This nick is registered. Please identify if this is your nick.");
    delMetaValue(&U->metadata, "NICK");
    if((waitStr = getMetaValue(&newAcc->metadata, "KILLWAIT")))
        wait = (unsigned char)waitStr[0];
    else
        waitStr = defaultWait;
    switch(waitStr[0]){/* still signed */
    case -2:
        return;
    case -1:
        args[0] = L->id;
        args[1] = newNick;
        ns_protection_enforce(2, args);
        break;
    case 0:
        tmpconf = getConfigValue("NickServDefaultProtection");
        if(tmpconf){
            wait = atoi(tmpconf);
            if(wait==-2)
                return;
            if(wait==-1){
                args[0] = L->id;
                args[1] = newNick;
                ns_protection_enforce(2, args);
                break;
            }
        }else{
            wait = 20;
        }
    default:
        addTimerEvent(ns_protection_enforce, time(NULL)+wait, 2, L->id, newNick);
    }
}

void ns_set_kill(char *uid, char *target, char *msg){
    /*
killVal[0]:
-2 = none
-1 = instant
0 = default
1 = 1 sec
2 = 2 sec
...
*/
    char *waitStr, *spaces, *tmpconf, killVal[2];
    nickaccount *acc;
    int wait;
    waitStr = strtok_r(msg, " ", &spaces);
    if(!waitStr){
        ns_message(uid, "Syntax: SET KILL {\x02seconds\x02|\x02OFF\x02|\x02INSTANT\x02|\x02\x44\x45\x41ULT\x02}");
        return;
    }
    acc = getNickAccountByNick(target);
    if(!hasNickServPermission(uid, acc, 2, "SETKILL", "SET")){
        ns_message(uid, "Access denied.");
        return;
    }
    killVal[1] = 0;
    if(!strcasecmp(waitStr, "OFF")){
        killVal[0] = '\xfe';
        ns_message(uid, "Your nick's protection is now off."); 
        setMetaValue(&acc->metadata, "KILLWAIT", killVal);
        return;
    }else if(!strcasecmp(waitStr, "INSTANT")){
        killVal[0] = '\xff';
    }else if(!strcasecmp(waitStr, "DEFAULT")){
        delMetaValue(&acc->metadata, "KILLWAIT");
        ns_message(uid, "Your nick's protection has returned to the default."); 
        return;
    }else{
        wait = atoi(waitStr);
        tmpconf = getConfigValue("NickServMaxKillTime");/* hard max 253 */
        if(tmpconf){
            if(wait>atoi(tmpconf)){
                ns_message(uid, "KILL time cannot be greater than %s seconds.", tmpconf);
                return;
            }
        }
        killVal[0] = wait;
    }
    setMetaValue(&acc->metadata, "KILLWAIT", killVal);
    ns_message(uid, "Your nick's protection will take place after %d seconds.", killVal[0]); 
    
}

void ns_sethelp_kill(char *uid, char *msg){
    ns_message(uid, "Syntax: SET KILL \x02seconds\x02");
}

void INIT_MOD(){
    hook_event(EVENT_NICK, ns_protection_onnick);
    /*TODO: hook_event(EVENT_CONNECT, ns_protection_onconnect);*/
    guestSeed = time(NULL)&0xFFFF;
    addNickServSetOption("KILL", "Sets the time for the user to identify before having their nick changed.", ns_sethelp_kill, ns_set_kill);
}
