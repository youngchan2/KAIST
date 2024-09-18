/* Testing Code */

#include <limits.h>
#include <math.h>

/* Routines used by floation point test code */

/* Convert from bit level representation to floating point number */
float u2f(unsigned u) {
  union {
    unsigned u;
    float f;
  } a;
  a.u = u;
  return a.f;
}

/* Convert from floating point number to bit-level representation */
unsigned f2u(float f) {
  union {
    unsigned u;
    float f;
  } a;
  a.f = f;
  return a.u;
}

/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* We do not support C11 <threads.h>.  */

int test_isAscii(int x) {
  return (0x30 <= x) && (x <= 0x39);
}

int test_fourthBits(void) {
  int result = 0;
  int i;
  for (i = 0; i < 32; i+=4)
    result |= 1<<i;
  return result;
}

int test_countOneBits(int x) {
  int result = 0;
  int i;
  for (i = 0; i < 32; i++)
    result +=  (x >> i) & 0x1;
  return result;
}

int test_countPattern(int x) {
    int shiftedPattern = 0xf;
    int count = 0;
    
    for (int i = 0; i <= 28; i++) {
      if ((x & shiftedPattern) == shiftedPattern) {
        count = count + 1; // Pattern found
      }
      x = x >> 1; // Shift the word right by 1 bit
    }

    return count;
}

int test_subOverflowFree(int x, int y) {
  long long ldiff = (long long) x - y;
  return ldiff == (int) ldiff;
}

int test_mulSevenSixteenth(int x)
{
 return (int)(((long long int)x * 7) / 16);
}

int test_sm2tc(int x) {
  int sign = x < 0;
  int mag  = x & 0x7FFFFFFF;
  return sign ? -mag : mag;
}

unsigned test_float_abs(unsigned uf) {
  float f = u2f(uf);
  unsigned unf = f2u(-f);
  if (isnan(f))
    return uf;
  /* An unfortunate hack to get around a limitation of the BDD Checker */
  if ((int) uf < 0)
      return unf;
  else
      return uf;
}

unsigned test_integer_to_float(int x) {
  float f = (float) x;
  return f2u(f);
}

unsigned test_real_quarter(unsigned uf) {
  float f = u2f(uf);
  float hf = 0.25*f;
  if (isnan(f))
    return uf;
  else
    return f2u(hf);
}

