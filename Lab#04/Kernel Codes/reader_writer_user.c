#include "types.h"
#include "stat.h"
#include "user.h"
#define SEM_ID 4
#define VALUE 3

int
main()
{

    semaphore_initialize(SEM_ID, 1, 0);
    if(fork() == 0){
        //printf(0, "WRITER\n");
        semaphore_acquire(SEM_ID);
        // printf(0, "writer acquired semaphore\n");
        write_buffer(VALUE);
        semaphore_release(SEM_ID);
        // printf(0, "writer released semaphore\n");
        //printf(0, "writer exiting\n");
        //exit();
    }
    else if(fork() == 0){
        //printf(0, "READER\n");
        semaphore_acquire(SEM_ID);
        // printf(0, "reader acquired semaphore\n");
        read_buffer();
        //read_buffer();
        semaphore_release(SEM_ID);
        // printf(0, "writer released semaphore\n");
        //printf(0, "reader exiting\n");
        //exit();
    }
    else{
        // printf(0, "parent waiting\n");
        for(int i = 0; i < 2; i++)
            wait();
        //printf(0, "all childs exited\n");
    }
    //printf(0,"process exiting\n");
    exit();
}