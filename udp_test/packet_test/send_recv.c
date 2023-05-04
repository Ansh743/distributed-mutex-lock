#include "helper.h"

extern int init(ds_lock *);
extern int lock(ds_lock *);
extern int destroy(ds_lock *);

int main()
{
    ds_lock *ds_lck = malloc(sizeof(ds_lock));
    char op[4];
    int i;

    init(ds_lck);

    while (1)
    {
        printf("Do you want to send HELLO? (y/n): ");
        fgets(op, sizeof(op), stdin);
        if (strcmp("y\n", op) == 0)
        {
            lock(ds_lck);
            printf("Locked\n");
            scanf("%d", &i);
            printf("Unlocked\n");
            unlock(ds_lck);
        }
        else if(strcmp("n\n", op) == 0)
            break;
        else
        {
            printf("Okay\n");
        }
    }
    
    destroy(ds_lck);

    return 0;
}
