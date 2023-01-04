typedef struct {
        int year;
        int day;
        int month;
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
        double *b_fifo;
        double cost_basis;
        struct buy_tx *next;
} buy_tx;

typedef struct sell_tx {
        tx *sell;
        double proceeds;
        struct sell_tx *next;
} sell_tx;

typedef struct {
        struct buy_tx *head;
        struct buy_tx *tail;
} buy_queue;

typedef struct {
        struct sell_tx *head;
        struct sell_tx *tail;
} sell_queue;

buy_queue* init_buy_queue();
sell_queue* init_sell_queue();
void enqueue_buy(buy_queue *b, buy_tx *tx);
void dequeue_buy(buy_queue *b);
void enqueue_sell(sell_queue *s, sell_tx *tx);
void dequeue_sell(sell_queue *s);
void release(buy_queue *bq);
void print_sells(sell_queue *q);
void print_buys(buy_queue *q);
