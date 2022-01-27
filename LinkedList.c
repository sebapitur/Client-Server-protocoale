#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/LinkedList.h"

/*
 * Functie care trebuie apelata dupa alocarea unei liste simplu inlantuite, pentru a o initializa.
 * (Setare valori initiale pentru campurile specifice structurii LinkedList).
 */
void init_list(struct LinkedList *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/*
 * Pe baza datelor trimise prin pointerul new_data, se creeaza un nou nod care e adaugat pe pozitia n a listei
 * reprezentata de pointerul list. Pozitiile din lista sunt indexate incepand cu 0 (i.e. primul nod din lista se afla
 * pe pozitia n=0). Daca n >= nr_noduri, noul nod se adauga la finalul listei.
 */
void add_nth_node(struct LinkedList *list, int n, void *new_data)
{
    struct Node *prev, *curr;
    struct Node *new_node;

    if (list == NULL)
    {
        return;
    }

    /* n >= list->size inseamna adaugarea unui nou nod la finalul listei. */
    if (n > list->size)
    {
        n = list->size;
    }
    else if (n < 0)
    {
        return;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0)
    {
        prev = curr;
        curr = curr->next;
        --n;
    }

    new_node = (struct Node *)malloc(sizeof(struct Node));
    if (new_node == NULL)
    {
        perror("Not enough memory to add element!");
        return;
    }

    new_node->data = new_data;
    new_node->next = curr;
    if (prev == NULL)
    {
        /* Adica n == 0. */
        list->head = new_node;
    }
    else
    {
        prev->next = new_node;
    }

    if (new_node->next == NULL)
    {
        list->tail = new_node;
    }

    list->size++;
}

void add_nth_node_improved(struct LinkedList *list, int n, void *new_data, int new_data_bytes)
{
    struct Node *prev, *curr;
    struct Node *new_node;

    if (list == NULL)
    {
        return;
    }

    /* n >= list->size inseamna adaugarea unui nou nod la finalul listei. */
    if (n > list->size)
    {
        n = list->size;
    }
    else if (n < 0)
    {
        return;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0)
    {
        prev = curr;
        curr = curr->next;
        --n;
    }

    new_node = (struct Node *)malloc(sizeof(struct Node));
    if (new_node == NULL)
    {
        perror("Not enough memory to add element!");
        return;
    }
    new_node->data = malloc(new_data_bytes);
    memcpy(new_node->data, new_data, new_data_bytes);
    // new_node->data = new_data;
    new_node->next = curr;
    if (prev == NULL)
    {
        /* Adica n == 0. */
        list->head = new_node;
    }
    else
    {
        prev->next = new_node;
    }

    if (new_node->next == NULL)
    {
        list->tail = new_node;
    }

    list->size++;
}

void add_last(struct LinkedList *list, void *new_data)
{
    struct Node *new_node;
    if (!list)
        return;
    new_node = malloc(sizeof(struct Node));
    new_node->data = new_data;
    list->size++;
    if (!list->head)
    {
        list->head = new_node;
        list->tail = new_node;
    }
    else
    {
        list->tail->next = new_node;
        list->tail = new_node;
    }
    new_node->next = NULL;
}

void add_last_improved(struct LinkedList *list, void *new_data, int new_data_bytes)
{
    struct Node *new_node;
    if (!list)
        return;
    new_node = malloc(sizeof(struct Node));
    new_node->data = malloc(new_data_bytes);
    memcpy(new_node->data, new_data, new_data_bytes);
    list->size++;
    if (!list->head)
    {
        list->head = new_node;
        list->tail = new_node;
    }
    else
    {
        list->tail->next = new_node;
        list->tail = new_node;
    }
    new_node->next = NULL;
}

struct Node *remove_first(struct LinkedList *list)
{
    struct Node *first;
    if (!list || !list->head)
        return NULL;
    first = list->head;
    if (!first->next)
        list->tail = NULL;
    list->head = first->next;
    list->size--;
    return first;
}

int exist(struct LinkedList *list, void *new_data)
{
    struct Node *curr;
    if (!list)
        return 0;
    curr = list->head;
    while (curr)
    {
        if (*(int64_t *)curr->data == *(int64_t *)new_data)
            return 1;
        curr = curr->next;
    }
    return 0;
}

/*
 * Elimina nodul de pe pozitia n din lista al carei pointer este trimis ca parametru.
 * Pozitiile din lista se indexeaza de la 0 (i.e. primul nod din lista se afla pe pozitia n=0). Daca n >= nr_noduri - 1,
 * se elimina nodul de la finalul listei.
 * Functia intoarce un pointer spre acest nod proaspat eliminat din lista.
 * Este responsabilitatea apelantului sa elibereze memoria acestui nod.
 */
struct Node *remove_nth_node(struct LinkedList *list, int n)
{
    struct Node *prev, *curr;

    if (list == NULL)
    {
        return NULL;
    }

    if (list->head == NULL)
    { /* Lista este goala. */
        return NULL;
    }

    /* n >= list->size - 1 inseamna eliminarea nodului de la finalul listei. */
    if (n > list->size - 1)
    {
        n = list->size - 1;
    }
    else if (n < 0)
    {
        return NULL;
    }

    curr = list->head;
    prev = NULL;
    while (n > 0)
    {
        prev = curr;
        curr = curr->next;
        --n;
    }

    if (prev == NULL)
    {
        /* Adica n == 0. */
        list->head = curr->next;
    }
    else
    {
        prev->next = curr->next;

        if (prev->next == NULL)
        {
            list->tail = prev;
        }
    }

    list->size--;
    return curr;
}

/*
 * Functia intoarce numarul de noduri din lista al carei pointer este trimis ca parametru.
 */
int get_size(struct LinkedList *list)
{
    if (list == NULL)
    {
        return -1;
    }

    return list->size;
}

/*
 * Procedura elibereaza memoria folosita de toate nodurile din lista, iar la sfarsit, elibereaza memoria folosita
 * de structura lista si actualizeaza la NULL valoarea pointerului la care pointeaza argumentul (argumentul este un
 * pointer la un pointer).
 */
void free_list(struct LinkedList *pp_list)
{
    struct Node *currNode;

    if (pp_list == NULL)
    {
        return;
    }

    while (get_size(pp_list) > 0)
    {
        currNode = remove_first(pp_list);
        free(currNode);
    }

    free(pp_list);
    pp_list = NULL;
}

void free_list_improved(struct LinkedList *pp_list)
{
    struct Node *currNode;

    if (pp_list == NULL)
    {
        return;
    }

    while (get_size(pp_list) > 0)
    {
        currNode = remove_first(pp_list);
        free(currNode->data);
        free(currNode);
    }

    free(pp_list);
    pp_list = NULL;
}

/*
 * Atentie! Aceasta functie poate fi apelata doar pe liste ale caror noduri STIM ca stocheaza int-uri.
 * Functia afiseaza toate valorile int stocate in nodurile din lista inlantuita separate printr-un spatiu.
 */
void print_int64_t_linkedlist(struct LinkedList *list)
{
    struct Node *curr;

    if (list == NULL)
    {
        return;
    }

    curr = list->head;
    while (curr != NULL)
    {
        printf("%ld ", *((int64_t *)curr->data));
        curr = curr->next;
    }

    printf("\n");
}

/*
 * Atentie! Aceasta functie poate fi apelata doar pe liste ale caror noduri STIM ca stocheaza string-uri.
 * Functia afiseaza toate string-urile stocate in nodurile din lista inlantuita, separate printr-un spatiu.
 */
void print_string_linkedlist(struct LinkedList *list)
{
    struct Node *curr;

    if (list == NULL)
    {
        return;
    }

    curr = list->head;
    while (curr != NULL)
    {
        printf("%s ", (char *)curr->data);
        curr = curr->next;
    }

    printf("\n");
}
