#include <debug.h>
#include <stdio.h>
#include <string.h>

#include "userprog/gdt.h"      /* SEL_* constants */
#include "userprog/process.h"
#include "userprog/load.h"
#include "userprog/pagedir.h"  /* pagedir_activate etc. */
#include "userprog/tss.h"      /* tss_update */
#include "filesys/file.h"
#include "threads/flags.h"     /* FLAG_* constants */
#include "threads/thread.h"
#include "threads/vaddr.h"     /* PHYS_BASE */
#include "threads/interrupt.h" /* if_ */

/* Headers not yet used that you may need for various reasons. */
#include "threads/synch.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"

#include "userprog/flist.h"
#include "userprog/plist.h"

/* HACK defines code you must remove and implement in a proper way */
//#define HACK

struct process_list process_id_table;
/* This function is called at boot time (threads/init.c) to initialize
 * the process subsystem. */
void process_init(void)
{
   init_fatlock(&process_id_table);
  //sema_init()
}

/* This function is currently never called. As thread_exit does not
 * have an exit status parameter, this could be used to handle that
 * instead. Note however that all cleanup after a process must be done
 * in process_cleanup, and that process_cleanup are already called
 * from thread_exit - do not call cleanup twice! */
void process_exit(int status UNUSED)
{
  plist_set_exit_status(&process_id_table, thread_current()->tid,status);
}

/* Print a list of all running processes. The list shall include all
 * relevant debug information in a clean, readable format. */
void process_print_list()
{
  plist_print_list(&process_id_table);
}


struct parameters_to_start_process
{
  char* command_line;
  struct semaphore semaphore_process_id; 
  int process_id;
  int parent_id;
};

static void
start_process(struct parameters_to_start_process* parameters) NO_RETURN;

/* Starts a new proccess by creating a new thread to run it. The
   process is loaded from the file specified in the COMMAND_LINE and
   started with the arguments on the COMMAND_LINE. The new thread may
   be scheduled (and may even exit) before process_execute() returns.
   Returns the new process's thread id, or TID_ERROR if the thread
   cannot be created. */
