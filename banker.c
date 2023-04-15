#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

int logged_in = 0;
account *acc_curr;
account all_accs[N_ACCOUNTS];

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
    FILE *fp;

    if ((fp = fopen(f, "r")) == NULL)
    {
        // file doesn't exist, create it
        fp = fopen(f, "w");
    }
    else
    {
        // file exists
        fclose(fp);
    }

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

int login_loop()
{
    int choice;
    clear_screen();
    printf("Hello, %s\n", acc_curr->name);
    printf("Current balance: %f\n", acc_curr->balance);
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

int get_all_acc(account **accounts)
{
    account acc;
    FILE *infile;
    int i = 0;
    // TODO: Entering CS
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
    // TODO: Exiting CS
    return i;
}

int set_all_acc(account **accounts)
{
    account acc;
    FILE *infile;
    int i = 0;
    // TODO: Entering CS
    infile = fopen(ACCOUNTS_FILE, "rb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return 1;
    }

    while (fread(&acc, sizeof(acc), 1, infile) && i < N_ACCOUNTS)
    {
        accounts[i++] = &acc;
    }
    fclose(infile);
    // TODO: Exiting CS
    return 0;
}

int create_account()
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

    // TODO: Entering CS
    infile = fopen(ACCOUNTS_FILE, "wb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        return 1;
    }
    fwrite(&acc, sizeof(account), 1, infile);

    fclose(infile);
    // TODO: Exiting CS
    printf("Account Created\n");
    sleep(3);
    return 0;
}

int acc_details(account *acc)
{

    printf("Name(ID): %s(%ld)\n", acc->name, acc->id);
    printf("Balance: %f\n", acc->balance);
    return 0;
}

int transfer()
{
    lock();
    return 0;
}
int withdraw()
{
    return 0;
}
int deposit()
{
    return 0;
}
int logout()
{
    acc_curr = (void *)NULL;
    logged_in = 0;
    return 0;
}

int login()
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

    // TODO: Entering CS
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
            acc_curr = &acc;
            printf("Logged in to %s account\n", name);
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
    // TODO: Exiting CS
    sleep(3);
    clear_screen();

    int choice;
    while ((choice = login_loop()) != 0)
    {
        if (choice == 1)
            transfer();
        else if (choice == 2)
            withdraw();
        else if (choice == 3)
            deposit();
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
    if (acc_curr != NULL)
    {
        free(acc_curr);
    }
    exit(0);
}

// Driver program
int main()
{
    FILE *infile;
    int choice;

    check_file(ACCOUNTS_FILE);

    while ((choice = main_menu()) != 0)
    {
        if (choice == 1)
            create_account();
        else if (choice == 2)
            login();
        else if (choice == 0)
            break;
    }

    // if (logged_in == 1)
    //     free(acc_curr);
    return 0;
}