#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
        int year;
        unsigned int day:5;
        unsigned int month:4;
} date;

typedef struct {
        date *t_date;
        char *ticker;
        double asset;
        double value_at_tx;
        double fee;
        double fifo;
} tx;

typedef struct buy_tx {
        tx *buy;
        double cost_basis;
        struct buy_tx *next;
} buy_tx;

typedef struct sell_tx {
        tx *sell;
        double proceeds;
        struct sell_tx *next;
} sell_tx;

typedef struct {
        double asset;
        char *ticker;
        date date_aquired;
        date date_sold;
        double proceeds;
        double cost_basis;
        double net_capital;
} entry;

typedef struct {
        struct buy_tx *head;
        struct buy_tx *tail;
} buy_queue;


typedef struct {
        struct sell_tx *head;
        struct sell_tx *tail;
} sell_queue;

date* create_date(int year, int day, int month);
buy_tx* create_buy_tx(tx *b);
void enqueue_buy(buy_queue *b, buy_tx *tx) ;
void dequeue_buy(buy_queue *b);
void enqueue_sell(sell_queue *s, sell_tx *tx) ;
void dequeue_sell(sell_queue *s);
sell_tx* create_sell_tx(tx *s);
tx* create_tx(date *d, char *t, double a, double val, double fee, double fifo);

void init_buy_queue(buy_queue *b)
{
        b->head = NULL;
        b->tail = NULL;
}

void init_sell_queue(sell_queue *s)
{
        s->head = NULL;
        s->tail = NULL;
}

date* create_date(int year, int day, int month)
{
        date *i = malloc(sizeof(date));

        i->year = year;
        i->day = day;
        i->month = month;

        return i;
}

// add a buy tansaction to the buy queue
void enqueue_buy(buy_queue *b, buy_tx *tx) 
{
        // if the queue is empty
        if (b->tail == NULL) {
                b->head = tx;
                b->tail = tx;
                return;
        }

        b->tail->next = tx; // set old tail next to new tx
        tx->next = b->tail; // set new tx next to old tail

        b->tail = tx; // update tail to new tx
}

// remove the first buy transation from the buy queue
void dequeue_buy(buy_queue *b)
{
        // if the queue is empty
        if (b->tail == NULL) {
                fprintf(stderr, "Queue is empty");
                return;
        }

        buy_tx *tmp = b->head;
        b->head->next = b->head;
        free(tmp);
}

// add a sell tansaction to the sell queue
void enqueue_sell(sell_queue *s, sell_tx *tx) 
{
        // if the queue is empty
        if (s->tail == NULL) {
                s->head = tx;
                s->tail = tx;
                return;
        }

        s->tail->next = tx; // set old tail next to new tx
        tx->next = s->tail; // set new tx next to old tail

        s->tail = tx; // update tail to new tx
}

// remove the first sell transation from the sell queue
void dequeue_sell(sell_queue *s)
{
        // if the queue is empty
        if (s->tail == NULL) {
                fprintf(stderr, "Sell queue is empty");
                return;
        }

        sell_tx *tmp = s->head;
        s->head->next = s->head;
        free(tmp);
}

// create a buy transaction
buy_tx* create_buy_tx(tx *b)
{
        buy_tx *i = malloc(sizeof(buy_tx));

        i->buy = b; // should this be `= &b`?
        i->cost_basis;
        i->next = NULL;

        return i;
}

// create a sell transaction
sell_tx* create_sell_tx(tx *s)
{
        sell_tx *i = malloc(sizeof(sell_tx));

        i->sell = s; // should this be = &s?
        i->proceeds;
        i->next = NULL;

        return i;
}

// create a transaction
tx* create_tx(date *d, char *t, double a, double val, double fee, double fifo)
{
        tx *i = malloc(sizeof(tx));

        i->t_date = d; // should this be = &d?
        i->ticker = strdup(t);
        i->asset = a;
        i->value_at_tx = val;
        i->fifo = fifo;

        return i;
}

// read csv data and put into stuct
void read_csv(char *file, buy_queue *bq, sell_queue *sq)
{
        char *tok;
        FILE *in;
        if (!(in = fopen(file, "r"))) {
                fprintf(stderr, "Can't open file.\n");
                return;
        }
        char line[80];

        int count = 0;
        int year;
        int day;
        int month;
        char ticker[4];
        double asset;
        char action[4];
        double val;
        double fee;
        int cols;

        do {
               //fgets(line, 80, in);                                    
               cols = fscanf(in, "%d-%d-%d,%4[^,],%lf,%4[^,],%lf,%lf",
                               &year, &day, &month,
                               ticker, &asset, action,
                               &val, &fee);
               if (cols == 6) {
                       ++count;
               }
               if (cols != 6 && !feof(in)) {
                       fprintf(stderr, "File format incorrect: %d\n", month);
                       return;
               }
               printf("Line %d: %d\n", count, month);
        } while (!feof(in));
        /*date *d = create_date(2022, 21, 12);
        tx *t = create_tx(d, "BTC", 0.5, 10000, 250, 0.5);
        buy_tx *b = create_buy_tx(t);
        enqueue_buy(&buys, b); */

        /*
        free(d);
        free(t);
        free(b);
        */

        fclose(in);
        return;
}

int main(int argc, char *argv[])
{
        buy_queue buys;
        init_buy_queue(&buys);

        sell_queue sells;
        init_sell_queue(&sells);

        if (argc > 0) {
                char *file = argv[1];
                read_csv(file, &buys, &sells);
        }

        return 0;
}
