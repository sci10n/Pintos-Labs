#include "plist.h"
#include <stdio.h>
#include <stdlib.h>
int main(){
	
	process_list* list = plist_allocate_list_entry(plist_form_process_info(0,0));
	/*
	*				0
	*               |  
	*               1         
	*             /   \            
	*            2     3                   
	*           / \                           
	*          4   5                             
	*             / 
	*            6
	*		remove 5 then 6 then 2 then 3 then 1 then 4
	*/

	int p1 = plist_insert(list, plist_form_process_info(1,0));
	int p2 = plist_insert(list, plist_form_process_info(2,p1));
	int p3 = plist_insert(list, plist_form_process_info(3,p2));
	int p4 = plist_insert(list, plist_form_process_info(4,p3));
	int p5 = plist_insert(list, plist_form_process_info(5,p4));
	int p6 = plist_insert(list, plist_form_process_info(6,p5));
	plist_print_list(list);

	printf("\n");
	plist_remove(list, p5, -1);
	plist_print_list(list);
	plist_free(list);
	printf("\n");

	plist_remove(list, p6, -1);
	plist_print_list(list);
	plist_free(list);
	printf("\n");

	plist_remove(list, p2, -1);
	plist_print_list(list);
	plist_free(list);
	printf("\n");

	plist_remove(list, p3, -1);
	plist_print_list(list);
	plist_free(list);
	printf("\n");

	plist_remove(list, p1, -1);
	plist_print_list(list);
	plist_free(list);
	printf("\n");

	plist_remove(list, p4, -1);
	plist_free(list);
	plist_print_list(list);
	
	printf("\n | STRESS TEST |\n");
	int i = 0;
	int n = 1000;
	int vals[n];
	for(; i < n; i++)
		vals[i] = plist_insert(list, plist_form_process_info(i+1, i == 0? 0:vals[i-1]));

	plist_print_list(list);
	printf("\n");
	for(i = 0; i <= n; i++)
		plist_remove(list, vals[i] , i+1);
	printf("\n");
	plist_free(list);
	plist_print_list(list);
	return 0;
}