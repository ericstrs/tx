#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tx_queue.h"

typedef struct {
        double asset;
        char *ticker;
        date *date_acquired;
        date *date_sold;
        double proceeds;
        double cost_basis;
        double net_gain;
} entry;

date* create_date(int year, int day, int month)
{
        date *i = malloc(sizeof(date));

        i->year = year;
        i->day = day;
        i->month = month;

        return i;
}

// create a buy transaction
buy_tx* create_buy_tx(tx *b)
{
        buy_tx *i = malloc(sizeof(buy_tx));

        i->buy = b;

        // determine the cost basis
        int a = b->asset;
        i->cost_basis = (b->value_at_tx * a + b->fee) / a;

        i->next = NULL;

        return i;
}

// create a sell transaction
sell_tx* create_sell_tx(tx *s)
{
        sell_tx *i = malloc(sizeof(sell_tx));

        i->sell = s;
        i->proceeds = (s->asset * s->value_at_tx) - s->fee;
        i->next = NULL;

        return i;
}

// create a transaction
tx* create_tx(date *d, char *t, double a, double val, double fee)
{
        tx *i = malloc(sizeof(tx));

        i->t_date = d;
        i->ticker = strdup(t);
        i->asset = a;
        i->value_at_tx = val;
        i->fifo = a;
        i->fee = fee;

        return i;
}

void release_subset(buy_queue *bq)
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
}

void print_entry(entry *e, FILE *out)
{
        if (e == NULL) {
                fprintf(stderr, "Entry is empty\n");
                return;
        }

       fprintf(out,"%.1lf,%s,%02d/%02d/%d,%02d/%02d/%d,%.1lf,%.1lf,%.1lf\n",
                       e->asset, e->ticker, e->date_acquired->month, 
                       e->date_acquired->day, e->date_acquired->year, 
                       e->date_sold->month, e->date_sold->day, e->date_sold->year,
                       e->proceeds, e->cost_basis, e->net_gain);
}

entry* create_entry(double a, char *t, date *aq, date *sold, double p, double c)
{
        entry *i = malloc(sizeof(entry));
        i->asset = a;
        i->ticker = strdup(t); 
        i->date_acquired = aq;
        i->date_sold = sold;
        i->proceeds = p;
        i->cost_basis = c;
        i->net_gain = p - c;

        return i;
}

// returns a copy of a buy transction
buy_tx* copy_buy(buy_tx *b)
{
        if (b == NULL) {
                fprintf(stderr, "Transaction is null.\n");
        }

        date *d = create_date(b->buy->t_date->year,
                        b->buy->t_date->day,
                        b->buy->t_date->month);
        tx *to = create_tx(d, b->buy->ticker, b->buy->asset,
                        b->buy->value_at_tx, b->buy->fee);
        buy_tx* i = create_buy_tx(to);
        i->b_fifo = &b->buy->fifo;
        i->cost_basis = b->cost_basis;

        return i;
}

// subset of buys with a sell's ticker.
buy_queue* subset_buys(buy_queue *bq, char *t)
{
        buy_queue *subset = init_buy_queue();
        // find buys with ticker t
        if (bq->head == NULL) {
                fprintf(stderr, "Buy queue is empty.\n");
        }

        buy_tx *i;
        buy_tx *tmp = bq->head;

        // traverse bq
        while (tmp != NULL) {
                i = copy_buy(tmp);
                if (*i->b_fifo > 0 && strcmp(t, i->buy->ticker) == 0) {
                        enqueue_buy(subset, i);
                }
                else{
                        free(i->buy->ticker);
                        free(i->buy->t_date);
                        free(i->buy);
                        free(i);
                }
                tmp = tmp->next;
        }
        return subset;
}

