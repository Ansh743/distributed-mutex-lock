#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define u_long unsigned long int
#define ID_LENGTH 10
#define ACCOUNTS_FILE "accounts.dat"

typedef struct account
{
    u_long id;
    char name[20];
    char password[20];
    float balance;
} account;

int acc_details(account *acc)
{

    printf("Name(ID): %s(%ld)\n", acc->name, acc->id);
    printf("Password: %s\n", acc->password);
    printf("Balance: %f\n", acc->balance);
    return 0;
}
int main(int argc, char const *argv[])
{
    account acc;
    FILE *infile;
    infile = fopen(ACCOUNTS_FILE, "rb");
    while (fread(&acc, sizeof(acc), 1, infile))
    {
        acc_details(&acc);
    }
    fclose(infile);
    return 0;
}
