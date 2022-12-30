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
        date date_acquired;
        date date_sold;
        double proceeds;
        double cost_basis;
        double net_gain;
} entry;

typedef struct {
        struct buy_tx *head;
        struct buy_tx *tail;
} buy_queue;


typedef struct {
        struct sell_tx *head;
        struct sell_tx *tail;
        int rows;
} sell_queue;

date* create_date(int year, int day, int month);
buy_tx* create_buy_tx(tx *b);
void enqueue_buy(buy_queue *b, buy_tx *tx) ;
void dequeue_buy(buy_queue *b);
void enqueue_sell(sell_queue *s, sell_tx *tx) ;
void dequeue_sell(sell_queue *s);
sell_tx* create_sell_tx(tx *s);
tx* create_tx(date *d, char *t, double a, double val, double fee);
entry* create_entry(double a, char *t, date aq, date sold, double p, double c);
void create_entries(buy_queue *bq, sell_queue *sq, char *file);

buy_queue* init_buy_queue()
{
        buy_queue *b = malloc(sizeof(buy_queue));
        b = malloc(sizeof(buy_queue));
        b->head = NULL;
        b->tail = NULL;

        return b;
}

sell_queue* init_sell_queue()
{
        sell_queue *s = malloc(sizeof(sell_queue));
        s->head = NULL;
        s->tail = NULL;
        s->rows = 0;

        return s;
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
        if (b->tail != NULL) {
                b->tail->next = tx;
        }
        b->tail = tx;

        // if it is the first item in the queue
        if (b->head == NULL) {
                b->head = tx;
        } 
}

// remove the first buy transation from the buy queue
void dequeue_buy(buy_queue *b)
{
        // if the queue is empty
        if (b->head == NULL) {
                fprintf(stderr, "Queue is empty");
                return;
        }

        buy_tx *tmp = b->head;

        b->head = b->head->next;
        if (b->head = NULL) { // this makes no sense to me. don't think it will run.
                b->tail = NULL;
        }

        free(tmp->buy->ticker);
        free(tmp->buy);
        free(tmp);
}

// add a sell tansaction to the sell queue
void enqueue_sell(sell_queue *s, sell_tx *tx) 
{
        // if the queue is empty
        if (s->tail != NULL) {
                s->tail->next = tx;
        }
        s->tail = tx;

        // if it is the first item in the queue
        if (s->head == NULL) {
                s->head = tx;
        }
        s->rows = s->rows + 1;
}

// remove the first sell transation from the sell queue
void dequeue_sell(sell_queue *s)
{
        // if the queue is empty
        if (s->head == NULL) {
                fprintf(stderr, "Sell queue is empty");
                return;
        }

        sell_tx *tmp = s->head;

        s->head = s->head->next;
        if (s->head = NULL) { // this makes no sense to me. don't think it will run.
                s->tail = NULL;
        }

        free(tmp->sell->ticker);
        free(tmp->sell);
        free(tmp);
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

        return i;
}

void print_entry(entry *e, char *file)
{
        if (e == NULL) {
                fprintf(stderr, "Entry is empty\n");
                return;
        }

        FILE *out;
        if (!(out = fopen(file, "w"))) {
                fprintf(stderr, "Can't open out \"%s\" file.", file);
                return;
        }

       fprintf(out,"%lf,%s,%d/%d/%d,%d/%d/%d,%lf,%lf,%lf\n",
                       e->asset, e->ticker, e->date_acquired.month, 
                       e->date_acquired.day, e->date_acquired.year, 
                       e->date_sold.month, e->date_sold.day, e->date_sold.year,
                       e->proceeds, e->cost_basis, e->net_gain);
        
}

