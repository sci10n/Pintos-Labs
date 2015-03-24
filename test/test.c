#include <stdio.h>
#include "Map.h"

void print(int p1, char* val, int aux){
	printf("#%i- %s\n",p1, val);
}

bool rem(int p1, char* val, int aux){
	return val[0] == 'h';
}

int main(void){
	map m;
	map_init(&m);
	int p1 = map_insert(&m, (char*)"HELLO WORLD");
	printf("Value @%i: \t %s\n",p1, map_find(&m,p1));

	int p2 = map_insert(&m, (char*)"YO MOMA");
	printf("Value @%i: \t %s\n",p2, map_find(&m,p2));

	map_remove_if(&m, &rem, 0);
	map_for_each(&m, &print, 0);
	return 0;
}