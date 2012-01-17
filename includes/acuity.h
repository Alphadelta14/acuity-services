#ifndef _ACUITY_H
#define _ACUITY_H

extern const char __version__[];
extern time_t starttime;

//#ifndef MODULE
void (*aclog)(int level, char *fmt, ...);
//#endif /* MODULE */
#define LOG_DEBUG    0x001 /* Very, very verbose for debugging. DO NOT use on production nets */
#define LOG_ERROR    0x002 /* Errors breaking Acuity or being close to that */
#define LOG_MODINFO  0x004 /* compat */
#define LOG_SERVICE  0x008 /* compat */
#define LOG_OPTION   0x010 /* Usage of option changing commands (this should mainly include all
                           * changes that could cause a user to ask in #help why X is broken.
                           * Applies to ALL services. */
#define LOG_OVERRIDE 0x020 /* Usage of admin commands (F*, OperServ minus help/staff, ...) */
#define LOG_REGISTER 0x040 /* Usage of all REGISTER commands and expirations */
#define LOG_REQUEST  0x080 /* Usage of all request commands. Includes hostserv request */
#define LOG_ACCESS   0x100 /* Usage of access change commands (xop,access,flags,akick(!)) */
#define LOG_REGAIN   0x200 /* Usage of INVITE/GETKEY/CLEAR/... used to regain chan in a silly
                            * takeover attempt */
#define LOG_IDFAIL   0x400 /* Failed IDENTIFY attempts. Succeeded ones aren't relevant for opers
                            * simply doing their job. If you REALLY want them, use LOG_DEBUG */
/* Default collections [DO NOT WRITE TO THESE, THEY'RE JUST USED FOR OUTPUT!] */
#define LOG_STANDARD LOG_ERROR|LOG_OPTION|LOG_OVERRIDE|LOG_REQUEST|LOG_IDFAIL
#define LOG_VERBOSE  LOG_ERROR|LOG_OPTION|LOG_OVERRIDE|LOG_REGISTER|LOG_REQUEST|LOG_ACCESS|LOG_REGAIN|LOG_IDFAIL
/* Output debug: */
#define LOG_ODEBUG   LOG_ERROR|LOG_OPTION|LOG_OVERRIDE|LOG_REGISTER|LOG_REQUEST|LOG_ACCESS|LOG_REGAIN|LOG_IDFAIL|LOG_DEBUG

#endif /* _ACUITY_H */
