#include <services.h>
#include <stdio.h>
#include <string.h>

static char servicesId[] = "000";

bool isUser(char *target);

line_t parseLineInsp(char *data){
    /* :SENDER COMMAND PARAM1 PARAM2 PARAM3 :TEXT */
    line_t line;
    char *dataptr;
    int paramCount = 0;

    line.id = NULL;
    line.command = NULL;
    line.params = NULL;
    line.paramCount = 0;
    line.text = NULL;
    if(!data||!data[0])
        return line;
    if(data[0] == ':'){
        line.id = data+1;
        while(data[0] != ' ') data++;
        data[0] = '\0';
        data++;
    }
    line.command = data;
    while(data[0] != ' '){
        if(data[0] == '\0') return line;
        data++;
    }
    data[0] = '\0';
    dataptr = ++data;
    while((dataptr[0] != ':')||((dataptr[0] == ':')&&(dataptr[-1] != ' ')&&(dataptr[-1] != '\0'))){
        /* include the last space, so that paramCount increases */
        if(dataptr[0] == ' '){
            paramCount++;
        }else if(dataptr[0] == '\0'){
            paramCount++;
            break;
        }
        dataptr++;
    }
    line.paramCount = paramCount;
    if(line.paramCount){
        paramCount = 0;
        line.params = (char**)malloc(sizeof(char*)*line.paramCount);
        line.params[paramCount++] = data;
        while((data[1] != ':')||((data[1] == ':')&&(data[0] != ' ')&&(data[0] != '\0'))){
            if(data[0] == ' '){
                line.params[paramCount++] = data+1;
                data[0] = '\0';
            }else if(data[0] == '\0'){
                return line;
            }
            data++;
        }
    }
    if(data[0] == ' '){
        data[0] = '\0';
        data++;
    }
    if(data[0] == ':')
        line.text = data+1;
    return line;
}

