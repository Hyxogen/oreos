sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= $(d)/psf.o $(d)/default_psf.o

DEPS_$(d)	:= $(OBJS_$(d):%.o=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

LFLAGS		+= 

KBD_VER		:= 2.6.4
KBD_SHA256	:= 519f8d087aecca7e0a33cd084bef92c066eb19731666653dcc70c9d71aa40926
CONSOLEFONT	:= lat0-08.psfu
FONT		:= $(VENDOR_DIR)/kbd-$(KBD_VER)/data/consolefonts/$(CONSOLEFONT) 

$(VENDOR_DIR)/kbd-$(KBD_VER):
	mkdir -p $(@D)
	wget --no-clobber -P $(VENDOR_DIR) "https://mirrors.edge.kernel.org/pub/linux/utils/kbd/kbd-$(KBD_VER).tar.xz"
	echo "$(KBD_SHA256) $(VENDOR_DIR)/kbd-$(KBD_VER).tar.xz" > $(VENDOR_DIR)/kbd-sha256sum.txt
	sha256sum --strict --check $(VENDOR_DIR)/kbd-sha256sum.txt
	rm -f kbd-sha256sum.txt
	tar -C $(VENDOR_DIR) -xvf $(VENDOR_DIR)/kbd-$(KBD_VER).tar.xz > /dev/null
	rm -f $(VENDOR_DIR)/kbd-$(KBD_VER).tar.xz
	rm -f $(VENDOR_DIR)/kbd-sha256sum.txt

$(FONT): $(VENDOR_DIR)/kbd-$(KBD_VER)

$(OBJ_DIR)/$(d)/default_psf.c: $(FONT) $(d)/Rules.mk
	xxd -include $< | \
		sed -E 's/unsigned char(.*)\[\]/unsigned char psf2_default_font[]/g' | \
		sed -E 's/unsigned int (.*)_len/unsigned int psf2_default_font_len/g' > $@

#$(OBJ_DIR)/$(d)/font.o: $(d)/font.psfu
#	objcopy -O elf32-i386 -B i386 -I binary $< $@

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
