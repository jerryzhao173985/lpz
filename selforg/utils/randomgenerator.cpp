/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/
#include "randomgenerator.h"

#ifndef _GNU_SOURCE
#ifdef WIN32
#include <inttypes.h>
#endif

int __drand48_iterate(unsigned short int xsubi[3], struct drand48_data* buffer);

int __erand48_r(unsigned short int xsubi[3], struct drand48_data* buffer, double* result);

int
__drand48_iterate(unsigned short int xsubi[3], struct drand48_data* buffer) {
  uint64_t X;
  uint64_t result;

  /* Initialize buffer, if not yet done.  */
  if (__builtin_expect(!buffer->__init, 0)) {
    buffer->__a = 0x5deece66dull;
    buffer->__c = 0xb;
    buffer->__init = 1;
  }

  /* Do the real work.  We choose a data type which contains at least
     48 bits.  Because we compute the modulus it does not care how
     many bits really are computed.  */

  X = static_cast<uint64_t>(xsubi[2]) << 32 | static_cast<uint32_t>(xsubi[1]) << 16 | xsubi[0];

  result = X * buffer->__a + buffer->__c;

  xsubi[0] = result & 0xffff;
  xsubi[1] = (result >> 16) & 0xffff;
  xsubi[2] = (result >> 32) & 0xffff;

  return 0;
}

int
__erand48_r(unsigned short int xsubi[3], struct drand48_data* buffer, double* result) {
  union ieee754_double temp;

  /* Compute next state.  */
  if (__drand48_iterate(xsubi, buffer) < 0)
    return -1;

  /* Construct a positive double with the 48 random bits distributed over
     its fractional part so the resulting FP number is [0.0,1.0).  */

  temp.ieee.negative = 0;
  temp.ieee.exponent = IEEE754_DOUBLE_BIAS;
  temp.ieee.mantissa0 = (xsubi[2] << 4) | (xsubi[1] >> 12);
  temp.ieee.mantissa1 = ((xsubi[1] & 0xfff) << 20) | (xsubi[0] << 4);

  /* Please note the lower 4 bits of mantissa1 are always 0.  */
  *result = temp.d - 1.0;

  return 0;
}

int
srand48_r(long int seedval, struct drand48_data* buffer) {
  /* The standards say we only have 32 bits.  */
  if (sizeof(long int) > 4)
    seedval &= 0xffffffffl;

  buffer->__x[2] = seedval >> 16;
  buffer->__x[1] = seedval & 0xffffl;
  buffer->__x[0] = 0x330e;

  buffer->__a = 0x5deece66dull;
  buffer->__c = 0xb;
  buffer->__init = 1;

  return 0;
}

int
drand48_r(struct drand48_data* buffer, double* result) {
  return __erand48_r(buffer->__x, buffer, result);
}

#endif
