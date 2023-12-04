#include <stdio.h>

#include <fcntl.h>
#include <signal.h>

#include <sys/mman.h>

#include <x86linux/helper.h>

const char *mmap_file;
void sigint_handler(__attribute__((unused)) int sig) {
  log_abort_on_error(unlink(mmap_file));
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s MMAP_FILE DEC_SIZE\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  mmap_file = argv[1];
  size_t size = strtoul(argv[2], NULL, 10);

  log_init(NULL, 0, -1, 1, 0);
  log_enable(LOG_DEBUG);

  size_t pagesize = getpagesize();
  log_abort_if_false(!(size % pagesize));

  int dev_mem_fd;
  log_abort_on_error(
      dev_mem_fd = open(mmap_file, O_CREAT | O_TRUNC | O_RDWR | O_ASYNC, 0600));
  log_abort_on_error(ftruncate64(dev_mem_fd, size));
  void *virt_pmem;
  log_abort_on_error(virt_pmem =
                         mmap64(NULL, size, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_POPULATE, dev_mem_fd, 0));

  memset(virt_pmem, 0, size);
  barrier();

  log_warning("[virt_start: %p] Start sleeping now...", virt_pmem);

  log_abort_on_error(signal(SIGINT, sigint_handler));
  while (1)
    sleep(1);

  return EXIT_FAILURE;
}
