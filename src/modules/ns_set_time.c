#include <services.h>
#include <string.h>
#include <stdlib.h>
#include "nickserv.h"

/*
http://en.wikipedia.org/wiki/Tz_database
/usr/share/zoneinfo
*/
void ns_set_time_zone(char *uid, nickaccount *acc, char *arg){
    /* TZ Format:
tz[0] (1 char) contains all data:
0  = -12:00
1  = -12:15
2  = -12:30
3  = -12:45
4  = -11:00
8  = -10:00
...
48 = +0:00
52 = +1:00
56 = +2:00
...
96 = +12:00
97 = +12:15
98 = +12:30
99 = +12:45
...
112= +14:00

*/
    char tz[2];
    int offs;
    if(!arg){
        ns_message(uid, "Syntax: SET TIME ZONE {\x02UTC+/-Off:set\x02|\x02\x44\x45\x41ULT\x02}");
        return;
    }
    if(!strcasecmp("DEFAULT", arg)){
        delMetaValue(&acc->group->metadata, "TIMEZONE");
        return;
    }
    if(strncasecmp("UTC", arg, 3)){
        ns_message(uid, "Syntax: SET TIME ZONE {\x02UTC+/-Off:set\x02|\x02\x44\x45\x41ULT\x02}");
        return;
    }
    arg += 3;
    offs = atoi(arg);
    if(offs<-12){
        ns_message(uid, "Timezone offset cannot be any later than UTC-12:00");
        return;
    }
    if(offs>14){
        ns_message(uid, "Timezone offset cannot be any earlier than UTC+14:00");
        return;
    }
    tz[0] = (offs+12)<<2;/* 0 (-12) to 112 (+14) */
    if((strlen(arg)>2)&&(arg[2]==':')){
        offs = atoi(arg+3);
        tz[0] |= ((offs%60)/15);/* 0 = :00, 1 = :15, 2 = :30, 3 = :45 */
    }
    tz[1] = 0;
    setMetaValue(&acc->group->metadata, "TIMEZONE", tz);
    ns_message(uid, "Your timezone has been changed. It is currently %s.", getLocalTimeString(uid, time(NULL))); 
}

void ns_set_time(char *uid, char *target, char *msg){
    char *cmd, *arg, *spaces;
    nickaccount *acc;
    cmd = strtok_r(msg, " ", &spaces);
    arg = strtok_r(NULL, " ", &spaces);
    acc = getNickAccountByNick(target);
    if(!hasNickServPermission(uid, acc, 2, "SETTIME", "SET")){
        ns_message(uid, "Access denied.");
        return;
    }
    if(!cmd){/* apparently strcmp can't handle NULL pointers */
        ns_message(uid, "Syntax: SET TIME \x02{ZONE|DST} arguments\x02");
        return;
    }
    if(!strcasecmp("ZONE", cmd))
        ns_set_time_zone(uid, acc, arg);
    else
        ns_message(uid, "Syntax: SET TIME \x02{ZONE|DST} arguments\x02");
}

void ns_sethelp_time(char *uid, char *msg){
    ns_message(uid, "Syntax: SET TIME \x02\x63ommand arguments\x02");
}

void INIT_MOD(){
    addNickServSetOption("TIME", "Sets time information for a group", ns_sethelp_time, ns_set_time);
}
