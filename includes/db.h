#ifndef DB_H
#define DB_H

/* XXX: DB_MYSQL ? */
#define DB_SQL 0x1

int db_query(const char *query, void **result, char *fmt, ...);
int db_fetch_row(void *result, char *fmt, ...);

int open_database(void);
int close_database(void);

#endif /* DB_H */
