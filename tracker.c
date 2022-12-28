#include <stdio.h>

//typedef enum {BUY, SELL} action;

typedef struct {
        int year;
        unsigned int day:5;
        unsigned int month:4;
} date;

typedef struct {
        date t_date;
        char *ticker;
        double asset;
        double value_at_tx;
        double fee;
        unsigned double fifo = asset;
} tx;

typedef struct {
        tx *buy;
        unsigned double cost_basis;
} buy_tx;

typedef struct {
        tx *sell;
        unsigned proceeds;
} sell_tx;

typedef struct {
        double asset;
        char *ticker;
        date date_aquired;
        date date_sold;
        unsigned double proceeds;
        unsigned double cost_basis;
        double net_capital;


/*void print_tx(tx t)
{
        printf("Trasaction date: %d-%d-%d\n", t.t_date.year, t.t_date.day, t.t_date.month);
}*/

// read csv data and put into stuct
void read_csv(char *files)
{

}

int main()
{
        /*char ticker[] = "BTC";
        date t_date = {2022, 21, 12};
        tx t = {t_date, ticker, 0.5, 15000, 0};
        print_tx(t);*/

        return 0;
}
