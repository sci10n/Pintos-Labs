#include <stddef.h>
#include "plist.h"
#include <stdio.h>
#define offset 0
void plist_init(struct plist* m)
{
  int i;
  for(i = 0; i < PLIST_SIZE; i++)
    m->content[i] =NULL;
}

plist_key_t plist_insert(struct plist* m, plist_value_t v)
{
  debug("Inserting a id %i\n", v->proc_id);
  int i = 0;
  // printf("INSERTING INTO FLE TABLE\n");
  if(v == NULL)
    return -1;
  if(m->content[v->proc_id] == NULL)
    {
      m->content[v->proc_id] = v;
       return v->proc_id; 
    }
  free(v);
  return -1;
}

plist_value_t plist_find(struct plist* m, plist_key_t k)
{
  return m->content[k - offset];
}

void plist_remove(struct plist* m, plist_key_t k, int status)
{
  plist_value_t v = m->content[k];
      debug("Remove process id %i with status %i\n",k,status);
  if(v == NULL)
    {
      debug("Error while removing %i\n", k);
      return;
    }

 v->exit_status = status;
  v->alive = false;
  plist_value_t vp =plist_find(m, v->parent_id); 
  v->parent_alive = vp == NULL ? false : vp->alive;
  if(!v->parent_alive)
      v->free = true;
  plist_update(m);
}

void plist_for_each(struct plist* m, void(*exec)(plist_key_t k, plist_value_t v, int aux), int aux)
{
  int i = 0;
  for(; i < PLIST_SIZE; i++)
    if(plist_find(m,i) != NULL)
      exec(i, plist_find(m,i), aux);
}
/*
void plist_remove_if(struct plist* m, bool (*cond)(plist_key_t k, plist_value_t v, int aux), int aux)
{
  int i = 0;
  for(; i < PLIST_SIZE; i++)
    if(plist_find(m,i) != NULL)
      if(cond(i, plist_find(m,i), aux))
	plist_remove(m,i);
}
*/
void plist_print(struct plist*m)
{
  int i = 0;
  for(; i < PLIST_SIZE; i++)
    {
      plist_value_t t = plist_find(m,i);
      if(t != NULL)
	debug("List entry: %i \tFree: %i \tProc_id:%i \tParent_id:%i \texit_status:%i \talive:%i \tparent_alive:%i \n",i,t->free,t->proc_id, t->parent_id, t->exit_status, t->alive, t->parent_alive);
    }
}

void plist_update(struct plist*m)
{
  int i = 0;
  for(; i < PLIST_SIZE; i++)
    {
      plist_value_t t = plist_find(m,i);
      if(t != NULL)
	{
	  if(t->free)
	    {
	      debug("REMOVING INDEX %i FROM PROCESS ID TABLE\n", t->proc_id);
	      m->content[t->proc_id] = NULL;
	      free(t);
	      t = NULL;
	    }
	}
    }
}
#include "plist.h"
