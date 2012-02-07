#include <services.h>
#include <stdio.h>
#include <stdlib.h>
#include "operserv.h"

user *operserv = NULL;
commandnode *operservcmds = NULL;

void createOperServ(line *L){
    char defaultnick[] = "OperServ",
         defaulthost[] = "operator.black.box",
        defaultident[] = "OperServ",
        defaultgecos[] = "Operator Services";
    char *tmp, *nick, *host, *ident, *gecos;

    if((tmp = getConfigValue("OperServNick"))) nick = tmp;
    else nick = defaultnick;
    if((tmp = getConfigValue("OperServHost"))) host = tmp;
    else host = defaulthost;
    if((tmp = getConfigValue("OperServIdent"))) ident = tmp;
    else ident = defaultident;
    if((tmp = getConfigValue("OperServGECOS"))) gecos = tmp;
    else gecos = defaultgecos;


    operserv = createService(nick, host, ident, gecos);
}

void registerOperServCommand(char *cmd, void (*callback)(char *uid, char *msg)){
    addServiceCommand(&operservcmds, cmd, callback);
}

void fireOperServCommand(line *l){
    fireServiceCommand(&operservcmds, operserv, l);
}

void os_quit(char *uid, char *msg){
    usernode *service;
    char buff[128], defaultOperServQuit[] = "Services are shutting down.", *quit;
    aclog(LOG_ERROR|LOGFLAG_SRC, operserv, "Services are shutting down without saving databases... now!\n");
    if(!(quit = getConfigValue("OperServQuit")))
        quit = defaultOperServQuit;
    service = serviceusers;
    while(service){
        sprintf(buff, ":%s QUIT :%s.\r\n", service->U->uid, quit);
        send_raw_line(buff);
        service = service->next;
    }
    /* TODO: SQUIT() here */
    exit(0);
}

void os_test(char *uid, char *msg){
    /* because we're operserv */
    char buff[128];
    sprintf(buff,":%s KILL %s :Test succeeded.\r\n",operserv->uid,uid);
    send_raw_line(buff);
}

void INIT_MOD(){
    hook_event(EVENT_LINK, createOperServ);
    hook_event(EVENT_MESSAGE, fireOperServCommand);
    registerOperServCommand("test", os_test);
    registerOperServCommand("quit", os_quit);
}