void create_entries(buy_queue *bq, sell_queue *sq, char *file)
{
        double asset, proceeds, final_basis, gain;
        date acquired, sold;
        char *ticker;

        FILE *out;
        if (!(out = fopen(file, "w"))) {
                fprintf(stderr, "Can't open out \"%s\" file.", file);
                return;
        }

        buy_queue *buys_subset;
        double *buy_fifo, *sell_fifo, *head_fifo;
        sell_tx *s = sq->head;

        while (s != NULL) {
                sell_fifo = &s->sell->fifo;
                if (*sell_fifo <= 0) {
                        s = s->next;
                        continue;
                }
                ticker = s->sell->ticker;

                buys_subset = subset_buys(bq, ticker); // subset buys on ticker
                buy_tx *first = buys_subset->head; // get head of queue
                head_fifo = first->b_fifo;

                if (*sell_fifo > *head_fifo) {
                        asset = *sell_fifo;
                        *sell_fifo = *sell_fifo - *head_fifo;
                        asset = asset - *sell_fifo;
                        *head_fifo = 0;

                        acquired = *first->buy->t_date;
                        sold = *s->sell->t_date;
                        proceeds = (s->sell->value_at_tx * asset) - s->sell->fee;
                        final_basis = first->cost_basis * asset;

                        // create entry with fields
                        entry *e = create_entry(asset, ticker, 
                                        &acquired, &sold, proceeds, final_basis);
                        // fprintf entry to file
                        print_entry(e, out);

                        // free memory
                        free(e->ticker);
                        free(e);
                        release_subset(buys_subset);

                        continue;
                }
                if (*sell_fifo < *head_fifo) {
                        asset = *sell_fifo;
                        acquired = *first->buy->t_date;
                        sold = *s->sell->t_date;
                        proceeds = (s->sell->value_at_tx * asset) - s->sell->fee;
                        final_basis = first->cost_basis * asset;

                        *head_fifo = *head_fifo - *sell_fifo;
                        *sell_fifo = 0;

                        // create entry with fields
                        entry *e = create_entry(asset, ticker, 
                                        &acquired, &sold, proceeds, final_basis);
                        // fprintf entry to file
                        print_entry(e, out);

                        // free memory
                        free(e->ticker);
                        free(e);
                        release_subset(buys_subset);

                        s = s->next;
                        continue;
                }
                if (*sell_fifo == *head_fifo) {
                        asset = *sell_fifo;
                        acquired = *first->buy->t_date;
                        sold = *s->sell->t_date;
                        proceeds = (s->sell->value_at_tx * asset) - s->sell->fee;
                        final_basis = first->cost_basis * asset;

                        *sell_fifo = 0;
                        *head_fifo = 0;

                        // create entry with fields
                        entry *e = create_entry(asset, ticker, 
                                        &acquired, &sold, proceeds, final_basis);

                        // fprintf entry to file
                        print_entry(e, out);

                        // free memory
                        free(e->ticker);
                        free(e);
                        release_subset(buys_subset);

                        s = s->next;
                        continue;
                }
        }
        fclose(out);
}

// read csv data and put into stuct
void read_csv(char *file, buy_queue *bq, sell_queue *sq)
{
        FILE *in;
        if (!(in = fopen(file, "r"))) {
                fprintf(stderr, "Can't open file.\n");
                return;
        }

        int year, day, month, cols;
        double asset, val, fee;
        char ticker[4];
        char action[4];

        while (!feof(in)) {
                cols = fscanf(in, "%d-%d-%d,%4[^,],%lf,%4[^,],%lf,%lf\n",
                                &year, &day, &month,
                                ticker, &asset, action,
                                &val, &fee);

                if (cols != 8 && !feof(in)) {
                        fprintf(stderr, "Incorrect format.\n");
                        return;
                }

                date *d = create_date(year, day, month);
                tx *t = create_tx(d, ticker, asset, val, fee);

                if (strcmp("buy", action) == 0) {
                        buy_tx *b = create_buy_tx(t);
                        enqueue_buy(bq, b);
                        continue;
                }
                if (strcmp("sell", action) == 0) {
                        sell_tx *s = create_sell_tx(t);
                        enqueue_sell(sq, s);
                        continue;
                }
        }

        fclose(in);
        return;
}

int main(int argc, char *argv[])
{
        buy_queue *buys = init_buy_queue();
        sell_queue *sells = init_sell_queue();

        char *in;
        char *out;
        if (argc > 0) {
                in = argv[1];
                read_csv(in, buys, sells);
                out = argv[2];
        }

        print_buys(buys);
        print_sells(sells);

        create_entries(buys, sells, out);

        release(buys, sells); 

        return 0;
}
