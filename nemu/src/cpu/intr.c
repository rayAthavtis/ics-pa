#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  // TODO();
  rtl_push((rtlreg_t *)&cpu.eflags);
  rtl_push((rtlreg_t *)&cpu.cs);
  rtl_push((rtlreg_t *)&ret_addr);

  uint32_t base = cpu.idtr.base;

  uint32_t low = vaddr_read(base+NO*8, 4) & 0xffff;
  uint32_t high = vaddr_read(base+NO*8+4, 4) & 0xffff0000;
  decoding.jmp_eip = low | high;
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
}