void handleLineInsp(line_t *l){
    /* let's give everyone some nice variables to work with */
    char s[512];
    user_t *user;
    chan_t *chan;

    if(!strcmp(l->command,"PING")){
        sprintf(s,":%s PONG %s %s\r\n", servicesId,
            l->params[1], l->params[0]);
        send_raw(s);
        fire_event("ping", l, 0);
    } else if(!strcmp(l->command, "CAPAB")){
        if(l->paramCount < 1)
            return;
        if(!strcmp(l->params[0], "CAPABILITIES")){
            /*if(l->text)
                updateRemoteConf(l->text);*/
/*CAPAB ((null)) +1:
	param[0]: CAPABILITIES
	text: NICKMAX=32 CHANMAX=65 MAXMODES=20 IDENTMAX=12 MAXQUIT=256 MAXTOPIC=308 MAXKICK=256 MAXGECOS=129 MAXAWAY=201 IP6SUPPORT=1 PROTOCOL=1202 CHALLENGE=gLKvIN{mvTOq\a}ZU|ve PREFIX=(Yqaohv)!~&@%+ CHANMODES=IXbe,k,FLjl,ACKMNOQRSTcimnprst USERMODES=,,s,ABHIRSWcdhikorwx SVSPART=1*/
        } else if(!strcmp(l->params[0],"END")){
            sprintf(s, ":%s BURST\r\n:%s VERSION :%s\r\n", servicesId, servicesId, __version__);
            send_raw(s);
            fire_event("link", l, 0);
            sprintf(s, ":%s ENDBURST\r\n", servicesId);
            send_raw(s);
        }
        if(l->paramCount < 2)
            return;
        if(!strcmp(l->params[0], "START")){
            sprintf(s, "CAPAB START %s\r\nCAPAB CAPABILITIES :PROTOCOL=%s\r\n"
                "CAPAB END\r\n", l->params[1], l->params[1]);
            send_raw(s);
            sprintf(s, "SERVER %s %s 0 %s :%s\r\n",
                get_config_value("ServerName"), get_config_value("HubPass"),
                get_config_value("ServerId"), get_config_value("ServerDesc"));
            send_raw(s);
        }
    } else if(!strcmp(l->command, "PRIVMSG")){
        fire_event("message", l, 0);
/*PRIVMSG (644AAAAAA) +1:
	param[0]: 4CEAAAAAB
	text: hi*/
    } else if(!strcmp(l->command, "BURST")){
/*BURST (644) +1:
	param[0]: 1325188756*/
    } else if(!strcmp(l->command, "ENDBURST")){
        fire_event("endlink", l, 0);
/*ENDBURST (644) +0:*/
    } else if(!strcmp(l->command, "VERSION")){
/*VERSION (644) +0:
	text: InspIRCd-2.0 ext.pokesplash.net :*/
    } else if(!strcmp(l->command, "SERVER")){
/*SERVER ((null)) +4:
	param[0]: ext.pokesplash.net
	param[1]: as2bitz
	param[2]: 0
	param[3]: 644
	text: AlphaServ*/
    } else if(!strcmp(l->command, "ADDLINE")){
/*ADDLINE (644) +5:
	param[0]: Q
	param[1]: ChanServ
	param[2]: <Config>
	param[3]: 1325174985
	param[4]: 0
	text: Reserved For Services*/
    } else if(!strcmp(l->command, "FJOIN")){
        addChannel(l->params[0], l->params[2], l->paramCount-3, l->params+3 );
        if(l->text){
            chanStatusAppend(l->params[0], l->text);
        } else {
            chanStatusAppend(l->params[0], l->params[l->paramCount-1]);
        }
        fire_event("join", l, 0);
/*FJOIN (644) +4:
	param[0]: #lobby
	param[1]: 1325186943
	param[2]: +nt
	param[3]: o,644AAAAAA

FJOIN (644) +4:
	param[0]: #lobby
	param[1]: 1325526922
	param[2]: +lnt
	param[3]: 60
	text: o,644AAAAAA ,644AAAAAC*/
    } else if(!strcmp(l->command, "UID")){
        addUser(l->params[0], l->params[2], l->params[5], l->params[3],
            l->params[6], l->params[4], l->text, l->params[8]);
        fire_event("connect", l, 0);
/*UID (644) +9:
	param[0]: 644AAAAAA
	param[1]: 1325186942
	param[2]: Yellow
	param[3]: 127.0.0.1
	param[4]: as-i9h.1pp.0.127.IP
	param[5]: Amarillo
	param[6]: 127.0.0.1
	param[7]: 1325186947
	param[8]: +ix
	text: Yellow
-- ipv6 --
UID (644) +9:
	param[0]: 644AAAAAB
	param[1]: 1325455521
	param[2]: Yellow
	param[3]: 2001:470:1f06:1057::2
	param[4]: as-k62bl1.ajk7.13gs.0470.2001.IP
	param[5]: Alpha
	param[6]: 2001:470:1f06:1057::2
	param[7]: 1325455526
	param[8]: +ix
	text: Alpha*/
    } else if(!strcmp(l->command, "QUIT")){
        fire_event("quit", l, 0);
/*QUIT (644AAAAAA) +5:
	param[0]: :Quit:
	param[1]: Connection
	param[2]: reset
	param[3]: by
	param[4]: FEAR*/
    } else if(!strcmp(l->command, "NICK")){
        user = getUser(l->id);
        changeNick(user, l->params[0]);
        fire_event("nick", l, 0);
/*NICK (644AAAAAA) +2:
	param[0]: Yellow
	param[1]: 1326066127 */
    } else if(!strcmp(l->command, "MODE")){
        if(!isUser(l->params[0])){
            chan = getChannel(l->params[0]);
            changeMode(&chan->modes, l->params[1]);
        }else{
            user = getUser(l->params[0]);
            changeMode(&user->modes, l->params[1]);
        }
        /* TODO: the last parameter */
/*MODE (644) +3:
	param[0]: 644AAAAAA
	param[1]: +s
	param[2]: +aAcCdDfFgGjJkKlLnNoOqQtTvVxX*/
        fire_event("mode", l, 0);
    } else if(!strcmp(l->command,"FMODE")){
        if(!isUser(l->params[0])){
            chan = getChannel(l->params[0]);
            changeMode(&chan->modes, l->params[2]);
        }else{
            user = getUser(l->params[0]);
            changeMode(&user->modes, l->params[2]);
        }/* TODO: the last parameter */
/*FMODE (644AAAAAA) +4:
	param[0]: #lobby
	param[1]: 1325526922
	param[2]: +h
	param[3]: 644AAAAAC*/
        fire_event("fmode", l, 0);
    } else if(!strcmp(l->command, "TOPIC")){
/*TOPIC (644AAAAAA) +1:
	param[0]: #lobby
	text: hi*/
    } else if(!strcmp(l->command, "FTOPIC")){
/*FTOPIC (644) +3:
	param[0]: #lobby
	param[1]: 1325533724
	param[2]: Yellow!Amarillo@netadmin.pokesplash.net
	text: hi*/
    } else if(!strcmp(l->command, "PART")){
        fire_event("part", l, 0);
        delChannelUser(getChannel(l->params[0]), getUser(l->id));
/*PART (644AAAAAC) +1:
	param[0]: #lobby
	text: Removed by Yellow: No reason given*/
    } else if(!strcmp(l->command, "OPERTYPE")){
/*OPERTYPE (644AAAAAA) +1:
	param[0]: NetAdmin*/
    } else if(!strcmp(l->command, "FHOST")){
/*FHOST (644AAAAAA) +1:
	param[0]: netadmin.pokesplash.net*/
    } else if(!strcmp(l->command, "SAVE")){
/*SAVE (644) +2:
	param[0]: 4CEAAAAAB
	param[1]: 1325633878*/
    } else if(!strcmp(l->command, "IDLE")){
        if(l->paramCount>1){
           aclog(LOG_ERROR, "handleLineInsp(): We got a reply to IDLE, but"
                "didn't send any WHOISes?!");
            return;
        }
        sprintf(s, ":%s IDLE %s %lu 0\r\n", l->params[0], l->id, (long unsigned)starttime);
        send_raw(s);
/*IDLE (392AAAAAH) +1:
	param[0]: 5RVAAAAAA
	-- This acts as a remote WHOIS, but is InspIRCd-specific! Handle it here.
	   I've just decided that we'd just always send out the 0 as idle time.
	   Honestly, who cares about real idle time? --culex
*/
    }else{
        aclog(LOG_DEBUG, "Got unhandled command: %s\n", l->command);
    }
}

void handleCloseInsp(char *reason){
    char buff[512];
    snprintf(buff, 512, ":%s SNONOTICE L :%s\r\n", servicesId, reason);
    send_raw(buff);
    snprintf(buff, 512, ":%s SQUIT %s :%s\r\n", servicesId, get_config_value("ServerName"), reason);
    send_raw(buff);
}

bool isUser(char *target){
    /* is first character a number? if so, then it's a user and not a channel */
    return ((target[0] <= '9')&&(target[0] >= '0')) ? TRUE : FALSE;
}

void INIT_MOD(){
    char *sid;
    sid = get_config_value("ServerId");
    if(sid)
        strncpy(servicesId, sid, 3);
    else
        aclog(LOG_DEBUG, "No `ServerId` supplied in config file. Using '000'.\n");
    parseLine = &parseLineInsp;
    handleLine = &handleLineInsp;
    handleClose = &handleCloseInsp;
}
