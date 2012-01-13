#ifndef _ACUITY_H
#define _ACUITY_H

extern const char __version__[];
extern time_t starttime;

//#ifndef MODULE
void (*aclog)(int level, char *fmt, ...);
//#endif /* MODULE */
#define LOG_DEBUG   0x01
#define LOG_ERROR   0x02
#define LOG_MODINFO 0x04
#define LOG_SERVICE 0x08

#endif /* _ACUITY_H */
