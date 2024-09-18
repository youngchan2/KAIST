/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
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

/* 
 * isAscii - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAscii(0x35) = 1.
 *            isAscii(0x3a) = 0.
 *            isAscii(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAscii(int x) {
  return !((x>>4)^3)&1&((!((x>>3)&1))|(!((x>>1)^28)&1));
}

/* 
 * fourthBits - return word with every fourth bit (starting from the LSB) set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int fourthBits(void) {
  int mask = 1;
  mask = mask|mask<<4;
  mask = mask|mask<<8;
  mask = mask|mask<<16;
  return mask;
}

/*
 * countOneBits - returns count of number of 1's in word
 *   Examples: countOneBits(5) = 2, countOneBits(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int countOneBits(int x) {
  int mask11 = 0x55|(0x55)<<8;
  int mask1 = mask11|mask11<<16;
  int mask22 = 0x33|(0x33)<<8;
  int mask2 = mask22|mask22<<16;
  int mask33 = 0x0f|(0x0f)<<8;
  int mask3 = mask33|mask33<<16;
  int mask4 = 0xff|(0xff)<<16;
  int mask5 = 0xff|(0xff)<<8;

  x = (x&mask1) + ((x>>1)&mask1);
  x = (x&mask2) + ((x>>2)&mask2);
  x = (x&mask3) + ((x>>4)&mask3);
  x = (x&mask4) + ((x>>8)&mask4);
  x = (x&mask5) + ((x>>16)&mask5);
  
  return x;
}

/* 
 * countPattern - returns the number of found "1111" in the given number x
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 80
 *   Rating: 6
 */
int countPattern(int x) {  
  int mask11 = 0x55|(0x55)<<8;
  int mask1 = mask11|mask11<<16;
  int mask22 = 0x33|(0x33)<<8;
  int mask2 = mask22|mask22<<16;
  int mask33 = 0x0f|(0x0f)<<8;
  int mask3 = mask33|mask33<<16;
  int mask4 = 0xff|(0xff)<<16;
  int mask5 = 0xff|(0xff)<<8;
  int mask6 = 0x1f<<24|0xff<<16|mask5;

  x = x&(x>>1)&(x>>2)&(x>>3)&mask6;
  x = (x&mask1) + ((x>>1)&mask1);
  x = (x&mask2) + ((x>>2)&mask2);
  x = (x&mask3) + ((x>>4)&mask3);
  x = (x&mask4) + ((x>>8)&mask4);
  x = (x&mask5) + ((x>>16)&mask5);
  
  return x;
}

/* 
 * subOverflowFree - Determine if can compute x-y without overflow
 *   Example: subOverflowFree(0x80000000,0x80000000) = 1,
 *            subOverflowFree(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOverflowFree(int x, int y) {
  int sub = x+(~y+1);
  int xmsb = x>>31;
  int ymsb = y>>31;
  int submsb = sub>>31;

  return !((xmsb^ymsb)&(xmsb^submsb));
  // xmsb = 1, ymsb = 0, submsb = 0 => oveflow (xmsb = 1 & ymsb = 1 => 더할경우 oveflow)
}

/*
 * mulSevenSixteenth - multiply by 7/16 rounding toward 0, avoiding overflow.
 *    Examples: mulSevenSixteenth(22) = 3
 *    mulSevenSixteenth(0x40000000) = 469762048 (no overflow)

 *    Legal ops: ! ~ & ^ | + << >>
 *    Max ops: 25
 *    Rating: 4
 */
int mulSevenSixteenth(int x) {
  int sign = x>>31;
  int mask = sign & 15;
  int divide = x>>4;
  int remain = x&15;
  int mul = divide + (divide<<1) + (divide<<2);
  remain = remain + (remain<<1) + (remain<<2);//나머지를 7배 하면서 증가한 부분 round to zero
  return mul + ((remain+mask)>>4);

}

/* 
 * sm2tc - Convert from sign-magnitude to two's complement
 *   where the MSB is the sign bit
 *   Example: sm2tc(0x80000005) = -5.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
int sm2tc(int x) {
  int mask1 = 0x7f<<24;
  int mask2 = 0xff+(0xff<<8)+(0xff<<16);
  int mask = mask1+mask2;
  
  int sign = x>>31;
  int mag = x&mask;
  return (mag^sign)+(sign&1);

  /*
  음수일 경우 magnitude를 보수로 양수일 경우 그대로 나와야됨
  0을 input과 xor할 경우 input이 그대로 나옴
  1을 input과 xor할 경우 ~input으로 나옴
  
  이를 이용해 음수일 경우 sign은 0xffffffff이고 양수일 경우 sign은 0
  sign을 mag와 xor하면 음수일 경우 mag를 not하고 양수일 경우 그대로 나오는 operation이 가능
  보수는 +1을 해야되므로 마찬가지로 sign>>31을 더해주면
  음수일 경우 보수, 양수일 경우 그대로
  */
}

/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 4
 */
unsigned float_abs(unsigned uf) {
  int exp = uf & 0x7f800000;
  int frac = uf & 0x007fffff;

  if(exp == 0x7f800000 && frac != 0)
    return uf;
  else
    return uf & 0x7fffffff;
}

/* 
 * integer_to_float - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 6
 */  
unsigned integer_to_float(int x) {
  int sign = x&0x80000000;
  int e = 31;
  int fracmask = 0x7fffffff;
  int exp, frac;
  if(x == 0)
    return 0;
  else if(x == 0x80000000)
    return 0xcf000000;
  else{
    if (sign)
      x = ~x + 1;
    //2^e를 구하기 위한 가장 왼쪽에 있는 1비트 찾기
    while((!(x>>e)&1)) {
      e--;
    }
    exp = e+127;
    x = x<<(31-e);
    frac = (x&(fracmask))>>8;
    // round 조건
    if((((x&0x80)==0x80) && (frac&1)) || ((x&0xff)>0x80))
      frac++;
  }
  return sign + (exp<<23) + frac;
}

/* 
 * real_quarter - Return bit-level equivalent of expression 0.25*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 8
 */
unsigned real_quarter(unsigned uf) {
  unsigned exp = (uf&0x7f800000)>>23;
  unsigned frac = uf&0x007fffff;
  unsigned s = uf&0x80000000;
  if(exp==0xff||uf==0||uf==0x80000000)
    return uf;
  else if(uf==0x800000)
    return 0x200000;
  else{
    /*
    exp가 2보다 크면 2만 줄여주면 됨
    */
    if(exp>2){
      exp = ((exp-2)<<23);
      return s|exp|frac;
    }
    /*
    아닐 경우 sign bit 제외 31개 bit를 shift 연산
    이후 round
    */
    else{
      exp = exp<<23;
      frac = (frac+exp)>>2;
      if((((uf&3)==2)&&(frac&1)) || ((uf&3)>2)){
        frac++;
      }
    }
    return s|frac;
  }
}
