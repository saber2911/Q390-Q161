#ifndef _NN_H_
#define _NN_H_

#include "comm.h"


/* NN.H - header file for NN.C
 */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
     Inc., created 1991. All rights reserved.
 */

/* Type definitions.
 */
typedef unsigned int NN_DIGIT;
typedef unsigned short NN_HALF_DIGIT;

/* Constants.

   Note: MAX_MY_NN_DIGITS is long enough to hold any RSA modulus, plus
   one more digit as required by R_GeneratePEMKeys (for n and phiN,
   whose lengths must be even). All natural numbers have at most
   MAX_MY_NN_DIGITS digits, except for double-length intermediate values
   in MY_NN_Mult (t), MY_NN_ModMult (t), MY_NN_ModInv (w), and MY_NN_Div (c).
 */
/* Length of digit in bits */
#define NN_DIGIT_BITS 32
#define NN_HALF_DIGIT_BITS 16
/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS / 8)
/* Maximum length in digits */
#define MAX_NN_DIGITS \
  ((MAX_RSA_MODULUS_LEN + NN_DIGIT_LEN - 1) / NN_DIGIT_LEN + 1)
/* Maximum digits */
#define MAX_NN_DIGIT 0xffffffff
#define MAX_NN_HALF_DIGIT 0xffff

/* Macros.
 */
#define LOW_HALF(x) ((x) & MAX_NN_HALF_DIGIT)
#define HIGH_HALF(x) (((x) >> NN_HALF_DIGIT_BITS) & MAX_NN_HALF_DIGIT)
#define TO_HIGH_HALF(x) (((NN_DIGIT)(x)) << NN_HALF_DIGIT_BITS)
#define DIGIT_MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 1)) & 1)
#define DIGIT_2MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 2)) & 3)

#define NN_ASSIGN_DIGIT(a, b, digits)	{NN_AssignZero (a, digits); a[0] = b;}
#define NN_EQUAL(a, b, digits)	(! NN_Cmp (a, b, digits))
#define NN_EVEN(a, digits)	(((digits) == 0) || ! (a[0] & 1))
/* CONVERSIONS
   MY_NN_Decode (a, digits, b, len)   Decodes character string b into a.
   MY_NN_Encode (a, len, b, digits)   Encodes a into character string b.

   ASSIGNMENTS
   MY_NN_Assign (a, b, digits)        Assigns a = b.
   MY_NN_ASSIGN_DIGIT (a, b, digits)  Assigns a = b, where b is a digit.
   MY_NN_AssignZero (a, b, digits)    Assigns a = 0.
   MY_NN_Assign2Exp (a, b, digits)    Assigns a = 2^b.

   ARITHMETIC OPERATIONS
   MY_NN_Add (a, b, c, digits)        Computes a = b + c.
   MY_NN_Sub (a, b, c, digits)        Computes a = b - c.
   MY_NN_Mult (a, b, c, digits)       Computes a = b * c.
   MY_NN_LShift (a, b, c, digits)     Computes a = b * 2^c.
   MY_NN_RShift (a, b, c, digits)     Computes a = b / 2^c.
   MY_NN_Div (a, b, c, cDigits, d, dDigits)  Computes a = c div d and b = c mod d.

   NUMBER THEORY
   MY_NN_Mod (a, b, bDigits, c, cDigits)  Computes a = b mod c.
   MY_NN_ModMult (a, b, c, d, digits) Computes a = b * c mod d.
   MY_NN_ModExp (a, b, c, cDigits, d, dDigits)  Computes a = b^c mod d.
   MY_NN_ModInv (a, b, c, digits)     Computes a = 1/b mod c.
   MY_NN_Gcd (a, b, c, digits)        Computes a = gcd (b, c).

   OTHER OPERATIONS
   MY_NN_EVEN (a, digits)             Returns 1 iff a is even.
   MY_NN_Cmp (a, b, digits)           Returns sign of a - b.
   MY_NN_EQUAL (a, digits)            Returns 1 iff a = b.
   MY_NN_Zero (a, digits)             Returns 1 iff a = 0.
   MY_NN_Digits (a, digits)           Returns significant length of a in digits.
   MY_NN_Bits (a, digits)             Returns significant length of a in bits.
 */
void NN_Decode (NN_DIGIT *a, unsigned int digits, unsigned char *b, unsigned int len);
void NN_Encode (unsigned char *a, unsigned int len, NN_DIGIT *b, unsigned int digits);
void NN_Assign (NN_DIGIT *a, NN_DIGIT *b, unsigned int digits);
void NN_AssignZero (NN_DIGIT *a, unsigned int digits);
void NN_Assign2Exp (NN_DIGIT *a, unsigned int b, unsigned int digits);
NN_DIGIT NN_Add (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int digits);
NN_DIGIT NN_Sub (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int digits);
void NN_Mult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int digits);
void NN_Div (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int cDigits, NN_DIGIT *d, unsigned int dDigits);
NN_DIGIT NN_LShift (NN_DIGIT *a, NN_DIGIT *b, unsigned int c, unsigned int digits);
NN_DIGIT NN_RShift (NN_DIGIT *a, NN_DIGIT *b, unsigned int c, unsigned int digits);
void NN_Mod (NN_DIGIT *a, NN_DIGIT *b, unsigned int bDigits, NN_DIGIT *c, unsigned int cDigits);
void NN_ModMult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d, unsigned int digits);
void NN_ModExp (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int cDigits, NN_DIGIT *d, unsigned int dDigits);
void NN_ModInv (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c,unsigned int digits);
void NN_Gcd (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int digits);
int NN_Cmp (NN_DIGIT *a, NN_DIGIT *b,unsigned int digits);
int NN_Zero (NN_DIGIT *a, unsigned int digits);
unsigned int NN_Digits (NN_DIGIT *a, unsigned int digits);
// add by liuxl for debug 20070612
NN_DIGIT subdigitmult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d, NN_DIGIT digits);



#endif

