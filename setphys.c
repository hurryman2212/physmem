#include "ptedit_header.h"

#include <x86linux/helper.h>

int main(int argc, char *argv[]) {
  if (argc != 6) {
    fprintf(
        stderr,
        "usage: %s DEC_PID HEX_VIRT_START DEC_SIZE HEX_PHYS_ADDR SHOW_PTE\n",
        argv[0]);
    exit(EXIT_FAILURE);
  }
  const int pid = atoi(argv[1]);
  const uintptr_t virt_start = strtoul(argv[2], NULL, 16);
  const size_t size = strtoul(argv[3], NULL, 10);
  const loff_t phys_addr = strtol(argv[4], NULL, 16);
  const int show_pte = atoi(argv[5]);

  log_init(NULL, 0, -1, 1, 0);
  log_enable(LOG_DEBUG);

  log_abort_on_error(ptedit_init());

  const size_t pagesize = ptedit_get_pagesize();
  log_abort_if_false(!(virt_start % pagesize));
  log_abort_if_false(!(phys_addr % pagesize));
  log_abort_if_false(!(size % pagesize));

  const size_t nr_pages = size / pagesize;
  for (size_t i = 0; i < nr_pages; ++i) {
    const uintptr_t virt_addr = virt_start + pagesize * i;

    ptedit_entry_t vm = ptedit_resolve(address_cast(virt_addr), pid);
    log_abort_if_false(vm.pgd);

    const size_t old_pfn = ptedit_get_pfn(vm.pte);
    const loff_t old_phys_addr = old_pfn * pagesize;
    log_info("OLD [virt: 0x%lx] (pfn: 0x%lx) phys: 0x%lx", virt_addr, old_pfn,
             old_phys_addr);
    if (show_pte) {
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_SOFTW4));
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT0));
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT1));
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT2));
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT3));
      ptedit_print_entry_t(vm);
    }

    const size_t pfn = phys_addr / pagesize;
    ptedit_pte_set_pfn(address_cast(virt_addr), pid, pfn);

    vm = ptedit_resolve(address_cast(virt_addr), pid);
    log_abort_if_false(vm.pgd);

    const size_t updated_pfn = ptedit_get_pfn(vm.pte);
    const loff_t updated_phys_addr = updated_pfn * pagesize;
    log_info("NEW [virt: 0x%lx] (pfn: 0x%lx) phys: 0x%lx", virt_addr,
             updated_pfn, updated_phys_addr);
    if (show_pte) {
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_SOFTW4));
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT0));
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT1));
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT2));
      log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT3));
      ptedit_print_entry_t(vm);
    }
  }
  ptedit_full_serializing_barrier();

  ptedit_cleanup();

  return EXIT_SUCCESS;
}
