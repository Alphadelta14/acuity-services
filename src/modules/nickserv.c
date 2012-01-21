/* * * *
NickServ: Nick Auth Module
Potential Configuration Values:
@ NickServNick = Nick for Nick Auth Services to use
@ NickServHost = Host for Nick Auth Services to use
@ NickServIdent = Ident for Nick Auth Services to use
@ NickServGECOS = GECOS (Real Name) for Nick Auth Services to use
@ NickServMinPasslen = Minimum password length
@ NickServMaxGroupedNicks = Maximum nicks per group
*/

#include <services.h>
#include <encrypt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nickserv.h"

user *nickserv = NULL;
commandnode *nickservcmds = NULL;
unsigned int cNickGroupID = 1;/* current group id */
nicklist *registerednicks = NULL;
nickgrouplist *registerednickgroups = NULL;
helpnode *nickservHelp = NULL;
setnode *nickservSetOpts = NULL;
int MODE_NSREGISTER = MODE_R;

void createNickServ(line *L){
    char defaultnick[] = "NickServ",
         defaulthost[] = "nick.auth.services",
        defaultident[] = "NickServ",
        defaultgecos[] = "Nick Authentication Services";
    char *tmp, *nick, *host, *ident, *gecos;

    if((tmp = getConfigValue("NickServNick"))) nick = tmp;
    else nick = defaultnick;
    if((tmp = getConfigValue("NickServHost"))) host = tmp;
    else host = defaulthost;
    if((tmp = getConfigValue("NickServIdent"))) ident = tmp;
    else ident = defaultident;
    if((tmp = getConfigValue("NickServGECOS"))) gecos = tmp;
    else gecos = defaultgecos;


    nickserv = createService(nick, host, ident, gecos);
}

void registerNickServCommand(char *cmd, void (*callback)(char *uid, char *msg)){
    commandnode *node, *prev;
    node = (commandnode*)malloc(sizeof(commandnode));
    if(!node){
        aclog(LOG_ERROR, "Failed to register %s as a NickServ command. Failed to allocate memory.\n",cmd);
        return;
    }
    node->cmd = (char*)malloc(sizeof(char)*(strlen(cmd)+1));
    if(!node->cmd){
        aclog(LOG_ERROR, "Failed to register %s as a NickServ command. Failed to allocate memory for command name.\n",cmd);
        return;
    }
    strcpy(node->cmd,cmd);
    node->callback = callback;
    node->next = NULL;
    prev = nickservcmds;
    if(!nickservcmds){
        nickservcmds = node;
    } else {
        while(prev->next) prev = prev->next;
        prev->next = node;
    }
}

void fireNickServCommand(line *l){
    int cmdlen;
    char *index;
    commandnode *node;
    if(strcmp(l->params[0],nickserv->uid))
        return;/* not us */
    index = strstr(l->text," ");
    if(index){
        cmdlen = (int)(index - l->text);
    } else {
        cmdlen = strlen(l->text);
    }
    node = nickservcmds;
    while(node){
        if(!strncasecmp(node->cmd,l->text,cmdlen)){
            node->callback(l->id,l->text);
            return;
        }
        node = node->next;
    }
}


void ns_message(char *uid, char *str, ...){
    va_list args;
    va_start(args, str);
    vservice_message(nickserv, uid, str, args);
}

char valid_email(char *email){
    /* very basic function */
    if(!strstr(email,"@"))
        return 0;
    if(!strstr(email,"."))
        return 0;
    if(strstr(email,"\""))
        return 0;
    if(strstr(email,"'"))
        return 0;
    if(strstr(email,"!"))
        return 0;
    if(strstr(email,","))
        return 0;
    if(strlen(email)<6)
        return 0;
    return 1;
}

unsigned int generateNickGroupID(void){
    return cNickGroupID++;
}

nickaccount *getNickAccountByNick(char *nick){
    nicklist *nicks;
    nicks = registerednicks;
    while(nicks){
        if(!irccasecmp(nicks->acc->nick,nick)){
            return nicks->acc;
        }
        nicks = nicks->next;
    }
    return NULL;
}

