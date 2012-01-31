#ifndef DB_H
#define DB_H

/* XXX: DB_MYSQL ? */
#define DB_SQL 0x1

int db_query(char *query, char *fmt, ...);

int open_database(void);
int close_database(void);

#endif /* DB_H */
