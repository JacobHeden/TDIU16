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
#include "userprog/flist.h" // map

static void syscall_handler (struct intr_frame *);
int s_write(int32_t* esp);
int s_read(int32_t* esp);
//File open create etc read and wirte to a bit. 
int s_open(int32_t* esp);
bool s_create(int32_t* esp);
bool s_remove(int32_t* esp);
void s_close(int32_t* esp);
void s_seek(int fd, unsigned position);
unsigned s_tell (int fd);
int s_filesize(int fd);
struct map* get_filemap(void);

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
       s_close(esp); 
      thread_exit();
      break;
    case SYS_WRITE :
      f->eax = s_write(esp);
      break;
    case SYS_READ :
      f->eax = s_read(esp);
      break;
    case SYS_OPEN:
      f->eax = s_open(esp);
      break;

    case SYS_CREATE:
      f->eax = s_create(esp);
      break;
	
    case SYS_REMOVE:
      {
	if((char*)esp[1] != NULL)
	  f->eax = s_remove(esp);
	else
	  f->eax = 0;
	break;
      }
    case SYS_CLOSE:
      {
	s_close(esp); 
	break;
      }
    case SYS_SEEK:
      {
	s_seek(esp[1], esp[2]);
	break;
      }
    case SYS_TELL:
      {
	f->eax = s_tell(esp[1]);
	break;
      }
    case SYS_FILESIZE:
      {
	f->eax = s_filesize(esp[1]);	
	break;
      }
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
  if (fd == STDIN_FILENO) // Tangentbordet 
    {
      return -1;
    }
  if(fd== STDOUT_FILENO)
    {
      // printf("Line on screen with size: %i",size);
      putbuf(buffer,size);
      return size;
    }
  else
    {
      // Skriva till fil
      struct file* write_to = map_find(get_filemap(), fd);
      if(write_to != NULL)
	{
	  // Det som finns i buffer hamnar i filen. 
	  // Om filen är för liten  tecken < size 
	  return file_write(write_to, buffer, size);
	}
      else 
	{	

	  return -1; // Filen existerar inte eller att vi inte skriver några tecken.
	}
    }
  
}
//Fråga om Read :))))
int s_read(int32_t* esp)
{
  int fd = (int*) esp[1];
  char *buffer = (char*) esp[2];
  int size = (int*) esp[3];
  int test; //för att använda putchar. 
  if (fd == STDOUT_FILENO)
    return -1; // Vi kan inte läsa från skärmen  

  int count;
  for(count=0;count < size;count++)
    {
      if(fd== STDIN_FILENO)
	{
	  // printf("size: %i \n ",count); Debugg
	  uint8_t key = input_getc();
	  if(key == 13)
	    {
	      buffer[count] = '\n';
	    }
	  else  

	    {
	      buffer[count]= key;
	    }
	  //	  putbuf(buffer,1); //Se whats pressed writes doubles so not so nice
	  test = putchar(key); //Se whats pressed faster!!!!.
	}
      else
	{
	  //Read from file
	  struct file* read_from = map_find(get_filemap(), fd);
	  if(read_from != NULL && size > 0)
	    return file_read(read_from, buffer, size);
	  else
	    return -1;
	}
    }
  return size;

}


int s_open(int32_t* esp)
{
  char* f_name = (char*) esp[1];
  struct file* temp = filesys_open(f_name);
  if(temp != NULL)
    {
      // Mata in pekaren till filen i mapen
      // filemap finns initierad i thread.h/c
      unsigned int t = map_insert(get_filemap(), temp); // returnerar värdet i mapen
      if (t == MAP_SIZE){
	filesys_close(temp);
	return -1;
      }
      else
	return t;
    }
  return -1; // filen fanns inte
}


bool s_create(int32_t* esp)
{
  char* name = (char*) esp[1];
  unsigned size = (unsigned) esp[2];
  if (filesys_create(name, size) && size >= 0) // Om det går att skapa en fil 
    {
      map_insert(get_filemap(), filesys_open(name));
      return true;
    }
  return false;
}

bool s_remove(int32_t* esp)
{
  char* file = (char*) esp[1];
  return filesys_remove(file);
}

void s_close(int32_t* esp)
{
  int fd = (int*) esp[1];
  struct file* close_file = map_find(get_filemap(), fd);
  if(close_file != NULL)
    {
      filesys_close(close_file);
      map_remove(get_filemap(), fd); //Viktigt att det tas bort från mapen. 
    }
}

void s_seek(int fd, unsigned position)
{
  struct file* file = map_find(get_filemap(), fd);
  if (file != NULL && position <= (unsigned)s_filesize(fd))
    file_seek(file, position);
}

unsigned s_tell (int fd)
{
  struct file* file = map_find(get_filemap(), fd);
  if (file != NULL)
    return file_tell(file);
  else
    return -1;
}


int s_filesize(int fd)
{
  struct file* file = map_find(get_filemap(), fd);
  if (file != NULL)
    return file_length(file);
  else
    return -1;
}

struct map* get_filemap()
{
  return &thread_current()->filemap; //.node;
}
