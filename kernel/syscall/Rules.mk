sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

OBJS_$(d)	:= \
	$(d)/write.o $(d)/read.o $(d)/exit.o $(d)/mmap.o $(d)/fork.o $(d)/sigreturn.o $(d)/signal.o \
	$(d)/getpid.o $(d)/kill.o $(d)/waitpid.o $(d)/alarm.o $(d)/pause.o $(d)/close.o $(d)/socketpair.o \
	$(d)/getuid.o $(d)/sched_yield.o
DEPS_$(d)	:= $(OBJS_$(d):%.o=%.d)
CLEAN		:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

OBJS		+= $(OBJS_$(d))
DEPS		+= $(DEPS_$(d))

-include $(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
