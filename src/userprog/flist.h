#ifndef _MAP_H_
#define _MAP_H_
#include <stdbool.h>
#include <stdlib.h>
#include "filesys/file.h"
#include "filesys/filesys.h"
typedef struct file* value_t;
typedef int key_t;
//typedef int mid_t;

#define MAP_SIZE 32

struct map
{
  value_t content[MAP_SIZE];
};
/*
struct maps
{
  mid_t id;
  struct map *map;
  struct maps* next = NULL;
};
//Functions for maps
key_t maps_insert(struct maps ** m, value_t t, mid_t id);
value_t maps_find(struct maps * m, key_t, mid_t id);
value_t maps_remove(struct maps * m, key_t k, mid_t id);
void maps_remove_all(struct maps * m, mid_t id);
//Extra
void maps_delete(struct maps *m, mid_t id);
*/
//Functions for map
void map_init(struct map* m);
key_t map_insert(struct map* m, value_t k);
value_t map_find(struct map* m, key_t k);
value_t map_remove(struct map*m, key_t k);
void map_for_each(struct map*m, void(*exec)(key_t k, value_t v, int aux), int aux);
void map_remove_if(struct map* m, bool (*cond)(key_t k, value_t v, int aux), int aux);

void map_close_file(struct map* m, key_t k);
void map_close_all_files(struct map * m);
//Please note that all indexes (FD) needs to be offset by 2 to accomodate screen and keyboard


/* Place functions to handle a process open files here (file list).
   
   flist.h : Your function declarations and documentation.
   flist.c : Your implementation.

   The following is strongly recommended:

   - A function that given a file (struct file*, see filesys/file.h)
   and a process id INSERT this in a list of files. Return an
   integer that can be used to find the opened file later.

   - A function that given an integer (obtained from above function)
   and a process id FIND the file in a list. Should return NULL if
   the specified process did not insert the file or already removed
   it.

   - A function that given an integer (obtained from above function)
   and a process id REMOVE the file from a list. Should return NULL
   if the specified process did not insert the file or already
   removed it.
   
   - A function that given a process id REMOVE ALL files the specified
   process have in the list.

   All files obtained from filesys/filesys.c:filesys_open() are
   considered OPEN files and must be added to a list or else kept
   track of, to guarantee ALL open files are eventyally CLOSED
   (probably when removed from the list(s)).
*/

#endif
