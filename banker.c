#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "dsm_lock_api.h"

#define u_long unsigned long int
#define ID_LENGTH 10
#define ACCOUNTS_FILE "accounts.dat"
#define N_ACCOUNTS 10

typedef struct account
{
    u_long id;
    char name[20];
    char password[20];
    float balance;
} account;

static unsigned short logged_in = 0, num_accounts = 0;
account *curr_acc;
account all_accs[N_ACCOUNTS];

int refresh_curr_acc(ds_lock * ds_lck)
{
    if (curr_acc == NULL)
        return -1;
    account acc;
    FILE *infile;
    dsm_lock(ds_lck);
    infile = fopen(ACCOUNTS_FILE, "rb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return -1;
    }
    while (fread(&acc, sizeof(acc), 1, infile))
    {
        if (strcmp(acc.name, curr_acc->name) == 0 && strcmp(acc.password, curr_acc->password) == 0)
        {
            curr_acc->balance = acc.balance;
            break;
        }
    }
    fclose(infile);
    dsm_unlock(ds_lck);

    return 0;
}

int acc_details(account *acc)
{

    printf("Name(ID): %s(%ld)\n", acc->name, acc->id);
    printf("Password: %s\n", acc->password);
    printf("Balance: %f\n", acc->balance);
    return 0;
}
void clear_screen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int check_file(char *f)
{
    FILE *infile;

    if ((infile = fopen(f, "r")) == NULL)
    {
        // file doesn't exist, create it
        infile = fopen(f, "w");
    }

    // file exists
    fclose(infile);

    return 0;
}

