#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int map_no = is_mmio(addr);
  if (map_no==-1)
  	return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  else
	return mmio_read(addr, len, map_no);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int map_no = is_mmio(addr);
  if (map_no==-1)
  	memcpy(guest_to_host(addr), &data, len);
  else
	mmio_write(addr, len, data, map_no);
}

paddr_t page_translate(vaddr_t addr, int mode) {
  if (cpu.cr0.paging && cpu.cr0.protect_enable) {
		PDE pde, *pgdir;
		PTE pte, *pgtable;

		pgdir = (PDE *)(intptr_t)(cpu.cr3.page_directory_base<<12);
		pde.val = paddr_read((intptr_t)&pgdir[(addr>>22) & 0x3ff], 4);
		assert(pde.present);
		pde.accessed = 1;
		
		pgtable = (PTE *)(intptr_t)(pde.page_frame<<12);
		pte.val = paddr_read((intptr_t)&pgtable[(addr>>12) & 0x3ff], 4);
		assert(pte.present);
		pte.accessed = 1;

		if (mode) { pte.dirty = 1; }

    uint32_t offset = addr & PAGE_MASK;
		paddr_t paddr = (pte.page_frame<<12) | offset;
  	return paddr;
  }
  else { return addr; }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (((addr+len-1)&~PAGE_MASK)!=(addr&~PAGE_MASK)) {
		// assert(0); 
		union {
			uint8_t bytes[4];
			uint32_t dword;
		} data = {0};
		for (int i=0; i<len; i++) {
			paddr_t paddr = page_translate(addr+i, 0);
			data.bytes[i] = paddr_read(paddr, 1);
		}
		return data.dword;
	}
	else {
  	paddr_t paddr = page_translate(addr, 0);
  	return paddr_read(paddr, len);
	}
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (((addr+len-1)&~PAGE_MASK)!=(addr&~PAGE_MASK))  { 
		assert(0); 
	}
	
  paddr_t paddr = page_translate(addr, 1);
  paddr_write(paddr, len, data);
}
