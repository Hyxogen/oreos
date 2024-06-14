sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

dir		:= $(d)/i386
include		$(dir)/Rules.mk

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
