#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "main.h"

#define DEBUG 0

pthread_t th_main;
pthread_t producers[TH_SZ];
pthread_t consumers[TH_SZ];
pthread_mutex_t q_lock;
pthread_mutex_t counter_lock;

//create an empty queue
queue *create_queue(size_t c)
{
    queue *q = malloc(sizeof(queue));
    if (q == NULL)
    {
        printf("error create: malloc failed\n");
        return NULL;
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->cap = c;
}

//return -1 if cap>size
int enqueue(queue *q, char *s)
{

    //cap full
    if (q->cap <= q->size)
        return -1;

    packet *p = malloc(sizeof(packet));

    if (p == NULL)
    {
        printf("error enqueue: malloc failed\n");
        return -2;
    }

    p->val = s;

    if (q->tail == NULL)
    {
        q->head = p;
    }
    else
    {
        q->tail->next = p;
    }

    q->tail = p;
    q->size++;

    return 0;
}

//return NULL if f is empty
packet *dequeue(queue *q)
{
    if (q == NULL || q->head == NULL)
    {
        printf("error dequeue: the queue is empty\n");
        return NULL;
    }
    packet *hd = q->head;
    q->head = q->head->next;

    if (q->head == NULL)
        q->tail = NULL;

    q->size--;

    return hd;
}

//destory the queue
int free_queue(queue *q)
{
    packet *cur = q->head;
    packet *next;

    while (cur)
    {
        next = cur->next;
        free(cur->val);
        free(cur);
        cur = next;
    }

    free(q);
}

//print the queue
void print_queue(queue *q)
{
    pthread_mutex_lock(&q_lock);
    packet *cur = q->head;
    printf("queue: [");
    while (cur)
    {
        printf("%s ", cur->val);
        cur = cur->next;
    }
    printf("]\n");
    pthread_mutex_unlock(&q_lock);
}

void test_queue()
{
    queue *q = create_queue(6);
    enqueue(q, "x1");
    print_queue(q);

    enqueue(q, "x2");
    print_queue(q);
    dequeue(q);
    dequeue(q);

    enqueue(q, "x3");

    print_queue(q);

    dequeue(q);
    print_queue(q);

    free_queue(q);
}

///////////////////////////////QUEUE//////////////////////////////////////

void *produce(void *arg)
{
    s_target *param = (s_target *)arg;
    queue *buf = param->buf;
    char *name = param->name;
    int t = param->target;

    int i = 0;

    while (i < t)
    {
        char *s = malloc(sizeof(char) * MAX_STR);
        sprintf(s, "%s %d", name, i + 1);

        pthread_mutex_lock(&q_lock);
        int r = enqueue(buf, s);
        pthread_mutex_unlock(&q_lock);

        if (r < 0)
        {
            sleep(2);
        }
        else
        {
            i++;
            printf("produce : %s\n", s);
        }

        // print_queue(buf);
    }
}

void *consume(void *arg)
{
    consumer *cons = (consumer *)arg;
    int id = cons->id;
    queue *buf = cons->buf;
    while (*cons->counter)
    {
        pthread_mutex_lock(&q_lock);
        packet *p = dequeue(buf);
        pthread_mutex_unlock(&q_lock);

        if (p == NULL)
        {
            sleep(2);
        }
        else
        {
            printf("C%d mange %s\n", id, p->val);
            pthread_mutex_lock(&counter_lock);
            (*cons->counter)--;
            pthread_mutex_unlock(&counter_lock);
        }
    }
}

// n = number of producers, m = number of consumers
void *init_queue(void *nothing)
{
    int n = 5, m = 4;
    int cap = 3;
    char *names[] = {"Apple", "BlackBerry", "Snack", "Mango", "Orange"};
    int produce_target = 3;

    queue *buf = create_queue(cap);

    for (int i = 0; i < n; i++)
    {

        s_target *args = malloc(sizeof(s_target));
        args->name = names[i];
        args->target = produce_target;
        args->buf = buf;

        pthread_create(&producers[i], NULL, produce, args);
    }

    int counter = n * produce_target;

    for (int i = 0; i < m; i++)
    {

        consumer *args = malloc(sizeof(consumer));
        args->id = i + 1;
        args->counter = &counter;
        args->buf = buf;

        pthread_create(&consumers[i], NULL, consume, args);
    }

    while (counter)
    {
    }

    free_queue(buf);

    return 0;
}

int main(int argc, char const *argv[])
{
    pthread_create(&th_main, NULL, init_queue, NULL);

    pthread_join(th_main, NULL);

    if (DEBUG)
    {
        test_queue();
    }
    return EXIT_SUCCESS;
}
