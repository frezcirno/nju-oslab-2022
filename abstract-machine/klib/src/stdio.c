#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char *convert(char *buf, unsigned int num, int base)
{
  const char *digits = "0123456789ABCDEF";
  char *ptr;

  ptr = &buf[49];
  *ptr = '\0';

  do {
      *--ptr = digits[num % base];
      num /= base;
  } while (num);

  return ptr;
}

#define PARSE(fmt, ap, collect)      \
  unsigned int __i;                  \
  char __buf[50];                    \
  char *__s;                         \
                                     \
  while (*fmt)                       \
  {                                  \
    while (*fmt && *fmt != '%')      \
    {                                \
      collect(*fmt);                 \
      fmt++;                         \
    }                                \
    if (!*fmt)                       \
      break;                         \
    fmt++;                           \
    switch (*fmt)                    \
    {                                \
    case 'c':                        \
      __i = va_arg(ap, int);         \
      collect(__i);                  \
      break;                         \
                                     \
    case 'd':                        \
      __i = va_arg(ap, int);         \
      if (__i < 0)                   \
      {                              \
        __i = -__i;                  \
        collect('-');                \
      }                              \
      __s = convert(__buf, __i, 10); \
      putstr(__s);                   \
      break;                         \
                                     \
    case 'o':                        \
      __i = va_arg(ap, int);         \
      if (__i < 0)                   \
      {                              \
        __i = -__i;                  \
        collect('-');                \
      }                              \
      __s = convert(__buf, __i, 8);  \
      putstr(__s);                   \
      break;                         \
                                     \
    case 'x':                        \
      __i = va_arg(ap, int);         \
      if (__i < 0)                   \
      {                              \
        __i = -__i;                  \
        collect('-');                \
      }                              \
      __s = convert(__buf, __i, 16); \
      putstr(__s);                   \
      break;                         \
                                     \
    case 's':                        \
      __s = va_arg(ap, char *);      \
      putstr(__s);                   \
      break;                         \
                                     \
    default:                         \
      break;                         \
    }                                \
    fmt++;                           \
  }

int vfprintf(const char *fmt, va_list ap)
{
#define collect(char) putch(char)
  PARSE(fmt, ap, collect);
#undef collect
  return 0;
}

int printf(const char *fmt, ...) {
  int res;
  va_list ap;
  va_start(ap, fmt);
  res = vfprintf(fmt, ap);
  va_end(ap);
  return res;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int i = 0;
#define collect(char) ({ i++; *out++ = (char); })
  PARSE(fmt, ap, collect);
#undef collect
  *out = '\0';
  return i;
}

int sprintf(char *out, const char *fmt, ...) {
  int res;
  va_list ap;
  va_start(ap, fmt);
  res = vsprintf(out, fmt, ap);
  va_end(ap);
  return res;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  int res;
  va_list ap;
  va_start(ap, fmt);
  res = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return res;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  size_t i = 0;
#define collect(char) ({ if (i<n) { i++; *out++ = (char); } })
  PARSE(fmt, ap, collect);
#undef collect
  *out = '\0';
  return 0;
}

#endif
