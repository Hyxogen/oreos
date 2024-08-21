#include <boot/multiboot2.h>

#include <stddef.h>

#include <kernel/kernel.h>

struct mb2_info *mm_init(const struct mb2_info *mb, size_t mb_size);
void fb_init(struct mb2_info *mb);
void gdt_init(void);

// https://f.osdev.org/viewtopic.php?p=230374&sid=99b22aa6f322a817de79fb61778e78c6#p230374
void early_main(struct mb2_info *mb, size_t mb_size)
{
	mb = mm_init(mb, mb_size);
	gdt_init();
	fb_init(mb);
}
