#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tx_queue.h"

void free_tx(tx *t);

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

int print_entry(entry *e, FILE *out)
{
        if (e == NULL) {
                fprintf(stderr, "Entry is empty\n");
                return 1;
        }

       fprintf(out,"%.1lf,%s,%02d/%02d/%d,%02d/%02d/%d,%.1lf,%.1lf,%.1lf\n",
                       e->asset, e->ticker, e->date_acquired->month, 
                       e->date_acquired->day, e->date_acquired->year, 
                       e->date_sold->month, e->date_sold->day, e->date_sold->year,
                       e->proceeds, e->cost_basis, e->net_gain);
       return 0;
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

void free_tx(tx *t)
{
        free(t->ticker);
        free(t->t_date);
        free(t);
}

/*
 * In FIFO fashion, find first buy with: tx ticker and 
 * non-zero fifo. Then, add transfer fees to buy cost basis.
 */
int transfer(buy_queue *bq, tx *t, FILE *out)
{
        if (bq->head == NULL) {
                fprintf(stderr, "ERROR: Buy queue is empty.\n");
                return 1;
        }
        if (t->fee == 0) {
                /* just create an entry. This is only useful if you
                 * attach location data.
                 */
                fprintf(stderr, "ERROR: Attemping to enter a transfer with a fee of 0.\n");
                return 1;
        }

        buy_tx *b = bq->head;
        while (t->fifo > 0){
                if (b == NULL) {
                        fprintf(stderr, "ERROR: There is no %.3lf %s to transfer\n", t->fifo, t->ticker);
                        return 1;
                }
                if (b->buy->fifo > 0 && strcmp(t->ticker, b->buy->ticker) == 0) {
                        // update transfer asset
                        double fee_to_asset = t->fee / t->value_at_tx;
                        t->asset = t->asset - fee_to_asset;
                        t->fifo -= b->buy->asset;

                        /* AGRESSIVE APPROACH. Unfinished: would need to 
                         * modify buy basis with fee ticker *not*
                         * the basis for buy with transaction ticker.
                         *
                        //recalculate matching buy's cost basis
                        b->cost_basis = (b->buy->value_at_tx * b->buy->asset + 
                                        b->buy->fee + t->fee) / b->buy->asset;
                         */

                        entry *e = create_entry(t->asset, t->ticker, b->buy->t_date, t->t_date, 0, 0);
                        if (print_entry(e, out))
                                return 1;
                        free(e->ticker);
                        free(e);
                }
                b = b->next; 
        }
        return 0;
}

int sell(buy_queue *bq, sell_tx *s, FILE *out)
{
        if (bq->head == NULL) {
                fprintf(stderr, "Buy queue is empty.\n");
                return 1;
        }

        buy_tx *b = bq->head;
        while (s->sell->fifo != 0) {
                if (b == NULL) {
                        fprintf(stderr, "ERROR: there is not enough %s to sell.\n", s->sell->ticker);
                        return 1;;
                }

                if (b->buy->fifo > 0 && strcmp(s->sell->ticker, b->buy->ticker) == 0) {
                        double proceeds, final_basis, gain;
                        date acquired = *b->buy->t_date;
                        date sold = *s->sell->t_date;
                        double asset = s->sell->fifo;

                        if (s->sell->fifo > b->buy->fifo) {
                                s->sell->fifo = s->sell->fifo - b->buy->fifo;
                                b->buy->fifo = 0;

                                asset = asset - s->sell->fifo;
                                proceeds = (s->sell->value_at_tx * asset) - s->sell->fee;
                                s->sell->fee = 0;
                                final_basis = b->cost_basis * asset;
                        }
                        else if (s->sell->fifo < b->buy->fifo) {
                                proceeds = (s->sell->value_at_tx * asset) - s->sell->fee;
                                s->sell->fee = 0;
                                final_basis = b->cost_basis * asset;

                                b->buy->fifo = b->buy->fifo - s->sell->fifo;
                                s->sell->fifo = 0;
                        }
                        else if (s->sell->fifo == b->buy->fifo) {
                                proceeds = (s->sell->value_at_tx * asset) - s->sell->fee;
                                s->sell->fee = 0;
                                final_basis = b->cost_basis * asset;

                                s->sell->fifo = 0;
                                b->buy->fifo = 0;
                        }
                        // create entry with fields
                        entry *e = create_entry(asset, s->sell->ticker,
                                &acquired, &sold, proceeds, final_basis);

                        // print entry to file
                        if (print_entry(e, out))
                                return 1;

                        // free memory
                        free(e->ticker);
                        free(e);
                }
                b = b->next;
        }
        return 0;
}

int create_exchange(tx *t, buy_queue *bq, char *ticker, double to_val, FILE *out)
{
        double proceeds;
        sell_tx *s = create_sell_tx(t);
        if (sell(bq, s, out)) {
                return 1;
        }
        proceeds = s->proceeds;

        double asset = proceeds / to_val;
        date *d = create_date(t->t_date->year, t->t_date->day, t->t_date->month);
        tx *to = create_tx(d, ticker, asset, to_val, 0);
        buy_tx *b = create_buy_tx(to);
        enqueue_buy(bq, b);

        free_tx(s->sell);
        free(s);
}

// read csv data and put into stuct
int read_csv(FILE *in, FILE *out, buy_queue *bq)
{
        int year, day, month, cols;
        double asset, val, fee;
        char ticker[4], action[9];

        while (!feof(in)) {
                cols = fscanf(in, "%8[^,],", action);

                if (cols != 1) {
                        fprintf(stderr, "Incorrect tx format: first column must be action\n");
                        return 1;
                }

                // if the transaction is an exchange/swap
                if (strcmp("exchange", action) == 0) {
                        char to_ticker[4];
                        double to_val;
                        cols = fscanf(in, "%d-%2d-%2d,%4[^,],%lf,%lf,%4[^,],%lf,%lf\n",
                                &year, &day, &month, ticker, &asset,
                                &val, to_ticker, &to_val, &fee);
                        if (cols != 9) {
                                fprintf(stderr, "Incorrect exchange tx format.\n");
                                return 1;
                        }
                        date *from_d = create_date(year, day, month);
                        tx *from_t = create_tx(from_d, ticker, asset, val, fee);

                        if (create_exchange(from_t, bq, to_ticker, to_val, out))
                                return 1;
                        continue;
                }

                cols = fscanf(in, "%d-%2d-%2d,%4[^,],%lf,%lf,%lf\n",
                                &year, &day, &month,
                                ticker, &asset, &val, &fee);

                if (cols != 7) {
                        fprintf(stderr, "Incorrect format.\n");
                        return 1;
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
                        if (sell(bq, s, out))
                                return 1;
                        free_tx(s->sell);
                        free(s);
                        continue;
                }
                if (strcmp("transfer", action) == 0) {
                        transfer(bq, t, out);
                        free_tx(t);
                        continue;
                }
        }
        return 0;
}

int main(int argc, char *argv[])
{
        buy_queue *buys = init_buy_queue();

        char *in, *out;
        FILE *i, *o;
        if (argc > 2) {
                in = argv[1];
                if (!(i = fopen(in, "r"))) {
                        fprintf(stderr, "Can't open out \"%s\" file.", in);
                        return 1;
                }
                out = argv[2];
                if (!(o = fopen(out, "w"))) {
                        fprintf(stderr, "Can't open out \"%s\" file.", out);
                        return 1;
                }
                // read in buy/sells tx's and exchange tx's from csv
                if (read_csv(i, o, buys))
                        return 1;
        }

        //print_buys(buys);

        release(buys); 

        fclose(i);
        fclose(o);

        return 0;
}
