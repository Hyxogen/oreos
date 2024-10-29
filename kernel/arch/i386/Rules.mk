sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= \
	$(d)/framebuf.o $(d)/gdt.o $(d)/platform.o $(d)/ps2.o $(d)/boot.o $(d)/paging.o $(d)/mmu.o \
	$(d)/idt.o $(d)/isr.o $(d)/io.o $(d)/apic.o $(d)/timer.o $(d)/reload_segments.o $(d)/proc.o \
	$(d)/syscall.o $(d)/serial.o $(d)/uaccess.o $(d)/uaccess_impl.o $(d)/pagefault.o
DEPS_$(d)	:= $(OBJS_$(d):%.o=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

LFLAGS		+= -T $(d)/linker.ld

#$(OBJ_DIR)/$(d)/font.o: $(d)/font.psfu
#	objcopy -O elf32-i386 -B i386 -I binary $< $@

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
