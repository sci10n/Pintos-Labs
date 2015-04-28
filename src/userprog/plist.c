#include "plist.h"
#include <stdio.h>
#define undefined 0x0FFFFFFF
process_list* plist_allocate_list_entry(plist_value_t v)
{   
  static plist_key_t id_generator = 0;
  debug("###first in allocate list entry\n");
  process_list * l = (process_list*)malloc(sizeof(process_list));
  l->element = v;
  l->next = NULL;
  l->element_id = id_generator++;
  debug("###second in allocate list entry\n");
  return l;
}

plist_value_t plist_form_process_info(int proc_id,int parent_id)
{
  plist_value_t t;
  t.alive = true;
  if(parent_id < 2)
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
  if(list == NULL)
    return NULL;
  if(list->element_id == element_id)
    return &(list->element);
  return plist_find(list->next, element_id);
}    

plist_key_t plist_insert(process_list* list, plist_value_t v)
{    
  if(list->next == NULL)
    {
      list->next = plist_allocate_list_entry(v);
      return list->element_id;
    }
  return plist_insert(list->next, v);
}

bool plist_remove(process_list*list, plist_key_t element_id, int exit_status)
{
  if(list == NULL)
    {
      return false;
    }
  if(list->element_id == element_id)
    {

      printf("#found element to remove\n");
      list->element.alive = false;
      list->element.exit_status = exit_status;
      list->element.free = !list->element.parent_alive;

      plist_remove_children(list->next,element_id);
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
      printf("#found child element to %i\n", parent_id);
      //list->element.alive = false;
      list->element.parent_alive = false;
      list->element.free = !list->element.alive;
      plist_remove_children(list->next, list->element_id);
    }
  return plist_remove_children(list->next, parent_id);
}


void plist_free(process_list** list)
{    
  if(!*list)
    return;
  process_list* current = *list;
  if(current && current->element.free)
    {
      process_list* tmp = current;
      current = current->next;
      free( tmp);
    }
  (*list) = current;
  process_list* prev = NULL;
  while(current)
    {
      if(current->element.free)
	{
	  printf("##FREEING A VALUE FROM LIST\n");
	  prev->next = current->next;
	  process_list* tmp = current;
	  current = current->next;
	  free(tmp);
	  continue;
	}
      prev = current;
      current = current->next;
    }
}

void plist_print_list(process_list * list)
{
  if(list == NULL)
    return;
  // printf("list:%i id:%i pid:%i Alive:%i pa:%i es:%i \tf:%i\n",list->element_id, list->element.proc_id, list->element.parent_id, list->element.alive, list->element.parent_alive, list->element.exit_status, list->element.free);
  plist_print_list(list->next);
}
