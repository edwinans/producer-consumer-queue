#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "main.h"

pthread_t producers[TH_SZ];
pthread_t consumers[TH_SZ];
pthread_mutex_t q_lock;

//create an empty queue
queue *create_queue(size_t c)
{
    queue *f = malloc(sizeof(queue));
    if (f == NULL)
    {
        printf("error create: malloc failed\n");
        return NULL;
    }
    f->head = NULL;
    f->tail = NULL;
    f->size = 0;
    f->cap = c;
}

//return -1 if cap>size
int enqueue(queue *f, char *s)
{

    //cap atteinte
    if (f->cap <= f->size)
        return -1;

    packet *p = malloc(sizeof(packet));

    if (p == NULL)
    {
        printf("error enqueue: malloc failed\n");
        return -2;
    }

    p->val = s;

    if (f->tail == NULL)
    {
        f->tail = p;
        f->head = p;
        f->head->next = f->tail;
    }
    else
    {
        f->tail->next = p;
        f->tail = p;
    }

    f->size++;

    return 0;
}

//return NULL if f is empty
packet *dequeue(queue *f)
{
    if (f == NULL || f->head == NULL)
    {
        printf("error dequeue: the queue is empty\n");
        return NULL;
    }
    packet *hd = f->head;
    f->head = f->head->next;
    f->size--;
    return hd;
}

int free_queue(queue *f)
{
    packet *cur = f->head;
    packet *next;

    while (cur)
    {
        next = cur->next;
        free(cur);
        cur = next;
    }
}

void print_queue(queue *f)
{
    pthread_mutex_lock(&q_lock);
    packet *cur = f->head;
    printf("[");
    while (cur)
    {
        printf("%s ", cur->val);
        cur = cur->next;
    }
    printf("]\n");
    pthread_mutex_unlock(&q_lock);
}

void *produce(void *arg)
{

    s_target *param = (s_target *)arg;
    queue *buf = param->buf;
    char *name = param->name;
    int t = param->target;

    int i = 0;

    while (i < t)
    {
        char s[MAX_STR] = {0};
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

        // printf("queue: ");
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
            (*cons->counter)--;
        }
    }
}

// n = number of producers, m = number of consumers
void *init_queue(void *nothing)
{
    int n = 3, m = 2;
    int cap = 5;
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
    int *counter_ref = &counter;

    for (int i = 0; i < m; i++)
    {

        consumer *args = malloc(sizeof(consumer));
        args->id = i + 1;
        args->counter = counter_ref;
        args->buf = buf;

        pthread_create(&consumers[i], NULL, consume, args);
    }

    for (int i = 0; i < TH_SZ; i++)
    {
        pthread_join(consumers[i], NULL);
        pthread_join(producers[i], NULL);
    }

    while (*counter_ref)
    {
    }

    free_queue(buf);
    exit(0);

    // print_queue(buf);
}

pthread_t th_main;

int main(int argc, char const *argv[])
{
    pthread_create(&th_main, NULL, init_queue, NULL);

    pthread_join(th_main, NULL);

    return EXIT_SUCCESS;
}
