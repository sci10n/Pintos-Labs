#ifndef _PLIST_H_
#define _PLIST_H_
#include <stdbool.h>
#include <stdlib.h>
#include "threads/synch.h"
typedef struct process_info plist_value_t;
typedef struct process_list process_list;
typedef int plist_key_t;
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
/*
 *	info struct, the static variable is used to create new identifiers
 * 	unsigned since process id will be > 0 and (might) overflow in longrun
 */
struct process_info
{
     	
bool free;
int proc_id;
int parent_id;
int exit_status;
bool alive;
bool parent_alive;
};

/*
 *	simple linked list design
 *	insert 	O(n)
 *	delete 	O(n)
 * 	find 	O(n)
 */
struct process_list
{
  plist_key_t element_id;
  plist_value_t element;
  process_list * next;
};

/*
 *	used to create a list_entry
 */
process_list* plist_allocate_list_entry(plist_value_t v);

/*
 *	used to create a new process_info struct
 */
plist_value_t plist_form_process_info(int proc_id, int parent_id);

/*
 *	Will assume the list has been inizialized
 * 	is recursive
 * 	returns new process_id @see process_info
 */
plist_key_t plist_insert(process_list* list, plist_value_t v);

/*
 *	Finds and returns info corresponding with a identifier
 */
plist_value_t* plist_find(process_list*list, plist_key_t element_id);

/*
 *	Takes a id and marks the entry as dead will not remove entry
 */
bool plist_remove(process_list*list, plist_key_t process_id, int exit_status);
void plist_remove_children(process_list*list, int parent_id);
/*
 *	will remove if parent is dead
 *	If parent dies it wil take all children to it's grave
 */
void plist_free(process_list**list);

/*
 *	prints the list
 */
void plist_print_list(process_list * list);

#endif
