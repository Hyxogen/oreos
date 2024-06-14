#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H

#include <stddef.h>
#include <lib/error.h>

void *memccpy(void *restrict, const void *restrict, int, size_t);
void *mempcpy(void *restrict dest, const void *restrict src, size_t n);
void *memchr(const void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memcpy(void *restrict, const void *restrict, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
char *stpcpy(char *restrict, const char *restrict);
char *stpncpy(char *restrict, const char *restrict, size_t);
char *strcat(char *restrict, const char *restrict);
char *strchr(const char *, int);
int strcmp(const char *, const char *);
int strcoll(const char *, const char *);
char *stpcpy(char *restrict dst, const char *restrict src);
char *strcpy(char *restrict, const char *restrict);
size_t strcspn(const char *, const char *);
char *strdup(const char *);
char *strndup(const char *, size_t);
char *strerror(int);
int strerror_r(int, char *, size_t);
size_t strlen(const char *);
char *strncat(char *restrict, const char *restrict, size_t);
int strncmp(const char *, const char *, size_t);
char *strncpy(char *restrict, const char *restrict, size_t);
size_t strnlen(const char *, size_t);
char *strpbrk(const char *, const char *);
char *strrchr(const char *, int);
char *strsignal(int);
size_t strspn(const char *, const char *);
char *strstr(const char *, const char *);
char *strtok(char *restrict, const char *restrict);
char *strtok_r(char *restrict, const char *restrict, char **restrict);
size_t strxfrm(char *restrict, const char *restrict, size_t);

#endif
