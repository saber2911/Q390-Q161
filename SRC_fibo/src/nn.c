/* NN.C - natural numbers routines
 */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
     Inc., created 1991. All rights reserved.
 */
#include "comm.h"



/* RSA key lengths. */
#define MIN_RSA_MODULUS_BITS 508
/* 
	 PGP 2.6.2 Now allows 2048-bit keys changing below will allow this.
     It does lengthen key generation slightly if the value is increased.
*/
//#define MAX_RSA_MODULUS_BITS 1024
#define MAX_RSA_MODULUS_BITS 2048
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)

#define R_memset(x, y, z)	memset(x, y, z)
#define R_memcpy(x, y, z)	memcpy(x, y, z)
static unsigned int NN_DigitBits (NN_DIGIT a);

//    __asm( "UMULL %0, %1, %2, %3" : "=r"(*cLow), "=r"(*cHigh) : "r"(a), "r"(b));

//void dmult(NN_DIGIT a, NN_DIGIT b, NN_DIGIT *cHigh, NN_DIGIT *cLow)
//{
//	NN_DIGIT *aa,*bb;
//	
//	*aa=a;
//	*bb=b;
//	//__asm( "UMULL [%0], [%1], %2, %3" : "r"(cLow), "r"(cHigh) : "=r"(a), "=r"(b));
//  //  __asm(	"UMULL cLow ,cHigh ,a ,b");
//      __asm(	"UMULL cLow ,cHigh ,aa ,bb");
//	
//}
 //NN_DIGIT          a, b;
 //NN_DIGIT         *high;
 //NN_DIGIT         *low;

 void dmult(NN_DIGIT a,NN_DIGIT b, NN_DIGIT *high, NN_DIGIT *low)
{
	NN_HALF_DIGIT al, ah, bl, bh;
	NN_DIGIT m1, m2, m, ml, mh, carry = 0;

	al = (NN_HALF_DIGIT)LOW_HALF(a);
	ah = (NN_HALF_DIGIT)HIGH_HALF(a);
	bl = (NN_HALF_DIGIT)LOW_HALF(b);
	bh = (NN_HALF_DIGIT)HIGH_HALF(b);

	*low = (NN_DIGIT) al*bl;
	*high = (NN_DIGIT) ah*bh;

	m1 = (NN_DIGIT) al*bh;
	m2 = (NN_DIGIT) ah*bl;
	m = m1 + m2;

	if(m < m1)
        carry = 1L << (NN_DIGIT_BITS / 2);

	ml = (m & MAX_NN_HALF_DIGIT) << (NN_DIGIT_BITS / 2);
	mh = m >> (NN_DIGIT_BITS / 2);

	*low += ml;

	if(*low < ml)
		carry++;

	*high += carry + mh;
}







/* Decodes character mstring b into a, where character string is ordered
   from most to least significant.

   Lengths: a[digits], b[len].
   Assumes b[i] = 0 for i < len - digits * NN_DIGIT_LEN. (Otherwise most
   significant bytes are truncated.)
 */
void NN_Decode (NN_DIGIT *a, unsigned int digits, unsigned char *b, unsigned int len)
{
  NN_DIGIT t;
  int j;
  unsigned int i, u;

  for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
    t = 0;
    for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
      t |= ((NN_DIGIT)b[j]) << u;
    a[i] = t;
  }

  for (; i < digits; i++)
    a[i] = 0;
}

/* Encodes b into character string a, where character string is ordered
   from most to least significant.

   Lengths: a[len], b[digits].
   Assumes NN_Bits (b, digits) <= 8 * len. (Otherwise most significant
   digits are truncated.)
 */
void NN_Encode (unsigned char *a, unsigned int len, NN_DIGIT *b, unsigned int digits)
{
  NN_DIGIT t;
  int j;
  unsigned int i, u;

  for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
    t = b[i];
    for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
      a[j] = (unsigned char)(t >> u);
  }

  for (; j >= 0; j--)
    a[j] = 0;
}

