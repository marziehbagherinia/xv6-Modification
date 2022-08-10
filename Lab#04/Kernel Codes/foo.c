#include "types.h"
#include "stat.h"
#include "user.h"

int
main()
{
    int p1 = fork();
    
    if (p1 == 0) {
        int a = 1;
        while (1) {
            a++;
            if (a == 1000000000) break;
        }
        sleep(3000);
        printf(1, "Done! %d \n", get_parent_id());

    } else 
    {
        wait();
        //set_tickets(p1, 45);
        set_level(p1, 3);
        int a = 1;
        while (1) {
            a++;
            if (a == 5000) {
                break;
                //set_level(p1, 1);
            }
        }

        sleep(100);
        printf(1, "Done! %d \n", get_parent_id());
        
    }
    
    exit();
}
