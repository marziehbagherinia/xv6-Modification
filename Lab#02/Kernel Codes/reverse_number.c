#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc,char* argv[]) 
{
    int inputNumber;
    int tmp;
    asm ("movl %%edi, %0;"
                    : "=r" (tmp)
                    :
                    : "%edi");
    inputNumber = atoi(argv[1]);
    asm ("movl %0, %%edi;"
                    : 
                    : "r" (inputNumber)
                    : "%edi");
    printf(1,"The result is %d\n",reverse_number());
    asm ("movl %0, %%edi;"
                    : 
                    : "r" (tmp)
                    : "%edi");
    exit();
}