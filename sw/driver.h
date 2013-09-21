#ifndef _DRIVER_H
#define _DRIVER_H

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>

class Driver
{
 public:
  Driver(uintptr_t paddr)
  {
    this->paddr = paddr;
    uintptr_t pgsize = sysconf(_SC_PAGESIZE);
    uintptr_t regsize = regs * sizeof(uintptr_t);
    uintptr_t memsize = (regsize-1)/pgsize*pgsize+1;

    /*printf("opening interface at %p\n", (void*)paddr);*/

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    assert(fd != -1);

    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_SHARED;
    vaddr = (uintptr_t)mmap(0, memsize, prot, flags, fd, paddr & ~(pgsize-1));
    assert((void*)vaddr != MAP_FAILED);
    vaddr = vaddr + (paddr & (pgsize-1));
  }

  void reset()
  {
    *(volatile uintptr_t*)(vaddr + 0x100) = 0xA;
  }

  uintptr_t read(int port)
  {
    return *reg_addr(port);
  }

  void write(int port, uintptr_t value)
  {
    *reg_addr(port) = value;
  }

  int num_ports() {
    return regs;
  }

 private:
  uintptr_t vaddr;
  uintptr_t paddr;
  static const int regs = 8;

  volatile uintptr_t* reg_addr(int port)
  {
    return (volatile uintptr_t*)vaddr + (regs - port - 1);
  }
};

#endif
