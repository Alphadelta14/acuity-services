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

    }else if(!strcasecmp(cmd, "LIST")){

    }
}

void INIT_MOD(){
    registerOperServCommand("mod", os_mod);

}
