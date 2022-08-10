#include "types.h"
#include "stat.h"
#include "user.h"
 
int main() 
{
    int (*fun_ptr)(void) = (int(*)(void)) 0x80105bd0;   
  
    (*fun_ptr)(); 
  
    return 0; 
} 
