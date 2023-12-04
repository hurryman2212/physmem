#include "ptedit_header.h"

#include <math.h>

#include <x86linux/helper.h>

int main(int argc, char *argv[]) {
  if (argc != 19) {
    fprintf(stderr,
            "usage: %s DEC_PID HEX_VIRT_START DEC_SIZE PTE_NX HEX_PFN PTE_H "
            "PTE_W3 PTE_W2 PTE_W1 PTE_G PTE_S PTE_D PTE_A PTE_UC PTE_WT PTE_U "
            "PTE_W PTE_P\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }
  const int pid = atoi(argv[1]);
  const uintptr_t virt_start = strtoul(argv[2], NULL, 16);
  const size_t size = strtoul(argv[3], NULL, 10);

  const uint64_t pte_nx = atoi(argv[4]);
  const size_t pfn = strtol(argv[5], NULL, 16);
  const uint64_t pte_h = atoi(argv[6]);
  const uint64_t pte_w3 = atoi(argv[7]);
  const uint64_t pte_w2 = atoi(argv[8]);
  const uint64_t pte_w1 = atoi(argv[9]);
  const uint64_t pte_g = atoi(argv[10]);
  const uint64_t pte_s = atoi(argv[11]);
  const uint64_t pte_d = atoi(argv[12]);
  const uint64_t pte_a = atoi(argv[13]);
  const uint64_t pte_uc = atoi(argv[14]);
  const uint64_t pte_wt = atoi(argv[15]);
  const uint64_t pte_u = atoi(argv[16]);
  const uint64_t pte_w = atoi(argv[17]);
  const uint64_t pte_p = atoi(argv[18]);

  log_init(NULL, 0, -1, 1, 0);
  log_enable(LOG_DEBUG);

  log_abort_on_error(ptedit_init());

  const size_t pagesize = ptedit_get_pagesize();
  log_abort_if_false(!(virt_start % pagesize));
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
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_SOFTW4));
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT0));
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT1));
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT2));
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT3));
    ptedit_print_entry_t(vm);

    size_t new_entry = pfn << (int)log2(pagesize);
    pte_nx ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_NX) : 0;
    pte_h ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_PAT_LARGE) : 0;
    pte_w3 ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_SOFTW3) : 0;
    pte_w2 ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_SOFTW2) : 0;
    pte_w1 ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_SOFTW1) : 0;
    pte_g ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_GLOBAL) : 0;
    pte_s ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_PSE) : 0;
    pte_d ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_DIRTY) : 0;
    pte_a ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_ACCESSED) : 0;
    pte_uc ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_PCD) : 0;
    pte_wt ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_PWT) : 0;
    pte_u ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_USER) : 0;
    pte_w ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_RW) : 0;
    pte_p ? x86_set_bit_nonatomic(&new_entry, PTEDIT_PAGE_BIT_PRESENT) : 0;

#ifdef _PGD
    size_t *const entry_ptr = &vm.pgd;
#elif defined(_P4D)
    size_t *const entry_ptr = &vm.p4d;
#elif defined(_PUD)
    size_t *const entry_ptr = &vm.pud;
#elif defined(_PMD)
    size_t *const entry_ptr = &vm.pmd;
#else
    size_t *const entry_ptr = &vm.pte;
#endif
    *entry_ptr = new_entry;
    ptedit_update(address_cast(virt_addr), pid, &vm);

    vm = ptedit_resolve(address_cast(virt_addr), pid);
    log_abort_if_false(vm.pgd);

    const size_t updated_pfn = ptedit_get_pfn(vm.pte);
    const loff_t updated_phys_addr = updated_pfn * pagesize;
    log_info("NEW [virt: 0x%lx] (pfn: 0x%lx) phys: 0x%lx", virt_addr,
             updated_pfn, updated_phys_addr);
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_SOFTW4));
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT0));
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT1));
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT2));
    log_abort_if_false(!x86_test_bit(&vm.pte, PTEDIT_PAGE_BIT_PKEY_BIT3));
    ptedit_print_entry_t(vm);
  }
  ptedit_full_serializing_barrier();

  ptedit_cleanup();

  return EXIT_SUCCESS;
}
