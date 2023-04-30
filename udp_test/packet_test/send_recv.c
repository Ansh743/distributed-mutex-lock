#include "helper.h"

extern int init(ds_lock *);
extern int lock(ds_lock *);
extern int destroy(ds_lock *);

int main()
{
    ds_lock *ds_lck = malloc(sizeof(ds_lock));
    char op[4];

    init(ds_lck);

    while (1)
    {
        printf("Do you want to send HELLO? (y/n): ");
        fgets(op, sizeof(op), stdin);
        if (strcmp("y\n", op) == 0)
        {
            lock(ds_lck);
            printf("Alright!\n");

        }
        else
        {
            destroy(ds_lck);
            break;
        }
    }

    
    return 0;
}
