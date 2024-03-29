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
   	int sys_read_arg_count = argc[ esp[0] ];
  switch ( esp[0] /* retrive syscall number */ )
   	{
   		case SYS_HALT:
   		power_off();
   		break;
   		case SYS_EXIT:
   		{
   			int exit_status = (int)(esp[1]);
   			process_exit(exit_status);
   			thread_exit();
   		}
   		break;
   		case SYS_WAIT:
   		{
   			int pid = esp[1];
   			int status = process_wait (pid);
   			f->eax = status;
   		}
   		break;
   		case SYS_READ:
   		{
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
   			char* file = (char*)esp[1];
   			int fd = map_insert(&(thread_current()->open_file_table), filesys_open(file));
   			f->eax = fd; 
   		}
   		break;
   		case SYS_CREATE:
   		{
   			char * file = (char*) esp[1];
   			unsigned int size = esp[2];
   			f->eax = filesys_create(file,size);
   			break;
   		}
   		case SYS_REMOVE:
   		{
   			char * file = (char*) esp[1];
   			f->eax = filesys_remove(file);
   			
   			break;
   		}
   		case SYS_CLOSE:
   		{
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
   		{
   			debug("Sleep with: %i\n", (int)(esp[1]));
   			timer_msleep((int)(esp[1]));
   		}
   		break;
   		case SYS_PLIST:
   		{
   			process_print_list();
   		}      
   		break;
   		case SYS_EXEC:
   		{
   			f->eax = process_execute(esp[1]);
   		}
   		break;
   		default:
   		{     
   			thread_exit ();
   		}
   		
   	}
   }
