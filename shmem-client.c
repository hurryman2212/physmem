#include <stdio.h>

#include <fcntl.h>
#include <signal.h>

#include <sys/mman.h>

#include <x86linux/helper.h>

void sigint_handler(__attribute__((unused)) int sig) { exit(EXIT_SUCCESS); }

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s HEX_PHYS_START DEC_SIZE\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  loff_t phys_start = strtol(argv[1], NULL, 16);
  size_t size = strtoul(argv[2], NULL, 10);

  log_init(NULL, 0, -1, 1, 0);
  log_enable(LOG_DEBUG);

  size_t pagesize = getpagesize();
  log_abort_if_false(!(size % pagesize));

  int dev_mem_fd;
  log_abort_on_error(dev_mem_fd = open("/dev/mem", O_RDWR | O_ASYNC));
  void *virt_pmem;
  log_abort_on_error(virt_pmem = mmap64(NULL, size, PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_POPULATE, dev_mem_fd,
                                        phys_start));

  size_t nr_pages = size / pagesize;
  log_warning("Start monitoring now...");
  log_abort_on_error(signal(SIGINT, sigint_handler));
  while (1) {
    for (size_t pg_off = 0; pg_off < nr_pages; ++pg_off) {
      for (size_t cl = 0; cl < pagesize / 64; ++cl) {
        unsigned char *cptr =
            (unsigned char *)((uintptr_t)virt_pmem + (pg_off * pagesize) +
                              (cl * 64));
        printf("[virt: %p]", cptr);
        /* Print each L1$D line (64B). */
        for (size_t c = 0; c < 64; ++c) {
          printf(" 0x%02hhX", *(cptr + c));
        }
        printf("\n");
      }
    }
    printf("\n");
    sleep(1);
  }

  return EXIT_FAILURE;
}
