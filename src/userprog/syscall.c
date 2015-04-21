#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/flist.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:
   
   int sys_read_arg_count = argc[ SYS_READ ];
   
   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1, 
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;
  //printf("LOG [DEBUG]: in syscall_handler\n");
  int sys_read_arg_count = argc[ esp[0] ];
  //printf("LOG [DEBUG]: argc = %c\n", ('0' +sys_read_arg_count));
  switch ( esp[0] /* retrive syscall number */ )
    {
    case SYS_HALT:
      // printf("LOG [DEBUG]: in SYS_HALT\n");
      power_off();
      break;
    case SYS_EXIT:
      // printf("LOG [DEBUG]: in SYS_EXIT\n");
      // printf("LOG [DEBIG]: argument: %i\n",esp[1]);
      thread_exit();
      break;
    case SYS_READ:
      {
	//printf("LOG [DEBUG]: in SYS_READ\n");
	int fd = esp[1];
	char *buffer = (char*)esp[2];
	int size = esp[3];
	if(fd == STDOUT_FILENO || fd == -1)
	  {
	    f->eax = -1;
	    break;
	  }
	
	else if(fd == STDIN_FILENO)
	  {
	    int counter = 0;
	    while(counter < size)
	      {
		char c = input_getc();
		if(c == '\r')
		  c = '\n';
		*(buffer + counter) = c;
		putbuf((char*)&c,1);
		counter++;
	      }
	    f->eax = counter;
	  }
	else
	  {
	    struct file * file = map_find(&(thread_current()->open_file_table), fd);
	    if(file == NULL)
	      {
		f->eax = -1;
		break;
	      }
	    f->eax  = file_read(file, buffer, size);
	  }
      }
      break;
    case SYS_WRITE:
      {
	int fd = esp[1];
	char * buffer = (char*) esp[2];
	int size = esp[3];
	if(fd == STDIN_FILENO || fd == -1)
	  {
	    f->eax = -1;
	    break;
	  }
	else if(fd == STDOUT_FILENO)
	  {
	    putbuf(buffer, size);
	    f->eax = size;
	  }
	else
	  {
	    struct file * file = map_find(&(thread_current()->open_file_table), fd);
	    if(file == NULL)
	      {
		f->eax = -1;
		break;
	      }
	    f->eax = file_write(file, buffer,size);
	  }
      }
      break;
    case SYS_OPEN:
      {
	//	printf("Opend a file\n");
	char* file = (char*)esp[1];
	int fd = map_insert(&(thread_current()->open_file_table), filesys_open(file));
	f->eax = fd; 
      }
      break;
    case SYS_CREATE:
      {
	//	printf("Created a file\n");
	char * file = (char*) esp[1];
	unsigned int size = esp[2];
	f->eax = filesys_create(file,size);
	break;
      }
    case SYS_REMOVE:
      {
      //printf("Removing a file\n");
      char * file = (char*) esp[1];
      f->eax = filesys_remove(file);
      
      break;
      }
    case SYS_CLOSE:
      {
      // printf("Closed a file\n");
      int fd = esp[1];
      if(fd > 1)
	map_close_file(&(thread_current()->open_file_table), fd);
      break;
      }
    case SYS_SEEK:
      {
	int fd = esp[1];
	int pos = esp[2];
	struct file * file = map_find(&(thread_current()->open_file_table), fd);
	if(file == NULL)
	  {
	    break;
	  }
	file_seek(file, pos);
      }
      break;
    case SYS_TELL:
      {
	int fd = esp[1];
	struct file * file = map_find(&(thread_current()->open_file_table), fd);
	if(file == NULL)
	  {
	    f->eax = -1;
	    break;
	  }
	f->eax = file_tell(file);
      }
      break;
    case SYS_FILESIZE:
      {
	int fd = esp[1];
	struct file * file = map_find(&(thread_current()->open_file_table), fd);
	if(file == NULL)
	  {
	    f->eax = -1;
	    break;
	  }
	f->eax = file_length(file);
      }
      break;
    case SYS_SLEEP:

      break;
    case SYS_PLIST:

      break;
    case SYS_EXEC:

      break;
    default:
      {
	printf ("Executed an unknown system call!\n");
      
	printf ("Stack top + 0: %d\n", esp[0]);
	printf ("Stack top + 1: %d\n", esp[1]);
      
	thread_exit ();
      }
      
    }
  //printf("LOG [DEBUG]: end of syscall_handler\n");
}
