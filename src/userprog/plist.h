#ifndef _PLIST_H_
#define _PLIST_H_
#include <stdbool.h>
#include <stdlib.h>
#include "threads/synch.h"
typedef struct process_info plist_value_t;
typedef struct process_list process_list;
typedef int plist_key_t;
#define PLIST_MAX 100
/* Place functions to handle a running process here (process list).
   
   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
   inserts this in a list of running processes and return an integer
   that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
   FIND the process information in the list. Should return some
   failure code if no process matching the integer is in the list.
   Or, optionally, several functions to access any information of a
   particular process that you currently need.

   - A function that given an integer REMOVE the process information
   from the list. Should only remove the information when no process
   or thread need it anymore, but must guarantee it is always
   removed EVENTUALLY.
     
   - A function that print the entire content of the list in a nice,
   clean, readable format.
     
*/
struct process_info
{
  bool free;
  int parent_id;
  int exit_status;
  bool alive;
  bool parent_alive;
  bool is_waiting;
  struct semaphore is_done;
  //semaphore wait for child?
};

struct process_list
{
plist_value_t table[PLIST_MAX];
};

void init_fatlock(process_list* list);

plist_value_t plist_form_process_info(int parent_id);
//Returns a number to find and remove the element later NOT THE PROCESS ID
plist_key_t plist_insert(process_list* list, plist_value_t v);

//returns -1 if cant find and places return value in parameter return_value
int plist_find(process_list* list,plist_value_t*return_value,  plist_key_t element_id);

void plist_set_exit_status(process_list* list, plist_key_t element_id, int exit_status);
int plist_get_exit_status(process_list* list, plist_key_t element_id);
bool plist_remove(process_list* list, plist_key_t element_id);

void plist_remove_children(process_list* list, int parent_element_id);

void plist_clean(process_list* list);

void plist_print_list(process_list*  list);


#endif
