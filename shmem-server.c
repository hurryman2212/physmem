#include <stdio.h>

#include <fcntl.h>
#include <signal.h>

#include <sys/mman.h>

#include <x86linux/helper.h>

void sigint_handler(__attribute__((unused)) int sig) { exit(EXIT_SUCCESS); }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s DEC_SIZE\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  size_t size = strtoul(argv[1], NULL, 10);

  log_init(NULL, 0, -1, 1, 0);
  log_enable(LOG_DEBUG);

  size_t pagesize = getpagesize();
  log_abort_if_false(!(size % pagesize));

  int memfd;
  log_abort_on_error(memfd = memfd_create("shmem-server", 0));
  log_abort_on_error(ftruncate64(memfd, size));
  void *virt_shmem;
  log_abort_on_error(virt_shmem = mmap64(NULL, size, PROT_READ | PROT_WRITE,
                                         MAP_SHARED | MAP_POPULATE, memfd, 0));

  memset(virt_shmem, 0, size);
  barrier();

  log_warning("[virt_start: %p] Start sleeping now...", virt_shmem);

  log_abort_on_error(signal(SIGINT, sigint_handler));
  while (1)
    sleep(1);

  return EXIT_FAILURE;
}
