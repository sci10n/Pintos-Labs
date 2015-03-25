#include <stdio.h>
#include "Map.h"

void print(int p1, char* val, int aux){
	printf("#%i- %s\n",p1, val);
}

bool rem(int p1, char* val, int aux){
	return val[0] == 'H';
}

int main(void){
  /*map m;
	map_init(&m);
	int p1 = map_insert(&m, (char*)"HELLO WORLD");
	printf("Value @%i: \t %s\n",p1, map_find(&m,p1));

	int p2 = map_insert(&m, (char*)"YO MOMA");
	printf("Value @%i: \t %s\n",p2, map_find(&m,p2));

	map_remove_if(&m, &rem, 0);
	map_for_each(&m, &print, 0);
  */
	printf("+++++++++++++++++\n");

	maps *ms = NULL;
	int p1 = maps_insert(&ms, (char*)"SUPERMAN", 0);
	
	printf("Value @%i: \t %s\n",p1, maps_find(ms,p1,0));
	
	int p2 = maps_insert(&ms, (char*)"BATMAN", 0);
	printf("Value @%i: \t %s\n",p2, maps_find(ms,p2,0));
       	maps_remove(ms, p2, 0);
	printf("Value @%i: \t %s\n",p2, maps_find(ms,p2,0));
	int p3 = maps_insert(&ms, (char*)"SUPERMAN2", 1);
	printf("Value @%i: \t %s\n",p3, maps_find(ms,p3,1));
	
	int p4 = maps_insert(&ms, (char*)"BATMAN", 1);
	printf("Value @%i: \t %s\n",p4, maps_find(ms,p4,1));

	int p5 = maps_insert(&ms, (char*)"SUPERMAN2", 0);
	printf("Value @%i: \t %s\n",p5, maps_find(ms,p5,1));

	int p6 = maps_insert(&ms, (char*)"HELLO WORLD", 1);
	printf("Value @%i: \t %s\n",p6, maps_find(ms,p5,1));
	maps_delete(ms,0);
	maps_delete(ms,1);
	return 0;
}
