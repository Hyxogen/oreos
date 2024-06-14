sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= $(d)/framebuf.o $(d)/gdt.o $(d)/platform.o $(d)/ps2.o $(d)/boot.o
DEPS_$(d)	:= $(OBJS_$(d):%=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

#TODO generate on make
OBJS_$(d)	+= $(d)/font.o

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

LFLAGS		+= -T $(d)/linker.ld

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