/* Assigns a = b.

   Lengths: a[digits], b[digits].
 */
void NN_Assign (NN_DIGIT *a, NN_DIGIT *b, unsigned int digits)
{
// change by liuxl for debug 20070612
	/*
  unsigned int i;

  for (i = 0; i < digits; i++)
    a[i] = b[i];
//	*/
	if(digits)
    {
        do
        {
            *a++ = *b++;
        }while(--digits);
    }
}

/* Assigns a = 0.

   Lengths: a[digits].
 */
void NN_AssignZero (NN_DIGIT *a, unsigned int digits)
{
// change by liuxl for debug 20070612
	/*
  unsigned int i;

  for (i = 0; i < digits; i++)
    a[i] = 0;
*/
  if(digits)
    {
        do
        {
            *a++ = 0;
        }while(--digits);
    }
}

/* Assigns a = 2^b.   Lengths: a[digits].   Requires b < digits * MY_NN_DIGIT_BITS. */
void NN_Assign2Exp (NN_DIGIT *a, unsigned int b, unsigned int digits)
{  
	NN_AssignZero (a, digits);  
	if (b >= digits * NN_DIGIT_BITS)    
		return;  
	a[b / NN_DIGIT_BITS] = (NN_DIGIT)1 << (b % NN_DIGIT_BITS);
}

/* Computes a = b + c. Returns carry.   Lengths: a[digits], b[digits], c[digits]. */
NN_DIGIT NN_Add (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int digits)
{  
	NN_DIGIT ai, carry;  unsigned int i;  
	carry = 0;  
	for (i = 0; i < digits; i++) 
	{    
		if ((ai = b[i] + carry) < carry)      
			ai = c[i];    
		else if ((ai += c[i]) < c[i])      
			carry = 1;    
		else      
			carry = 0;    

		a[i] = ai;  
	}  

	return (carry);
}

/* Computes a = b - c. Returns borrow.

   Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Sub (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int digits)
{
  NN_DIGIT ai, borrow;
  unsigned int i;

  borrow = 0;

  for (i = 0; i < digits; i++) {
    if ((ai = b[i] - borrow) > (MAX_NN_DIGIT - borrow))
      ai = MAX_NN_DIGIT - c[i];
    else if ((ai -= c[i]) > (MAX_NN_DIGIT - c[i]))
      borrow = 1;
    else
      borrow = 0;
    a[i] = ai;
  }

  return (borrow);
}

/* Computes a = b * c.

   Lengths: a[2*digits], b[digits], c[digits].
   Assumes digits < MAX_NN_DIGITS.
 */
void NN_Mult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int digits)
{
	NN_DIGIT t[2*MAX_NN_DIGITS];
//  unsigned int bDigits, cDigits, i; // for debug liuxl 20070612

  // add by liuxl for debug 20070612
    NN_DIGIT dhigh, dlow, carry;
    NN_DIGIT bDigits, cDigits, i, j;
	// add end

  NN_AssignZero (t, 2 * digits);

  bDigits = NN_Digits (b, digits);
  cDigits = NN_Digits (c, digits);

  /* for debug liuxl 20070612
  for (i = 0; i < bDigits; i++)
    t[i+cDigits] += MY_NN_AddDigitMult (&t[i], &t[i], b[i], c, cDigits);
	*/
  // add by liuxl for debug 20070612
   for (i = 0; i < bDigits; i++)
    {
        carry = 0;
        if(*(b+i) != 0)
        {
            for(j = 0; j < cDigits; j++)
            {
                dmult(*(b+i), *(c+j), &dhigh, &dlow);
                if((*(t+(i+j)) = *(t+(i+j)) + carry) < carry)
                    carry = 1;
                else
                    carry = 0;
                if((*(t+(i+j)) += dlow) < dlow)
                    carry++;
                carry += dhigh;
            }
        }
        *(t+(i+cDigits)) += carry;
    }
   // add end

  NN_Assign (a, t, 2 * digits);

  /* Zeroize potentially sensitive information.
   */
  R_memset((uint8 *)t, 0, sizeof (t));
}

