#include <services.h>
#include "operserv.h"
#include <string.h>

void os_mod_load(char *uid, char *args){
    char *modname, *spaces;
    void *handle;
    modname = strtok_r(args, " ", &spaces);
    if(!modname){
        os_message(uid, "Syntax: MOD LOAD modulename");
        return;
    }
    handle = loadModule(modname);
    if(!handle){
        os_message(uid, "Failed to load %s: %s", modname, moduleError());
    }else{
        os_message(uid, "Loaded %s", modname);
    }
}

void os_mod_unload(char *uid, char *args){
    char *modname, *spaces;
    modname = strtok_r(args, " ", &spaces);
    if(!modname){
        os_message(uid, "Syntax: MOD UNLOAD modulename");
        return;
    }
    unloadModule(modname);
    os_message(uid, "Unloaded %s", modname);
}

void os_mod_list(char *uid, char *args){
    modnode *node;
    int count = 0;
    node = modlist;
    os_message(uid, "Loaded modules:");
    while(node){
        os_message(uid, "    %s", node->name);
        count++;
        node = node->next;
    }
    os_message(uid, "Total modules: %d", count);
}


void os_mod(char *uid, char *msg){
    char *cmd, *spaces;
    strtok_r(msg, " ", &spaces);/* MOD */
    cmd = strtok_r(NULL, " ", &spaces);/* load/unload/list */
    if(!cmd){
        os_message(uid, "Syntax: MOD {LOAD|UNLOAD|LIST} parameters");
        return;
    }
    if(!strcasecmp(cmd, "LOAD")){
        os_mod_load(uid, spaces);
    }else if(!strcasecmp(cmd, "UNLOAD")){
        os_mod_unload(uid, spaces);
    }else if(!strcasecmp(cmd, "LIST")){
        os_mod_list(uid, spaces);
    }
}

void INIT_MOD(){
    registerOperServCommand("mod", os_mod);

}
