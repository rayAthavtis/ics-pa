#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
	PDE *pde, *pgdir = p->ptr;
	PTE *pgtable;
	pde = &pgdir[PDX(va)];
	if (*pde &PTE_P) { pgtable = (PTE *)PTE_ADDR(*pde); }
	else {
		pgtable = (PTE *)palloc_f();
		for (int i=0; i<NR_PTE; i++) {
			pgtable[i] = 0;
		}
		*pde = PTE_ADDR(pgtable) | PTE_P;
	}
	pgtable[PTX(va)] = PTE_ADDR(pa) | PTE_P;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  uint32_t *stack = ustack.end - 4;
  for (int i=0; i<3; i++) {
    *stack = 0x0;
    stack--;
  }

  struct { _RegSet *tf; } *pcb = ustack.start;
  pcb->tf = (void *)(stack - sizeof(_RegSet));
  pcb->tf->eflags = 0x202;
  pcb->tf->cs = 0x8;
  pcb->tf->eip = (uint32_t)entry;
  
  return pcb->tf;
  // return NULL;
}
