#ifndef _PLIST_H_
#define _PLIST_H_


/* Place functions to handle a running process here (process list).
   
   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.
     
   - A function that print the entire content of the list in a nice,
     clean, readable format.
     
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
typedef  int pid;
typedef struct table_ele* table_value;
#define MAP_SIZE 128

//Elementen som ska läggas till i proc tabelen. 
struct table_ele
{
  int proc_id;
  int parent_id;
  int exit_status;
  bool alive;
  bool parent_avlive;
  // char* name; // Kan behövas du de vill att man skriver ut name
};



struct proc_map
{
table_value content[MAP_SIZE];
};


void proc_map_init(struct proc_map* m);
table_value proc_map_find(struct proc_map* m, pid k);
int proc_map_insert(struct proc_map* m, table_value v);
table_value proc_map_remove(struct proc_map* m, pid k);

void proc_map_for_each(struct proc_map* m,
void (*exec)(int k, table_value v, int aux),
int aux);

void proc_map_remove_if(struct proc_map* m,
bool (*cond)(int k, table_value v, int aux),
int aux);


#endif


