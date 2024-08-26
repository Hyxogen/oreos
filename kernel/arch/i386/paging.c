#include <boot/multiboot2.h>
#include <kernel/align.h>
#include <kernel/arch/i386/mmu.h>
#include <lib/string.h>

#include <kernel/debug.h>

#define VADDR_TO_PADDR(vaddr) \
	((void *)((uintptr_t)(vaddr) - (uintptr_t) & _kernel_addr))

// TODO rename ".multiboot.text" to something like ".bootstrap"
// These functions their relocation addresses are the their physical load
// addresses
#define BOOTSTRAP_CODE __attribute__((section(".multiboot.text")))
#define BOOTSTRAP_DATA __attribute__((section(".multiboot.data")))

static struct mmu_pde _page_dir[1024] __attribute__((aligned(MMU_PAGESIZE)));
static struct mmu_pte _page_table1[1024] __attribute__((aligned(MMU_PAGESIZE)));
BOOTSTRAP_DATA struct mb2_info *_multiboot_info = NULL;


BOOTSTRAP_CODE __attribute__((noreturn)) void early_panic(void)
{
	while (1)
		continue;
}

BOOTSTRAP_CODE void setup_early_pagetables(void)
{
	volatile struct mmu_pde *_phys_page_dir = VADDR_TO_PADDR(_page_dir);
	volatile struct mmu_pte *_phys_page_table1 = VADDR_TO_PADDR(_page_table1);
	
	size_t size = PTR_ALIGN_UP(&_kernel_vend, MMU_PAGESIZE) - PTR_ALIGN_DOWN(&_kernel_vstart, MMU_PAGESIZE);
	size_t pages = size / MMU_PAGESIZE;

	size_t pte_off = MMU_PTE_IDX(&_kernel_pstart);

	//TODO map rodata als readonly
	for (size_t i = 0, off = 0; i < pages; i++, off += MMU_PAGESIZE) {
		volatile struct mmu_pte *pte = &_phys_page_table1[pte_off + i];

		pte->pfn = MMU_PADDR_TO_PFN(&_kernel_pstart + off);
		pte->present = true;
		pte->rw = true;
	}

	// identity mapping
	_phys_page_dir[0].pfn = MMU_PADDR_TO_PFN(_phys_page_table1);
	_phys_page_dir[0].present = true;
	_phys_page_dir[0].rw = true;

	//TODO remove 1024 magic number

	// setup higher half kernel mapping
	size_t higher_mem = (uintptr_t)&_kernel_addr / (MMU_PAGESIZE * 1024);
	_phys_page_dir[higher_mem].pfn = MMU_PADDR_TO_PFN(_phys_page_table1);
	_phys_page_dir[higher_mem].present = true;
	_phys_page_dir[higher_mem].rw = true;

	// setup recursive pagetable
	_phys_page_dir[1023].pfn = MMU_PADDR_TO_PFN(_phys_page_dir);
	_phys_page_dir[1023].present = true;
	_phys_page_dir[1023].rw = true;
}

BOOTSTRAP_CODE void check_size(struct mb2_info *info)
{
	size_t size = &_kernel_vend - &_kernel_vstart;
	if (size + info->total_size > 1024 * MMU_PAGESIZE) {
		// kernel is too large to load into memory
		early_panic();
	}
}

BOOTSTRAP_CODE void setup_multiboot_pagetables(struct mb2_info *info)
{
	size_t page_count =
	    (PTR_ALIGN_UP((u8 *)info + info->total_size, MMU_PAGESIZE) -
	    PTR_ALIGN_DOWN((u8 *)info, MMU_PAGESIZE)) / MMU_PAGESIZE;

	volatile struct mmu_pte *_phys_page_table1 = VADDR_TO_PADDR(_page_table1);

	void *page_addr = PTR_ALIGN_UP(&_kernel_vend, MMU_PAGESIZE);
	size_t pte_idx = MMU_PTE_IDX(page_addr);
	size_t pfn = MMU_PADDR_TO_PFN(info);

	for (size_t i = 0; i < page_count; i++) {
		_phys_page_table1[pte_idx + i].pfn = pfn + i;
		_phys_page_table1[pte_idx + i].present = true;
		_phys_page_table1[pte_idx + i].rw = true;
	}

	size_t off = (uintptr_t)info & 0xfff;
	_multiboot_info = (struct mb2_info *)((u8 *)page_addr + off);
}

BOOTSTRAP_CODE void load_pagedir(void)
{
	uintptr_t paddr = (uintptr_t) VADDR_TO_PADDR(_page_dir);
	__asm__ volatile("mov %%cr3, %0" : : "r"(paddr) : "memory");
}

BOOTSTRAP_CODE __attribute__((noreturn)) void enable_paging_and_jump_to_kmain(void);

BOOTSTRAP_CODE void setup_paging(struct mb2_info *info)
{
	check_size(info);
	setup_early_pagetables();
	setup_multiboot_pagetables(info);
	BOCHS_BREAK;
	load_pagedir();

	enable_paging_and_jump_to_kmain();
}

void init_paging(void)
{
	// remove identity mapping
	memset(&_page_dir[0], 0, sizeof(_page_dir[0]));
	mmu_flush_tlb();
}
