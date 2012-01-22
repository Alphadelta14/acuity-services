#include <actypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <acuity.h>
#include <config.h>

confval *conflist = NULL;
char none[] = "";
defval defaultconf[] = {
{"HubPort","6662"},
{"ServerName","Acuity Services for IRC"},
{"ServicesModes","+Ik"},
{NULL, NULL}
};

void setConfigValue(char *name, char *value){
    confval *C, *d;

    C = conflist;
    if(!C){
        safemallocvoid(C,confval);
        C->name = name;
        C->value = value;
        conflist = C;
        return;
    }
    while(C->next){
        if(!strcmp(C->name, name)){
            if(C->value){
                free(C->value);
            }
            C->value = value;
            return;
        }
        C = C->next;
    }
    if(!strcmp(C->name, name)){
        C->value = value;
        return;
    }
    safemallocvoid(d,confval);
    d->name = name;
    d->value = value;
    d->next = NULL;
    C->next = d;
}

char *getConfigValue(char *name){
    confval *C;
    C = conflist;
    while(C){
        if(!strcmp(C->name, name)){
            if(C->value)
                return C->value;
            else
                return none;
        }
        C = C->next;
    }
    return NULL;
}

void printConfig(){
    confval *C;
    C = conflist;
    while(C){
        printf("%s: %s\n",C->name,C->value);
        C = C->next;
    }
}

void loadDefaults(){
    /* contents need to be copied so that dynamic memory releasing is possible */
    defval *def;
    char *name, *value;

    def = &defaultconf[0];
    while(def->name){
        safenmallocvoid(name,char,sizeof(char)*strlen(def->name)+1);
        strcpy(name, def->name);
        safenmallocvoid(value,char,sizeof(char)*strlen(def->value)+1);
        strcpy(value, def->value);
        setConfigValue(name,value);
        def++;
    }
}

void loadConfig(){
    /* TODO: log malformed lines */
    FILE *f;
    char *name, *value, tmp[512];
    int len, match;

    loadDefaults();
    f = fopen("acuity.conf","r");
    while(1){
        match = fscanf(f,"%s ",tmp);
        if(match==EOF) break;
        if(match<1){
            if(!fgets(tmp,512,f)) break;
            continue;
        }
        if(tmp[0]=='#'){
            if(!fgets(tmp,512,f)) break;
            continue;
        }
        len = strlen(tmp)+1;
        safenmallocvoid(name,char,sizeof(char)*len);
        strcpy(name, tmp);
        match = fscanf(f,"\"%[^\"]\"\n",tmp);
        if(match==EOF) break;
        if(match<1){
            if(!fgets(tmp,512,f)) break;
            continue;
        }
        len = strlen(tmp)+1;
        safenmallocvoid(value,char,sizeof(char)*len);
        strcpy(value, tmp);
        setConfigValue(name,value);
        /*printf("Setconf: %s to %s\n",name,value);*/
    }
}

#ifdef CONFTEST
int main(int c, char **v){
    loadConfig();
    printConfig();
}
#endif
