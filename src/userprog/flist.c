#include <stddef.h>
#include <stdio.h>
#include "flist.h"
#define offset 2
#define flist_debug 0

void map_init(struct map* m)
{
#if flist_debug
  debug("Enterd map_init\n");
#endif
  int i;
  for (i = 0; i < MAP_SIZE; i++)
    m->content[i] = NULL;
  lock_init(&(m->lock));
#if flist_debug
  debug("Exit map_init\n");
#endif
}

key_t map_insert(struct map* m, value_t v)
{
#if flist_debug
  debug("Enterd map_insert\n");
#endif
  int i = 0;
  if (v == NULL)
  {
#if flist_debug
    debug("Exit map_insert with -1\n");
#endif
    return -1;
  }

  for (i = 0; i < MAP_SIZE; i++) {
 lock_acquire(&(m->lock));
    if (m->content[i] == NULL)
    {
      m->content[i] = v;
       lock_release(&(m->lock));
#if flist_debug
      debug("Exit map_insert with %i\n", i + offset);
#endif
      return i + offset;
    }
    else
      lock_release(&(m->lock));
  }
#if flist_debug
  debug("Exit map_insert with -1\n");
#endif
  return -1;
}

value_t map_find(struct map* m, key_t k)
{
  if(k-offset < 0 || k-offset > MAP_SIZE)
    return NULL;
   lock_acquire(&(m->lock));
  value_t ret = m->content[k - offset];
   lock_release(&(m->lock));
  return ret;
}

value_t map_remove(struct map* m, key_t k)
{
  lock_acquire(&(m->lock));
#if flist_debug
  debug("Enterd map_remove with k: %i , offset: %i\n", k, offset);
#endif
  if(k-offset < 0 || k-offset > MAP_SIZE)
  {
      lock_release(&(m->lock));
        return NULL;
  }

  value_t rvalue = m->content[k - offset];
  m->content[k - offset] = NULL;
#if flist_debug
  debug("Exit map_remove\n");
#endif
  lock_release(&(m->lock));
  return rvalue;
}

void map_for_each(struct map* m, void(*exec)(key_t k, value_t v, int aux), int aux)
{
  int i = 0;
  for (; i < MAP_SIZE; i++)
    if (map_find(m, i) != NULL)
      exec(i, map_find(m, i), aux);
}

void map_remove_if(struct map* m, bool (*cond)(key_t k, value_t v, int aux), int aux)
{
  int i = 0;
  for (; i < MAP_SIZE; i++)
    if (map_find(m, i) != NULL)
      if (cond(i, map_find(m, i), aux))
        map_remove(m, i);
}

void map_close_file(struct map* m, key_t k)
{
#if flist_debug
  debug("Enterd map_close_files\n");
#endif
  value_t t = map_remove(m, k);
#if flist_debug
  debug("map_remove returned: %i\n", t);
#endif
  if (t != NULL)
  {
    filesys_close(t);
  }
#if flist_debug
  debug("Exit map_close_files\n");
#endif
}

void map_close_all_files(struct map * m)
{
#if flist_debug
  debug("Enterd map_close_all_files\n");
#endif
  if (m == NULL)
  {
#if flist_debug
    debug("Exit map_close_all_files\n");
#endif
    return;
  }

  int i;
  for (i = 0; i < MAP_SIZE; i++)
  {
    lock_acquire(&(m->lock));
    value_t t = m->content[i];
    lock_release(&(m->lock));
    //debug("What's inside?: %i, %i\n",t,i);
    if (t != NULL)
    {
      map_close_file(m, i);
    }

  }
#if flist_debug
  debug("Exit map_close_all_files\n");
#endif
}
