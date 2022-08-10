#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#include "mmu.h"
#include "mmap.h"

int create_and_write(char* fileName)
{
  int fd = open(fileName, O_RDWR | O_CREATE);

  if(fd <= 0)
  {
    printf(1, "File open failed.\n");
    return 0;
  }

  if (write(fd, "This is a test for OS-LAB-5!", 29) != 29)
  {
    printf(1, "Write in file failed.\n");
    return 0;
  }

  if(close(fd) != 0)
  {
    printf(1, "File close failed.\n");
    return 0;
  }

  return 1;
}

int read_file(char* fileName)
{
  int sz, fd;
  char buff[128];
  fd = open(fileName, O_RDWR);

  if(fd <= 0)
  {
    printf(1, "File open failed.\n");
    return 0;
  }

  sz = read(fd, buff, 50);
  buff[sz] = '\0';
  
  printf(1, "File content is: %s\n", (char*)buff);

  if(close(fd) != 0)
  {
    printf(1, "File close failed.\n");
    return 0;
  }

  return 1;
}

int
main(int argc, char *argv[])
{
  char buff[128];
  char fileName[20] = "sample1.txt";
  
  if(!create_and_write(fileName))
  {
    printf(1, "Write in file failed.\n");
    exit();
  }

  if(!read_file(fileName))
    exit();

  int fd = open(fileName, O_RDWR);
  if(fd <= 0)
  {
    printf(1, "File open failed.\n");
    exit();
  }

  char *addr = (char*) mmap(0, 50, PROT_WRITE, MAP_SHARED, fd, 0);

  if (addr <= 0)
  {
    printf(1, "Mmap failed.\n");
    exit();
  }
  printf(1, "Mmap suceeded.\n");

  strcpy(buff, addr);
  printf(1, "Before write, mmaped's content is: %s\n", buff);

  strcpy((char*)addr, "This content has been rewritten!");
  msync(addr, 50);

  strcpy(buff, addr);
  printf(1, "After write, mmaped's content is: %s\n", buff);

  if(!read_file(fileName))
    exit();

  if (munmap(addr, 50) < 0) 
  {
    printf(1, "Munmap failed.\n");
    exit();
  }
  printf(1, "Munmap suceeded.\n");

  if(close(fd) != 0)
  {
    printf(1, "File close failed.\n");
    exit();
  }
  printf(1, "File close suceeded.\n");

  exit();
}