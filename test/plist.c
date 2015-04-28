#include "plist.h"
#include <stdio.h>
#define undefined 0x0000100
process_list* plist_allocate_list_entry(plist_value_t v)
{   
  static plist_key_t id_generator = 0;
  process_list * l = (process_list*)malloc(sizeof(process_list));
  l->element = v;
  l->next = NULL;
  l->element_id = id_generator++;

  return l;
}

plist_value_t plist_form_process_info(int proc_id,int parent_id)
{
  //sema_down
  plist_value_t ret = plist_form_process_info_r(proc_id, parent_id);
  //sema_up
  return ret;
}

plist_value_t plist_form_process_info_r(int proc_id,int parent_id)
{
  plist_value_t t;
  t.alive = true;
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
  //sema_down
  plist_value_t* ret = plist_find_r(list, element_id);
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
 //sema_down
  plist_key_t ret = plist_insert_r(list, v);
 //sema_up
  return ret;
}

plist_key_t plist_insert_r(process_list* list, plist_value_t v)
{    
  if(list->next == NULL)
    {
      list->next = plist_allocate_list_entry(v);
      return list->next->element_id;
    }
  return plist_insert(list->next, v);
}

bool plist_remove(process_list*list, plist_key_t element_id, int exit_status)
{
  //sema_down
    bool ret = plist_remove_r(list, element_id, exit_status);
  //sema_up
    return ret;
}

bool plist_remove_r(process_list*list, plist_key_t element_id, int exit_status)
{
  if(list == NULL)
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
  return plist_remove(list->next, element_id, exit_status);
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
  if(list == NULL)
    return;
    process_list* current = list;
    process_list* prev = NULL;

    while(current != NULL)
    {

      if(current->next == NULL )
      {
        if(current->element.free)
        {
            prev->next = NULL;
        }
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
}

void plist_print_list(process_list * list)
{
  //sema_down
  plist_print_list_r(list);
  //sema_up
}

void plist_print_list_r(process_list * list)
{
  if(list == NULL)
    return;
   printf("list:%i id:%i pid:%i Alive:%i pa:%i es:%i \tf:%i\n",list->element_id, list->element.proc_id, list->element.parent_id, list->element.alive, list->element.parent_alive, list->element.exit_status, list->element.free);
  plist_print_list(list->next);
}
