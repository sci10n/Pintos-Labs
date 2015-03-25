#include <stddef.h>
#include <stdio.h>
#include "Map.h"

key_t maps_insert(struct maps ** m, value_t v, mid_t id)
{
  if((*m) == NULL)
    {
      (*m) = (struct maps*) malloc(sizeof (struct maps));
      (*m)->id = id;
      (*m)->next = NULL;
      (*m)->map = (struct map*) malloc(sizeof(struct map));
      map_init((*m)->map);
      return map_insert((*m)->map,v);
    }
  if((*m)->id == id)
    {
      return map_insert((*m)->map, v);
    }
  return maps_insert(&(*m)->next, v, id);
}

value_t maps_find(struct maps * m, key_t k, mid_t id)
{
  if(m == NULL)
    return "";
  if(m->id == id)
    return map_find(m->map,k);
  return maps_find(m->next, k, id);
}

value_t maps_remove(struct maps * m, key_t k, mid_t id)
{
  struct maps* ptr = m;
  while(ptr != NULL)
    {
      if(ptr->id == id)
	return map_remove((ptr->map), k);
      ptr = ptr->next;
    }
  return NULL;
}

void maps_delete(struct maps * m, mid_t id)
{
  struct maps * ptr = m;
  if(ptr->id == id)
    {
      free(ptr->map);
      free(ptr);
      ptr = ptr->next;
      return;
    }
  while(ptr->next != NULL)
    {
      if(ptr->next->id == id)
	{
	  struct maps* td = ptr->next;
	  ptr->next = ptr->next->next;
	  free(td->map);
	  free(td);
	  return;
	}
      ptr = ptr->next;
    }
}

void map_init(struct map* m)
{
  int i;
  for(i = 0; i < MAP_SIZE; i++)
    m->content[i] =NULL;
}

key_t map_insert(struct map* m, value_t v)
{

  int i = 0;

  for(i = 0; i <MAP_SIZE; i++){
    //printf("On index %i %s\n", i, m->content[i]);
    if(m->content[i] == NULL)
      {
	m->content[i] = v;
	//	printf("Return: %i for %u\n", i , v);
	return i;
      }
  }
  return -1;
}

value_t map_find(struct map* m, key_t k)
{
  return m->content[k];
}

value_t map_remove(struct map* m, key_t k)
{
  value_t rvalue = m->content[k];
  m->content[k] = NULL;
  return rvalue;
}

void map_for_each(struct map* m, void(*exec)(key_t k, value_t v, int aux), int aux)
{
  int i = 0;
  for(; i < MAP_SIZE; i++)
    if(map_find(m,i) != NULL)
      exec(i, map_find(m,i), aux);
}

void map_remove_if(struct map* m, bool (*cond)(key_t k, value_t v, int aux), int aux)
{
  int i = 0;
  for(; i < MAP_SIZE; i++)
    if(map_find(m,i) != NULL)
      if(cond(i, map_find(m,i), aux))
	map_remove(m,i);
}
