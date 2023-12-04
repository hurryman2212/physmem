#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>

#include <sys/mman.h>

#include <x86linux/helper.h>

int memvcmp(void *memory, uint8_t val, size_t size) {
  uint8_t *mm = (uint8_t *)memory;
  return (*mm == val) && memcmp(mm, mm + 1, size - 1) == 0;
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr, "usage: %s MMAP_FILE HEX_OFFSET DEC_SIZE HEX_FILL1B\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  const char *mmap_file = argv[1];
  loff_t offset = strtol(argv[2], NULL, 10);
  size_t size = strtoul(argv[3], NULL, 10);
  uint8_t fill1B = (uint8_t)strtoul(argv[4], NULL, 16);

  log_init(NULL, 0, -1, 1, 0);
  log_enable(LOG_DEBUG);

  int dev_mem_fd;
  log_abort_on_error(dev_mem_fd = open(mmap_file, O_RDWR | O_ASYNC));
  void *virt_pmem;
  log_abort_on_error(virt_pmem =
                         mmap64(NULL, size, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_POPULATE, dev_mem_fd, offset));

  memset(virt_pmem, fill1B, size);
  barrier();
  log_abort_if_false(memvcmp(virt_pmem, fill1B, size));

  return EXIT_SUCCESS;
}
