#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netutil.h"

typedef struct _cell_t_ {
    char             key[MAC_LEN+1];
    int              sock;
    struct _cell_t_ *next;
} Cell;

typedef struct _table_t_ {
    Cell *table[MAXIFNUM];
} HashTable;

/* hash api */
HashTable *hash_init_table();
int        hash_uninit_table(HashTable *ht);
int        hash_put(HashTable *ht, char *key, int sock);
int        hash_get(HashTable *ht, char *key);
int        hash_delete(HashTable *ht, char *key);
void       print_table(HashTable *ht);

/* static */
static void  hash_free_cells(Cell *cell);
static int   hash_calc(char *key);
static Cell *hash_get_cell(HashTable *ht, char *key);


HashTable *hash_init_table()
{
    int        i;
    HashTable *ht;

    ht = (HashTable*)malloc(sizeof(HashTable));
    if (ht == NULL) {
        return NULL;
    }

    for (i=0; i<MAXIFNUM; i++) {
        ht->table[i] = NULL;
    }

    return ht;
}

int hash_uninit_table(HashTable *ht)
{
    int i;
    for (i=0; i<MAXIFNUM; i++) {
        hash_free_cells(ht->table[i]);
    }
    return 0;
}

int hash_put(HashTable *ht, char *key, int socknum)
{
    int   hash_val;
    Cell *cell, *newcell;

    /* replace sock if (key,sock) pair already exists */
    cell = hash_get_cell(ht, key);
    if (cell) {
        cell->sock = socknum;
        printf("put item, key:%s, sock:%d\n", cell->key, cell->sock);
        return 0;
    }

    /* create new cell */
    newcell = (Cell*)malloc(sizeof(Cell));
    if (newcell == NULL) {
        return -1;
    }
    strncpy(newcell->key, key, MAC_LEN);
    newcell->sock = socknum;
    newcell->next = NULL;

    hash_val = hash_calc(key);
    cell     = ht->table[hash_val];

    if (cell) {
        while (cell->next) cell = cell->next;
        cell->next              = newcell;
    } else {
        ht->table[hash_val]     = newcell;
    }

    printf("put item, key: %s, sock: %d\n", newcell->key, newcell->sock);
    return 0;
}

int hash_get(HashTable *ht, char *key)
{
    Cell *cell;
    
    cell = hash_get_cell(ht, key);
    if (cell) {
        printf("get sock of hashtable(MAC: %s): sock: %d\n", key, cell->sock);
    } else {
        printf("get sock of hashtable(MAC: %s): NOT FOUND\n", key);
    }
    
    return cell->sock;
}

int hash_delete(HashTable *ht, char *key)
{
    int   hash_val;
    Cell *cell, *tmp;

    hash_val = hash_calc(key);
    cell     = ht->table[hash_val];
    tmp      = NULL;
    while (cell) {
        if (strncmp(cell->key, key, strlen(cell->key)) == 0) {
            if (tmp == NULL) {
                ht->table[hash_val] = cell->next;
            } else {
                tmp->next           = cell->next;
            }
            printf("delete item, key: %s, sock: %d\n", cell->key, cell->sock);
            free(cell);
            return 0;
        }
        tmp  = cell;
        cell = cell->next;
    }
    return -1;                  /* not exists */
}

void print_table(HashTable *ht)
{
    int   i;
    Cell *cell;
    puts("=== table ===");
    for (i=0; i<MAXIFNUM; i++) {
        printf("%d:\n", i);
        cell     = ht->table[i];
        while (cell) {
            printf("  key: %s, sock: %d\n", cell->key, cell->sock);
            cell = cell->next;
        }
    }
    puts("=============");
}

static void hash_free_cells(Cell *cell)
{
    if (cell == NULL) {
        return;
    }
    if (cell->next != NULL) {
        hash_free_cells(cell->next);
    }
    free(cell);
    return;
}

static int hash_calc(char *key)
{
    int hash_val       = 0;
    int c;
    while ((c = *key) != '\0') {
        hash_val      += (int)c;
        key++;
    }
    
    return (hash_val % MAXIFNUM);
}

static Cell *hash_get_cell(HashTable *ht, char *key)
{
    int   hash_val;
    Cell *cell;

    hash_val                                            = hash_calc(key);
    cell                                                = ht->table[hash_val];
    while (cell) {
        if (strncmp(cell->key, key, strlen(cell->key)) == 0) {
            return cell;
        }
        cell = cell->next;
    }
    return NULL;
}
#if 0
int main(int argc, char* argv[])
{
    HashTable *ht;
    char      *val;

    ht = hash_init_table();

    hash_put(ht, "apple", "red");
    hash_put(ht, "lemon", "yellow");
    hash_put(ht, "orange", "orange");
    
    print_table(ht);

    hash_put(ht, "orange", "green");

    print_table(ht);

    hash_delete(ht, "orange");

    print_table(ht);

    val = hash_get(ht, "apple");
    val = hash_get(ht, "lemon");
    val = hash_get(ht, "orange");

    hash_uninit_table(ht);
    return 0;
}
#endif
