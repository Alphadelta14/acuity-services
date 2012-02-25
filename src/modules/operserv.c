#include <services.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "operserv.h"

user *operserv = NULL;
commandnode *operservcmds = NULL;
helpnode *operservHelp = NULL;

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

void os_message(char *uid, char *str, ...){
    va_list args;
    va_start(args, str);
    vservice_message(operserv, uid, str, args);
}

void addOperServHelp(char *command, char *shorthelp, void (*longhelp)(char *uid, char *msg)){
    addHelp(&operservHelp, command, shorthelp, longhelp);
}

void os_help(char *uid, char *msg){
    fireHelp(operserv, operservHelp, uid, msg);
}

void os_panicquit(char *uid, char *msg){
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
    closeIRC(quit);
    exit(0);
}

void os_panicquithelp(char *uid, char *msg){
    os_message(uid,
        "Syntax: PANICQUIT\n"
        " \n"
        "Quits all services clients immediately and does NOT save databases.\n"
        "Unless your services are completely beyond help, or your database\n"
        "was loaded in a broken state and you do not wish to save those, do\n"
        "not use this command. For all intents and purposes of\n"
        "normal services usage, use SHUTDOWN instead.");
}

void os_shutdown(char *uid, char *msg){
    /* XXX: should call sigquit, though, not available for the call */
    kill(getpid(), SIGQUIT);
}

void os_shutdownhelp(char *uid, char *msg){
    os_message(uid,
        "Syntax: SHUTDOWN\n"
        " \n"
        "Shuts services down cleanly. This will cleanly save the database and\n"
        "quit all services clients properly.");
}

void os_perm(char *uid, char *msg){
    char *cmd, *spaces, *class, *permname, *value;
    user *U;
    U = getUser(uid);
    if(!U)
        return;
    strtok_r(msg, " ", &spaces);/* PERM */
    cmd = strtok_r(NULL, " ", &spaces);
    if(!cmd){
        os_message(uid, "Syntax: PERM {SET|LIST|ADDCLASS} class permission value");
        return;
    }
    if(strcasecmp(cmd, "SET")){
        return;
    }
    /* TODO: sanitize */
    class = strtok_r(NULL, " ", &spaces);
    permname = strtok_r(NULL, " ", &spaces);
    value = strtok_r(NULL, " ", &spaces);
    setPermission(class, permname, atoi(value));
    /* os_message( */
    aclog(LOG_OVERRIDE, "%s changed %s's %s to %s\n", U->nick, class, permname, value);
}

void os_permhelp(char *uid, char *msg){
    os_message(uid,
        "Syntax: PERM {SET|LIST|ADDCLASS} class permission value\n"
        " \n"
        "The PERM command allows you to manipulate the permission lists.\n"
        " \n"
        "PERM SET changes the value of a permission for a certain class.\n"
        "Note that, unlike PERM LIST, you can set any arbitrary string as\n"
        "permission successfully; it won't show up in PERM LIST unless a module\n"
        "requires it, however.\n"
        " \n"
        "PERM LIST lists all permissions that are currently used by modules.\n"
        " \n"
        "PERM ADDCLASS allows you to add another permission class. It can be\n"
        "named anything, as long as it doesn't contain spaces. There is a class\n"
        "called default that is used as a base skeleton for permission building\n"
        "and therefore cannot be used for ADDCLASS.");
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
    registerOperServCommand("help",os_help);
    registerOperServCommand("test", os_test);
    registerOperServCommand("panicquit", os_panicquit);
    addOperServHelp("PANICQUIT", "Shuts Acuity down without saving databases", os_panicquithelp);
    registerOperServCommand("shutdown", os_shutdown);
    addOperServHelp("SHUTDOWN", "Shuts Acuity down properly", os_shutdownhelp);
    registerOperServCommand("perm", os_perm);
    addOperServHelp("PERM", "Change or list oper permissions and classes", os_permhelp);
    loadModule("os_mod");
}
