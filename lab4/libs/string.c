#include "string.h"

int strlen(char *src) {
  int i;
  for (i = 0; src[i] != '\0'; i++)
    ;
  return i;
}

void memcpy(uint8_t *dest, uint8_t *src, uint32_t len) {
  for (; len != 0; len--) {
    *dest++ = *src++;
  }
}

void memset(void *dest, uint8_t val, uint32_t len) {
  uint8_t *dst = (uint8_t *)dest;

  for (; len != 0; len--) {
    *dst++ = val;
  }
}

void bzero(void *dest, uint32_t len) { memset(dest, 0, len); }

int strcmp(char *str1, char *str2) {
  int l1 = strlen(str1);
  int l2 = strlen(str2);
  int i;

  for (i = 0; i < l1 || i < l2; i++) {
    if (str1[i] > str2[i]) {
      return 1;
    } else if (str1[i] < str2[i]) {
      return -1;
    }
  }

  return 0;
}

int memcmp(char *str1, char *str2, uint32_t size) {
  int i;

  for (i = 0; i < size; i++) {
    if (str1[i] > str2[i]) {
      return 1;
    } else if (str1[i] < str2[i]) {
      return -1;
    }
  }
  return 0;
}

void strcpy(char *dest, char *src) {
  int l = strlen(src);
  int i;

  for (i = 0; i < l; i++) {
    dest[i] = src[i];
  }

  dest[i] = '\0';
}

uint64_t str_toi(char *str) {
  int base = 10;
  if ((str[0] == '0' && str[1] == 'x') || (str[0] == '0' && str[1] == 'X')) {
    base = 16;
    str += 2;
  }
  uint64_t ret = 0;
  while (*str != '\0') {
    if ('0' <= *str && *str <= '9') {
      ret = ret * base + (*str - '0');
    } else if (base == 16) {
      if ('a' <= *str && *str <= 'f') {
        ret = ret * base + (*str - 'a' + 10);
      } else if ('A' <= *str && *str <= 'F') {
        ret = ret * base + (*str - 'A' + 10);
      } else {
        return 0;
      }
    } else {
      return 0;
    }
    ++str;
  }
  return ret;
}