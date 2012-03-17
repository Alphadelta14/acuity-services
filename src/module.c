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

void (*initModule)(void);
void (*termModule)(void);

modnode *modlist = NULL;
int MOD_STATE = MOD_IDLE;

char *module_error(){
    return moderror();
}

void *load_module(const char *modname){
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

void unload_module(char *name){
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

void unload_modules(){
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

void reload_module(char *modname){
    unload_module(modname);
    load_module(modname);
}

bool is_module_loaded(char *modname){
    modnode *node;
    node = modlist;
    if(EMPTY(node))
        return FALSE;
    do{
        if(!strcasecmp(node->name, modname))
            return TRUE;
    }while(ITER(node));
    return FALSE;
}

void load_dependencies(int count, ...){
    int i;
    va_list args;
    char *modname;
    va_start(args, count);
    aclog(LOG_DEBUG,"%d depends\n", count);
    for(i=0; i < count; i++){
        modname = va_arg(args, char*);
        aclog(LOG_DEBUG, "Checking for %s\n", modname);
        if(!is_module_loaded(modname)){
            aclog(LOG_DEBUG, "Loading depended module: %s\t\n", modname);
            load_module(modname);
        }
    }
    va_end(args);
}
