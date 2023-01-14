#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "tx_queue.h"
#include "input.h"

void free_tx(tx *t);
void error(char *m);

typedef struct {
        double asset;
        char *ticker;
        date *date_acquired;
        date *date_sold;
        double proceeds;
        double cost_basis;
        double net_gain;
        char *term;
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
        float a = b->asset;
        i->cost_basis = (b->value_at_tx * a + b->fee) / a;
        //printf("COST: %lf\n", i->cost_basis);
        //printf("COMPONENTS: val = %lf\tasset = %lf\tfee = %lf\n",b->value_at_tx, a, b->fee);

        i->reward = 0;
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
                error("Entry is empty.");
        }

       int check = fprintf(out,"%.1lf,%s,%02d/%02d/%d,%02d/%02d/%d,%.1lf,%.1lf,%.1f,%s\n",
                       e->asset, e->ticker, e->date_acquired->month, 
                       e->date_acquired->day, e->date_acquired->year, 
                       e->date_sold->month, e->date_sold->day, e->date_sold->year,
                       e->proceeds, e->cost_basis, (double)e->net_gain, e->term);
       if (check == 0) {
                error("Couldn't print to file");
       }
       return 0;
}

/* returns 1 if dates are a year apart and 0 otherwise. */
int get_term(date *b, date *s)
{
        struct tm bought = { 0 };
        bought.tm_isdst = -1;
        bought.tm_year = b->year;
        bought.tm_mday = b->day;
        bought.tm_mon = b->month;

        struct tm sold = { 0 };
        sold.tm_isdst = -1;
        sold.tm_year = s->year;
        sold.tm_mday = s->day;
        sold.tm_mon = s->month;

        /* TODO: this modifies the `tm` structs. Whenever a field 
         * is close their max (31 for days and 12 for months) 
         * they ciricle around to their min. For instance, 
         * the dates 2023/01/31 and 2024/02/01 does not signal a year.
         */
        int seconds = difftime(mktime(&sold), mktime(&bought));
        if (seconds >= 31536000) {
                return 1;
        }
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
        i->term = "short";
        if (get_term(i->date_acquired, i->date_sold)) {
                i->term = "long";
        }

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
                error("Buy queue is empty.");
        }
        if (t->fee == 0) {
                /* just create an entry. This is only useful if you
                 * attach location data.
                 */
                fprintf(stderr, "ERROR: Attemping to enter a transfer with a fee of 0.\n");
                return 0;
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

                        entry *e = create_entry(t->asset, t->ticker, b->buy->t_date, t->t_date, 0, 0);
                        print_entry(e, out);
                        free(e->ticker);
                        free(e);
                         */
                }
                b = b->next; 
        }
        return 0;
}

