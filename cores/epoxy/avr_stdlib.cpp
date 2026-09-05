/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "avr_stdlib.h"

// The widest conversion is base 2 of an unsigned long: one character per bit,
// plus a sign for ltoa() and the NUL. This was sizeof(int) * 8 + 1, which is 33
// on a 64-bit host -- eight short of what ultoa(ULONG_MAX, buf, 2) writes.
#define BUFSIZE (sizeof(unsigned long) * 8 + 2)

// Copied and modified from https://people.cs.umu.se/isak/snippets/ltoa.c
// Copyright 1988-90 by Robert B. Stout dba MicroFirm
// Released to public domain, 1991.
char *itoa(int n, char *str, int base) {
  if (36 < base || 2 > base) {
    base = 10; /* can only use 0-9, A-Z        */
  }

  char buf[BUFSIZE];
  char *tail = &buf[BUFSIZE - 1]; /* last character position      */
  *tail-- = '\0';

  char *head = str;
  unsigned uarg;
  if (10 == base && n < 0) {
    *head++ = '-';
    // Negating INT_MIN/LONG_MIN as a signed type is undefined; do it unsigned.
    uarg = 0U - (unsigned) n;
  } else {
    uarg = n;
  }

  unsigned i = 2;
  if (uarg) {
    for (i = 1; uarg; ++i) {
      unsigned rem = uarg % (unsigned) base;
      *tail-- = (char)(rem + ((9 < rem) ? ('A' - 10) : '0'));
      uarg /= (unsigned) base;
    }
  } else {
    *tail-- = '0';
  }

  memcpy(head, ++tail, i);
  return str;
}

// Copied and modified from https://people.cs.umu.se/isak/snippets/ltoa.c
// Copyright 1988-90 by Robert B. Stout dba MicroFirm
// Released to public domain, 1991.
char *utoa(unsigned n, char *str, int base) {
  if (36 < base || 2 > base) {
    base = 10; /* can only use 0-9, A-Z        */
  }

  char buf[BUFSIZE];
  char *tail = &buf[BUFSIZE - 1]; /* last character position      */
  *tail-- = '\0';

  unsigned i = 2;
  if (n) {
    for (i = 1; n; ++i) {
      unsigned rem = n % (unsigned) base;
      *tail-- = (char)(rem + ((9 < rem) ? ('A' - 10) : '0'));
      n /= (unsigned) base;
    }
  } else {
    *tail-- = '0';
  }

  memcpy(str, ++tail, i);
  return str;
}

// Copied and modified from https://people.cs.umu.se/isak/snippets/ltoa.c
// Copyright 1988-90 by Robert B. Stout dba MicroFirm
// Released to public domain, 1991.
char *ltoa(long n, char *str, int base) {
  if (36 < base || 2 > base) {
    base = 10; /* can only use 0-9, A-Z        */
  }

  char buf[BUFSIZE];
  char *tail = &buf[BUFSIZE - 1]; /* last character position      */
  *tail-- = '\0';

  char *head = str;
  unsigned long uarg;
  if (10 == base && n < 0) {
    *head++ = '-';
    // Negating INT_MIN/LONG_MIN as a signed type is undefined; do it unsigned.
    uarg = 0UL - (unsigned long) n;
  } else {
    uarg = n;
  }

  unsigned i = 2;
  if (uarg) {
    for (i = 1; uarg; ++i) {
      unsigned long rem = uarg % (unsigned long) base;
      *tail-- = (char)(rem + ((9 < rem) ? ('A' - 10) : '0'));
      uarg /= (unsigned long) base;
    }
  } else {
    *tail-- = '0';
  }

  memcpy(head, ++tail, i);
  return str;
}

// Copied and modified from https://people.cs.umu.se/isak/snippets/ltoa.c
// Copyright 1988-90 by Robert B. Stout dba MicroFirm
// Released to public domain, 1991.
char *ultoa(unsigned long n, char *str, int base) {
  if (36 < base || 2 > base) {
    base = 10; /* can only use 0-9, A-Z        */
  }

  char buf[BUFSIZE];
  char *tail = &buf[BUFSIZE - 1]; /* last character position      */
  *tail-- = '\0';

  unsigned i = 2;
  if (n) {
    for (i = 1; n; ++i) {
      unsigned long rem = n % (unsigned long) base;
      *tail-- = (char)(rem + ((9 < rem) ? ('A' - 10) : '0'));
      n /= (unsigned long) base;
    }
  } else {
    *tail-- = '0';
  }

  memcpy(str, ++tail, i);
  return str;
}

// This is a terrible, hacky implementation of dtostrf() but this will never be
// used in production. It is only used to allow Arduino unit tests using AUnit
// to compile under Linux and MacOS.
char *dtostrf(double val, signed char width, unsigned char prec, char *s) {
  char format[13];
  char swidth[5];
  itoa(width, swidth, 10);
  char sprec[5];
  utoa(prec, sprec, 10);

  strcpy(format, "%");
  strcat(format, swidth);
  strcat(format, ".");
  strcat(format, sprec);
  strcat(format, "f");

  sprintf(s, format, val);
  return s;
}