/* Computes a = b * 2^c (i.e., shifts left c bits), returning carry.

   Lengths: a[digits], b[digits].
   Requires c < NN_DIGIT_BITS.
 */
NN_DIGIT NN_LShift (NN_DIGIT *a, NN_DIGIT *b, unsigned int c, unsigned int digits)
{
  NN_DIGIT bi, carry;
  unsigned int i, t;

  if (c >= NN_DIGIT_BITS)
    return (0);

  t = NN_DIGIT_BITS - c;

  carry = 0;

  for (i = 0; i < digits; i++) {
    bi = b[i];
    a[i] = (bi << c) | carry;
    carry = c ? (bi >> t) : 0;
  }

  return (carry);
}

/* Computes a = c div 2^c (i.e., shifts right c bits), returning carry.

   Lengths: a[digits], b[digits].
   Requires: c < NN_DIGIT_BITS.
 */
NN_DIGIT NN_RShift (NN_DIGIT *a, NN_DIGIT *b, unsigned int c, unsigned int digits)
{
  NN_DIGIT bi, carry;
  int i;
  unsigned int t;

  if (c >= NN_DIGIT_BITS)
    return (0);

  t = NN_DIGIT_BITS - c;

  carry = 0;

  for (i = digits - 1; i >= 0; i--) {
    bi = b[i];
    a[i] = (bi >> c) | carry;
    carry = c ? (bi << t) : 0;
  }

  return (carry);
}

/* Computes a = c div d and b = c mod d.

   Lengths: a[cDigits], b[dDigits], c[cDigits], d[dDigits].
   Assumes d > 0, cDigits < 2 * MAX_NN_DIGITS,
           dDigits < MAX_NN_DIGITS.
 */
