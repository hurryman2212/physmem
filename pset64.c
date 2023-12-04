#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>

#include <x86linux/helper.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s HEX_PHYS HEX_VAL\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  loff_t phys = strtol(argv[1], NULL, 16);
#ifdef _SET8
  uint8_t val = strtol(argv[2], NULL, 16);
#elif defined(_SET16)
  uint16_t val = strtol(argv[2], NULL, 16);
#elif defined(_SET32)
  uint32_t val = strtol(argv[2], NULL, 16);
#else
  uint64_t val = strtol(argv[2], NULL, 16);
#endif

  log_init(NULL, 0, -1, 1, 0);
  log_enable(LOG_DEBUG);

  int dev_mem_fd;
  log_abort_on_error(dev_mem_fd = open("/dev/mem", O_RDWR | O_ASYNC));
  void *virt_pmem;
  long pagesize = getpagesize();
  loff_t pagemask = ~(pagesize - 1);
  log_abort_on_error(virt_pmem = mmap64(NULL, pagesize, PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_POPULATE, dev_mem_fd,
                                        phys & pagemask));

#ifdef _SET8
  uint8_t *ptr = (uint8_t *)((uintptr_t)virt_pmem + (phys & (pagesize - 1)));
#elif defined(_SET16)
  uint16_t *ptr = (uint16_t *)((uintptr_t)virt_pmem + (phys & (pagesize - 1)));
#elif defined(_SET32)
  uint32_t *ptr = (uint32_t *)((uintptr_t)virt_pmem + (phys & (pagesize - 1)));
#else
  uint64_t *ptr = (uint64_t *)((uintptr_t)virt_pmem + (phys & (pagesize - 1)));
#endif
  *ptr = val;
  barrier();

  return EXIT_SUCCESS;
}
