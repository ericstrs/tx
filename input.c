#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void replace_newline(char *m);
void clear_input_buff(char *m);

void check_exit(char *m) {
        if (strncasecmp(m, "q\n", 1) == 0 || strncasecmp(m, "quit\n", 4) == 0){
                printf("Quiting\n");
                exit(0);
        }
}

int check_enter(char *m) {
        if (strncmp(m, "\n", 1) == 0) {
                fprintf(stderr, "\tERROR: must enter a value.\n");
                return 1;
        }
        return 0;
}

int prompt_str(char *f, size_t len, char *m)
{
        printf("%s", m);
        fgets(f, len, stdin);

        check_exit(f);

        if (check_enter(f)) {
                return 1;
        }

        clear_input_buff(f);
        replace_newline(f);

        return 0;
}

void clear_input_buff(char *m) {
        int i = strlen(m) - 1;
        // check if the string contains a newline
        if (m[i] != '\n') {
                // consume characters up to '\n'
                while((getchar() != '\n'));
        }
}

/* replace '\n' with ',' for csv output */
void replace_newline(char *m) {
        int i;
        for (i = 0; *(m+i) != '\0'; i++) {
        }
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
                fprintf(stderr, "\tERROR: year must be past 1900.\n");
                return 1;
        }

        char *p = ++end_ptr;
        int day = strtol(p, &end_ptr, 10);
        if (0 > day || day > 31) {
                fprintf(stderr, "\tERROR: day must be in the interval [1,31]\n");
                return 1;
        }

        p = ++end_ptr;
        int month = strtol(p, &end_ptr, 10);
        if (0 > month || month > 12) {
                fprintf(stderr, "\tERROR: month must be in the interval [1,12]\n");
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
        if (0 < i && i > 4) {
                fprintf(stderr, "\tERROR: ticker length must be in the interval [1,4]\n");
                return 1;
        }
        return 0;
}

void make_upper(char *m)
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
                fprintf(stderr, "\tERROR: asset must be a positive number.\n");
                return 1;
        }
        return 0;
}

void prompt_date(int *d)
{
        char date[12];
        for (;;) {
                printf("Enter date (press enter for today): ");
                fgets(date, 12, stdin);
                check_exit(date);
                clear_input_buff(date);

                /* default is todays date */
                if (strncmp(date, "\n", 1) == 0) {
                        time_t t;
                        time(&t);
                        struct tm local = *localtime(&t);
                        d[0] = local.tm_year + 1900;
                        d[1] = local.tm_mday;
                        d[2] = local.tm_mon + 1;
                        break;
                }
                // else
                if (check_date(date, d)) {
                        fprintf(stderr, "\tNOTE: date must be of the form YYYY-MM-DD.\n");
                        continue;
                }
                break;
        }
}

void prompt_ticker(char *ticker)
{
        for (;;) {
                if (prompt_str(ticker, 8, "Enter ticker: ")) {
                        continue;
                }
                if (check_ticker(ticker)) {
                        continue;
                }
                make_upper(ticker);
                break;
        }
}

void prompt_asset(char *asset)
{
        for (;;) {
                if (prompt_str(asset, 16, "Enter asset amount: ")) {
                        continue;
                }
                if (check_double(asset)) {
                        continue;
                }
                break;
        }
}

void prompt_value(char *value)
{
        for (;;) {
                if(prompt_str(value, 16, "Enter asset value at transaction: ")) {
                        continue;
                }
                if (check_double(value)) {
                        continue;
                }
                break;
        }
}

void prompt_fee(char *fee)
{
        /* NOTE: trailing '\n' and leave out ','. */
        for (;;) {
                printf("Enter transaction fee: ");
                fgets(fee, 16, stdin);
                clear_input_buff(fee);
                if (check_double(fee)) {
                        continue;
                }
                break;
        }
}

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
        char action[10], *entry;
        int a, *c;

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
                        fprintf(stderr, "\tERROR: action must be \"buy\", \"sell\", \"transfer\", or \"exchange\".\n");
                        continue;
                };

                printf("Appended to output file: %s\n", action);
        }
        fclose(o);
}

