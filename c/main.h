#ifndef MAIN_H
#define MAIN_H

#define TH_SZ 10
#define MAX_STR 100

typedef struct packet
{
    char *val;
    struct packet *next;
} packet;

typedef struct queue
{
    struct packet *head;
    struct packet *tail;
    size_t size;
    size_t cap;
} queue;

typedef struct s_target
{
    queue *buf;
    char *name;
    int target;
} s_target;

typedef struct consumer
{
    queue *buf;
    int id;
    int *counter;
} consumer;

#endif
