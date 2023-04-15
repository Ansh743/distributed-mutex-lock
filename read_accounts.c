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

    if (infile == NULL)
    {
        printf("Error: cannot open file %s\n", ACCOUNTS_FILE);
        exit(1);
    }

    account accounts[100];  // create an array to store up to 100 accounts
    int count = 0;

    while (fread(&acc, sizeof(account), 1, infile))
    {
        accounts[count] = acc;  // store the account in the array
        count++;
    }

    fclose(infile);

    for (int i = 0; i < count; i++)
    {
        acc_details(&accounts[i]);  
    }

    return 0;
}
