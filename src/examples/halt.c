/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

#include <syscall.h>

int
main (void)
{
  char c[10];
  read(0,c, sizeof c);
  write(1,c, 6000);
  halt ();
  /* not reached */
}
