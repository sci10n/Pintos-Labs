#include "plist.h"
#include <stdio.h>
#include "threads/synch.h"
#define undefined 0x0000100
struct semaphore plist_fatlock;
plist_key_t id_generator = 0;
void init_fatlock()
{
sema_init(&plist_fatlock,1);
}

process_list* plist_allocate_list_entry(plist_value_t v)
{   
  debug("Enterd plist_allocate_list_entry\n");
  process_list * l = (process_list*)malloc(sizeof(process_list));
  l->element = v;
  l->next = NULL;
  l->element_id = id_generator++;
  debug("Exit plist_allocate_list_entry\n");
  return l;
}

plist_value_t plist_form_process_info(int proc_id,int parent_id)
{
  debug("Enterd process_info\n");
  //sema_down
  sema_down(&plist_fatlock);
  plist_value_t ret = plist_form_process_info_r(proc_id, parent_id);
  sema_up(&plist_fatlock);
  //sema_up
  debug("Exit process_info\n");
  return ret;
}

plist_value_t plist_form_process_info_r(int proc_id,int parent_id)
{

  plist_value_t t;
  t.alive = true;
  debug("HELLO");
  if(parent_id == 0)
    t.parent_alive = false;
  else
    t.parent_alive = true;

  t.proc_id = proc_id;
  t.parent_id = parent_id;
  t.free = false;
  t.exit_status = undefined;

  return t;
}

plist_value_t* plist_find(process_list*list, plist_key_t element_id)
{
  sema_down(&plist_fatlock);
  //sema_down
  debug("Enterd find\n");

  plist_value_t* ret = plist_find_r(list, element_id);

  debug("Exit find\n");
  sema_up(&plist_fatlock);
  //sema_up
  return ret;
}

plist_value_t* plist_find_r(process_list*list, plist_key_t element_id)
{
  if(list == NULL)
    return NULL;
  if(list->element_id == element_id)
    return &(list->element);
  return plist_find(list->next, element_id);
}    

plist_key_t plist_insert(process_list* list, plist_value_t v)
{
  //sema_dow
  debug("Enterd insert\n");
  sema_down(&plist_fatlock);
  plist_key_t ret = plist_insert_r(list, v);
  sema_up(&plist_fatlock);
  //sema_up
  debug("Exit insert\n");
  return ret;
}

plist_key_t plist_insert_r(process_list* list, plist_value_t v)
{   
  if(list == NULL)
    {
      debug("Insert in empty list\n");
      return -1;
    }

  if(list->next == NULL)
    {
      list->next = plist_allocate_list_entry(v);
      if(list->next == NULL)
	return -1;
      return list->next->element_id;
    }
  return plist_insert_r(list->next, v);
}

bool plist_remove(process_list*list, plist_key_t element_id, int exit_status)
{
  debug("Enterd remove\n");
  //sema_down
  sema_down(&plist_fatlock);
  bool ret = plist_remove_r(list, element_id, exit_status);
  sema_up(&plist_fatlock);
  debug("Exit remove\n");
  //sema_up
  return ret;
}

bool plist_remove_r(process_list*list, plist_key_t element_id, int exit_status)
{
  if(list == NULL || element_id < 0)
    {
      return false;
    }
  if(list->element_id == element_id)
    {
      list->element.alive = false;
      list->element.exit_status = exit_status;
      list->element.free = !list->element.parent_alive;
      plist_remove_children(list,element_id);
      return true;
    }
  return plist_remove_r(list->next, element_id, exit_status);
}

void plist_remove_children(process_list*list, int parent_id)
{
  if(list == NULL)
    return;
  if(list->element.parent_id == parent_id)
    {
      //list->element.alive = false;
      list->element.parent_alive = false;
      list->element.free = !list->element.alive;
      plist_remove_children(list->next, list->element_id);
    }
  return plist_remove_children(list->next, parent_id);
}


void plist_free(process_list* list)
{    
  debug("Enterd free\n");
  sema_down(&plist_fatlock);
  if(list == NULL)
    {
      debug("Exit free empty list\n");
      sema_up(&plist_fatlock);
      return;
    }

  process_list* current = list;
  process_list* prev = NULL;

  while(current != NULL)
    {

      if(current->next == NULL )
	{
	  if(current->element.free && prev != NULL)
	    {
	      free(current);
	      current = NULL;
	      prev->next = NULL;
	      debug("Exit free 1\n");
	      sema_up(&plist_fatlock);
	      return;
	    }
	  prev = current;
	  current = current->next;
	}
      else if(current->next->element.free)
	{
	  process_list*tmp = current->next;
	  prev = current;
	  current->next = current->next->next;
	  free(tmp);
	}
      else
	{
	  prev = current;
	  current = current->next;
	}
    }
  sema_up(&plist_fatlock);
  debug("Exit free\n");
}

void plist_print_list(process_list * list)
{
  debug("Enterd print\n");
  //sema_down
  sema_down(&plist_fatlock);
  plist_print_list_r(list);
  sema_up(&plist_fatlock);
  debug("Exit print\n");
  //sema_up
}

void plist_print_list_r(process_list * list)
{
  if(list == NULL)
    return;
  //debug("list:%i id:%i pid:%i Alive:%i pa:%i es:%i \tf:%i\n",list->element_id, list->element.proc_id, list->element.parent_id, list->element.alive, list->element.parent_alive, list->element.exit_status, list->element.free);
  if(list->next == NULL)
    return;  
  plist_print_list_r(list->next);
}
