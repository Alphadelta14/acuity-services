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
#include <time.h>
#include "nickserv.h"

user *nickserv = NULL;
commandnode *nickservcmds = NULL;
unsigned int cNickGroupID = 1;/* current group id */
unsigned int cNickID = 1;
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
    addServiceCommand(&nickservcmds, cmd, callback);
}

void fireNickServCommand(line *l){
    fireServiceCommand(&nickservcmds, nickserv, l);
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

void addNickElement(nicklist **list, nickaccount *acc){
    nicklist *nicks, *nextnode, *prevnode;
    char *nick;
    nicks = *list;
    nick = acc->nick;
    if(!nicks){
        safemallocvoid(nicks, nicklist);
        nicks->acc = acc;
        nicks->next = NULL;
        *list = nicks;
    }else if(irccasecmp(nicks->acc->nick, nick)>0){
        nextnode = nicks;
        safemallocvoid(nicks, nicklist);
        nicks->acc = acc;
        nicks->next = nextnode;
        *list = nicks;
    }else{
        prevnode = nicks;
        nextnode = prevnode->next;
        while(nextnode){
            if(irccasecmp(nextnode->acc->nick, nick)>0){
                safemallocvoid(prevnode->next, nicklist);
                prevnode->next->acc = acc;
                prevnode->next->next = nextnode;
                return;
            }
            prevnode = nextnode;
            nextnode = prevnode->next;
        }
        safemallocvoid(prevnode->next, nicklist);
        prevnode->next->acc = acc;
        prevnode->next->next = NULL;
        return;
    }
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

void createSavedNickGroup(nickgroup *group){
    blobdata passwd, passmethod;
    passwd.size = 4;
    passmethod.size = 32;
    passwd.data = group->passwd;
    passmethod.data = group->passmethod;
    db_query("INSERT INTO `nickgroup` (`groupid`, `mainnick`, `email`, `passwd`, `passmethod`) VALUES(?, ?, ?, ?, ?);", NULL, "iisbb", group->groupid, group->main->nickid, group->email, passwd, passmethod);
}

void createSavedNickAccount(nickaccount *acc){
    db_query("INSERT INTO `nickaccount` (`nickid`, `nick`, `groupid`, `regtime`) VALUES(?, ?, ?, ?);", NULL, "isii", acc->nickid, acc->nick, acc->group->groupid, acc->regtime);
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
    group->class = defaultpermclass;
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
    clearMetadata(&group->metadata);
    free(group);
}

nickaccount *createNickAccount(char *nick){
    nickaccount *acc;
    safemalloc(acc,nickaccount,NULL);
    safenmalloc(acc->nick,char,sizeof(nick)+1,NULL);
    strcpy(acc->nick,nick);
    acc->nickid = cNickID++;
    acc->regtime = time(NULL);
    acc->metadata = NULL;
    acc->group = NULL;
    addNickElement(&registerednicks, acc);
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
                prev->next = nicks->next;
                free(nicks);
                break;
            }
            prev = nicks;
            nicks = nicks->next;
        }
    }
    removeNickFromGroup(acc, acc->group);
    free(acc->nick);
    clearMetadata(&acc->metadata);
    free(acc);
}

void addNickToGroup(nickaccount *acc, nickgroup *group){
    if((!acc)||(!group))
        return;
    removeNickFromGroup(acc, acc->group);
    acc->group = group;
    addNickElement(&group->nicks, acc);
}

void removeNickFromGroup(nickaccount *acc, nickgroup *group){
    nicklist *nicks, *prev;
    if(!group)
        return;
    prev = nicks = group->nicks;
    if(nicks->acc == acc){
        group->nicks = nicks->next;
        free(nicks);
    } else {
        while(nicks){
            if(nicks->acc == acc){
                prev->next = nicks->next;
                free(nicks);
                break;
            }
            prev = nicks;
            nicks = nicks->next;
        }
    }
    if(!group->nicks){
        deleteNickGroup(group);
        return;
    }
    if(group->main == acc) /* still a pointer */
        group->main = group->nicks->acc;
}


