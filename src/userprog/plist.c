#include <stddef.h>
#include "plist.h"
#define offset 0
void plist_init(struct plist* m)
{
  int i;
  for(i = 0; i < PLIST_SIZE; i++)
    m->content[i] =NULL;
}

plist_key_t plist_insert(struct plist* m, plist_value_t v)
{

  int i = 0;
  // printf("INSERTING INTO FLE TABLE\n");
  if(v == NULL)
    return -1;
  for(i = 0; i <PLIST_SIZE; i++){
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

plist_value_t plist_find(struct plist* m, plist_key_t k)
{
  return m->content[k - offset];
}

plist_value_t plist_remove(struct plist* m, plist_key_t k)
{
  plist_value_t rvalue = m->content[k - offset];
  m->content[k - offset] = NULL;
  return rvalue;
}

void plist_for_each(struct plist* m, void(*exec)(plist_key_t k, plist_value_t v, int aux), int aux)
{
  int i = 0;
  for(; i < PLIST_SIZE; i++)
    if(plist_find(m,i) != NULL)
      exec(i, plist_find(m,i), aux);
}

void plist_remove_if(struct plist* m, bool (*cond)(plist_key_t k, plist_value_t v, int aux), int aux)
{
  int i = 0;
  for(; i < PLIST_SIZE; i++)
    if(plist_find(m,i) != NULL)
      if(cond(i, plist_find(m,i), aux))
	plist_remove(m,i);
}

#include "plist.h"
