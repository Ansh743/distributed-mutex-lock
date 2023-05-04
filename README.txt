Group Member details and contribution:
    Anshul Anil Maske (2830914) - 50%
    Hema Latha Samudrala (2836602) - 50%

Design details
    The dsm_lock_api is a distributed lock service that can be run on machines connected via LAN and have NFS enabled. The API uses the ds_lock struct. The ds_lock struct has the following signature:
    typedef struct ds_lock
    {
        int socket_fd;
        struct sockaddr_in s_in, from;
        smallint should_listen : 1;
        smallint in_cs : 1;
        smallint requesting : 1;
        smallint active_hosts;
        char *file_name;
        char *self_ip_addr;
        char *ip_addrs[NUM_HOSTS];
        int thread_ret;
        sem_t semaphore;
        smallint shared;
        smallint d_array[NUM_HOSTS];
        ip_id ip_ids[NUM_HOSTS];
        pthread_t tid;
        u_short lamport_clock;
    } ds_lock;

    The API has four functions that are interfaced in the dsm_lock_api.h header file. The functions are:
        1. int dsm_init(ds_lock *) - Initializes the ds_lock struct so that mutual lock can be used.
        2. int dsm_lock(ds_lock *) - Sends a REQUEST to all other participating processes.
        3. int dsm_unlock(ds_lock *) - Check for any pending REQUEST 
        4. int dsm_init(ds_lock *) - Clean up the structure and close open descriptors.

    The API uses sem_init(), sem_wait() and sem_post() to make the dsm_lock() call suspend the calling thread until all replies are received. 
    


Compiling instruction and execution sequence (with commands)
    To test the lock service API a test program is provided (banker.c): 
        1. The user must first remove a file $HOME/processes.hosts (if exists). 
        2. run make 
        3. Run ./banker (test application provided to test the API) preferably on the spirit machine
        4. Run ./banker on other machines as well.

A sample test run
    spirit:
        anmaske@spirit:~/CIS620_ADV_OS/group18_p4$ ./banker
        Banking System
                1. Create a new account
                2. Log In to an account
                0. Exit
        Enter your choice: 1
        Creating new account
                Enter name (no space): ansh
                Enter password (no space): pass
        Account Created
        ---------------CLRSCR----------------
        Banking System
                1. Create a new account
                2. Log In to an account
                0. Exit
        Enter your choice: 2
        Creating new account
                Enter name (no space): ansh
                Enter password (no space): pass
        logged in to ansh account
        ---------------CLRSCR----------------
        Hello, ansh
        Current balance: 0.000000
                1. Transfer money
                2. Withdraw
                3. Deposit
                0. Logout
        Enter your choice: 3
        Enter amount to deposit: 100
        ---------------CLRSCR----------------
        Hello, ansh
        Current balance: 100.000000
                1. Transfer money
                2. Withdraw
                3. Deposit
                0. Logout
        Enter your choice: 3
        Enter amount to deposit: 100
        ---------------CLRSCR----------------
        Hello, ansh
        Current balance: 100.000000
                1. Transfer money
                2. Withdraw
                3. Deposit
                0. Logout
        Enter your choice: 1
        Enter amount to transfer: 20
        Enter username to transfer: hema
        ---------------CLRSCR----------------
        Hello, ansh
        Current balance: 80.000000
                1. Transfer money
                2. Withdraw
                3. Deposit
                0. Logout
        Enter your choice: 

    Follow the same on other machines and create new accounts(max 10 bank accounts supported). All the process is synchronized by using our dsm_lock_api. All the send and receive are logged into a <current_machine_ip>.log file.
    A sample log file:
        23:32:45: Thread started
        23:32:45: Listening for incoming messages...
        23:33:24: Sending REQUEST to
        23:33:24: Sending REQUEST to
        23:33:24: Sending REQUEST to
        23:33:24: Received REPLY 1/3
        23:33:24: Listening for incoming messages...
        23:33:24: Received REPLY 2/3
        ...
        23:38:08: Sending REQUEST to
        23:38:08: Sending REQUEST to
        23:38:08: Sending REQUEST to
        23:38:08: Received REPLY 1/3
        23:38:08: Listening for incoming messages...
        23:38:08: Received REPLY 2/3
        23:38:08: Listening for incoming messages...
        23:38:08: Received REPLY 3/3
        23:38:08: Listening for incoming messages...
 

Please explicitly state which part, if there is any, does not work and the possible reasons why that module does not work.
    Suppose there are 3 machines active and are using the dsm_lock_api. The current state is as follows with the Lamport clocks in the bracket.
        machine1(20) - Currently in CS
        machine2(16) - Sends REQUEST to machine1 and machine3
        machine3(12) - Sends REQUEST to machine2 and machine3
    When machine1 exits CS it sends REPLY to machine2 and machine3. Ideally, machine3 should get the lock since its timestamp is lower than machine2 however our code lets machine2 acquire the lock and once machine2 exits CS it sends REPLY to machine3. This bug is known to us but not the reason. 