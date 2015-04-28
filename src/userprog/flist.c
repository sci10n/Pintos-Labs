#include <stddef.h>
#include <stdio.h>
#include "flist.h"
#define offset 2
void map_init(struct map* m)
{
  int i;
  for(i = 0; i < MAP_SIZE; i++)
    m->content[i] =NULL;
  lock_init(&(m->lock));
}

key_t map_insert(struct map* m, value_t v)
{
  
  int i = 0;
  // printf("INSERTING INTO FLE TABLE\n");
  if(v == NULL)
    {

      return -1;
    }

  for(i = 0; i <MAP_SIZE; i++){
    //printf("On index %i %s\n", i, m->content[i]);
    if(m->content[i] == NULL)
      {
	m->content[i] = v;
	//	printf("Return: %i for %u\n", i , v);

	return i + offset;
      }
  }
  
  return -1;
}

value_t map_find(struct map* m, key_t k)
{
  value_t ret = m->content[k - offset];
  return ret;
}

value_t map_remove(struct map* m, key_t k)
{

  value_t rvalue = m->content[k - offset];
  m->content[k - offset] = NULL;

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

void map_close_file(struct map* m, key_t k)
{
  value_t t = map_remove(m,k);
  if(t != NULL)
    {
      filesys_close(t);
    }
}

void map_close_all_files(struct map * m)
{
  int i;
  for(i = 0; i< MAP_SIZE; i++)
    {
      value_t t = m->content[i];
      if(t != NULL)
	{
	  map_close_file(m, i+offset);
	}

    }
}
