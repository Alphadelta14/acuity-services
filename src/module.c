#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>

#include <services.h>

#define modopen dlopen
#define moderror dlerror
#define modsymbol dlsym
#define modclose dlclose

#ifndef MODEXT
#define modext ".so"
#else
#define modext XSTR(MODEXT)
#endif /* MODEXT */

modnode *modlist = NULL;
int MOD_STATE = MOD_IDLE;

char *moduleError(){
    return moderror();
}

void *loadModule(const char *modname){
    char *fname;
    void *modhandle;
    modnode *node;

    node = modlist;
    if(node)
        do{
            if(!strcasecmp(node->name, modname)){
                return node;
            }
        }while(ITER(node));
    MOD_STATE = MOD_LOAD;
    aclog(LOG_DEBUG, "Loading module: %s", modname);
    safenmalloc(fname, char, 10+strlen(modname)+strlen(modext));
        /*"modules/"+%s+modext*/
    strcpy(fname, "modules/");
    strcat(fname, modname);
    strcat(fname, modext);
    modhandle = modopen(fname, RTLD_NOW|RTLD_GLOBAL);
    free(fname);
    if(!modhandle){
        aclog(LOG_DEBUG, "  [FAIL]\n");
        aclog(LOG_ERROR, "Could not open '%s' module: %s\n", modname,
            moderror());
        return NULL;
    }
    *(void **) (&initModule) = modsymbol(modhandle, "INIT_MOD");
    safemalloc(node, modnode);
    node->handle = modhandle;
    safenmalloc(node->name, char, strlen(modname)+1);
    strcpy(node->name, modname);
    node->next = modlist;
    modlist = node;
    aclog(LOG_DEBUG, "  [OK]\n");
    if(initModule)
        initModule();
    MOD_STATE = MOD_IDLE;
    return modhandle;
}

void unloadModule(char *name){
    modnode *node, *prev = NULL;
    node = modlist;
    MOD_STATE = MOD_UNLOAD;
    if(node)
        do{
            if(!strcasecmp(node->name, name)){
                aclog(LOG_DEBUG, "Unloading module: %s", node->name);
                *(void **) (&termModule) = modsymbol(node->handle, "TERM_MOD");
                if(termModule)
                    termModule();
                safefree(node->name);
                if(prev)
                    prev->next = node->next;
                else
                    modlist = node;
                modclose(node->handle);
                safefree(node);
                aclog(LOG_DEBUG, "\t[OK]\n");
                return;
            }
            prev = node;
        }while(ITER(node));
}

void unloadModules(){
    modnode *node, *prev;
    node = modlist;
    MOD_STATE = MOD_UNLOAD;
    while(node){
        aclog(LOG_DEBUG, "Unloading module: %s", node->name);
        *(void **) (&termModule) = modsymbol(node->handle, "TERM_MOD");
        if(termModule)
            termModule();
        prev = node;
        ITER(node);
        safefree(prev->name);
        modclose(prev->handle);
        safefree(prev);
        aclog(LOG_DEBUG, "\t[OK]\n");
    }
    MOD_STATE = MOD_IDLE;
    modlist = NULL;
}

void reloadModule(char *modname){
    unloadModule(modname);
    loadModule(modname);
}

char isModuleLoaded(char *modname){
    modnode *node;
    node = modlist;
    if(EMPTY(node))
        return 0;
    do{
        if(!strcasecmp(node->name, modname))
            return 1;
    }while(ITER(node));
    return 0;
}

void loadDependencies(int count, ...){
    int i;
    va_list args;
    char *modname;
    va_start(args,count);
    aclog(LOG_DEBUG,"%d depends\n", count);
    for(i=0;i<count;i++){
        modname = va_arg(args, char*);
        aclog(LOG_DEBUG,"Checking for %s\n", modname);
        if(!isModuleLoaded(modname)){
            aclog(LOG_DEBUG,"Loading depended module: %s\t\n", modname);
            loadModule(modname);
        }
    }
    va_end(args);
}
