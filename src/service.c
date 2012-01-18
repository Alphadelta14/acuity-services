#include <services.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

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
    char obuff[512], buff[512];
    sprintf(obuff, ":%s NOTICE %s :%s\r\n", U->uid, uid, str);
    vsprintf(buff, obuff, args);
    va_end(args);
    send_raw_line(buff);
}

void service_message(user *U, char *uid, char *str, ...){
    va_list args;
    va_start(args,str);
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
        }
        safemalloc(node->next, helpnode, );
        node = node->next;
    }else{
        safemalloc(*list, helpnode, );
        node = *list;
    }
    node->command = command;
    node->shorthelp = shorthelp;
    node->longhelp = longhelp;
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
                    node->longhelp(uid, msg);
                else
                    service_message(U, uid, "No additional help is available for this \x02%s\x02.", arg);
                return;
            }
            node = node->next;
        }
        service_message(U, uid, "No additional help is available for \x02%s\x02.", arg);
    }else{
        while(node){
            if(node->shorthelp)
                service_message(U, uid, "    \x02%12s\x02%s", node->command, node->shorthelp);
            node = node->next;
        }
    }
}

void addSetOption(setnode **list, char *option, char *shorthelp, void (*longhelp)(char *uid, char *msg), void (*callback)(char *uid, char *msg)){
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
        }
        safemalloc(node->next, setnode, );
        node = node->next;
    }else{
        safemalloc(*list, setnode, );
        node = *list;
    }
    node->option = option;
    node->shorthelp = shorthelp;
    node->longhelp = longhelp;
    node->callback = callback;
}
