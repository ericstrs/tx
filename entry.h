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
        double asset;
        char *ticker;
        date *date_acquired;
        date *date_sold;
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
} sell_queue;

date* create_date(int year, int day, int month);
buy_tx* create_buy_tx(tx *b);
void enqueue_buy(buy_queue *b, buy_tx *tx) ;
void dequeue_buy(buy_queue *b);
void enqueue_sell(sell_queue *s, sell_tx *tx) ;
void dequeue_sell(sell_queue *s);
sell_tx* create_sell_tx(tx *s);
tx* create_tx(date *d, char *t, double a, double val, double fee);
entry* create_entry(double a, char *t, date *aq, date *sold, double p, double c);
void create_entries(buy_queue *bq, sell_queue *sq, char *file);
