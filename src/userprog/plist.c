#include "plist.h"
#include <stdio.h>
#include "threads/synch.h"
#define undefined 0x100
struct semaphore plist_fatlock;
void init_fatlock(process_list * list)
{
   debug("Enter fatlock\n");
  sema_init(&plist_fatlock,1);

 int i = 0;
 for(; i < PLIST_MAX; i++)
   {
   list->table[i].free = true;
   list->table[i].parent_id = undefined;
   list->table[i].alive = false;
   list->table[i].parent_alive= false;
   list->table[i].exit_status = undefined;
   sema_init(&(list->table[i].is_done),0);
   }

  debug("Exit fatlock\n");
}


plist_value_t plist_form_process_info(int parent_id)
{
  debug("Enterd process_info\n");
  sema_down(&plist_fatlock);
  debug("Enterd process_info\n");
  plist_value_t t;
  t.alive = true;
  //check if parent = idle thread (tid = -1)
  t.parent_alive = parent_id != -1;
  t.parent_id = parent_id;
  t.free = false;
  t.exit_status = undefined;
  sema_up(&plist_fatlock);
  debug("Exit process_info\n");
  return t;
}

int plist_find(process_list* list,plist_value_t* return_value,  plist_key_t element_id)
{
  debug("Enterd find\n");
  sema_down(&plist_fatlock);

  plist_value_t ret = list->table[element_id];
  if(ret.free || ret.parent_id == undefined)
    return -1;
  return_value = &(list->table[element_id]);
  sema_up(&plist_fatlock);
  debug("Exit find\n");
  return 1;
}


plist_key_t plist_insert(process_list* list, plist_value_t v)
{
  //sema_dow
  debug("Enterd insert\n");
  sema_down(&plist_fatlock);
  int i = 0;
  plist_key_t ret = -1;
  for(;i < PLIST_MAX; i++)
  {
     if(list->table[i].free)
     {
      list->table[i] = v;
      sema_init(&(list->table[i].is_done),0);
      ret = i;
      break;
     }
  }
  sema_up(&plist_fatlock);
  debug("Exit insert with %i\n",ret);
  return ret;
}

void plist_set_exit_status(process_list* list, plist_key_t element_id, int exit_status)
{
  debug("Enterd set_exit_status\n");
  sema_down(&plist_fatlock);
  bool isFree = list->table[element_id].free;
  sema_up(&plist_fatlock);
  if(!isFree)
    {
      sema_down(&plist_fatlock);
      list->table[element_id].exit_status = exit_status;
      sema_up(&(list->table[element_id].is_done));
      sema_up(&plist_fatlock);

    }

  debug("Exit exit_status\n");
}

int plist_get_exit_status(process_list* list, plist_key_t element_id)
{
  debug("Enterd get_exit_status\n");
  sema_down(&plist_fatlock);
  int ret = -1;
  bool isFree = list->table[element_id].free;
  sema_up(&plist_fatlock);
  if(isFree)
    return -1;
  sema_down(&plist_fatlock);
  ret = list->table[element_id].exit_status;
  list->table[element_id].free = true;
  sema_up(&plist_fatlock);
  debug("Exit get exit_status with %i\n",ret);
  return ret;
}

void plist_wait_for_pid(process_list*list,plist_key_t element_id)
{
  sema_down(&(list->table[element_id].is_done));
}

bool plist_remove(process_list* list, plist_key_t element_id)
{
  debug("Enterd remove\n");
  //sema_down
  sema_down(&plist_fatlock);
  bool ret = false;
    bool is_empty = list->table[element_id].free;
    if(!is_empty)
    { 
      int parent_id = element_id;
      int i = 0;
      for(; i < PLIST_MAX; i++)
      {
        if(!list->table[i].free && (list->table[i].parent_id == parent_id || parent_id == -1))
        {
          list->table[i].parent_alive = false;
        }
      }
      list->table[element_id].alive = false;
      list->table[element_id].free = !list->table[element_id].alive && !list->table[element_id].parent_alive;
      //
      ret = true;
    }
    sema_up(&plist_fatlock);
   
  debug("Exit remove\n");
  //sema_up
  return ret;
}

void plist_clean(process_list* list)
{    
  debug("Enterd free\n");
  sema_down(&plist_fatlock);
   int i = 0;
    for(; i < PLIST_MAX; i++)
    {
      if(!list->table[i].free && (!list->table[i].alive && !list->table[i].parent_alive))
      {
        list->table[i].free = true;
      }
      }
  sema_up(&plist_fatlock);
  debug("Exit free\n");
}

void plist_print_list(process_list* list)
{
  debug("Enterd print\n");
  //sema_down
  sema_down(&plist_fatlock);
  int i = 0;
  for(;i < PLIST_MAX; i++)
  {
      if(!list->table[i].free)
         debug("id:%i pid:%i Alive:%i pa:%i es:%i \tf:%i\n",i, list->table[i].parent_id, list->table[i].alive, list->table[i].parent_alive, list->table[i].exit_status, list->table[i].free);
  }
  sema_up(&plist_fatlock);
  debug("Exit print\n");
  //sema_up
}