char hasNickServPermission(char *uid, nickaccount *acc, char *permname){
    permclass *class;
    int perm, srcperm;
    nickaccount *source;
    user *U;
    if(!acc)
        return 0;/* not a valid nick account */
    if(!(U = getUser(uid)))
        return 0;/* not a valid user */
    class = acc->group->class;
    perm = getPermission(class->name, permname);
    if(irccasecmp(acc->nick, U->nick)){/* source acting on target (acc) */
        source = getNickAccountByNick(U->nick);
        if(!source)
            return 0;
        srcperm = getPermission(source->group->class->name, permname);
        if((srcperm&PERM_LEVEL)<(perm&PERM_LEVEL))
            return 0;
        else if(((srcperm&PERM_LEVEL)==(perm&PERM_LEVEL))&&(srcperm&~PERM_EQUAL))
            return 0;
        perm = srcperm;
    }
    return perm&PERM_LEVEL;
}

char *getLocalTimeString(char *uid, time_t time){
    user *U;
    nickaccount *acc;
    char *tzInfo, *dstInfo, newTzInfo[] = "\x30";/* newTzInfo gets overwritten */
    U = getUser(uid);
    if(!U)
        return getTimeString("\xff", time);
    if(!(acc = getNickAccountByNick(U->nick)))
        return getTimeString("\xff", time);
    tzInfo = getMetaValue(&acc->group->metadata, "TIMEZONE");
    dstInfo = getMetaValue(&acc->group->metadata, "DST");
    if(!tzInfo)
        return getTimeString("\xff", time);
    newTzInfo[0] = tzInfo[0];
    if(dstInfo&&(dstInfo[0]=='S'))
        newTzInfo[0] += 4;
    return getTimeString(newTzInfo, time);
}

void ns_register(char *uid, char *msg){
    user *U;
    char *pass, *email, *spaces, *tmpconf, *modes;
    nickaccount *acc;
    nickgroup *group;
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
        ns_message(uid,"This email already belongs to a nick group. For help on grouping a nick, use HELP GROUP.");
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
    group = createNickGroup(acc, pass, email);
    createSavedNickGroup(group);
    createSavedNickAccount(acc);
    modes = buildModes(1, MODE_NSREGISTER);
    setMode(nickserv->uid, uid, modes);
    free(modes);
    setMetaValue(&U->metadata, "nick", U->nick);
    ns_message(uid,"Your account has been registered with the password %s. Please remember this for when you identify to your new nick.",pass);
    aclog(LOG_REGISTER,"New Nick: %s!%s@%s has registered their nick with the email %s.\n", U->nick, U->ident, U->host, email);
}

