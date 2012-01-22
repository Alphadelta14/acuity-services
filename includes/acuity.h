#ifndef _ACUITY_H
#define _ACUITY_H

extern const char __version__[];
extern time_t starttime;

//#ifndef MODULE
void (*aclog)(int flags, ...);
//#endif /* MODULE */
#define LOG_DEBUG    0x001 /* Very, very verbose for debugging. DO NOT use on production nets */
#define LOG_ERROR    0x002 /* Errors breaking Acuity or being close to that */
/*#define LOG_MODINFO  0x004 unused, replaced with LOG_DEBUG */
/*#define LOG_SERVICE  0x008 unused, replaced with the LOG_* following this line */
#define LOG_OPTION   0x010 /* Usage of option changing commands (this should mainly include all
                           * changes that could cause a user to ask in #help why X is broken.
                           * Applies to ALL services. */
#define LOG_OVERRIDE 0x020 /* Usage of admin commands (F*, OperServ minus help/staff, ...) */
#define LOG_REGISTER 0x040 /* Usage of all REGISTER commands and expirations */
#define LOG_REQUEST  0x080 /* Usage of all request commands. Includes hostserv request */
#define LOG_ACCESS   0x100 /* Usage of access change commands (xop,access,flags,akick(!)) */
#define LOG_REGAIN   0x200 /* Usage of INVITE/GETKEY/CLEAR/... used to regain chan in a silly
                            * takeover attempt */
#define LOG_NOAUTH   0x400 /* Failed IDENTIFY attempts. Succeeded ones aren't relevant for opers
                            * simply doing their job. If you REALLY want them, use LOG_DEBUG.
                            * NOAUTH also catches OperServ being used by non-opers and other access
                            * denied messages. Could become somewhat spammy/spying when used with
                            * channel actions being rejected? Not sure. */

#define LOGFLAG_SRC  0x1000000 /* Flag combined with other log options allows a user to specified 
                                * as the source of a log entry */

/* Default collections [DO NOT WRITE TO THESE, THEY'RE JUST USED FOR OUTPUT!] */
#define LOG_STANDARD LOG_ERROR|LOG_OPTION|LOG_OVERRIDE|LOG_REQUEST|LOG_NOAUTH
#define LOG_VERBOSE  LOG_ERROR|LOG_OPTION|LOG_OVERRIDE|LOG_REGISTER|LOG_REQUEST|LOG_ACCESS|LOG_REGAIN|LOG_NOAUTH
/* Output debug: */
#define LOG_ODEBUG   LOG_ERROR|LOG_OPTION|LOG_OVERRIDE|LOG_REGISTER|LOG_REQUEST|LOG_ACCESS|LOG_REGAIN|LOG_NOAUTH|LOG_DEBUG

#endif /* _ACUITY_H */
