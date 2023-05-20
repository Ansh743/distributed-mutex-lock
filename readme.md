# Distributed Mutex Lock
### _A mutual exclusion C API for shared NFS mounted systems_

> NOTE: `dsm_lock_api` assumes that the API is being called by processes running on different Linux machines. It relies on the $HOME/process.hosts file to obtain machine IPs that are participating in the Lock/Unlock operations. The presence of a shared file system across these machines is recommended to ensure consistent access to the $HOME/process.hosts file.

## Features
- Plug and play API 🕹️✨
- Logs all background activity 🗒️
- Uses Lamport Scalar Clock ⏱️ for synchronization 
- Spawns a network thread 🧵 to receive incoming messages 👂🏼
- Performs background operations in a non-blocking manner 🟢
- Uses semaphores for blocking ⛔ when requesting for lock


- `dsm_init()` initializes 📦 the ds_lock structure
- `dsm_lock()` sends a REQUEST to all other hosts to acquire lock 🔒
- `dsm_lock()` blocks 🛑 the callee thread until every host sends a REPLY
- `dsm_unlock()` well 🤷🏻‍♂️ unlocks the acquired lock 🔓
- `dsm_destroy()` disposes the ds_lock structure 🗑️
## Usage
```c
#include "dsm_lock_api.h" // Include the API header file


int main(int argc, char *argv[]){
    ds_lock *ds_lck = malloc(sizeof(ds_lock)); //Initialize the ds_lock struct pointer

    dsm_lock(ds_lck); // Request for lock

    do_something(); // Perform operation on a shared resource

    dsm_unlock(ds_lck); // Remove the lock

    dsm_destroy(ds_lck); // Dispose the ds_lock struct pointer

    return 0;
}
```