void ns_registerhelp(char *uid, char *msg){
    /*ns_message(uid,"moo\r\nsupermoo");
    return;*/
    char *tmpconf;

    ns_message(uid,
        "Syntax: REGISTER password email\n"
        " \n"
        "The REGISTER command allows you to register a nick. Please note that\n"
        "your password is case-sensitive and will be stored in a way that\n"
        "prevents reading your plain-text password again. Therefore, it is\n"
        "recommended that you choose a password you can remember.");

    tmpconf = getConfigValue("NickServMinPasslen");
    if(tmpconf)
        ns_message(uid,"Additionally, your password must be at least %s characters long.",tmpconf);

    ns_message(uid,"Your e-mail address will be treated confidentally, but\n"
        "checked for validity.\n"
        " \n"
        "Registering your nick prevents it from being used by others and allows\n"
        "you to get automatic status in channels if you are on their access\n"
        "list and more.\n"
        " \n"
        "If you already own a nick, consider grouping it instead.");
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
        aclog(LOG_DEBUG | LOG_NOAUTH, "%s failed to group to %s's group. Password did not match.",U->nick, target);
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
            aclog(LOG_DEBUG, "%s failed to group to %s's group. Max members has already been reached.",U->nick, target);
            ns_message(uid, "This group already has %d members in it. No more can be added.", membercount);
            return;
        }
    }
    acc = getNickAccountByNick(U->nick);
    if(!acc){
        acc = createNickAccount(U->nick);
        aclog(LOG_REGISTER,"New Nick: %s!%s@%s has grouped their new nick to %s.\n", U->nick, U->ident, U->host, target);
        modes = buildModes(1, MODE_NSREGISTER);
        setMode(nickserv->uid, uid, modes);
        free(modes);
        addNickToGroup(acc, newgroup);
        createSavedNickAccount(acc);
    } else {
        if(acc->group == newgroup){
            ns_message(uid, "You already belong to that group.");
            return;
        }
        addNickToGroup(acc, newgroup);
        aclog(LOG_REGISTER,"Group: %s!%s@%s has grouped their nick to %s.\n", U->nick, U->ident, U->host, target);
    }
    setMetaValue(&U->metadata, "nick", U->nick);
    ns_message(uid, "You have joined %s's group.", target);
}

void ns_grouphelp(char *uid, char *msg){
    char *tmpconf;

    ns_message(uid,
        "Syntax: GROUP target password\n"
        " \n"
        "The GROUP command will add your current nickname to the specified\n"
        "target group. A nick group points to just a single services account,\n"
        "so you may own multiple nicks on the same account/same group. For\n"
        "example, if you had the nicks A and B in your group, you could\n"
        "identify with either A or B and have the same access. Additionally,\n"
        "someone could add B to channel access and you would still get it\n"
        "from nick A.");
    tmpconf = getConfigValue("NickServMaxGroupedNicks");
    if(tmpconf)
        ns_message(uid,"A group may contain %s nicks at maximum.", tmpconf);
}

void ns_identify(char *uid, char *msg){
    user *U;
    char *pass, *spaces, *modes;
    nickaccount *acc;
    U = getUser(uid);
    if(!U)
        return;
    strtok_r(msg," ",&spaces);/* identify */
    /* XXX: Allow for id from another nick? /ns id culex pass */
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
        aclog(LOG_NOAUTH, "%s!%s@%s entered an invalid password.\n", U->nick, U->ident, U->vhost);
        return;
    }
    modes = buildModes(1, MODE_NSREGISTER);
    setMode(nickserv->uid, uid, modes);
    free(modes);
    setMetaValue(&U->metadata, "nick", U->nick);
    ns_message(uid, "You have identified for %s.", U->nick);
    aclog(LOG_DEBUG, "%s!%s@%s has identified.", U->nick, U->ident, U->vhost);
}

