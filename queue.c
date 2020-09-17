#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    void *val;
    struct node *next;
} node;

typedef struct queue
{
    node *head;
    node *tail;
} queue;

node *newNode()
{
    node *n = (node *)malloc(sizeof(node));
    n->val = NULL;
    n->next = n;
    return n;
}

queue *newQueue()
{
    queue *q = (queue *)malloc(sizeof(queue));
    q->head = newNode();
    q->tail = q->head;
    return q;
}

void push(queue *q, char *val)
{
    q->tail->next = newNode();
    q->tail->val = val;
    q->tail = q->tail->next;
}

void *pop(queue *q)
{
    void *res = q->head->val;
    node *old = q->head;
    q->head = q->head->next;
    if (q->head != old)
    {
        free(old);
    }
    return res;
}

void freeQueue(queue *q)
{
    void *val;
    while ((val = pop(q)))
    {
        free(val);
    };
    free(q->head);
    free(q);
}