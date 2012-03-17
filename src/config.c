#include <services.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct defval_s {
    char *name;
    char *value;
} defval_t;

static confval_t *conflist = NULL;
static char none[] = "";
static defval_t defaultconf[] = {
{"HubPort","6662"},
{"ServerName","Acuity Services for IRC"},
{"ServicesModes","+Ik"},
{NULL, NULL}
};

void set_config_value(char *name, char *value){
    confval_t *C;

    C = conflist;
    if(EMPTY(C)){
        safemalloc(C, confval_t);
        C->name = name;
        C->value = value;
        C->next = NULL;
        conflist = C;
        return;
    }
    do{
        if(!strcmp(C->name, name)){
            if(C->value){
                safefree(C->value);
            }
            C->value = value;
            return;
        }
    }while(ITER(C));
    safemalloc(C, confval_t);
    C->name = name;
    C->value = value;
    C->next = conflist;
    conflist = C;
}

char *get_config_value(char *name){
    confval_t *C;

    C = conflist;
    if(EMPTY(C))
        return NULL;
    do{
        if(!strcmp(C->name, name)){
            if(C->value)
                return C->value;
            else
                return none;
        }
    }while(ITER(C));
    return NULL;
}

static void loadDefaults(){
    /* contents need to be copied so that dynamic memory releasing is possible */
    defval_t *def;
    char *name, *value;

    def = &defaultconf[0];
    while(def->name){
        safenmalloc(name, char, strlen(def->name)+1);
        strcpy(name, def->name);
        safenmalloc(value, char, strlen(def->value)+1);
        strcpy(value, def->value);
        set_config_value(name, value);
        def++;
    }
}

void load_config(int argc, char *argv[]){
    /* TODO: log malformed lines */
    FILE *f = NULL;
    char *name, *value, tmp[512];
    int len, match, i;

    loadDefaults();
    for(i = 0; i < argc; i++){
        if(!strcmp(argv[i], "-c")){
            f = fopen(argv[i+1], "r");
            if(!f){
                aclog(LOG_ERROR, "Could not open the configuration file "
                    "supplied by '-c': %s\n", argv[i+1]);
            }
        }
    }
    if(!f)
        f = fopen("acuity.conf", "r");
    if(!f){
        aclog(LOG_ERROR, "Could not open the configuration file 'acuity.conf'."
            " Be sure to create this file.\n");
        return;
    }
    while(1){
        match = fscanf(f, "%s ", tmp);
        if(match == EOF) break;
        if(match < 1){
            if(!fgets(tmp, 512, f)) break;
            continue;
        }
        if(tmp[0] == '#'){
            if(!fgets(tmp, 512, f)) break;
            continue;
        }
        len = strlen(tmp)+1;
        safenmalloc(name, char, len);
        strcpy(name, tmp);
        match = fscanf(f, "\"%[^\"]\"\n", tmp);
        if(match == EOF) break;
        if(match < 1){
            if(!fgets(tmp, 512, f)) break;
            continue;
        }
        len = strlen(tmp)+1;
        safenmalloc(value, char, len);
        strcpy(value, tmp);
        set_config_value(name, value);
    }
}

void clear_config(){
    confval_t *C, *next;

    C = conflist;
    if(EMPTY(C))
        return;
    do{
        next = C->next;
        safefree(C->name);
        safefree(C->value);
        safefree(C);
    }while((C = next));
    conflist = NULL;
}
