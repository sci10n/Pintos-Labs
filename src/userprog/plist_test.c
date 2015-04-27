#include "plist_h.h"
#include <stdio.h>
#include <stdlib.h>
int main(){
	
	process_list* list = plist_allocate_list_entry(plist_form_process_info(0,0));
	plist_insert(list, plist_form_process_info(1,0));
	plist_insert(list, plist_form_process_info(2,1));
	plist_insert(list, plist_form_process_info(3,1));
	plist_insert(list, plist_form_process_info(4,2));
	plist_insert(list, plist_form_process_info(5,2));
	plist_insert(list, plist_form_process_info(6,5));
	plist_print_list(list);

	printf("\n");
	plist_remove(list, 5, -1);
	plist_print_list(list);
	plist_free(&list);
	printf("\n");
	plist_print_list(list);
	return 0;
}