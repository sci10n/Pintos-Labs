#ifndef _PLIST_H_
#define _PLIST_H_
#include <stdbool.h>
#include <stdlib.h>
typedef struct process_info* plist_value_t;
typedef int plist_key_t;

#define PLIST_SIZE 32
struct process_info
{
  bool free;
  int proc_id;
  int parent_id;
  int exit_status;
  bool alive;
  bool parent_alive;
};

struct plist
{
  plist_value_t content[PLIST_SIZE];
};
//Functions for plist
void plist_init(struct plist* m);
plist_key_t plist_insert(struct plist* m, plist_value_t k);
plist_value_t plist_find(struct plist* m, plist_key_t k);
plist_value_t plist_remove(struct plist*m, plist_key_t k);
void plist_for_each(struct plist*m, void(*exec)(plist_key_t k, plist_value_t v, int aux), int aux);
void plist_remove_if(struct plist* m, bool (*cond)(plist_key_t k, plist_value_t v, int aux), int aux);
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


#endif