void ns_identifyhelp(char *uid, char *msg){
    ns_message(uid,
        "Syntax: IDENTIFY password\n"
        " \n"
        "Identifies you for the current nick. Please note that the password is\n"
        "case-sensitive!");
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

static void setupTables(){
    void *result;
    int nickid, regtime, groupid = 0, oldGroupid = 0, maxnickid = 0, mainnick;
    nickaccount *acc;
    nickgroup *group;
    nickgrouplist *groups;
    char *nick, *email, *passwd, *passmethod;
    db_query("CREATE TABLE IF NOT EXISTS `nickaccount` ("
    "`nickid` int unsigned NOT NULL UNIQUE PRIMARY KEY,"
    "`nick` text (255) NOT NULL,"
    "`groupid` int unsigned NOT NULL,"
    "`regtime` int unsigned NOT NULL);", NULL, NULL);
    db_query("CREATE TABLE IF NOT EXISTS `nickgroup` ("
    "`groupid` int unsigned NOT NULL PRIMARY KEY,"
    "`mainnick` int NOT NULL,"
    "`email` text (255) NOT NULL,"
    "`passwd` blob (32),"
    "`passmethod` blob (4));", NULL, NULL);
    db_query("SELECT `nickid`, `nick`, `regtime`, `nickgroup`.`groupid` AS `groupid`, `mainnick`, `email`, `passwd`, `passmethod` FROM `nickgroup` LEFT JOIN `nickaccount` ON  `nickaccount`.`groupid` = `nickgroup`.`groupid` ORDER BY `nickgroup`.`groupid`;", &result, NULL);
    while(db_fetch_row(result, "isiiisbb", &nickid, &nick, &regtime, &groupid, &mainnick, &email, &passwd, &passmethod)==8){
        if(groupid!=oldGroupid){
            safemallocvoid(group, nickgroup);
            group->groupid = groupid;
            safenmallocvoid(group->email, char, strlen(email)+1);
            strcpy(group->email, email);
            safemallocvoid(group->nicks, nicklist);
            group->nicks = NULL;/* add later */
            group->metadata = NULL;
            group->main = NULL;/* set later */
            group->class = defaultpermclass;
            memcpy(group->passmethod, passmethod, 4);
            memcpy(group->passwd, passwd, 32);
            oldGroupid = groupid;
            groups = registerednickgroups;
            if(!groups){
                safemallocvoid(registerednickgroups, nickgrouplist);
                registerednickgroups->group = group;
                registerednickgroups->next = NULL;
            } else {
                while(groups->next) groups = groups->next;
                safemallocvoid(groups->next, nickgrouplist);
                groups->next->group = group;
                groups->next->next = NULL;
            }
        }
        acc = createNickAccount(nick);
        acc->nickid = nickid;
        acc->regtime = (time_t)regtime;
        if(nickid>maxnickid)
            maxnickid = nickid;
        if(nickid==mainnick)
            group->main = acc;
        addNickToGroup(acc, group);
    }
    cNickGroupID = groupid+1;
    cNickID = maxnickid+1;
}

void ns_save(){
    nickgroup *group;
    nickgrouplist *groups;
    blobdata passwd, passmethod;
    groups = registerednickgroups;
    passwd.size = 32;
    passmethod.size = 4;
    while(groups){
        group = groups->group;
        passwd.data = group->passwd;
        passmethod.data = group->passmethod;
        db_query("UPDATE `nickgroup` SET `passmethod` = ?, `passwd` = ?, `email` = ?, `mainnick` = ? WHERE `groupid` = ?; (`groupid`, `mainnick`, `email`, `passwd`, `passmethod`) VALUES(?, ?, ?, ?, ?);", NULL, "bbsii", passmethod, passwd, group->email, group->main->nickid, group->groupid);
        groups = groups->next;
    }
    /* TODO: Save metadata */
    aclog(LOG_DEBUG,"NickServ Database saved\n");
}

void INIT_MOD(){
    setupTables();
    hook_event(EVENT_LINK, createNickServ);
    hook_event(EVENT_MESSAGE, fireNickServCommand);
    registerNickServCommand("help",ns_help);
    registerNickServCommand("test",testCmd);
    registerNickServCommand("register",ns_register);
    addNickServHelp("REGISTER", "Registers your nick",ns_registerhelp);
    registerNickServCommand("group",ns_group);
    addNickServHelp("GROUP", "Groups your nick",ns_grouphelp);
    registerNickServCommand("identify",ns_identify);
    addNickServHelp("IDENTIFY", "Identifies your nick",ns_identifyhelp);
    registerNickServCommand("set",ns_set);
    addNickServHelp("SET", "Sets options for your nick",ns_sethelp);
    loadModule("ns_set_basic");
    loadModule("ns_info");
    loadModule("ns_set_time");
    loadModule("ns_list");
    loadModule("ns_protection");
    /* loadModule("ns_drop"); Broken */
}

void TERM_MOD(){
    ns_save();
}
