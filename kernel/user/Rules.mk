sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= $(d)/init.o

DEPS_$(d)	:= $(OBJS_$(d):%.o=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

USER_CFLAGS	:= -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -masm=intel -g -Iwhot
USER_LFLAGS	:= -ffreestanding -nostdlib -lgcc

USER_OBJS	:= $(d)/shell.o $(d)/util.o $(d)/syscall.o $(d)/signal.o $(d)/printf.o
USER_LINKSCRIPT	:= $(d)/linker.ld

USER_OBJS	:= $(addprefix $(OBJ_DIR)/, $(USER_OBJS))

LFLAGS		+= 

$(OBJ_DIR)/$(d)/init.o: $(OBJ_DIR)/$(d)/init.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/$(d)/%.o: $(OBJ_DIR)/$(d)/%.c $(d)/Rules.mk
	@mkdir -p $(@D)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(OBJ_DIR)/$(d)/%.o: $(OBJ_DIR)/$(d)/%.asm $(d)/Rules.mk
	@mkdir -p $(@D)
	$(AS) -felf32 -g $< -o $@

$(OBJ_DIR)/$(d)/init.out: $(USER_OBJS) $(USER_LINKSCRIPT) $(d)/Rules.mk
	$(LD_CMD) $(USER_OBJS) -T $(USER_LINKSCRIPT) -o $@ $(USER_LFLAGS)
	cp $@ init-syms.out
	strip $@

$(OBJ_DIR)/$(d)/init.c: $(OBJ_DIR)/$(d)/init.out
	xxd -include $< | \
		sed -E 's/unsigned char(.*)\[\]/unsigned char user_init[]/g' | \
		sed -E 's/unsigned int (.*)_len/unsigned int user_init_len/g' \
		> $@

#$(OBJ_DIR)/$(d)/font.o: $(d)/font.psfu
#	objcopy -O elf32-i386 -B i386 -I binary $< $@

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