// returns a copy of a buy transction
buy_tx* copy_buy(buy_tx *b)
{
        date *d = create_date(b->buy->t_date->year,
                        b->buy->t_date->day,
                        b->buy->t_date->month);
        tx *to = create_tx(d, b->buy->ticker, b->buy->asset,
                        b->buy->value_at_tx, b->buy->fee);
        buy_tx* i = create_buy_tx(to) ;
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

        buy_tx *i = copy_buy(bq->head);

        //printf("bq->head->buy->fifo: %lf\n", bq->head->buy->fifo);
        //printf("i.buy->fifo: %lf", i.buy->fifo);

        // traverse bq
        while (i != NULL) {
                if (i->buy->fifo  > 0 && strcmp(t, i->buy->ticker) == 0) {
                        enqueue_buy(subset, i);
                }
                i = i->next;
        }
        return bq;
}

entry* create_entry(double a, char *t, date aq, date sold, double p, double c)
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

void create_entries(buy_queue *bq, sell_queue *sq, char *file)
{
        int rows = sq->rows;
        double *buy_fifo;
        double asset; 
        date acquired; // Need to make these a pointer? so assigment works?
        date sold;
        double proceeds;
        char *ticker;
        double final_basis;
        double gain;

        int i = 1;        
        sell_tx *s = sq->head;
        double *sell_fifo = &s->sell->fifo;
        double *head_fifo;
        while (*sell_fifo > 0 && !(i > rows)) {
                ticker = s->sell->ticker;

                // subset buys on ticker
                buy_queue *buys_subset = subset_buys(bq, ticker);
                                                      
                buy_tx *first = buys_subset->head; // get head of queue
                //printf("first: %lf", first->buy->fifo);

                head_fifo = &first->buy->fifo;

                if (*sell_fifo > *head_fifo) {
                        asset = *sell_fifo;
                        *sell_fifo = *sell_fifo - *head_fifo;
                        asset = asset - *sell_fifo;
                        head_fifo = 0;

                        acquired = *first->buy->t_date;
                        sold = *s->sell->t_date;
                        proceeds = s->sell->value_at_tx * asset;
                        final_basis = first->cost_basis * asset;

                        // create entry with fields
                        entry *e = create_entry(asset, ticker, 
                                        acquired, sold, proceeds, final_basis);
                        // fprintf entry to file
                        print_entry(e, file);

                        // free memory
                        free(e->ticker);
                        free(e);
                        //free(entry_queue);
                        free(buys_subset);
                }
                // set s to next sell
                //
        }
}

// read csv data and put into stuct
void read_csv(char *file, buy_queue *bq, sell_queue *sq)
{
        FILE *in;
        if (!(in = fopen(file, "r"))) {
                fprintf(stderr, "Can't open file.\n");
                return;
        }

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
               cols = fscanf(in, "%d-%d-%d,%4[^,],%lf,%4[^,],%lf,%lf\n",
                               &year, &day, &month,
                               ticker, &asset, action,
                               &val, &fee);

               if (cols == 8) {
                       ++count;
               }
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
        } while (!feof(in));

        fclose(in);
        return;
}

// dequeue every tx in given queues
void release(buy_queue *bq, sell_queue *sq)
{
        while (bq->head != NULL) {
                dequeue_buy(bq);
        }

        while (sq->head != NULL) {
                dequeue_sell(sq);
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
               printf("%d-%d-%d,%s,%.1lf,%.1lf,%.1lf\n",
                               tmp->buy->t_date->year, tmp->buy->t_date->day, 
                               tmp->buy->t_date->month, tmp->buy->ticker, 
                               tmp->buy->asset, tmp->buy->value_at_tx,
                               tmp->buy->fee);
               tmp = tmp->next;
        }
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
               printf("%d-%d-%d,%s,%.1lf,%.1lf,%.1lf\n",
                               tmp->sell->t_date->year, tmp->sell->t_date->day, 
                               tmp->sell->t_date->month, tmp->sell->ticker, 
                               tmp->sell->asset, tmp->sell->value_at_tx,
                               tmp->sell->fee);
               tmp = tmp->next;
        }
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

        // create entries
        create_entries(buys, sells, out);

        release(buys, sells); 
        //release(buys, sells, entries); 

        return 0;
}
