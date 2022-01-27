#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

struct Node
{
    void *data; /* Pentru ca datele stocate sa poata avea orice tip, folosim un pointer la void. */
    struct Node *next;
};

struct LinkedList
{
    struct Node *head;
    struct Node *tail;
    int size;
};

void init_list(struct LinkedList *list);

/*
 * Acestea sunt functiile pe care trebuie sa le implementam.
 * Implementarea acestora se va face in LinkedList.c .
 */

void add_nth_node(struct LinkedList *list, int n, void *new_data);

void add_nth_node_improved(struct LinkedList *list, int n, void *new_data, int new_data_bytes);

void add_last(struct LinkedList *list, void *new_data);

void add_last_improved(struct LinkedList *list, void *new_data, int new_data_bytes);

struct Node *remove_nth_node(struct LinkedList *list, int n);

struct Node *remove_first(struct LinkedList *list);

int exist(struct LinkedList *list, void *new_data);

int get_size(struct LinkedList *list);

void free_list(struct LinkedList *list);

void free_list_improved(struct LinkedList *list);

void print_int64_t_linkedlist(struct LinkedList *list);

void print_string_linkedlist(struct LinkedList *list);

#endif /* __LINKEDLIST_H__ */