void NN_Div (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int cDigits, NN_DIGIT *d, unsigned int dDigits)
{
 NN_DIGIT ai, cc[2*MAX_NN_DIGITS+1], dd[MAX_NN_DIGITS]; //  t; // del for debug liuxl 20070612
  int i;
  unsigned int ddDigits, shift;
  // add by liuxl for debug 20070612
  NN_DIGIT s;
    NN_DIGIT t[2], u, v, *ccptr;
    NN_HALF_DIGIT aHigh, aLow, cHigh, cLow;
	// add end

  ddDigits = NN_Digits (d, dDigits);
  if (ddDigits == 0)
    return;

  /* Normalize operands.
   */
  shift = NN_DIGIT_BITS - NN_DigitBits (d[ddDigits-1]);
  NN_AssignZero (cc, ddDigits);
  cc[cDigits] = NN_LShift (cc, c, shift, cDigits);
  NN_LShift (dd, d, shift, ddDigits);
  // changed by liuxl for debug 20070612
  // t = dd[ddDigits-1];
  s = dd[ddDigits-1];

  NN_AssignZero (a, cDigits);

  /* for debug liuxl 20070612
  for (i = cDigits-ddDigits; i >= 0; i--) {
    // Underestimate quotient digit and subtract.

    if (t == MAX_MY_NN_DIGIT)
      ai = cc[i+ddDigits];
    else
      MY_NN_DigitDiv (&ai, &cc[i+ddDigits-1], t + 1);
    cc[i+ddDigits] -= MY_NN_SubDigitMult (&cc[i], &cc[i], ai, dd, ddDigits);

    // Correct estimate.

    while (cc[i+ddDigits] || (MY_NN_Cmp (&cc[i], dd, ddDigits) >= 0)) {
      ai++;
      cc[i+ddDigits] -= MY_NN_Sub (&cc[i], &cc[i], dd, ddDigits);
    }

    a[i] = ai;
  }
  */
  // add by liuxl for debug 20070612
    for (i = cDigits-ddDigits; i >= 0; i--)
    {
        if (s == MAX_NN_DIGIT)
            ai = cc[i+ddDigits];
        else
        {
            ccptr = &cc[i+ddDigits-1];

            s++;
            cHigh = (NN_HALF_DIGIT)HIGH_HALF(s);
            cLow = (NN_HALF_DIGIT)LOW_HALF(s);

            *t = *ccptr;
            *(t+1) = *(ccptr+1);

            if (cHigh == MAX_NN_HALF_DIGIT)
                aHigh = (NN_HALF_DIGIT)HIGH_HALF(*(t+1));
            else
                aHigh = (NN_HALF_DIGIT)(*(t+1) / (cHigh + 1));
            u = (NN_DIGIT)aHigh * (NN_DIGIT)cLow;
            v = (NN_DIGIT)aHigh * (NN_DIGIT)cHigh;
            if ((*t -= TO_HIGH_HALF(u)) > (MAX_NN_DIGIT - TO_HIGH_HALF(u)))
                t[1]--;
            *(t+1) -= HIGH_HALF(u);
            *(t+1) -= v;

            while ((*(t+1) > cHigh) ||
                         ((*(t+1) == cHigh) && (*t >= TO_HIGH_HALF(cLow))))
            {
                if ((*t -= TO_HIGH_HALF(cLow)) > MAX_NN_DIGIT - TO_HIGH_HALF(cLow))
                    t[1]--;
                *(t+1) -= cHigh;
                aHigh++;
            }

            if (cHigh == MAX_NN_HALF_DIGIT)
                aLow = (NN_HALF_DIGIT)LOW_HALF(*(t+1));
            else
                aLow = (NN_HALF_DIGIT)((TO_HIGH_HALF(*(t+1)) + HIGH_HALF(*t)) / (cHigh + 1));
            u = (NN_DIGIT)aLow * (NN_DIGIT)cLow;
            v = (NN_DIGIT)aLow * (NN_DIGIT)cHigh;
            if ((*t -= u) > (MAX_NN_DIGIT - u))
                t[1]--;
            if ((*t -= TO_HIGH_HALF(v)) > (MAX_NN_DIGIT - TO_HIGH_HALF(v)))
                t[1]--;
            *(t+1) -= HIGH_HALF(v);

            while ((*(t+1) > 0) || ((*(t+1) == 0) && *t >= s))
            {
                if ((*t -= s) > (MAX_NN_DIGIT - s))
                    t[1]--;
                aLow++;
            }

            ai = TO_HIGH_HALF(aHigh) + aLow;
            s--;
        }

        cc[i+ddDigits] -= subdigitmult(&cc[i], &cc[i], ai, dd, ddDigits);

        while (cc[i+ddDigits] || (NN_Cmp(&cc[i], dd, ddDigits) >= 0))
        {
            ai++;
            cc[i+ddDigits] -= NN_Sub(&cc[i], &cc[i], dd, ddDigits);
        }

        a[i] = ai;
    }

  /* Restore result.
   */
  NN_AssignZero (b, dDigits);
  NN_RShift (b, cc, shift, ddDigits);

  /* Zeroize potentially sensitive information.
   */
  R_memset ((uint8 *)cc, 0, sizeof (cc));
  R_memset ((uint8 *)dd, 0, sizeof (dd));
}

/* Computes a = b mod c.

   Lengths: a[cDigits], b[bDigits], c[cDigits].
   Assumes c > 0, bDigits < 2 * MAX_MY_NN_DIGITS, cDigits < MAX_MY_NN_DIGITS.
 */
void NN_Mod (NN_DIGIT *a, NN_DIGIT *b, unsigned int bDigits, NN_DIGIT *c, unsigned int cDigits)
{
  NN_DIGIT t[2 * MAX_NN_DIGITS];

  NN_Div (t, a, b, bDigits, c, cDigits);

  /* Zeroize potentially sensitive information.
   */
  	R_memset ((uint8 *)t, 0, sizeof (t));
}

