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

//check if shared page exists, if not kalloc() a new one
//might need to use memset()
int i;
int flag = 0;
uint sz = PGROUNDUP(myproc()->sz); //or use SHM_BASE + PGSIZE*i
for(i = 0; i < 64; i++){
  if(shm_table.shm_pages[i].id == id){
    mappages(myproc()->pgdir, (char*)(sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
    shm_table.shm_pages[i].refcnt++;
    *pointer=(char *)(sz);
    //myproc()->sz = PGROUNDUP(myproc()->sz);
    flag = 1;
    break;
  }
}

if(flag == 0){
  for(i = 0; i < 64; i++){
    if(shm_table.shm_pages[i].id == 0 && shm_table.shm_pages[i].frame == 0 && shm_table.shm_pages[i].refcnt == 0){ //finds an empty page
      shm_table.shm_pages[i].id = id;
      shm_table.shm_pages[i].frame = kalloc();
      memset(shm_table.shm_pages[i].frame, 0, PGSIZE);
      mappages(myproc()->pgdir, (char*)(sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
      shm_table.shm_pages[i].refcnt = 1;
      *pointer=(char *)(sz);
      //myproc()->sz = PGROUNDUP(myproc()->sz);
    break;
  }
  }
}

//release lock
release(&(shm_table.lock));

return 0;
}


int shm_close(int id) {

acquire(&(shm_table.lock));
int i;
for(i = 0; i < 64; i++){
  if(shm_table.shm_pages[i].id == id){
    //decrement reference count
    shm_table.shm_pages[i].refcnt--;
    //if after decrementing, refcnt = 0, clear page 
    if(shm_table.shm_pages[i].refcnt == 0){
      shm_table.shm_pages[i].id = 0;
      shm_table.shm_pages[i].frame = 0;
    }
    break;
  }
}

release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}