int main_menu()
{
    int choice;
    clear_screen();
    printf("Banking System\n");
    printf("\t1. Create a new account\n");
    printf("\t2. Log In to an account\n");
    printf("\t0. Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    return choice;
}

int login_loop(ds_lock * ds_lck)
{
    int choice;
    clear_screen();
    refresh_curr_acc(ds_lck);
    printf("Hello, %s\n", curr_acc->name);
    printf("Current balance: %f\n", curr_acc->balance);
    printf("\t1. Transfer money\n");
    printf("\t2. Withdraw\n");
    printf("\t3. Deposit\n");
    printf("\t0. Logout\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    return choice;
}

u_long gen_id()
{
    char id[ID_LENGTH + 1];
    time_t now;
    struct tm *now_tm;
    int random_num;

    now = time(NULL);
    now_tm = localtime(&now);

    srand(now);
    random_num = rand();

    snprintf(id, ID_LENGTH + 1, "%02d%02d%02d%d",
             now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec, random_num);

    u_long res = strtoul(id, NULL, 10);

    return res;
}

int get_all_acc(account *accounts[], ds_lock * ds_lck)
{
    account acc;
    FILE *infile;
    int i = 0;
    dsm_lock(ds_lck);
    infile = fopen(ACCOUNTS_FILE, "rb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return -1;
    }
    while (fread(&acc, sizeof(acc), 1, infile) && i < N_ACCOUNTS)
    {
        accounts[i++] = &acc;
    }
    fclose(infile);
    dsm_unlock(ds_lck);
    num_accounts = i;
    return i;
}

int set_all_acc(account **accounts, ds_lock * ds_lck)
{
    FILE *infile;
    int i = 0;
    dsm_lock(ds_lck);
    infile = fopen(ACCOUNTS_FILE, "wb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return 1;
    }

    for (i = 0; i < num_accounts; i++)
    {
        fwrite(&accounts[i], sizeof(account), 1, infile);
    }

    fclose(infile);
    dsm_unlock(ds_lck);
    return 0;
}

int print_all_accounts(account **accounts)
{
    int i = 0;

    for (i = 0; i < num_accounts; i++)
    {
        acc_details(accounts[i]);
    }

    return 0;
}

int create_account(ds_lock * ds_lck)
{
    account acc;
    FILE *infile;
    printf("Creating new account\n");
    fflush(stdout);
    getchar();

    printf("\tEnter name (no space): ");
    fgets(acc.name, sizeof(acc.name), stdin);
    acc.name[strcspn(acc.name, "\n")] = '\0';

    printf("\tEnter password (no space): ");
    fgets(acc.password, sizeof(acc.password), stdin);
    acc.password[strcspn(acc.password, "\n")] = '\0';

    acc.balance = 0;
    acc.id = gen_id();

    dsm_lock(ds_lck);
    infile = fopen(ACCOUNTS_FILE, "ab+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return 1;
    }
    fwrite(&acc, sizeof(account), 1, infile);

    fclose(infile);
    dsm_unlock(ds_lck);
    printf("Account Created\n");
    sleep(2);
    return 0;
}

int transfer(ds_lock * ds_lck)
{
    account acc;
    FILE *infile;
    int i = 0, j = 0, transfer_from_i = -1, transfer_to_i = -1;

    float amount = 0.0;
    char name[20];
    printf("Enter amount to transfer: ");
    scanf("%f", &amount);
    printf("Enter username to transfer: ");
    scanf("%s", name);
    dsm_lock(ds_lck);
    infile = fopen(ACCOUNTS_FILE, "rb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return -1;
    }
    while (fread(&acc, sizeof(acc), 1, infile) && i < N_ACCOUNTS)
    {
        if (strcmp(acc.name, name) == 0)
        {
            transfer_to_i = i;
        }
        if (strcmp(acc.name, curr_acc->name) == 0 && strcmp(acc.password, curr_acc->password) == 0)
        {
            transfer_from_i = i;
        }
        all_accs[i++] = acc;
    }
    fclose(infile);

    if (transfer_to_i == -1)
    {
        printf("Username not found!\n");
        sleep(3);
        return -1;
    }

    if (all_accs[transfer_from_i].balance - amount < 0)
    {
        printf("Insufficient Funds!\n");
        sleep(3);
        return -1;
    }
    else
    {
        all_accs[transfer_from_i].balance -= amount;
        all_accs[transfer_to_i].balance += amount;
    }

    infile = fopen(ACCOUNTS_FILE, "wb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return 1;
    }

    for (j = 0; j < i; j++)
    {
        fwrite(&all_accs[j], sizeof(account), 1, infile);
    }

    fclose(infile);
    dsm_unlock(ds_lck);
    return 0;
}
int withdraw(ds_lock * ds_lck)
{
    account acc;
    FILE *infile;
    int i = 0, j = 0, done = 0;
    float amount = 0.0;
    printf("Enter amount to withdraw: ");
    scanf("%f", &amount);
    dsm_lock(ds_lck);
    infile = fopen(ACCOUNTS_FILE, "rb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return -1;
    }
    while (fread(&acc, sizeof(acc), 1, infile) && i < N_ACCOUNTS)
    {
        if (strcmp(acc.name, curr_acc->name) == 0 && strcmp(acc.password, curr_acc->password) == 0)
        {
            if (acc.balance - amount >= 0)
            {
                acc.balance -= amount;
                done = 1;
            }
        }
        all_accs[i++] = acc;
    }
    fclose(infile);
    infile = fopen(ACCOUNTS_FILE, "wb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return -1;
    }

    for (j = 0; j < i; j++)
    {
        fwrite(&all_accs[j], sizeof(account), 1, infile);
    }

    fclose(infile);
    dsm_unlock(ds_lck);
    if (done == 1)
        printf("Withdraw successful!\n");
    else
        printf("Not enough balance!\n");
    sleep(2);
    return done;
}
int deposit(ds_lock * ds_lck)
{
    account acc;
    FILE *infile;
    int i = 0, j = 0;
    float amount = 0.0;
    printf("Enter amount to deposit: ");
    scanf("%f", &amount);
    dsm_lock(ds_lck);
    infile = fopen(ACCOUNTS_FILE, "rb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        sleep(3);

        return -1;
    }
    while (fread(&acc, sizeof(acc), 1, infile) && i < N_ACCOUNTS)
    {
        if (strcmp(acc.name, curr_acc->name) == 0 && strcmp(acc.password, curr_acc->password) == 0)
        {
            acc.balance += amount;
        }
        all_accs[i++] = acc;
    }
    fclose(infile);
    infile = fopen(ACCOUNTS_FILE, "wb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return 1;
    }

    for (j = 0; j < i; j++)
    {
        fwrite(&all_accs[j], sizeof(account), 1, infile);
    }

    fclose(infile);
    dsm_unlock(ds_lck);
    return 0;
}
int logout()
{
    curr_acc = (void *)NULL;
    logged_in = 0;
    return 0;
}

int login(ds_lock * ds_lck)
{
    account acc;
    FILE *infile;
    char name[20], password[20];
    printf("Logging into account\n");
    fflush(stdout);
    getchar();

    printf("\tEnter name (no space): ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("\tEnter password (no space): ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';

    dsm_lock(ds_lck);
    infile = fopen(ACCOUNTS_FILE, "rb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return 1;
    }

    while (fread(&acc, sizeof(account), 1, infile))
    {
        if (strcmp(name, acc.name) == 0 && strcmp(password, acc.password) == 0)
        {
            logged_in = 1;
            curr_acc = &acc;
            printf("Logged in to %s account\n", name);
            sleep(3);
            clear_screen();
            break;
        }
    }
    if (logged_in == 0)
    {
        printf("Wrong name or password. Please try again.\n");
        sleep(3);
        clear_screen();
        return -1;
    }

    fclose(infile);
    dsm_unlock(ds_lck);

    int choice;
    while ((choice = login_loop(ds_lck)) != 0)
    {
        if (choice == 1)
            transfer(ds_lck);
        else if (choice == 2)
        {
            withdraw(ds_lck);
        }
        else if (choice == 3)
            deposit(ds_lck);
        else if (choice == 0)
        {
            logout();
            break;
        }
    }

    return 0;
}

void handle_sigint(int sig)
{
    if (curr_acc != NULL)
    {
        free(curr_acc);
    }
    exit(0);
}

// Driver program
int main()
{
    int choice;
    
    ds_lock *ds_lck = malloc(sizeof(ds_lock));
    dsm_init(ds_lck);

    check_file(ACCOUNTS_FILE);

    while ((choice = main_menu()) != 0)
    {
        if (choice == 1)
            create_account(ds_lck);
        else if (choice == 2)
            login(ds_lck);
        else if (choice == 0)
            break;
    }

    dsm_destroy(ds_lck);

    // if (logged_in == 1)
    //     free(curr_acc);
    return 0;
}