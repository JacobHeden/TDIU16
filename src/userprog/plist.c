#include <stddef.h>

#include "plist.h"
#include <stdio.h>
//Initierar mapen används i threads/thread.c
void proc_map_init(struct proc_map* m)
{
  lock_init(&m->proc_lock);

  unsigned i = 0;
  for(; i < MAP_SIZE; ++i)
    m->content[i] = NULL; //Nollställer
}
//Söker i mapen med hjälp av nyckeln
table_value proc_map_find(struct proc_map* m, pid k)
{
  // printf("TEST");
  lock_acquire(&m->proc_lock);
  // printf("TEST");


  if(k > 0 && k < MAP_SIZE)
    {
      lock_release(&m->proc_lock);
      return m->content[k]; //Är det tomot på den postionen retuneras NULL från att init initerat mapen.
    } 
  else
    {
      lock_release(&m->proc_lock);

      return NULL;
    }
    lock_release(&m->proc_lock);

}

//Insert
int proc_map_insert(struct proc_map* m, table_value v)
{
  lock_acquire(&m->proc_lock);
  unsigned i = 0; 
  while(m->content[i] != NULL && i < MAP_SIZE) //Hitta första tomma platsen
    ++i; 

  if(i == MAP_SIZE)
    {
      lock_release(&m->proc_lock);
      return MAP_SIZE; //Om mappen är full 
    }
  v->proc_id = i; 
  m->content[i] = v;
        
  lock_release(&m->proc_lock);
  //printf("PROC ID::::::: %d", i);
  return i; // Index där det läggs in 
}
//Kan var en del overhead vill man ha mindre måste pid läggas i thread 
table_value proc_map_remove(struct proc_map* m, pid k)
{
  lock_acquire(&m->proc_lock);

  table_value return_value = NULL;
 
  if(m->content[k] != NULL)
    {
      return_value = m->content[k];
      free(m->content[k]);
      m->content[k] = NULL; // Tar bort nyckeln. 
      lock_release(&m->proc_lock);

      return return_value;
    }
    

  lock_release(&m->proc_lock);

  return NULL;
}

void proc_map_for_each(struct proc_map* m,
		       void (*exec)(int k, table_value v, int aux),
		       int aux)
{
  lock_acquire(&m->proc_lock);

  unsigned i = 0;
  for(; i < MAP_SIZE; ++i)
    {
      if(m->content[i] != NULL)
	{
	  exec(i, m->content[i], aux); 
	}
    }
  lock_release(&m->proc_lock);

}

void proc_map_remove_if(struct proc_map* m,
			bool (*cond)(int k, table_value v, int aux),
			int aux)
{
  lock_acquire(&m->proc_lock);

  // printf("!!!!!!!!!!!!PROC_MAP_REMOVE_IF !!!!!!!!!!!!!!\n");
  unsigned i = 0;
  for(; i < MAP_SIZE; ++i)
    {
      if(cond(i, m->content[i], aux))
	{
	  free(m->content[i]);
	  m->content[i] = NULL; //Om aux är process id. dvs current_tid. 
	}
	
    }
  lock_release(&m->proc_lock);

}

//UNUSED så att man inte får waring unused parameter!!!!! !!! ! !! 
void print_element(int i UNUSED, table_value v, int aux UNUSED)
{
  //MAGIC !!! !! !  !!  !! \t\t\t
   printf("%i\t\t  %i\t\t   %s\t %s\t\t %i \n",
	  v->proc_id,
	  v->parent_id,
	  v->alive ? "Alive" : "Dead\t",
	  v->parent_alive ? "Alive" : "Dead",
	  v->exit_status);

}

