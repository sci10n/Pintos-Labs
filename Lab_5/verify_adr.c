#include <stdlib.h>
#include "pagedir.h"
#include "thread.h"


/* verfy_*_lenght �r t�nkta att anv�ndas i systemanrop som f�r in
 * op�litliga adresser fr�n user mode. Operativsystemet ska inte kunna
 * luras att � processens v�gnar anv�nda adresser processen inte har
 * tillg�ng till. I pagedir.h finns funktioner som kan hj�lpa dig sl�
 * upp adresser i pagetable. Fundra hur du kan g�ra s� f� slagningar
 * som m�jligt.
 *
 * Rekommenderat kompileringskommando:
 *
 *  gcc -Wall -Wextra -std=gnu99 -pedantic -g pagedir.o verify_adr.c
 */
// #error Read comment above and then remove this line.

/* Kontrollera alla adresser fr�n och med start till och inte med
 * (start+length). */
bool verify_fix_length(void* start, int length)
{
  // ADD YOUR CODE HERE
  void* page_start_address = pg_round_down(start);
  int bytes_used = start - page_start_address;
  int number_of_pages = (length + bytes_used) / PGSIZE;

  if((length + bytes_used) % PGSIZE != 0)
    {
      number_of_pages++;
    }

  for(int i = 0; i < number_of_pages; i++)
    {
      if(pagedir_get_page(thread_current()->pagedir, page_start_address) == NULL)
	{
	  return false;
	}

      page_start_address += PGSIZE;
    }
  return true;
}

/* Kontrollera alla adresser fr�n och med start till och med den
 * adress som f�rst inneh�ller ett noll-tecken, `\0'. (C-str�ngar
 * lagras p� detta s�tt.) */
bool verify_variable_length(char* start)
{
  // ADD YOUR CODE HERE

  char* address = start;
  void* page_start_address = pg_round_down((const void*) address);
  int bytes_used = (void*) start - page_start_address;
  int length = bytes_used;

  if(pagedir_get_page(thread_current()->pagedir, page_start_address) == NULL)
    {
      return false;
    }

  while(!is_end_of_string(address++))
    {
      
      length++;
      if(length % PGSIZE == 0)
	{
	  length = 1;
	  if(pagedir_get_page(thread_current()->pagedir, (const void*) address + 1) == NULL)
	    {
	      return false;
	    }
	}
    }
  return true;
}

/* Definition av testfall */
struct test_case_t
{
  void* start;
  unsigned length;
};

#define TEST_CASE_COUNT 6

const struct test_case_t test_case[TEST_CASE_COUNT] =
  {
    {(void*)100, 100}, /* one full page */
    {(void*)199, 102},
    {(void*)101, 98},
    {(void*)250, 190},
    {(void*)250, 200},
    {(void*)250, 210}
  };

/* Huvudprogrammet utv�rderar din l�sning. */
int main(int argc, char* argv[])
{
  int i;
  bool result;

  if ( argc == 2 )
    {
      simulator_set_pagefault_time( atoi(argv[1]) );
    }
  thread_init();
  
  
  
  /* Testa algoritmen med ett givet intervall (en buffer). */
  for (i = 0; i < TEST_CASE_COUNT; ++i)
    {
      start_evaluate_algorithm(test_case[i].start, test_case[i].length);
      result = verify_fix_length(test_case[i].start, test_case[i].length);
      evaluate(result);
      end_evaluate_algorithm();
    }
    
  /* Testa algoritmen med en str�ng. */
  for (i = 0; i < TEST_CASE_COUNT; ++i)
    {
      start_evaluate_algorithm(test_case[i].start, test_case[i].length);
      result = verify_variable_length(test_case[i].start);
      evaluate(result);    
      end_evaluate_algorithm();
    }
  return 0;
}
