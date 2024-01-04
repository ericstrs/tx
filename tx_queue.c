#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tx_queue.h"

buy_queue* init_buy_queue()
{
        buy_queue *b = malloc(sizeof(buy_queue));
        b->head = NULL;
        b->tail = NULL;

        return b;
}

sell_queue* init_sell_queue()
{
        sell_queue *s = malloc(sizeof(sell_queue));
        s->head = NULL;
        s->tail = NULL;

        return s;
}

// add a buy transaction to the buy queue
void enqueue_buy(buy_queue *b, buy_tx *tx)
{
        // if the queue is empty
        if (b->tail != NULL) {
                b->tail->next = tx;
        }
        b->tail = tx;

        // if tx first in the queue
        if (b->head == NULL) {
                b->head = tx;
        }
}

// remove the first buy transaction from the buy queue
void dequeue_buy(buy_queue *b)
{
        // if the queue is empty
        if (b->head == NULL) {
                fprintf(stderr, "Queue is empty");
                return;
        }

        buy_tx *tmp = b->head;

        printf("old head: %lf\n", b->head->buy->asset);
        b->head = b->head->next;
        if (b->head = NULL) {
                b->tail = NULL;
        }

        free(tmp->buy->ticker);
        free(tmp->buy->t_date);
        free(tmp->buy);
        free(tmp);
}

// add a sell transaction to the sell queue
void enqueue_sell(sell_queue *s, sell_tx *tx)
{
        // if the queue is empty
        if (s->tail != NULL) {
                s->tail->next = tx;
        }
        s->tail = tx;

        // if tx is first in the queue
        if (s->head == NULL) {
                s->head = tx;
        }
}

// remove the first sell transaction from the sell queue
void dequeue_sell(sell_queue *s)
{
        // if the queue is empty
        if (s->head == NULL) {
                fprintf(stderr, "Sell queue is empty");
                return;
        }

        sell_tx *tmp = s->head;

        s->head = s->head->next;
        if (s->head = NULL) {
                s->tail = NULL;
        }

        free(tmp->sell->ticker);
        free(tmp->sell->t_date);
        free(tmp->sell);
        free(tmp);
}

// dequeue every tx in given queues
void release(buy_queue *bq)
{

        buy_tx *b = bq->head;
        while (b != NULL) {
                buy_tx *tmp = b;
                free(tmp->buy->ticker);
                free(tmp->buy->t_date);
                free(tmp->buy);
                free(tmp);
                b = b->next;
        }
        free(bq);

        /*
        sell_tx *s = sq->head;
        while (s != NULL) {
                sell_tx *s_tmp = s;
                free(s_tmp->sell->ticker);
                free(s_tmp->sell->t_date);
                free(s_tmp->sell);
                free(s_tmp);
                s = s->next;
        }
        free(sq);
        */
}

void print_sells(sell_queue *q)
{
        sell_tx *tmp = q->head;
        if (tmp == NULL) {
                fprintf(stderr, "Sell queue is empty\n");
                return;
        }

        printf("SELLS:\n");
        while (tmp != NULL) {
               printf("%d-%d-%d,%s,%.1lf,%.1lf,%.1lf,%.1lf\n",
                               tmp->sell->t_date->year, tmp->sell->t_date->day,
                               tmp->sell->t_date->month, tmp->sell->ticker,
                               tmp->sell->asset, tmp->sell->value_at_tx,
                               tmp->sell->fee,tmp->proceeds);
               tmp = tmp->next;
        }
}

void print_buys(buy_queue *q)
{
        buy_tx *tmp = q->head;
        if (tmp == NULL) {
                fprintf(stderr, "Buy queue is empty\n");
                return;
        }

        printf("BUYS:\n");
        while (tmp != NULL) {
               printf("%d-%d-%d,%s,%.1lf,%.1lf,%.1lf,%.1lf\n",
                               tmp->buy->t_date->year, tmp->buy->t_date->day,
                               tmp->buy->t_date->month, tmp->buy->ticker,
                               tmp->buy->asset, tmp->buy->value_at_tx,
                               tmp->buy->fee,tmp->cost_basis);
               tmp = tmp->next;
        }
}
