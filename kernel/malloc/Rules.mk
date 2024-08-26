sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= $(d)/aligned_alloc.o $(d)/arena.o $(d)/calloc.o $(d)/chunk.o $(d)/common.o $(d)/debug.o \
	$(d)/extensions.o $(d)/free.o $(d)/libk.o $(d)/malloc.o $(d)/opts.o $(d)/oreos.o $(d)/pthread.o \
	$(d)/realloc.o $(d)/state.o
DEPS_$(d)	:= $(OBJS_$(d):%.o=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

#LFLAGS		+= -T $(d)/linker.ld

#$(OBJ_DIR)/$(d)/font.o: $(d)/font.psfu
#	objcopy -O elf32-i386 -B i386 -I binary $< $@

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
