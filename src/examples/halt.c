/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

#include <syscall.h>

int
main (void)
{
  create("xys",128);
  create("HELLOWORLD",256);
  int fd1 = open("xys");
  printf("Result: %i\n", remove("xys"));
  int fd2 = open("HELLOWORLD");
  printf("FD:S %i , %i \n", fd1, fd2);
  close(fd1);
  fd1 = open("xys");
  printf("FD:S %i , %i \n", fd1, fd2);
  close(fd2);
  
  close(fd1);
halt();
  /* not reached */
}
