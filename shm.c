#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

#define SHM_BASE KERNBASE - (64 * PGSIZE)

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

//you write this

//acquire lock
acquire(&(shm_table.lock));

int i;
int flag = 0; //flag for if discovered memory segment or not
uint sz = PGROUNDUP(myproc()->sz); //or use SHM_BASE + PGSIZE*i, is the size parameter
for(i = 0; i < 64; i++){
  if(shm_table.shm_pages[i].id == id){ //looks through the table for page that matches the id parameter
    mappages(myproc()->pgdir, (char*)(sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U); //maps physical address to virtual address
    shm_table.shm_pages[i].refcnt++; //increments refcnt as another process is mapped to page
    *pointer=(char *)(sz); //sets pointer to virtual address
    flag = 1; //sets flag so another page is not allocated
    break;
  }
}

if(flag == 0){ //goes through this if didnt find a page that matches the id parameter
  for(i = 0; i < 64; i++){
    if(shm_table.shm_pages[i].id == 0 && shm_table.shm_pages[i].frame == 0 && shm_table.shm_pages[i].refcnt == 0){ //finds first empty page in table
      shm_table.shm_pages[i].id = id; //sets id to id parameter
      shm_table.shm_pages[i].frame = kalloc(); //allocates page and stores address in frame
      memset(shm_table.shm_pages[i].frame, 0, PGSIZE); //initializes page memory
      mappages(myproc()->pgdir, (char*)(sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U); //maps physical address to virtual address
      shm_table.shm_pages[i].refcnt = 1; //increments refcnt as another process is mapped to page
      *pointer=(char *)(sz); //sets pointer to virtual address
    break;
  }
  }
}

//release lock
release(&(shm_table.lock));

return 0;
}


int shm_close(int id) {

//acquire lock
acquire(&(shm_table.lock));
int i;
for(i = 0; i < 64; i++){
  if(shm_table.shm_pages[i].id == id){ //looks through table for page with id of id parameter
    shm_table.shm_pages[i].refcnt--; //decrements reference count
    if(shm_table.shm_pages[i].refcnt == 0){ //if after decrementing, refcnt = 0, clear page 
      shm_table.shm_pages[i].id = 0;
      shm_table.shm_pages[i].frame = 0;
    }
    break;
  }
}

//release lock
release(&(shm_table.lock));

return 0;
}
