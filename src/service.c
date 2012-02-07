#include <services.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

/* reserved variables for time functions */
char timeZoneStr[40];/*"{$ASCTIME(26)} UTC+12:00"; it will be overwritten, with every timezone get call */
char defaultTZ = '\x30';

user *createService(char *nick, char *host, char *ident, char *gecos){
    char *uid;
    user *U;
    if(!isValidNick(nick)){
        aclog(LOG_ERROR, "Couldn't create service with the invalid nick: %s\n",nick);
        return NULL;
    }
    uid = generateUID();
    U = createUser(uid, nick, ident, host, "0.0.0.0", host, gecos, getConfigValue("ServicesModes"));
    return U;
}

void vservice_message(user *U, char *uid, char *str, va_list args){
    char obuff[512], buff[512], *line, *lines;
    /*sprintf(obuff, ":%s NOTICE %s :%s\r\n", U->uid, uid, str);
    vsprintf(buff, obuff, args);
    va_end(args);
    send_raw_line(buff);*/
    vsprintf(obuff, str, args);
    line = strtok_r(obuff, "\n", &lines);
    while(line){
        sprintf(buff, ":%s NOTICE %s :%s\r\n", U->uid, uid, line);
        send_raw_line(buff);
        line = strtok_r(NULL, "\n", &lines);
    }
}

void service_message(user *U, char *uid, char *str, ...){
    va_list args;
    va_start(args, str);
    vservice_message(U, uid, str, args);
}

void addHelp(helpnode **list, char *command, char *shorthelp, void (*longhelp)(char *uid, char *msg)){
    helpnode *node;
    node = *list;
    if(node){
        while(node->next){
            if(!strcasecmp(node->command,command)){
                node->command = command;
                node->shorthelp = shorthelp;
                node->longhelp = longhelp;
                return;
            }
            node = node->next;
        }
        safemallocvoid(node->next, helpnode);
        node = node->next;
    }else{
        safemallocvoid(*list, helpnode);
        node = *list;
    }
    node->command = command;
    node->shorthelp = shorthelp;
    node->longhelp = longhelp;
    node->next = NULL;
}

void fireHelp(user *U, helpnode *list, char *uid, char *msg){
    char *arg, *spaces;
    helpnode *node;
    if(!list){
        service_message(U, uid, "There is no help available for this service.");
        return;
    }
    node = list;
    strtok_r(msg, " ", &spaces);/* HELP */
    arg = strtok_r(NULL, " ", &spaces);
    if(arg){
        while(node){
            if(!strcasecmp(node->command, arg)){
                if(node->longhelp)
                    node->longhelp(uid, spaces);
                else
                    service_message(U, uid, "No additional help is available for \x02%s\x02.", arg);
                return;
            }
            node = node->next;
        }
        service_message(U, uid, "No additional help is available for \x02%s\x02.", arg);
    }else{
        while(node){
            if(node->shorthelp)
                service_message(U, uid, "    \x02%-12s\x02%s", node->command, node->shorthelp);
            node = node->next;
        }
    }
}

void addSetOption(setnode **list, char *option, char *shorthelp, void (*longhelp)(char *uid, char *msg), void (*callback)(char *uid, char *target, char *msg)){
    setnode *node;
    node = *list;
    if(node){
        while(node->next){
            if(!strcasecmp(node->option, option)){
                node->option = option;
                node->shorthelp = shorthelp;
                node->longhelp = longhelp;
                node->callback = callback;
                return;
            }
            node = node->next;
        }
        safemallocvoid(node->next, setnode);
        node = node->next;
    }else{
        safemallocvoid(*list, setnode);
        node = *list;
    }
    node->option = option;
    node->shorthelp = shorthelp;
    node->longhelp = longhelp;
    node->callback = callback;
    node->next = NULL;
}

void fireSetOption(user *U, setnode *list, char *uid, char *target, char *msg){
    char *option, *spaces;
    setnode *node;
    node = list;
    strtok_r(msg, " ", &spaces);/* SET */
    option = strtok_r(NULL, " ", &spaces);
    if(!option){
        service_message(U, uid, "Syntax: SET option parameters...");
        return;
    }
    while(node){
        if(!strcasecmp(node->option,option)){
            if(node->callback){
                node->callback(uid, target, spaces);
            }
            return;
        }
        node = node->next;
    }
    service_message(U, uid, "Syntax: Unknown option for SET, \x02%s\x02", option);

}

void fireSetHelp(user *U, setnode *list, char *uid, char *msg){
    char *arg, *spaces;
    setnode *node;
    arg = strtok_r(msg, " ", &spaces);
    node = list;
    if(arg){
        while(node){
            if(!strcasecmp(node->option, arg)){
                if(node->longhelp)
                    node->longhelp(uid, spaces);
                else
                    service_message(U, uid, "No additional help is available for SET \x02%s\x02.", arg);
                return;
            }
            node = node->next;
        }
        service_message(U, uid, "No additional help is available for SET \x02%s\x02.", arg);
    }else{
        while(node){
            if(node->shorthelp)
                service_message(U, uid, "    \x02%-12s\x02%s", node->option, node->shorthelp);
            node = node->next;
        }
    }
}

void addServiceCommand(commandnode **cmds, char *cmd, void (*callback)(char *uid, char *msg)){
    commandnode *node, *prev;
    safemallocvoid(node, commandnode);
    safenmallocvoid(node->cmd, char, sizeof(char)*(strlen(cmd)+1));
    strcpy(node->cmd, cmd);
    node->callback = callback;
    node->next = NULL;
    prev = *cmds;
    if(!prev){
        *cmds = node;
    } else {
        while(prev->next) prev = prev->next;
        prev->next = node;
    }
}

void fireServiceCommand(commandnode **cmds, user *service, line *l){
    int cmdlen;
    char *index, *badCmd;
    commandnode *node;
    if(strcmp(l->params[0], service->uid))
        return;/* not us */
    index = strstr(l->text," ");
    if(index){
        cmdlen = (int)(index-(l->text));
    } else {
        cmdlen = strlen(l->text);
    }
    node = *cmds;
    while(node){
        if(!strncasecmp(node->cmd, l->text, cmdlen)){
            node->callback(l->id, l->text);
            return;
        }
        node = node->next;
    }
    badCmd = strtok(index, " ");
    service_message(service, l->id, "Unknown command \"%s\"", l->text);
}

char *getTimeString(char *tz, time_t from){
    char tzChar, sign;
    int hour, minute;
    if(tz)
        tzChar = tz[0];
    else
        tzChar = 0;
    if(tzChar == '\xff')
        tzChar = defaultTZ;
    hour = tzChar>>2;
    minute = (tzChar&0x3)*15;
    from += minute*60 + hour*3600 + 43200;
    if(tzChar<48){
        sign = '-';
        hour = 12-hour;
    }else if(tzChar==48){
        sign = '\0';/* hide +00:00 */
    }else{
        sign = '+';
        hour -= 12;
    }
    snprintf(timeZoneStr, 40, "%s UTC%c%02d:%02d", strtok(asctime(gmtime(&from)),"\n"), sign, hour, minute);
    return timeZoneStr;
}
