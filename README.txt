The bank program has a simple structure with basic functionalities: create accounts, log in, transfer funds, deposit funds, and withdraw funds. To store the account information an accounts.dat file is used which is a binary file. The file is read/ write by following the struct account structure. The signature of struct account is:
    struct account{
        u_long id;
        char name[20];
        char password[20];
        float balance;
    }
