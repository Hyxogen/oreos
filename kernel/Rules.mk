sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= \
	$(d)/kernel.o $(d)/keycode.o $(d)/printk.o $(d)/ps2.o $(d)/tty.o $(d)/acpi.o $(d)/panic.o $(d)/sched.o \
	$(d)/spinlock.o $(d)/irq.o
DEPS_$(d)	:= $(OBJS_$(d):%.o=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

dir		:= $(d)/arch
include		$(dir)/Rules.mk

dir		:= $(d)/malloc
include		$(dir)/Rules.mk

dir		:= $(d)/libc
include		$(dir)/Rules.mk

dir		:= $(d)/font
include		$(dir)/Rules.mk

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