nickgroup *getNickGroupByEmail(char *email){
    nickgrouplist *groups;
    groups = registerednickgroups;
    while(groups){
        if(!irccasecmp(groups->group->email,email)){
            return groups->group;
        }
        groups = groups->next;
    }
    return NULL;
}

nickgroup *createNickGroup(nickaccount *acc, char *pass, char *email){
    nickgrouplist *groups;
    nickgroup *group;
    sha256_context ctx;
    FILE *rand;
    safemalloc(group,nickgroup,NULL);
    group->groupid = generateNickGroupID();
    safenmalloc(group->email,char,strlen(email)+1,NULL);
    strcpy(group->email,email);
    safemalloc(group->nicks,nicklist,NULL);
    group->nicks->acc = acc;
    group->nicks->next = NULL;
    group->metadata = NULL;
    group->main = acc;
    acc->group = group;
    /* removeNickFromGroup(acc, acc->group); */
    rand = fopen("/dev/urandom","r");
    if(rand){
        fread(group->passmethod,1,3,rand);
    }
    group->passmethod[3] = ENC_SHA256;
    fclose(rand);
    sha256_starts(&ctx);
    sha256_update(&ctx,(unsigned char*)pass,strlen(pass));
    sha256_update(&ctx,group->passmethod,3);
    sha256_finish(&ctx,group->passwd);
    groups = registerednickgroups;
    if(!groups){
        safemalloc(registerednickgroups,nickgrouplist,NULL);
        registerednickgroups->group = group;
        registerednickgroups->next = NULL;
    } else {
        while(groups->next) groups = groups->next;
        safemalloc(groups->next,nickgrouplist,NULL);
        groups->next->group = group;
        groups->next->next = NULL;
    }
    return group;
}

void deleteNickGroup(nickgroup *group){
    nickgrouplist *groups, *prev;
    if(!group)
        return;
    prev = groups = registerednickgroups;
    if(groups->group == group){
        registerednickgroups = groups->next;
    } else {
        while(groups){
            if(groups->group == group){
                prev->next = groups->next;
                free(groups);
                break;
            }
            prev = groups;
            groups = groups->next;
        }
    }
    free(group->email);
    clearMetadata(group->metadata);
    free(group);
}

nickaccount *createNickAccount(char *nick){
    nickaccount *acc;
    nicklist *nicks;
    safemalloc(acc,nickaccount,NULL);
    safenmalloc(acc->nick,char,sizeof(nick)+1,NULL);
    strcpy(acc->nick,nick);
    acc->regtime = time(NULL);
    acc->metadata = NULL;
    acc->group = NULL;
    nicks = registerednicks;
    if(!nicks){
        safemalloc(registerednicks,nicklist,NULL);
        registerednicks->acc = acc;
        registerednicks->next = NULL;
    } else {
        while(nicks->next) nicks = nicks->next;
        safemalloc(nicks->next,nicklist,NULL);
        nicks->next->acc = acc;
        nicks->next->next = NULL;
    }
    return acc;
}

void deleteNickAccount(nickaccount *acc){
    nicklist *nicks, *prev;
    prev = nicks = registerednicks;
    if(nicks->acc == acc){
        registerednicks = nicks->next;
        free(nicks);
    } else {
        while(nicks){
            if(nicks->acc == acc){
                prev = nicks->next;
                free(nicks);
                break;
            }
            prev = nicks;
            nicks = nicks->next;
        }
    }
    removeNickFromGroup(acc, acc->group);
    free(acc->nick);
    clearMetadata(acc->metadata);
    free(acc);
}

void addNickToGroup(nickaccount *acc, nickgroup *group){
    nicklist *members;
    if((!acc)||(!group))
        return;
    removeNickFromGroup(nick, nick->group);
    acc->group = group;
    members = group->nicks;
    if(!members){
        safemalloc(group->nicks,nicklist, );
        group->nicks->acc = acc;
        group->nicks->next = NULL;
    } else {
        while(members->next) members = members->next;
        safemalloc(members->next,nicklist, );
        members->next->acc = acc;
        members->next->next = NULL;
    }
}

