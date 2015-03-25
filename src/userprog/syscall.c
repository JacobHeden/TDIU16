#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);
int s_write(int32_t* esp);
int s_read(int32_t* esp);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:
   
   int sys_read_arg_count = argc[ SYS_READ ];
   
   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1, 
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t* esp = (int32_t*)f->esp;
  
  switch (*esp)
  {
  case SYS_HALT : 
    power_off();
    break; 
  case SYS_EXIT : ;
    char *name = (char*)thread_name(); 
    printf("Exit thread: %s: exit status: %i\n",name ,esp[1]);
     thread_exit();
     break;
  case SYS_WRITE :
    f->eax = s_write(esp);
    break;
  case SYS_READ :
    f->eax = s_read(esp);
    break;
  default:
    {
      printf ("Executed an unknown system call!\n");
      
      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);
      
      thread_exit ();
    }
  }
}

int s_write(int32_t* esp)
{  
  int fd = (int*) esp[1];
  char *buffer = (char*) esp[2];
  int size = (int*) esp[3];
  if(fd== STDOUT_FILENO)
    {
      // printf("Line on screen with size: %i",size);
      putbuf(buffer,size);
      return size;
    }
  else
    return -1;
  
}

int s_read(int32_t* esp)
{
   int fd = (int*) esp[1];
  char *buffer = (char*) esp[2];
  int size = (int*) esp[3];
  if(fd== STDIN_FILENO)
    {
      int count = 0;
      for(count;count <= size;count++)
	{
	  uint8_t key = input_getc();
	  char * p_key;
	  if(key == 13)
	    {
	      buffer[count] = '\n';
	      p_key = '\n';
	    }
	  else
	    {
	      buffer[count]= key;
	      p_key = key;
	    }
	  putbuf(buffer,1); //Se whats pressed writes doubles so not so nice
	}
      return size;
    }
  else 
    return -1;

}
