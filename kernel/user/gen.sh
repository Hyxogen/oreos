#!/bin/bash

printf "/* do not edit, automatically generated */\n"

printf "#ifndef __SYSCALL_NR_H\n"
printf "#define __SYSCALL_NR_H\n"

while IFS=$'\n' read -r line; do
	IFS=$' '
	printf "#define SYS_%s %s\n" $line
done

printf "#endif"
