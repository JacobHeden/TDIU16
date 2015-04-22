#include <stddef.h>

#include "plist.h"

//Initierar mapen används i threads/thread.c
void proc_map_init(struct proc_map* m)
{
  unsigned i = 0;
  for(; i < MAP_SIZE; ++i)
    m->content[i] = NULL; //Nollställer
}
//Söker i mapen med hjälp av nyckeln
table_value proc_map_find(struct proc_map* m, pid k)
{
  if(k > 0 && k < MAP_SIZE)
    return m->content[k]; //Är det tomot på den postionen retuneras NULL från att init initerat mapen. 
  else
    return NULL;
}

//Insert
int proc_map_insert(struct proc_map* m, table_value v)
{
  unsigned i = 0; 
  while(m->content[i] != NULL && i < MAP_SIZE) //Hitta första tomma platsen
    ++i; 

  if(i == MAP_SIZE)
    return MAP_SIZE; //Om mappen är full 
 
  v->proc_id = i; 
  m->content[i] = v;
  return i; // Index där det läggs in 
}
//Kan var en del overhead vill man ha mindre måste pid läggas i thread 
table_value proc_map_remove(struct proc_map* m, pid k)
{
  table_value return_value = NULL;
 
      if(m->content[k] != NULL)
	{
	  return_value = m->content[k];
	  m->content[k] = NULL; // Tar bort nyckeln. 
	  return return_value;
	}
    


 return NULL;
}

void proc_map_for_each(struct proc_map* m,
		  void (*exec)(int k, table_value v, int aux),
		  int aux)
{
  unsigned i = 0;
  for(; i < MAP_SIZE; ++i)
    {
      if(m->content[i] != NULL)
	exec(i, m->content[i], aux); 
    }
}

void proc_map_remove_if(struct proc_map* m,
		   bool (*cond)(int k, table_value v, int aux),
		   int aux)
{
  unsigned i = 0;
  for(; i < MAP_SIZE; ++i)
    {
      if(cond(i, m->content[i], aux))
	 m->content[i] = NULL; //Om aux är process id. dvs current_tid. 
    }
}


