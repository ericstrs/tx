#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void replace_newline(char *m);

void check_exit(char *m) {
        if (strncasecmp(m, "q\n", 1) == 0 || strncasecmp(m, "quit\n", 4) == 0){
                printf("Quiting\n");
                exit(0);
        }
}

void prompt_str(char *f, size_t len, char *m)
{
        printf("%s", m);
        fgets(f, len, stdin);
        check_exit(f);
        replace_newline(f);
        printf("STR: %s\n", f);
}

void replace_newline(char *m) {
        int i;
        for (i = 0; *(m+i) != '\0'; i++) {
        }
        // replace newline with `,`
        m[i - 1] = ',';
}

int check_date(char *date, int *d) {
        // test for sufficient length
        if (strlen(date) < 8) {
                return 1;
        }
        
        char *end_ptr = NULL;
        int year = strtol(date, &end_ptr, 10);
        if (year < 1900) {
                printf("\t\tERROR: year must be past 1900.\n");
                return 1;
        }

        char *p = ++end_ptr;
        int day = strtol(p, &end_ptr, 10);
        if (0 > day || day > 31) {
                printf("\t\tERROR: day must be in the interval [1,31]\n");
                return 1;
        }

        p = ++end_ptr;
        int month = strtol(p, &end_ptr, 10);
        if (0 > month || month > 12) {
                printf("\t\tERROR: month must be in the interval [1,12]\n");
                return 1;
        }
                                        
        d[0] = year;
        d[1] = day;
        d[2] = month;

        return 0;
}

int check_ticker(char *t)
{
        int i = strlen(t) - 1;
        printf("TICKER: %s\n", t);
        printf("strlen(t) - 1: %d\n", i);
        if (0 < i && i > 4) {
                printf("SHOULD BE TICK EERR\n");
                return 1;
        }
                printf("NO TICK EERR\n");
        return 0;
}

void lower_to_upper(char *m)
{
        for (int i = 0; m[i] != '\0'; i++) {
                if (m[i] >= 'a' && m[i] <= 'z') {
                        m[i] = m[i] - 32;
                }
        }
}

int check_double(char *m)
{
        char *end_ptr = NULL;
        float asset = strtof(m, &end_ptr);
        if (asset < 0) {
                printf("\t\tERROR: asset must be a positive number.\n");
                return 1;
        }
        return 0;
}

void prompt_date(int *d)
{
        char date[12];
        for (;;) {
                prompt_str(date, 12, "Enter date (press enter for todays date): ");

                /* default is todays date */
                if (strncmp(date, ",", 1) == 0) {
                        time_t t;
                        time(&t);
                        struct tm local = *localtime(&t);
                        d[0] = local.tm_year + 1900;
                        d[1] = local.tm_mday;
                        d[2] = local.tm_mon + 1;
                        break;
                }
                if (check_date(date, d)) {
                        printf("\t\tERROR: date must be of the form YYYY-MM-DD.\n");
                        continue;
                }
                break;
        }
}

void prompt_ticker(char *ticker)
{
        for (;;) {
                prompt_str(ticker, 8, "Enter ticker: ");
                if (check_ticker(ticker)) {
                        printf("\t\tERROR: ticker must have 4 characters.\n");
                        continue;
                }
                printf("why no check on: %s\n", ticker);
                lower_to_upper(ticker);
                break;
        }
}

void prompt_asset(char *asset)
{
        for (;;) {
                prompt_str(asset, 16, "Enter asset amount: ");
                if (check_double(asset)) {
                        continue;
                }
                break;
        }
}

void prompt_value(char *value)
{
        for (;;) {
                prompt_str(value, 16, "Enter asset value at transaction: ");
                if (check_double(value)) {
                        continue;
                }
                break;
        }
}

void prompt_fee(char *fee)
{
        /* NOTE: ensure no trailing `,`. This will cover the `\n`. */
        for (;;) {
                printf("Enter transaction fee: ");
                fgets(fee, 16, stdin);
                if (check_double(fee)) {
                        continue;
                }
                break;
        }
}

// TODO: need to "flush" data stream.
// OR, read in the entire line of input, that is the characters up
// until the null terminator.
int tx_input(char *entry, FILE *out) {
        int d[] = {0, 0, 0};
        prompt_date(d);

        char ticker[8], asset[16], value[16], fee[16];

        prompt_ticker(ticker);
        prompt_asset(asset);
        prompt_value(value);
        prompt_fee(fee);

        fprintf(out, "%s,%d-%02d-%02d,%s%s%s%s", entry, d[0], d[1], d[2], ticker, asset, value, fee);
        return 0;
}

// action, date, ticker1, asset1, val_tx1, ticker2, val_tx2, fee
int exchange_input(char *entry, FILE *out) {
        int d[] = {0, 0, 0};
        prompt_date(d);

        char from_ticker[8], from_value[16];
        char to_ticker[8], to_value[16];
        char asset[16], fee[16];

        prompt_ticker(from_ticker);
        prompt_asset(asset);
        prompt_value(from_value);

        prompt_ticker(to_ticker);
        prompt_value(to_value);
        prompt_fee(fee);

        fprintf(out, "%s,%d-%02d-%02d,%s%s%s%s%s%s", entry, d[0], d[1],
                        d[2], from_ticker, asset, from_value,
                        to_ticker, to_value, fee);
        return 0;
}

int find_action(char *a)
{
        const char *actions[] = {"buy\n", "sell\n", "transfer\n",
                "exchange\n", "q\n", "exit\n"};

        int len = sizeof(actions)/sizeof(actions[0]);
        for (int i = 0; i < len; i++) {
                if (strncasecmp(a, actions[i], 8) == 0) {
                        return i;
                }
        }
        // no match
        return -1;
}

void user_input(char *out)
{
        FILE *o = fopen(out, "a");
        if (!o) {
                fprintf(stderr, "Can't open file.\n");
                return;
        }
        const char *actions[] = {"buy", "sell", "transfer", "exchange"};
        char action[10];
        char e[64];
        char *entry = e;
        int *c, a;

        printf("Type q or exit to quit.\n");
        for (;;) {
                printf("Enter an action (): ");
                fgets(action, 10, stdin);

                a = find_action(action);

                switch (a) {
                case 0:
                        entry = "buy";
                        tx_input(entry, o);
                        break;
                case 1:
                        entry = "sell";
                        tx_input(entry, o);
                        break;
                case 2:
                        entry = "transfer";
                        tx_input(entry, o);
                        break;
                case 3:
                        entry = "exchange";
                        exchange_input(entry, o);
                        break;
                case 4:
                case 5:
                        printf("Quiting\n");
                        fclose(o);
                        exit(0);
                default:
                        printf("\t\tERROR: action must be \"buy\", \"sell\", \"transfer\", or \"exchange\".\n");
                        continue;
                };

                printf("Appended to output file: %s\n", action);
        }
        fclose(o);
}

