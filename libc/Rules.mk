sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= $(d)/assert.o $(d)/ctype.o $(d)/stdlib.o $(d)/string.o $(d)/strings.o
DEPS_$(d)	:= $(OBJS_$(d):%=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