void removeNickFromGroup(nickaccount *acc, nickgroup *group){
    nicklist *nicks, *prev;
    prev = nicks = group->nicks;
    if(nicks->acc == acc){
        group->nicks = nicks->next;
        free(nicks);
    } else {
        while(nicks){
            if(nicks->acc == acc){
                prev = nicks->next;
                free(nicks);
                break;
            }
            prev = nicks;
            nicks = nicks->next;
        }
    }
    if(group->main == acc) /* still a pointer */
        group->main = group->nicks->acc;
    if(!group->nicks)
        deleteNickGroup(group);
}


char hasNickServPermission(char *uid, nickaccount *acc, int flags, ...){
    /* TODO: this whole thing */
    return 1;
}

void ns_register(char *uid, char *msg){
    user *U;
    char *pass, *email, *spaces, *tmpconf, *modes;
    nickaccount *acc;
    U = getUser(uid);
    if(!U)
        return;
    strtok_r(msg," ",&spaces);/* register */
    pass = strtok_r(NULL," ",&spaces);
    email = strtok_r(NULL," ",&spaces);
    if((!pass)||(!email)){
        ns_message(uid,"Syntax: REGISTER password email");
        return;
    }
    if(getNickAccountByNick(U->nick)){
        ns_message(uid,"You have already registered your nick.");
        return;
    }
    if(getNickGroupByEmail(email)){
        ns_message(uid,"This email already belongs to a nick group. For help on grouping a nick, use HELP group.");
        return;
    }
    if(!valid_email(email)){
        ns_message(uid,"%s is not a valid email address.",email);
        return;
    }
    tmpconf = getConfigValue("NickServMinPasslen");
    if(tmpconf){
        if(strlen(pass)<atoi(tmpconf)){
            ns_message(uid,"Password must be at least %s characters long.",tmpconf);
            return;
        }
    }
    /* TODO: check if U->nick is registerable (mod hooks for Guest nicks, reserved nicks, etc) */
    acc = createNickAccount(U->nick);
    if(!acc){
        aclog(LOG_ERROR,"Failed to create account for %s (%s).\n", U->nick, uid);
        return;
    }
    createNickGroup(acc, pass, email);
    modes = buildModes(1, MODE_NSREGISTER);
    setMode(nickserv->uid, uid, modes);
    free(modes);
    setMetaValue(U->metadata, "nick", U->nick);
    ns_message(uid,"Your account has been registered with the password %s. Please remember this for when you identify to your new nick.",pass);
    aclog(LOG_SERVICE,"New Nick: %s!%s@%s has registered their nick with the email %s.\n", U->nick, U->ident, U->host, email);
}

void ns_group(char *uid, char *msg){
    user *U;
    char *target, *pass, *spaces, *tmpconf, *modes;
    nickaccount *acc, *T;
    nickgroup *newgroup;
    nicklist *nicks;
    int membercount = 0;
    U = getUser(uid);
    if(!U)
        return;
    strtok_r(msg," ",&spaces);/* group */
    target = strtok_r(NULL," ",&spaces);
    pass = strtok_r(NULL," ",&spaces);
    if((!pass)||(!target)){
        ns_message(uid,"Syntax: GROUP target password");
        return;
    }
    T = getNickAccountByNick(target);
    if(!T){
        ns_message(uid, "%s is not a registered user.",target);
        return;
    }
    newgroup = T->group;
    if(!newgroup){
        aclog(LOG_DEBUG, "%s does not have a group associated with their nick.",target);
        return;
    }
    if(!matchPassword(pass, newgroup->passwd, newgroup->passmethod)){
        /* TODO: password fail counter implementation */
        aclog(LOG_DEBUG | LOG_SERVICE, "%s failed to group to %s's group. Password did not match.",U->nick, target);
        ns_message(uid, "Access denied. Password does not match.");
        return;
    }
    nicks = newgroup->nicks;
    while(nicks){
        membercount++;
        nicks = nicks->next;
    }
    tmpconf = getConfigValue("NickServMaxGroupedNicks");
    if(tmpconf){
        if(membercount >= atoi(tmpconf)){
            aclog(LOG_DEBUG | LOG_SERVICE, "%s failed to group to %s's group. Max members has already been reached.",U->nick, target);
            ns_message(uid, "This group already has %d members in it. No more can be added.", membercount);
            return;
        }
    }
    acc = getNickAccountByNick(U->nick);
    if(!acc){
        acc = createNickAccount(U->nick);
        aclog(LOG_SERVICE,"New Nick: %s!%s@%s has grouped their new nick to %s.\n", U->nick, U->ident, U->host, target);
        modes = buildModes(1, MODE_NSREGISTER);
        setMode(nickserv->uid, uid, modes);
        free(modes);
        addNickToGroup(acc, newgroup);
    } else {
        if(acc->group == newgroup){
            ns_message(uid, "You already belong to that group.");
            return;
        }
        addNickToGroup(acc, newgroup);
        aclog(LOG_SERVICE,"Group: %s!%s@%s has grouped their nick to %s.\n", U->nick, U->ident, U->host, target);
    }
    setMetaValue(U->metadata, "nick", U->nick);
    ns_message(uid, "You have joined %s's group.", target);
}

