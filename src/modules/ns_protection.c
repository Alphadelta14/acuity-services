#include <services.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "nickserv.h"

int guestSeed = 0;
int guestRnd(){/* generate a 16 bit number */
    guestSeed += 0x527;
    guestSeed &= 0xFFFF;
    return guestSeed;
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
    guestSeed = time(NULL)&0xFFFF;
    addNickServSetOption("KILL", "Sets the time for the user to identify before having their nick changed.", ns_sethelp_kill, ns_set_kill);
}