/* Computes a = b * c mod d.

   Lengths: a[digits], b[digits], c[digits], d[digits].
   Assumes d > 0, digits < MAX_MY_NN_DIGITS.
 */
void NN_ModMult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d, unsigned int digits)
{
  NN_DIGIT t[2*MAX_NN_DIGITS];

  NN_Mult (t, b, c, digits);
  NN_Mod (a, t, 2 * digits, d, digits);

  /* Zeroize potentially sensitive information.
   */
  R_memset ((uint8 *)t, 0, sizeof (t));
}

/* Computes a = b^c mod d.

   Lengths: a[dDigits], b[dDigits], c[cDigits], d[dDigits].
   Assumes d > 0, cDigits > 0, dDigits < MAX_MY_NN_DIGITS.
 */
void NN_ModExp (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int cDigits, NN_DIGIT *d, unsigned int dDigits)
{
  NN_DIGIT bPower[3][MAX_NN_DIGITS], ci, t[MAX_NN_DIGITS];
  int i;
  unsigned int ciBits, j, s;

  /* Store b, b^2 mod d, and b^3 mod d.
   */
  NN_Assign (bPower[0], b, dDigits);
  NN_ModMult (bPower[1], bPower[0], b, d, dDigits);
  NN_ModMult (bPower[2], bPower[1], b, d, dDigits);

  NN_ASSIGN_DIGIT (t, 1, dDigits);

  cDigits = NN_Digits (c, cDigits);
  for (i = cDigits - 1; i >= 0; i--) {
    ci = c[i];
    ciBits = NN_DIGIT_BITS;

    /* Scan past leading zero bits of most significant digit.
     */
    if (i == (int)(cDigits - 1)) {
      while (! DIGIT_2MSB (ci)) {
        ci <<= 2;
        ciBits -= 2;
      }
    }

    for (j = 0; j < ciBits; j += 2, ci <<= 2) {
      /* Compute t = t^4 * b^s mod d, where s = two MSB's of ci.
       */
      NN_ModMult (t, t, t, d, dDigits);
      NN_ModMult (t, t, t, d, dDigits);
      if ((s = DIGIT_2MSB (ci)) != 0)
        NN_ModMult (t, t, bPower[s-1], d, dDigits);
    }
  }

  NN_Assign (a, t, dDigits);

  /* Zeroize potentially sensitive information.
   */

  R_memset ((uint8 *)bPower, 0, sizeof (bPower));
  R_memset ((uint8 *)t, 0, sizeof (t));

}

/* Compute a = 1/b mod c, assuming inverse exists.   
 Lengths: a[digits], b[digits], c[digits].   Assumes gcd (b, c) = 1, digits < MAX_MY_NN_DIGITS. 
 */
void NN_ModInv (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c,unsigned int digits)
{  
	NN_DIGIT q[MAX_NN_DIGITS], t1[MAX_NN_DIGITS], t3[MAX_NN_DIGITS], u1[MAX_NN_DIGITS], u3[MAX_NN_DIGITS], v1[MAX_NN_DIGITS], v3[MAX_NN_DIGITS], w[2*MAX_NN_DIGITS];  
	int u1Sign;  
	/* Apply extended Euclidean algorithm, modified to avoid negative     numbers.   */  
	NN_ASSIGN_DIGIT (u1, 1, digits);  
	NN_AssignZero (v1, digits);  
	NN_Assign (u3, b, digits);  
	NN_Assign (v3, c, digits);  
	u1Sign = 1;  
	while (! NN_Zero (v3, digits)) 
	{    
		NN_Div (q, t3, u3, digits, v3, digits);    
		NN_Mult (w, q, v1, digits);    
		NN_Add (t1, u1, w, digits);    
		NN_Assign (u1, v1, digits);    
		NN_Assign (v1, t1, digits);    
		NN_Assign (u3, v3, digits);    
		NN_Assign (v3, t3, digits);    
		u1Sign = -u1Sign;  
	}  
	/* Negate result if sign is negative.    */  
	if (u1Sign < 0)    
		NN_Sub (a, c, u1, digits);  
	else    
		NN_Assign (a, u1, digits);  

	/* Zeroize potentially sensitive information.   */  
	R_memset ((uint8 *)q, 0, sizeof (q));  
	R_memset ((uint8 *)t1, 0, sizeof (t1));  
	R_memset ((uint8 *)t3, 0, sizeof (t3));  
	R_memset ((uint8 *)u1, 0, sizeof (u1));  
	R_memset ((uint8 *)u3, 0, sizeof (u3));  
	R_memset ((uint8 *)v1, 0, sizeof (v1)); 
	R_memset ((uint8 *)v3, 0, sizeof (v3));  
	R_memset ((uint8 *)w, 0, sizeof (w));

}