void ns_identify(char *uid, char *msg){
    user *U;
    char *pass, *spaces, *modes;
    nickaccount *acc;
    U = getUser(uid);
    if(!U)
        return;
    strtok_r(msg," ",&spaces);/* identify */
    pass = strtok_r(NULL," ",&spaces);
    if(!pass){
        ns_message(uid,"Syntax: IDENTIFY password");
        return;
    }
    acc = getNickAccountByNick(U->nick);
    if(!acc){
        ns_message(uid,"Your nick is not registered.");
        return;
    }
    if(!matchPassword(pass, acc->group->passwd, acc->group->passmethod)){
        ns_message(uid, "Invalid password.");
        aclog(LOG_SERVICE, "%s!%s@%s entered an invalid password.\n", U->nick, U->ident, U->vhost);
        return;
    }
    modes = buildModes(1, MODE_NSREGISTER);
    setMode(nickserv->uid, uid, modes);
    free(modes);
    setMetaValue(U->metadata, "nick", U->nick);
    ns_message(uid, "You have identified for %s.", U->nick);
    aclog(LOG_SERVICE, "%s!%s@%s has identified.", U->nick, U->ident, U->vhost);
}

void addNickServHelp(char *command, char *shorthelp, void (*longhelp)(char *uid, char *msg)){
    addHelp(&nickservHelp, command, shorthelp, longhelp);
}

void ns_help(char *uid, char *msg){
    fireHelp(nickserv, nickservHelp, uid, msg);
}

void addNickServSetOption(char *option, char *shorthelp, void (*longhelp)(char *uid, char *msg), void (*callback)(char *uid, char *target, char *msg)){
    addSetOption(&nickservSetOpts, option, shorthelp, longhelp, callback);
}

void ns_set(char *uid, char *msg){
    fireSetOption(nickserv, nickservSetOpts, uid, getUser(uid)->nick, msg);
}

void ns_sethelp(char *uid, char *msg){
    fireSetHelp(nickserv, nickservSetOpts, uid, msg);
}

void testCmd(char *uid, char *msg){
    char buff[128];
    sprintf(buff,":%s NOTICE %s :Test succeeded.\r\n",nickserv->uid,uid);
    send_raw_line(buff);
    aclog(LOG_DEBUG,"Test message sent: %s\n",msg);
}

void INIT_MOD(){
    hook_event(EVENT_LINK, createNickServ);
    hook_event(EVENT_MESSAGE, fireNickServCommand);
    registerNickServCommand("help",ns_help);
    registerNickServCommand("test",testCmd);
    registerNickServCommand("register",ns_register);
    registerNickServCommand("group",ns_group);
    addNickServHelp("GROUP", "Groups your nick", NULL);
    registerNickServCommand("identify",ns_identify);
    addNickServHelp("IDENTIFY", "Identifies your nick", NULL);
    registerNickServCommand("set",ns_set);
    addNickServHelp("SET", "Sets options for your nick", ns_sethelp);
    loadModule("ns_set_basic");
    loadModule("ns_info");
}
