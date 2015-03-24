#include "Map.h"
#define NULL 0
void map_init(map* m)
{
	for(int i = 0; i <MAP_SIZE; i++)
		m->content[i] = NULL;
}

key_t map_insert(struct map* m, value_t v)
{
	for(int i = 0; i <MAP_SIZE; i++)
		if(m->content[i] == NULL)
		{
			m->content[i] = v;
			return i;
		}
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
	for(int i = 0; i < MAP_SIZE; i++)
		if(map_find(m,i) != NULL)
			exec(i, map_find(m,i), aux);
}

void map_remove_if(struct map* m, bool (*cond)(key_t k, value_t v, int aux), int aux)
{
	for(int i = 0; i < MAP_SIZE; i++)
		if(map_find(m,i) != NULL)
			if(cond(i, map_find(m,i), aux))
				map_remove(m,i);
}