#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while (*s++)
    len++;
  return len;
}

char *strcpy(char *dst, const char *src) {
  for (; *src; dst++, src++)
    *dst = *src;
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  for (size_t i = 0; *src && i < n; i++, dst++, src++)
    *dst = *src;
  return dst;
}

char *strcat(char *dst, const char *src) {
  while (*dst)
    dst++;
  for (; *src; dst++, src++)
    *dst = *src;
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  for (; *s1 && *s2; s1++, s2++)
    if (*s1 != *s2)
      return *s1 - *s2;
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (size_t i = 0; *s1 && *s2 && i < n; i++, s1++, s2++)
    if (*s1 != *s2)
      return *s1 - *s2;
  return *s1 - *s2;
}

void *memset(void *s, int c, size_t n) {
  for (size_t i = 0; i < n; i++)
    ((char *)s)[i] = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  if (src < dst && (char *)dst < (char *)src + n)
  {
    for (size_t i = n - 1; i >= 0; i--)
      i[(char *)dst] = i[(char *)src];
  }
  else
    memcpy(dst, src, n);
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  for (size_t i = 0; i < n; i++)
    i[(char *)out] = i[(char *)in];
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  for (size_t i = 0; i < n; i++)
    if (*(char *)s1 != *(char *)s2)
      return *(char *)s1 - *(char *)s2;
  return *(char *)s1 - *(char *)s2;
}

#endif
