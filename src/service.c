#include <services.h>
#include <stdlib.h>
#include <string.h>

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
