#include "ptedit_header.h"

#include <x86linux/helper.h>

int main(int argc, char *argv[]) {
  if (argc != 6) {
    fprintf(stderr,
            "usage: %s MMAP_FILE DEX_OFFSET_START DEC_OFFSET_END HEX_PHYS_ADDR "
            "SHOW_PTE\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  const char *mmap_file = argv[1];
  loff_t offset_start = strtoul(argv[2], NULL, 10);
  loff_t offset_end = strtoul(argv[3], NULL, 10);
  loff_t phys_addr = strtol(argv[4], NULL, 16);
  int show_pte = atoi(argv[5]);

  log_init(NULL, 0, -1, 1, 0);
  log_enable(LOG_DEBUG);

  log_abort_on_error(ptedit_init());

  size_t pagesize = ptedit_get_pagesize();
  log_abort_if_false(!(offset_start % pagesize));
  log_abort_if_false(!(offset_end % pagesize));
  log_abort_if_false(!(phys_addr % pagesize));

  size_t size = offset_end - offset_start;
  log_abort_if_false(!(size % pagesize));

  int dev_mem_fd;
  log_abort_on_error(dev_mem_fd = open(mmap_file, O_RDWR | O_ASYNC));
  void *virt_pmem;
  log_abort_on_error(virt_pmem = mmap64(NULL, size, PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_POPULATE, dev_mem_fd,
                                        offset_start));

  size_t nr_pages = size / pagesize + 1;
  log_warning("Searching %lu page(s)...", nr_pages);
  for (size_t i = 0; i < nr_pages; ++i) {
    void *virt_addr = (void *)((uintptr_t)virt_pmem + pagesize * i);

    ptedit_entry_t vm = ptedit_resolve(virt_addr, 0);
    log_abort_if_false(vm.pgd);

    size_t found_pfn = ptedit_get_pfn(vm.pte);
    loff_t found_phys_addr = found_pfn * pagesize;
    if (phys_addr == found_phys_addr) {
      log_info("FOUND [virt: %p] (pfn: 0x%lx) phys: 0x%lx", virt_addr,
               found_pfn, found_phys_addr);
      if (show_pte) {
        log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_SOFTW4));
        log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT0));
        log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT1));
        log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT2));
        log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT3));
        ptedit_print_entry_t(vm);
      }
    }
  }

  ptedit_cleanup();

  return EXIT_SUCCESS;
}