int
process_execute (const char *command_line) 
{
  char debug_name[64];
  int command_line_size = strlen(command_line) + 1;
  tid_t thread_id = -1;
  int  process_id = -1;

  /* LOCAL variable will cease existence when function return! */

  debug("%s#%d: process_execute(\"%s\") ENTERED\n",
        thread_current()->name,
        thread_current()->tid,
        command_line);
  struct parameters_to_start_process arguments;
  arguments.parent_id = thread_current()->tid;
  /* COPY command line out of parent process memory */
  arguments.command_line = malloc(command_line_size);
  strlcpy(arguments.command_line, command_line, command_line_size);


  strlcpy_first_word (debug_name, command_line, 64);
  
  sema_init(&(arguments.semaphore_process_id), 0);
  /* SCHEDULES function `start_process' to run (LATER) */
  thread_id = thread_create (debug_name, PRI_DEFAULT,
                             (thread_func*)start_process, &arguments);
  if(thread_id == -1)
    {
    debug("Error in thread create\n");
    process_id = -1;
    }  
  else
    {
      sema_down(&(arguments.semaphore_process_id));
      process_id =arguments.process_id;
      if(process_id != -1)
	{
	  debug("Returned to process_execute with valid id\n");
	  //Add to list
	}
    }
  /* AVOID bad stuff by turning off. YOU will fix this! */
  //power_off();

  
  /* WHICH thread may still be using this right now? */
  // free(arguments.command_line);

  debug("%s#%d: process_execute(\"%s\") RETURNS %d\n",
        thread_current()->name,
        thread_current()->tid,
        command_line, process_id);

  /* MUST be -1 if `load' in `start_process' return false */
  return process_id;
}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (struct parameters_to_start_process* parameters)
{
  /* The last argument passed to thread_create is received here... */
  struct intr_frame if_;
  bool success;

  char file_name[64];
  strlcpy_first_word (file_name, parameters->command_line, 64);
  
  debug("%s#%d: start_process(\"%s\") ENTERED\n",
        thread_current()->name,
        thread_current()->tid,
        parameters->command_line);
  
  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;

  success = load (file_name, &if_.eip, &if_.esp);

  debug("%s#%d: start_process(...): load returned %d\n",
        thread_current()->name,
        thread_current()->tid,
        success);
  if (success)
    {
      /* We managed to load the new program to a process, and have
	 allocated memory for a process stack. The stack top is in
	 if_.esp, now we must prepare and place the arguments to main on
	 the stack. */

      plist_value_t value = plist_form_process_info(parameters->parent_id);
      thread_current()->tid = plist_insert(&process_id_table,value );
      if(thread_current()->tid == -1)
	success = false;
      else
	{
	  /* A temporary solution is to modify the stack pointer to
	     "pretend" the arguments are present on the stack. A normal
	     C-function expects the stack to contain, in order, the return
	     address, the first argument, the second argument etc. */
	  if_.esp = createStack(parameters->command_line, if_.esp);
	  //HACK if_.esp -= 12; /* Unacceptable solution. */

	  /* The stack and stack pointer should be setup correct just before
	     the process start, so this is the place to dump stack content
	     for debug purposes. Disable the dump when it works. */
    
	  // dump_stack ( PHYS_BASE + 15, PHYS_BASE - if_.esp + 16 );


	}
	  parameters->process_id = thread_current()->tid;
	  sema_up(&(parameters->semaphore_process_id));
  }

  debug("%s#%d: start_process(\"%s\") DONE\n",
        thread_current()->name,
        thread_current()->tid,
        parameters->command_line);

  /* If load fail, quit. Load may fail for several reasons.
     Some simple examples:
     - File doeas not exist
     - File do not contain a valid program
     - Not enough memory
  */
  if ( ! success )
    {
      debug("# problem with start process\n");
      parameters->process_id = -1;
      sema_up(&(parameters->semaphore_process_id));
      thread_exit ();
    }
  /* Start the user process by simulating a return from an interrupt,
     implemented by intr_exit (in threads/intr-stubs.S). Because
     intr_exit takes all of its arguments on the stack in the form of
     a `struct intr_frame', we just point the stack pointer (%esp) to
     our stack frame and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Wait for process `child_id' to die and then return its exit
   status. If it was terminated by the kernel (i.e. killed due to an
   exception), return -1. If `child_id' is invalid or if it was not a
   child of the calling process, or if process_wait() has already been
   successfully called for the given `child_id', return -1
   immediately, without waiting.

   This function will be implemented last, after a communication
   mechanism between parent and child is established. */
int
process_wait (int child_id) 
{

  struct thread *cur = thread_current ();
  int status = -1;
  //debug("%s#%d: process_wait(%d) ENTERED\n",
  //      cur->name, cur->tid, child_id);
  /* Yes! You need to do something good here ! */
  if(plist_wait_for_pid(&process_id_table,child_id))
  {
    status = plist_get_exit_status(&process_id_table,child_id);
  }
  
  //debug("%s#%d: process_wait(%d) RETURNS %d\n",
  //      cur->name, cur->tid, child_id, status);
   return status;
 
}

/* Free the current process's resources. This function is called
   automatically from thread_exit() to make sure cleanup of any
   process resources is always done. That is correct behaviour. But
   know that thread_exit() is called at many places inside the kernel,
   mostly in case of some unrecoverable error in a thread.

   In such case it may happen that some data is not yet available, or
   initialized. You must make sure that nay data needed IS available
   or initialized to something sane, or else that any such situation
   is detected.
*/
  
void
process_cleanup (void)
{
  struct thread  *cur = thread_current ();
  uint32_t       *pd  = cur->pagedir;
  int status = plist_get_exit_status(&process_id_table,cur->tid);
  //int status = plist_get_exit_status(&process_id_table,cur->tid);
  
  debug("%s#%i: process_cleanup() ENTERED\n", cur->name, cur->tid);
  
  /* Later tests DEPEND on this output to work correct. You will have
   * to find the actual exit status in your process list. It is
   * important to do this printf BEFORE you tell the parent process
   * that you exit.  (Since the parent may be the main() function,
   * that may sometimes poweroff as soon as process_wait() returns,
   * possibly before the prontf is completed.)
   */
  printf("%s: exit(%i)\n", thread_name(), status);

  plist_remove(&process_id_table, cur->tid);
  plist_clean(&process_id_table);
  // plist_print_list(&process_id_table);
  
  
/* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  if (pd != NULL) 
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }  
  //debug("%s#%i: process_cleanup() DONE with status %i\n",
  //      cur->name, cur->tid, status);
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

void* createStack(const char * command_line, void* stack_top)
{
  /* Variable "esp" stores an address, and at the memory loaction
   * pointed out by that address a "struct main_args" is found.
   * That is: "esp" is a pointer to "struct main_args" */
  void* esp;
  int argc;
  int total_size;
  int line_size_nd;
  int line_size;
  /* "cmd_line_on_stack" and "ptr_save" are variables that each store
     * one address, and at that address (the first) char (of a possible
    * sequence) can be found. */
  char* cmd_line_on_stack;
  char* ptr_save;
  int i = 0;
  
  /* calculate the bytes needed to store the command_line */
  line_size = 1; 
  const char* c = command_line;
  while(*(c++) != '\0')
    {
      line_size++;
    }
  /* round up to make it even divisible by 4 */
  line_size_nd = line_size;
  line_size += line_size % 4 == 0 ? 0: 4 - line_size % 4;
  /* calculate how many words the command_line contain */
  argc = 0;
  c = command_line;
  char prev = ' ';
  while(*c != '\0')
    {
      
      if(*c != ' ' && prev == ' ')
   	argc++;
      prev = *(c++);
	
    }
  /* calculate the size of each word */
  int argv_size[argc];
  int tmp = 0;
  c = command_line;
  i = 0;
  while(i < argc)
    {
      if(*c == ' ' || *(c) == '\0')
	{
	  if(tmp != 0)
	    {
	      argv_size[i++] = tmp;
	      tmp = 0;
	    }
	}
      else
	tmp++;
      c++;
    }
  /* calculate the size needed on our simulated stack */
  total_size = line_size + argc*4 + 16;
  /* calculate where the final stack top will be located */
   esp =  stack_top - (total_size);
  /* setup return address and argument count */
  //esp->ret = 0;
  //esp->argc = argc;
  
  /* calculate where in the memory the words is stored */
  cmd_line_on_stack = stack_top - line_size;

  /* calculate where in the memory the argv array starts */
  //esp->argv = (cmd_line_on_stack - (argc+2)*4);

  /* copy the command_line to where it should be in the stack */
  int*address_of_null[argc+1];
  int temp_index = 1;
  address_of_null[0] = 0;
  int tmp_ptr = 0;
  prev = 0;
  for(i = 0; i<= line_size; i++)
    {
      char ch = command_line[ (line_size - i)];
      if(ch != ' ')
	tmp_ptr = stack_top -i;
      if((ch == ' ') && ( line_size - i < line_size_nd))
	{
	  ch = '\0';
	  if(prev != '\0')
	    address_of_null[temp_index++] = tmp_ptr;
	}
      (*(char*)(stack_top-i)) = ch; //might need to filter ' ' or '\0' or '.'
      prev = ch;
    }
  address_of_null[argc] = tmp_ptr;

  /* build argv array and insert null-characters after each word */
  for(i = 1; i<= argc+1; i++)
    {
      *(int*)(cmd_line_on_stack-i*4) = address_of_null[i-1];
    }

  *(int*)(cmd_line_on_stack - (argc+2)*4) = cmd_line_on_stack - (argc+1)*4;

  *(cmd_line_on_stack - (argc+3)*4) = argc;

  //*(cmd_line_on_stack - (argc+4)*4) = esp->ret;
  *(cmd_line_on_stack - (argc+4)*4) = 0;
  return esp; /* the new stack top */
}
