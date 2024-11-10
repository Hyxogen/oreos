sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= \
	$(d)/ps2.o $(d)/qwerty.o $(d)/azerty.o
DEPS_$(d)	:= $(OBJS_$(d):%.o=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
