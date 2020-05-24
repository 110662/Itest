#include "hashtable.h"

/* static */
static void  hash_free_cells(Cell *cell);
static int   hash_calc(char *mac);
static Cell *hash_get_cell(HashTable *ht, char *mac);

HashTable *hash_init_table()
{
    int        i;
    HashTable *ht;

    ht = (HashTable*)malloc(sizeof(HashTable));
    if (ht == NULL) {
        return NULL;
    }

    for (i=0; i<TABLE_SIZE; i++) {
        ht->table[i] = NULL;
    }

    return ht;
}

int hash_uninit_table(HashTable *ht)
{
    int i;
    for (i=0; i<TABLE_SIZE; i++) {
        hash_free_cells(ht->table[i]);
    }
    return 0;
}

Cell *hash_put(HashTable *ht, char *mac, char *ifname, int soc)
{
    int   hash_val;
    Cell *cell, *newcell;

    /* replace mac if (mac) already exists */
    cell = hash_get_cell(ht, mac);
    if (cell) {
        if (cell->ifname == NULL){
            strncpy(cell->ifname, ifname, IFNAME_LEN);
        }
        if (cell->soc == 0){
            cell->soc = soc; 
        }
        printf("put item, mac:%s, ifname:%s, soc:%d, hash_val:%d\n", cell->mac, cell->ifname, cell->soc, cell->hash_val);
        return cell;
    }

    /* create new cell */
    newcell = (Cell*)malloc(sizeof(Cell));
    if (newcell == NULL) {
        return NULL;
    }
    int len = strlen(ifname);
    strncpy(newcell->mac, mac, MAC_LEN);
    strncpy(newcell->ifname, ifname,len+1);
    newcell->soc = soc;
    newcell->next = NULL;

    hash_val = hash_calc(mac);
    cell     = ht->table[hash_val];
    newcell->hash_val = hash_val;

    if (cell) {
        while (cell->next) cell = cell->next;
        cell->next              = newcell;
    } else {
        ht->table[hash_val]     = newcell;
    }

    printf("put item, mac: %s, ifname: %s, soc:%d, hash_val: %d\n", newcell->mac, newcell->ifname, newcell->soc, newcell->hash_val);
    return newcell;
}

Cell *hash_get(HashTable *ht, char *mac)
{
    Cell *cell;
    
    cell = hash_get_cell(ht, mac);
    if (cell) {
        printf("get sock of hashtable(MAC: %s): hash_val: %d\n", mac, cell->hash_val);
        return cell;
    } else {
        printf("get sock of hashtable(MAC: %s): NOT FOUND\n", mac);
        return NULL;
    }
    
}

int hash_delete(HashTable *ht, char *mac)
{
    int   hash_val;
    Cell *cell, *tmp;

    hash_val = hash_calc(mac);
    cell     = ht->table[hash_val];
    tmp      = NULL;
    while (cell) {
        if (strncmp(cell->mac, mac, strlen(cell->mac)) == 0) {
            if (tmp == NULL) {
                ht->table[hash_val] = cell->next;
            } else {
                tmp->next           = cell->next;
            }
            printf("delete item, mac: %s, hash_val: %d\n", cell->mac, cell->hash_val);
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
    for (i=0; i<TABLE_SIZE; i++) {
        printf("%d:\n", i);
        cell = ht->table[i];
        while (cell) {
            printf("  mac: %s, hash_val: %d\n", cell->mac, cell->hash_val);
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

static int hash_calc(char *mac)
{
    int hash_val       = 0;
    int c;
    while ((c = *mac) != '\0') {
        hash_val      += (int)c;
        mac++;
    }
    
    return (hash_val % TABLE_SIZE);
}

static Cell *hash_get_cell(HashTable *ht, char *mac)
{
    int   hash_val;
    Cell *cell;

    hash_val = hash_calc(mac);
    cell = ht->table[hash_val];
    while (cell) {
        if (strncmp(cell->mac, mac, strlen(cell->mac)) == 0) {
            cell->hash_val = hash_val;
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
