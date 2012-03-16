#include <services.h>
#include <stdarg.h>
#include <string.h>

usernode_t *userlist = NULL;
channode_t *chanlist = NULL;

int char2mode(char modechar){
    /* converts 'A' (or 'a') to MODE_A */
    return 1<<((modechar&0x5F)-0x41);
}

char *build_modes(int count, ...){
    char *modes, modechar;
    va_list args;
    int addPos = 1, remPos, i, j, arg, mode;

    remPos = count+1;
    /* +ABC-DEF; addPos = A (1), remPos = F (7) */
    va_start(args, count);
    safenmalloc(modes, char, count+2);
    for(i = 0; i < count; i++){
        arg = va_arg(args, int);
        mode = (arg&MODE_CHAR);
        if(mode == MODE_NONE){
            modes[addPos++] = '/';/* filler character, will be replaced */
            continue;
        }
        j = 0;
        while(1<<j != mode) j++;
        modechar = j+0x41;
        if(!(arg&MODE_MAJOR))
            modechar |= 0x20;
        if(arg&MODE_REMOVE){
            modes[remPos--] = modechar;
        } else {
            modes[addPos++] = modechar;
        }
    }
    if(remPos == count+1)
        modes[remPos] = '\0';/* stop at - sign */
    else
        modes[remPos] = '-';
    if(addPos == 1){
        modes[0] = '/';
    }else{
        modes[0] = '+';
    }
    j = 0;
    for(i=1; i < count+3; i++){
        if(modes[i] != '/')
            modes[j++] = modes[i];
    }/* shift entire thing down */
    modes[j] = '\0';
    va_end(args);
    return modes;
}

bool check_modes(modes_t modes, int count, ...){
    va_list args;
    int i, mode, arg;

    for(i = 0; i < count; i++){
        arg = va_arg(args, int);
        mode = (arg&MODE_CHAR);
        if(arg&MODE_REMOVE){
            if((arg&MODE_MAJOR)&&(modes.major&mode)){
                return FALSE;
            }else if(!(arg&MODE_MAJOR)&&(modes.minor&mode)){
                return FALSE;
            }else{
                continue;
            }
        }else{
            if((arg&MODE_MAJOR)&&(modes.major&mode)){
                continue;
            }else if(!(arg&MODE_MAJOR)&&(modes.minor&mode)){
                continue;
            }else{
                return FALSE;
            }
        }
    }
    return TRUE;
}

int irccmp(char *str1, char *str2){
    int i, len;
    char c1, c2;

    if((!str1)&&(str2))
        return -1;
    if((str1)&&(!str2))
        return 1;
    if((!str1)&&(!str2))
        return 0;
    len = strlen(str1);
    for(i=0; i<len; i++){
        c1 = str1[i];
        c2 = str2[i];
        if(c1 < c2)
            return -1;
        if(c1 > c2)
            return 1;
    }
    if(str2[i])
        return -1;
    return 0;
}

int irccasecmp(char *str1, char *str2){
    int i, len;
    char c1, c2;

    if((!str1)&&(str2))
        return -1;
    if((str1)&&(!str2))
        return 1;
    if((!str1)&&(!str2))
        return 0;
    len = strlen(str1);
    for(i=0; i<len; i++){
        c1 = str1[i];
        c2 = str2[i];
        c1 = (c1 == '~') ? '~' : c1&0xDF;
        c2 = (c2 == '~') ? '~' : c2&0xDF;
        if(c1 < c2)
            return -1;
        if(c1 > c2)
            return 1;
    }
    if(str2[i])
        return -1;
    return 0;
}

user_t *addUser(char *uid, char *nick, char *ident, char *host, char *ip, 
    char *vhost, char *gecos, char *modes){
    /* all data is copied so that anything can be erased without this
        structure being disturbed. It will clean up after itself later */
    user_t *user;
    usernode_t *node;
    int i, modelen, minor, major;

    safemalloc(user, user_t);
    safecpy(user->uid, uid);
    safecpy(user->nick, nick);
    safecpy(user->ident, ident);
    safecpy(user->ip, host);
    safecpy(user->vhost, vhost);
    safecpy(user->gecos, gecos);
    major = 0;
    minor = 0;
    modelen = strlen(modes);
    for(i=0; i < modelen; i++){
        if(modes[i] == '+')
            continue;
        if(modes[i]&0x20)
            minor |= char2mode(modes[i]);
        else
            major |= char2mode(modes[i]);
    }
    user->modes.major = major;
    user->modes.minor = minor;
    user->chanlist = NULL;
    user->metadata = NULL;
    safemalloc(node, usernode_t);
    node->user = user;
    PREPEND(node, userlist);
    aclog(LOG_DEBUG, "Introduced user: %s (%s).\n", user->nick, user->uid);
    return user;
}