int sell(buy_queue *bq, sell_tx *s, FILE *out)
{
        if (bq->head == NULL) {
                error("Buy queue is empty.");
        }

        buy_tx *b = bq->head;
        while (s->sell->fifo != 0) {
                if (b == NULL) {
                        fprintf(stderr, "ERROR: there is not enough %s to sell %f\n", s->sell->ticker, s->sell->fifo);
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
                        print_entry(e, out);

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
        // create sell
        double proceeds;
        sell_tx *s = create_sell_tx(t);
        if (sell(bq, s, out)) {
                return 1;
        }
        proceeds = s->proceeds;

        // create buy
        double asset = proceeds / to_val;
        date *d = create_date(t->t_date->year, t->t_date->day, t->t_date->month);
        tx *to = create_tx(d, ticker, asset, to_val, 0);
        buy_tx *b = create_buy_tx(to);
        enqueue_buy(bq, b);

        free_tx(s->sell);
        free(s);
}

void portfolio(buy_queue *bq)
{
        buy_tx *b = bq->head;
        buy_tx *tmp;
        char *ticker;
        float total;
        while (b != NULL) {
                if (b->buy->fifo == 0) {
                        b = b->next;
                        continue;
                }
                total = 0;
                tmp = b;
                ticker = b->buy->ticker;
                // traverse the rest of the queue
                while (tmp != NULL) {
                        int i = strncmp(tmp->buy->ticker, ticker, 4);
                        if (tmp->buy->fifo && i == 0) {
                                // add matching assets to total
                                total += tmp->buy->fifo;
                                tmp->buy->fifo = 0;
                        }
                        tmp = tmp->next;
                }
                printf("%s, %lf\n", ticker, total);
                b = b->next;
        }
}

void create_reward(buy_queue *bq, tx *t)
{
        // create a buy tx
        buy_tx *b = create_buy_tx(t);
        // set the "reward" field to true;
        b->reward = 1;
        // enqueue the buy
        enqueue_buy(bq, b);
}

void print_rewards(buy_queue *bq)
{
        buy_tx *b = bq->head;
        buy_tx *tmp;
        char *ticker;
        float total = 0;
        // reward, date, TICKER, asset, val_at_tx, reward_value
        while (b != NULL) {
                if (b->reward == 1 && b->buy->fifo > 0) {
                        double reward_value = b->buy->fifo * b->buy->value_at_tx;
                        printf("Rewards for the year:\n");
                        printf("%d-%d-%d,%s,%.1lf,%.1lf,%.1lf\n",
                                        b->buy->t_date->year, b->buy->t_date->day,
                                        b->buy->t_date->month, b->buy->ticker,
                                        b->buy->asset, b->buy->value_at_tx,
                                        reward_value);
                        total += reward_value;
                        b->buy->fifo = 0;
                }
                b = b->next;
        }

        if (total != 0) {
                printf("Total income: %lf\n", total);
        }
}

// read csv data and put into stuct
int read_csv(FILE *in, FILE *out, buy_queue *bq)
{
        int year, day, month, cols;
        double asset, val, fee;
        char ticker[6], action[9];

        int line = 0;
        while (!feof(in)) {
                cols = fscanf(in, "%8[^,],", action);

                if (cols != 1) {
                        fprintf(stderr, "Line %d\t", line);
                        printf("ACTION: \"%s\" COLS: %d\n", action, cols);
                        error("Incorrect tx format: first column must be action");
                }

                // if the transaction is an exchange/swap
                if (strcmp("exchange", action) == 0) {
                        char to_ticker[6];
                        double to_val;
                        cols = fscanf(in, "%d-%2d-%2d,%5[^,],%lf,%lf,%5[^,],%lf,%lf\n",
                                &year, &day, &month, ticker, &asset,
                                &val, to_ticker, &to_val, &fee);
                        //printf("EX TICKER:%s\n", ticker);
                        if (cols != 9) {
                                fprintf(stderr, "Line %d\t", line);
                                error("Incorrect exchange tx format.");
                        }
                        date *from_d = create_date(year, day, month);
                        tx *from_t = create_tx(from_d, ticker, asset, val, fee);

                        if (create_exchange(from_t, bq, to_ticker, to_val, out)) {
                                fprintf(stderr, "Line %d\t", line);
                                error("Incorrect exchange tx format");
                        }
                        ++line;
                        continue;
                }

                cols = fscanf(in, "%d-%2d-%2d,%5[^,],%lf,%lf,%lf\n",
                                &year, &day, &month,
                                ticker, &asset, &val, &fee);

                if (cols != 7) {
                        fprintf(stderr, "Line %d\t", line);
                        error("Incorrect format.");
                }

                date *d = create_date(year, day, month);
                tx *t = create_tx(d, ticker, asset, val, fee);

                if (strcmp("buy", action) == 0) {
                        buy_tx *b = create_buy_tx(t);
                        enqueue_buy(bq, b);
                        ++line;
                        continue;
                }
                if (strcmp("sell", action) == 0) {
                        sell_tx *s = create_sell_tx(t);
                        if (sell(bq, s, out)) {
                                fprintf(stderr, "Line %d\t", line);
                                error("Incorrect sell format");
                        }
                        free_tx(s->sell);
                        free(s);
                        ++line;
                        continue;
                }
                if (strcmp("transfer", action) == 0) {
                        if (transfer(bq, t, out)) {
                                fprintf(stderr, "Line %d\n", line);
                                error("Incorrect transfer format");
                        }
                        free_tx(t);
                        ++line;
                        continue;
                }
                if (strncmp("reward", action,6) == 0) {
                        create_reward(bq, t);
                        ++line;
                        continue;
                }
        }
        print_rewards(bq); 
        printf("\nAssets owned:\n");
        portfolio(bq);
        return 0;
}

void error(char *m)
{
        fprintf(stderr, "%s: %s\n", m, strerror(errno));
        exit(1);
}

void usage() {
        error("usage: ./tracker <input.csv> [out.csv]");
}

int main(int argc, char *argv[])
{
        if (argc < 2) {
                usage();
        }

        char *in, *out;
        FILE *i, *o;

        in = argv[1];
        if (!(i = fopen(in, "r")))
                error("Can't open input file.");

        switch (argc) {
        case 2:
                user_input(in); // prompt user input
                break;
        case 3:
                out = argv[2];
                if (!(o = fopen(out, "w")))
                        error("Can't open output file.");

                buy_queue *buys = init_buy_queue();
                read_csv(i, o, buys); // create entries
                release(buys); 
                fclose(o);
                fclose(i);
                break;
        default:
                usage();
        };

        //print_buys(buys);

        return 0;
}