/* Computes a = gcd(b, c).   Lengths: a[digits], b[digits], c[digits].   Assumes b > c, digits < MAX_MY_NN_DIGITS. */
void NN_Gcd (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned int digits)
{  
	NN_DIGIT t[MAX_NN_DIGITS], u[MAX_NN_DIGITS], v[MAX_NN_DIGITS];  
	NN_Assign (u, b, digits);  
	NN_Assign (v, c, digits);  
	while (! NN_Zero (v, digits)) 
	{    
		NN_Mod (t, u, digits, v, digits);    
		NN_Assign (u, v, digits);    
		NN_Assign (v, t, digits);  
	}  
	NN_Assign (a, u, digits);  
	/* Zeroize potentially sensitive information.   */  
	R_memset ((uint8 *)t, 0, sizeof (t));  
	R_memset ((uint8 *)u, 0, sizeof (u));  
	R_memset ((uint8 *)v, 0, sizeof (v));
}

/* Returns sign of a - b.
   Lengths: a[digits], b[digits].
 */
int NN_Cmp (NN_DIGIT *a, NN_DIGIT *b,unsigned int digits)
{
  int i;

  for (i = digits - 1; i >= 0; i--) {
    if (a[i] > b[i])
      return (1);
    if (a[i] < b[i])
      return (-1);
  }

  return (0);
}

/* Returns nonzero iff a is zero.   Lengths: a[digits]. */
int NN_Zero (NN_DIGIT *a, unsigned int digits)
{  
	unsigned int i;  
	for (i = 0; i < digits; i++)    
		if (a[i])      
			return (0);  

	return (1);
}

/* Returns the significant length of a in digits.

   Lengths: a[digits].
 */
unsigned int NN_Digits (NN_DIGIT *a, unsigned int digits)
{
  int i;

  for (i = digits - 1; i >= 0; i--)
    if (a[i])
      break;

  return (i + 1);
}

/* Returns the significant length of a in bits, where a is a digit.
 */
static unsigned int NN_DigitBits (NN_DIGIT a)
{
  unsigned int i;

  for (i = 0; i < NN_DIGIT_BITS; i++, a >>= 1)
    if (a == 0)
      break;

  return (i);
}


/****************************************************************************

****************************************************************************/
NN_DIGIT subdigitmult(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d, NN_DIGIT digits)
{
    NN_DIGIT borrow, thigh, tlow;
    NN_DIGIT i;

    borrow = 0;

    if(c != 0)
    {
        for(i = 0; i < digits; i++)
        {
            dmult(c, d[i], &thigh, &tlow);
            if((a[i] = b[i] - borrow) > (MAX_NN_DIGIT - borrow))
                borrow = 1;
            else
                borrow = 0;
            if((a[i] -= tlow) > (MAX_NN_DIGIT - tlow))
                borrow++;
            borrow += thigh;
        }
    }

    return (borrow);
}


