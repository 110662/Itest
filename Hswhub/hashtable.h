#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netutil.h"

#define TABLE_SIZE 16 
#define mac_SIZE 18
#define VALUE_SIZE 1024

typedef struct _cell_t_ {
    char            mac[MAC_LEN+1];
    char            ifname[IFNAME_LEN+1];
    int             soc;
    int             hash_val;
    struct _cell_t_ *next;
} Cell;

typedef struct _table_t_ {
    Cell *table[TABLE_SIZE];
} HashTable;


/* hash api */
extern HashTable *hash_init_table();
extern int        hash_uninit_table(HashTable *ht);
extern Cell      *hash_put(HashTable *ht, char *mac, char *ifname, int sock);
extern Cell      *hash_get(HashTable *ht, char *mac);
extern int        hash_delete(HashTable *ht, char *mac);
extern void       print_table(HashTable *ht);

