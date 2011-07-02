//
//  FloatInspector.h
//  FloatInspector
//  
//  Copyright 2011 Stefan Reinhold <stefan@sreinhold.com>. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY STEFAN REINHOLD ``AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STEFAN REINHOLD OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Stefan Reinhold.
//  

#ifndef FloatInspector_FloatInspector_h
#define FloatInspector_FloatInspector_h

#include <inttypes.h>
#include <stdio.h>

#pragma mark Data Types

/* Datatype that contains some meta information about a given floating
 * point number.  */
typedef struct {

	/* Exponent bytes, always in Little Endian format.  */
	uint8_t *exponent;
	/* Number of bits of the exponent.  */
	unsigned int nExponentBits;
	/* Rounded up number of bytes of the exponent.  */
	unsigned int nExponentBytes;
	/* Number of non-zero bits in the exponent.  */
	unsigned int nNonZeroExponentBits;
	
	/* Mantissa bytes, always in Little Endian format.  */
	uint8_t *mantissa;
	/* Number of bits of the mantissa.  */
	unsigned int nMantissaBits;
	/* Rounded up number of bytes of the mantissa.  */
	unsigned int nMantissaBytes;
	/* Number of non-zero bits in the mantissa.  */
	unsigned int nNonZeroMantissaBits;
	
	/* Sign.  */
	enum {
		Positive = 0,
		Negative = 1
	} sign;
	
	/* Information about the type of the floating point number.  */
	enum {
		/* Float is normalized, e.g. with a hidden one.  */
		Normalized,
		/* Float is denormalozed, e.g. without a hidden one.  */
		Denormalized,
		/* Not a number.  */
		NaN,
		/* Infinity.  */
		Infinity
	} type;
	
} FloatInspectorMetaInformation;

/* Datatype to store statistical information about the usage of flaots.  */
typedef struct {
	
	/* Coarse grained statistics.  */
	unsigned int nEntries;
	unsigned int nDenormalized;
	unsigned int nNormalized;
	unsigned int nNegative;
	unsigned int nPositive;
	unsigned int nNaN;
	unsigned int nInf;
	
	enum PrecisionType {
		Float,
		Double,
		LongDouble
	} type;
	
	/* Fine grained statistics.  */
	unsigned int nBits;
	unsigned int nExponentBits;
	unsigned int nMantissaBits;
	
	unsigned int * restrict nNonZeroBitsNormalizedPositive;
	unsigned int * restrict nNonZeroBitsDenormalizedPositive;
	
	unsigned int * restrict nNonZeroBitsNormalizedNegative;
	unsigned int * restrict nNonZeroBitsDenormalizedNegative;
	
	
} _FloatInspectorStatistics;
typedef _FloatInspectorStatistics* FloatInspectorStatisticsRef;


#pragma mark constants

extern const FloatInspectorMetaInformation kFloatInspectorMetaInformationError;

#pragma mark Pulic Functions

const FloatInspectorMetaInformation 
FloatInspectorMetaInformationCreateWithFloat(float f);

const FloatInspectorMetaInformation 
FloatInspectorMetaInformationCreateWithDouble(double f);

const FloatInspectorMetaInformation 
FloatInspectorMetaInformationCreateWithLongDouble(long double f);

void 
FloatInspectorMetaInformationFree(const FloatInspectorMetaInformation meta);

const char *
FloatInspectorMetaInformationDescription(const FloatInspectorMetaInformation meta);

FloatInspectorStatisticsRef FloatInspectorStatisticsCreateFloat(void);
FloatInspectorStatisticsRef FloatInspectorStatisticsCreateDouble(void);
FloatInspectorStatisticsRef FloatInspectorStatisticsCreateLongDouble(void);

void FloatInspectorStatisticsFree(FloatInspectorStatisticsRef stats);

void FloatInspectorStatisticsUpdateWithFloat(FloatInspectorStatisticsRef stats, 
									float f);

void FloatInspectorStatisticsUpdateWithDouble(FloatInspectorStatisticsRef stats, 
									double f);

void FloatInspectorStatisticsUpdateWithLongDouble(FloatInspectorStatisticsRef stats, 
									long double f);

void FloatInspectorStatisticsUpdateWithMetaInformation(FloatInspectorStatisticsRef stats,
													   FloatInspectorMetaInformation meta);

void FloatInspectorStatisticsPrint(const FloatInspectorStatisticsRef stats,
								   FILE *restrict stream);

#endif