void changeMode(modes_t *modes, char *modestr){
    bool isAdding = TRUE;

    if(!modes||!modestr)
        return;
    while(modestr[0]){
        if(modestr[0] == '+'){
            isAdding = TRUE;
        }else if(modestr[0] == '-'){
            isAdding = FALSE;
        }else if(modestr[0]&0x20){
            if(isAdding)
                modes->minor |= char2mode(modestr[0]);
            else
                modes->minor &= ~char2mode(modestr[0]);
        }else{
            if(isAdding)
                modes->major |= char2mode(modestr[0]);
            else
                modes->major &= ~char2mode(modestr[0]);
        }
        modestr++;
    }
}

void changeNick(user_t *user, char *nick){
    if(!user||!nick)
        return;
    safefree(user->nick);
    safecpy(user->nick, nick);
}

user_t *getUser(char *uid){
    user_t *user;
    usernode_t *node;
    
    if(EMPTY(userlist))
        return NULL;
    node = userlist;
    do{
        user = node->user;
        if(!strcmp(uid, user->uid)){
            return user;
        }
    }while(ITER(node));
    return NULL;
}

user_t *getUserByNick(char *nick){
    user_t *user;
    usernode_t *node;
    
    if(EMPTY(userlist))
        return NULL;
    node = userlist;
    do{
        user = node->user;
        if(!strcmp(nick, user->nick)){
            return user;
        }
    }while(ITER(node));
    return NULL;
}

chan_t *addChannel(char *name, char *modes, int paramCount, char **params){
    chan_t *chan;
    channode_t *node;
    int i, modelen;
    if((chan = getChannel(name))){
        return chan;
    }
    safemalloc(chan, chan_t);
    safecpy(chan->name, name);
    chan->topic = NULL;
    chan->metadata = NULL;
    chan->modes.major = 0;
    chan->modes.minor = 0;
    if(modes){
        /* CHANMODES=IXbe,k,FLjl,ACKMNOQRSTcimnprst */
        modelen = strlen(modes);
        for(i=0; i<modelen; i++){
            if(modes[i]=='+')
                continue;
            if(modes[i]&0x20)
                chan->modes.minor |= char2mode(modes[i]);
            else
                chan->modes.major |= char2mode(modes[i]);
        }
    }
    chan->userlist = NULL;
    safemalloc(node, channode_t);
    node->chan = chan;
    PREPEND(node, chanlist);
    aclog(LOG_DEBUG, "Introduced channel %s.\n", chan->name);
    return chan;
}

chan_t *getChannel(char *name){
    chan_t *chan;
    channode_t *node;
    
    if(EMPTY(chanlist))
        return NULL;
    node = chanlist;
    do{
        chan = node->chan;
        if(!strcasecmp(name, chan->name)){
            return chan;
        }
    }while(ITER(node));
    return NULL;
}

statusnode_t *addChannelUser(chan_t *chan, user_t *user){
    statusnode_t *usernode;
    channode_t *channode;

    if((!chan)||(!user))
        return NULL;
    safemalloc(usernode, statusnode_t);
    safemalloc(channode, channode_t);
    usernode->user = user;
    usernode->modes.minor = 0;
    usernode->modes.major = 0;
    channode->chan = chan;
    PREPEND(usernode, chan->userlist);
    PREPEND(channode, user->chanlist);
    return usernode;
}

void delChannelUser(chan_t *chan, user_t *user){
    channode_t *cnode, *cprev = NULL;
    statusnode_t *unode, *uprev = NULL;
    
    if((!chan)||(!user))
        return;
    cnode = user->chanlist;
    while(cnode){
        if(cnode->chan == chan)
            break;
        cprev = cnode;
        ITER(cnode);
    }
    if(!cnode)
        return;
    if(!cprev)/* first in list */
        user->chanlist = cnode->next;
    else
        cprev->next = cnode->next;
    safefree(cnode);

    unode = chan->userlist;
    while(unode){
        if(unode->user == user)
            break;
        uprev = unode;
        ITER(unode);
    }
    if(!unode)
        return;
    if(!uprev)/* first in list */
        chan->userlist = unode->next;
    else
        uprev->next = unode->next;
    safefree(unode);
}

int chanStatusAppend(char *channame, char *status){
    /* appends multiple users the channel list from a status string formatted like "o,3AZAAAAAA vh,3AZAAAAAB" */
    int success = 0;
    chan_t *chan;
    user_t *user;
    char *mode, *uid, continueAfter=1;
    statusnode_t *snode;

    chan = getChannel(channame);
    if(!chan)
        return -1;
    mode = status;
    while(continueAfter){
        if(status[0] == ','){
            status[0] = '\0';
            uid = status+1;
        }else if((status[0] == ' ')||(status[0] == '\0')){
            continueAfter = status[0];
            status[0] = '\0';
            user = getUser(uid);
            snode = addChannelUser(chan, user);
            while(mode[0]){
                if(mode[0]&0x20)
                    snode->modes.minor |= char2mode(mode[0]);
                else
                    snode->modes.major |= char2mode(mode[0]);
                mode++;
            }
            aclog(LOG_DEBUG, "Added %s to %s with mode %d.\n", user->nick, chan->name, snode->modes.minor);
            success++;
            mode = status+1;
        }
        status++;
    }

    return success;
}

