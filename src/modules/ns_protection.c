#include <services.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "nickserv.h"

int guestSeed = 0;
int guestRnd(){/* generate a 4 digit number, max 8191 */
    int seed = 6;
    guestSeed += 0x527;
    guestSeed ^= (int)(&seed)[-1];/* lol, randomness of memory (run time stack) contents;
    note: this is most likely a chunk of an address of a config value pointer.
    (Lot's of internal work will show that). Either way, random enough! */
    guestSeed &= 0x1FFF;
    return guestSeed;
}


void ns_protection_enforce(int argc, char **argv){
    char *guestNick, guestFullNick[32], defaultGuest[] = "Guest", buff[512], *newNick;
    user *U;
    if(argc<2)
        return;
    U = getUser(argv[0]);
    if(!U)
        return;/* logged out */
    if(irccasecmp(argv[1],U->nick))
        return;/* changed nick */
    if((newNick = getMetaValue(&U->metadata, "NICK"))&&(!irccasecmp(argv[1],newNick)))
        return;/* identified */
    guestNick = getConfigValue("NickServGuestNick");
    if(!guestNick)
        guestNick = defaultGuest;
    sprintf(guestFullNick, "%s%d", guestNick, guestRnd());
    while(!getUserByNick(guestFullNick)){
        sprintf(guestFullNick, "%s%d", guestNick, guestRnd());
    }
    sprintf(buff, ":%s SVSNICK %s %s %d\r\n", getConfigValue("ServerId"), argv[0], guestFullNick, (int)time(NULL));
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
    newNick = U->nick;
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

void ns_protection_onconnect(line *L){
    user *U;
    char *uid, *newNick, *waitStr, *tmpconf, defaultWait[] = "", *args[] = {"", ""};
    nickaccount *newAcc;
    int wait;
    uid = L->params[0];
    U = getUser(uid);
    if(!U)
        return;
    newNick = U->nick;
    newAcc = getNickAccountByNick(newNick);
    if(!newAcc)
        return;
    ns_message(uid, "This nick is registered. Please identify if this is your nick.");
    if((waitStr = getMetaValue(&newAcc->metadata, "KILLWAIT")))
        wait = (unsigned char)waitStr[0];
    else
        waitStr = defaultWait;
    switch(waitStr[0]){
    case -2:
        return;
    case -1:
        args[0] = uid;
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
                args[0] = uid;
                args[1] = newNick;
                ns_protection_enforce(2, args);
                break;
            }
        }else{
            wait = 20;
        }
    default:
        addTimerEvent(ns_protection_enforce, time(NULL)+wait, 2, uid, newNick);
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
    if(!hasNickServPermission(uid, acc, "ns.set.kill")){
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
    char *tmpconf;
    ns_message(uid,
            "Syntax: SET KILL seconds\n"
            " \n"
            "Changes the amount of time until someone using your nick(s) will\n"
            "have their nick forcibly changed if they don't identify.");
    tmpconf = getConfigValue("NickServMaxKillTime");
    if(tmpconf){
        ns_message(uid,
            "You can set your kill time to a maximum value of %s.", tmpconf);
    }else{
        ns_message(uid,
            "You can set your kill time to a maximum value of 253.");
    }
    /* TODO: Stop being a lazy fuck and show the default time, and convert to
     * meaningful text values in INSTANT
     */
    ns_message(uid,
            "Special values for seconds:\n"
            "DEFAULT: returns you to the default kill time.\n"
            "INSTANT: immediately gets rid of anyone using your nick. This\n"
            "             is a potentially dangerous setting! Do not use it if\n"
            "             you aren't 100\% sure what you're doing!\n"
            "OFF:     Turns off nick protection; anyone can then use your\n"
            "             nick, but they will get a warning that the nick\n"
            "             belongs to someone else.");
}

void INIT_MOD(){
    addPermission("ns.set.kill");
    hook_event(EVENT_NICK, ns_protection_onnick);
    hook_event(EVENT_CONNECT, ns_protection_onconnect);
    guestSeed = time(NULL)&0xFFFF;
    addNickServSetOption("KILL", "Sets the time for the user to identify before having their nick changed.", ns_sethelp_kill, ns_set_kill);
}
