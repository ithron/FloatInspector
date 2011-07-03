//
//  FloatInspector.c
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


#include "FloatInspector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <assert.h>

#pragma mark Constant

const FloatInspectorMetaInformation kFloatInspectorMetaInformationError = {
	.exponent = NULL,
	.nExponentBits = 0,
	.nExponentBytes = 0,
	.nNonZeroExponentBits = 0,
	
	.mantissa = NULL,
	.nMantissaBits = 0,
	.nMantissaBytes = 0,
	.nNonZeroMantissaBits = 0,
	
	.sign = Positive,
	
	.type = NaN
	
};


#pragma mark Private Function Prototypes


const FloatInspectorMetaInformation 
FloatInspectorMetaInformationCreateGeneric(void *f, 
										   unsigned int nExp, 
										   unsigned int nMant);


#pragma mark Private Functions Implementations

const FloatInspectorMetaInformation 
FloatInspectorMetaInformationCreateGeneric(void *f, 
										   unsigned int nExp, 
										   unsigned int nMant) {
	
	const unsigned int overallBytes = (1 + nExp + nMant) >> 3;
	const unsigned int nExpBytes = 
		nExp % 8 == 0 ? nExp >> 3 : (nExp >> 3) + 1;
	const unsigned int nMantBytes = 
		nMant % 8 == 0 ? nMant >> 3 : (nMant >> 3) + 1;
	const uint8_t *bytes = (uint8_t *) f;
	
	FloatInspectorMetaInformation meta;
	
	/* Set size information.  */
	meta.nExponentBits = nExp;
	meta.nExponentBytes = nExpBytes;
	meta.nMantissaBits = nMant;
	meta.nMantissaBytes = nMantBytes;
	
	/* Allocate memory for exponent an mantissa.  */
	meta.exponent = (uint8_t *) malloc(nExpBytes);
	meta.mantissa = (uint8_t *) malloc(nMantBytes);
	
	bzero(meta.exponent, nExpBytes);
	bzero(meta.mantissa, nMantBytes);
	
	if ((meta.exponent == NULL) || (meta.mantissa == NULL)) {
		
		return kFloatInspectorMetaInformationError;
	}
	
	/* Extract sign.  */
	meta.sign = (bytes[overallBytes - 1] & (1 << 7)) != 0;
	
	/* Extract exponent.  */
	{
		const unsigned int offset = (nExp + 1) % 8;
		const unsigned int occupiedBytes = (nExp + 1) % 8 == 0 ?
			(nExp + 1) >> 3 : ((nExp + 1) >> 3) + 1;
		
		for (unsigned int i = overallBytes - occupiedBytes, j = 0;
			 i < overallBytes - 1;
			 i++, j++) {
			
			if (offset != 0) {
				
				meta.exponent[j] = (uint8_t) ((bytes[i] >> (8 - offset)) |
											  (bytes[i + 1] << offset));
			}
			else {
				
				meta.exponent[j] = bytes[i];
			}
			
		}
		
		meta.exponent[nExpBytes - 1] |= 
			(bytes[overallBytes - 1] & 0x7f) >> (offset == 0 ? 0 : (8 - offset));
	}
	
	/* Extract mantissa.  */
	{
		const unsigned int rest = nMant % 8;
		memcpy(meta.mantissa, bytes, nMantBytes - 1);
		/* Clear the upper 8-res' bits.  */
		uint8_t mask = 0x00;
		for (unsigned int i = 0; i < rest; i++) {
			
			mask = (uint8_t) (mask << 1) | 0x01;
		}
		meta.mantissa[nMantBytes - 1] = bytes[nMantBytes - 1] & mask;
	}
	
	/* Count non-zero exponent bits.  */
	{
		/* Count zero bytes.  */
		unsigned int nZeroBytes = 0;
		while ((nZeroBytes < nExpBytes) && 
			   (meta.exponent[nExpBytes - nZeroBytes - 1] == 0)) nZeroBytes++;
		
		
		/* Count non-zero bits in last non-zero byte.  */
		unsigned int nNonZeroBits = 0;
		if (nZeroBytes < nExpBytes) {
			
			uint8_t byte = meta.exponent[nExpBytes - nZeroBytes - 1];
			while (byte != 0) {
				
				nNonZeroBits++;
				byte >>= 1;
			}
		}
		
		if ((nZeroBytes << 3) < nExp) {
			
			if (nZeroBytes > 0) {
				
				meta.nNonZeroExponentBits = (nExp % 8) + 
					((nZeroBytes - 1) << 3) + nNonZeroBits;
			}
			else {
				
				meta.nNonZeroExponentBits = 
					nNonZeroBits + ((nExpBytes - 1) << 3);
			}
		}
		else {
			
			meta.nNonZeroExponentBits = 0;
		}
	}
	
	assert(meta.nNonZeroExponentBits <= meta.nExponentBits);
	
	/* Count non-zero mantissa bits.  */
	{
		/* Coount zero bytes.  */
		unsigned int nZeroBytes = 0;
		while ((nZeroBytes < nMantBytes) && (meta.mantissa[nZeroBytes] == 0))
			nZeroBytes++;
		
		/* Count non-zero bits in first non-zero byte.  */
		unsigned int nNonZeroBits = 0;
		if (nZeroBytes < nMantBytes) {
			
			uint8_t byte = meta.mantissa[nZeroBytes];
			while (byte != 0) {
				
				nNonZeroBits++;
				byte <<= 1;
			}
		}
		
		if ((nZeroBytes << 3) < nMant) {
			
			meta.nNonZeroMantissaBits = nNonZeroBits +
				((meta.nMantissaBytes - nZeroBytes - 1) << 3) -
				(8 - (nMant % 8));
		}
		else {
			
			meta.nNonZeroMantissaBits = 0;
		}
	}
	
	assert(meta.nNonZeroMantissaBits <= meta.nMantissaBits);
	
	/* Determine type.  */
	if (meta.nNonZeroExponentBits == 0) {
		
		meta.type = Denormalized;
	}
	else if (meta.nNonZeroExponentBits == meta.nExponentBits) {
		
		unsigned int unsetBitsFound = 0;
		for (unsigned int i = 0; i < meta.nExponentBytes - 1; i++) {
			if (meta.exponent[i] != 0xff) {
				unsetBitsFound = 1;
				break;
			}
		}
		
		if (unsetBitsFound == 0) {
			const unsigned int nBitsLeft = 
				meta.nExponentBits - (meta.nExponentBytes - 1) * 8;
			
			for (unsigned int i = 0; i < nBitsLeft; i++) {
				
				if ((meta.exponent[meta.nExponentBytes - 1] & (1 << i)) == 0) {
					
					unsetBitsFound = 1;
					break;
				}
			}
		}
		
		if (unsetBitsFound == 0) {
			if (meta.nNonZeroMantissaBits == 0) {
			
				meta.type = Infinity;
			}
			else {
			
				meta.type = NaN;
			}
		}
		else {
			
			meta.type = Normalized;
		}
	}
	else {
		
		meta.type = Normalized;
	}
	
	return meta;
}

#pragma mark Public Functions Implementations

const FloatInspectorMetaInformation 
FloatInspectorMetaInformationCreateWithFloat(float f) {
	
	const unsigned int nMant = FLT_MANT_DIG - 1;
	const unsigned int nExp = (unsigned int) (sizeof(f) << 3) - nMant - 1;
	
	return FloatInspectorMetaInformationCreateGeneric(&f, nExp, nMant);
}

const FloatInspectorMetaInformation 
FloatInspectorMetaInformationCreateWithDouble(double f) {
	
	const unsigned int nMant = DBL_MANT_DIG - 1;
	const unsigned int nExp = (unsigned int) (sizeof(f) << 3) - nMant - 1;
	
	return FloatInspectorMetaInformationCreateGeneric(&f, nExp, nMant);
}

const FloatInspectorMetaInformation 
FloatInspectorMetaInformationCreateWithLongDouble(long double f) {
	
	const unsigned int nMant = LDBL_MANT_DIG;
	const unsigned int nExp = (unsigned int) ceilf(log2f(LDBL_MAX_EXP - LDBL_MIN_EXP));
	
	return FloatInspectorMetaInformationCreateGeneric(&f, nExp, nMant);
}

void 
FloatInspectorMetaInformationFree(const FloatInspectorMetaInformation meta) {
	
	free(meta.exponent);
	free(meta.mantissa);
}


const char *
FloatInspectorMetaInformationDescription(const FloatInspectorMetaInformation meta) {
	
	static char buffer[1024];
	static char expBuffer[128];
	static char mantBuffer[128];
	
	for (unsigned int i = 0; i < meta.nExponentBytes; i++) {
		sprintf(&expBuffer[2 * i], "%.2x", meta.exponent[meta.nExponentBytes - i - 1]);
	}
	
	for (unsigned int i = 0; i < meta.nMantissaBytes; i++) {
		sprintf(&mantBuffer[2 * i], "%.2x", meta.mantissa[meta.nMantissaBytes - i - 1]);
	}
	
	const char *typeStr;
	
	switch (meta.type) {
		case Normalized:
			typeStr = "Normalized";
			break;
			
		case Denormalized:
			typeStr = "Denormalized";
			break;
			
		case NaN:
			typeStr = "Not a Number";
			break;
			
		case Infinity:
			typeStr = "Infinity";
			break;
			
		default:
			typeStr = "Unknown";
			break;
	}
	
	sprintf(buffer, 
			"Sign:\t\t\t\t\t\t\t\t%s\n"
			"Type:\t\t\t\t\t\t\t\t%s\n"
			"Exponent length:\t\t\t\t\t%u bits\n"
			"Number of non zero exponent bits:\t%u\n"
			"Exponent:\t\t\t\t\t\t\t0x%s\n"
			"Mantissa length:\t\t\t\t\t%u bits\n"
			"Number of non zero mantissa bits:\t%u\n"
			"Mantissa:\t\t\t\t\t\t\t0x%s\n",
			meta.sign == Positive ? "+" : "-",
			typeStr,
			meta.nExponentBits,
			meta.nNonZeroExponentBits,
			expBuffer,
			meta.nMantissaBits,
			meta.nNonZeroMantissaBits,
			mantBuffer
			);
	
	return buffer;
}

FloatInspectorStatisticsRef 
FloatInspectorStatisticsCreateFloat(void) {
	
	FloatInspectorStatisticsRef stats = malloc(sizeof(_FloatInspectorStatistics));
	_FloatInspectorStatistics _stats = {
	
		.nEntries		= 0,
		.nDenormalized	= 0,
		.nNormalized	= 0,
		.nNegative		= 0,
		.nPositive		= 1,
		.nNaN			= 0,
		.nInf			= 0,
		
		.type			= Float,
		
		.nBits			= sizeof(float) * 8,
		.nExponentBits	= sizeof(float) * 8 - FLT_MANT_DIG,
		.nMantissaBits	= FLT_MANT_DIG - 1,
		
		.nNonZeroBitsNormalizedPositive		= NULL,
		.nNonZeroBitsDenormalizedPositive	= NULL,
		
		.nNonZeroBitsNormalizedNegative		= NULL,
		.nNonZeroBitsDenormalizedNegative	= NULL
	};
	
	*stats = _stats;
	
	stats->nNonZeroBitsNormalizedPositive = (unsigned int *)
		malloc(sizeof(unsigned int) * (_stats.nExponentBits + 1) * 
			   (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsDenormalizedPositive = (unsigned int*)
		malloc(sizeof(unsigned int) * (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsNormalizedNegative = (unsigned int *)
		malloc(sizeof(unsigned int) * (_stats.nExponentBits + 1) *
			   (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsDenormalizedNegative = (unsigned int*)
		malloc(sizeof(unsigned int) * (_stats.nMantissaBits + 1));
	
	return stats;
}

FloatInspectorStatisticsRef 
FloatInspectorStatisticsCreateDouble(void) {
	
	FloatInspectorStatisticsRef stats = malloc(sizeof(_FloatInspectorStatistics));
	_FloatInspectorStatistics _stats = {
		
		.nEntries		= 0,
		.nDenormalized	= 0,
		.nNormalized	= 0,
		.nNegative		= 0,
		.nPositive		= 1,
		.nNaN			= 0,
		.nInf			= 0,
		
		.type			= Double,
		
		.nBits			= sizeof(double) * 8,
		.nExponentBits	= sizeof(double) * 8 - DBL_MANT_DIG,
		.nMantissaBits	= DBL_MANT_DIG - 1,
		
		.nNonZeroBitsNormalizedPositive		= NULL,
		.nNonZeroBitsDenormalizedPositive	= NULL,
		
		.nNonZeroBitsNormalizedNegative		= NULL,
		.nNonZeroBitsDenormalizedNegative	= NULL
	};
	
	*stats = _stats;
	
	stats->nNonZeroBitsNormalizedPositive = (unsigned int *)
	malloc(sizeof(unsigned int) * (_stats.nExponentBits + 1) * 
		   (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsDenormalizedPositive = (unsigned int*)
	malloc(sizeof(unsigned int) * (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsNormalizedNegative = (unsigned int *)
	malloc(sizeof(unsigned int) * (_stats.nExponentBits + 1) * 
		   (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsDenormalizedNegative = (unsigned int*)
	malloc(sizeof(unsigned int) * (_stats.nMantissaBits + 1));
	
	return stats;
}

FloatInspectorStatisticsRef 
FloatInspectorStatisticsCreateLongDouble(void) {
	
	FloatInspectorStatisticsRef stats = malloc(sizeof(_FloatInspectorStatistics));
	_FloatInspectorStatistics _stats = {
		
		.nEntries		= 0,
		.nDenormalized	= 0,
		.nNormalized	= 0,
		.nNegative		= 0,
		.nPositive		= 0,
		.nNaN			= 0,
		.nInf			= 0,
		
		.type			= LongDouble,
		
		.nBits			= sizeof(long double) * 8,
		.nExponentBits	= sizeof(long double) * 8 - LDBL_MANT_DIG,
		.nMantissaBits	= LDBL_MANT_DIG - 1,
		
		.nNonZeroBitsNormalizedPositive		= NULL,
		.nNonZeroBitsDenormalizedPositive	= NULL,
		
		.nNonZeroBitsNormalizedNegative		= NULL,
		.nNonZeroBitsDenormalizedNegative	= NULL
	};
	
	*stats = _stats;
	
	stats->nNonZeroBitsNormalizedPositive = (unsigned int *)
	malloc(sizeof(unsigned int) * (_stats.nExponentBits + 1) * 
		   (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsDenormalizedPositive = (unsigned int*)
	malloc(sizeof(unsigned int) * (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsNormalizedNegative = (unsigned int *)
	malloc(sizeof(unsigned int) * (_stats.nExponentBits + 1) * 
		   (_stats.nMantissaBits + 1));
	
	stats->nNonZeroBitsDenormalizedNegative = (unsigned int*)
	malloc(sizeof(unsigned int) * (_stats.nMantissaBits + 1));
	
	return stats;
}

void FloatInspectorStatisticsFree(FloatInspectorStatisticsRef stats) {
	
	free(stats->nNonZeroBitsNormalizedPositive);
	free(stats->nNonZeroBitsDenormalizedPositive);
	free(stats->nNonZeroBitsNormalizedNegative);
	free(stats->nNonZeroBitsDenormalizedNegative);
	
	free(stats);
}

void 
FloatInspectorStatisticsUpdateWithFloat(FloatInspectorStatisticsRef stats, 
										float f) {

	FloatInspectorMetaInformation meta = 
		FloatInspectorMetaInformationCreateWithFloat(f);
	
	FloatInspectorStatisticsUpdateWithMetaInformation(stats, meta);
	
	FloatInspectorMetaInformationFree(meta);
	
}

void 
FloatInspectorStatisticsUpdateWithDouble(FloatInspectorStatisticsRef stats, 
										 double f) {
	
	FloatInspectorMetaInformation meta = 
	FloatInspectorMetaInformationCreateWithDouble(f);
	
	FloatInspectorStatisticsUpdateWithMetaInformation(stats, meta);
	
	FloatInspectorMetaInformationFree(meta);
}

void
FloatInspectorStatisticsUpdateWithLongDouble(FloatInspectorStatisticsRef stats, 
											 long double f) {
	
	FloatInspectorMetaInformation meta = 
	FloatInspectorMetaInformationCreateWithLongDouble(f);
	
	FloatInspectorStatisticsUpdateWithMetaInformation(stats, meta);
	
	FloatInspectorMetaInformationFree(meta);
}

void 
FloatInspectorStatisticsUpdateWithMetaInformation(FloatInspectorStatisticsRef stats,
												  FloatInspectorMetaInformation meta) {
	
	const unsigned int width = stats->nExponentBits + 1;
	
	stats->nEntries++;
	
	switch (meta.type) {
		case Normalized:
		
			stats->nNormalized++;
		
			if (meta.sign == Positive) {
				
				stats->nPositive++;
				stats->nNonZeroBitsNormalizedPositive[meta.nNonZeroMantissaBits * width + meta.nNonZeroExponentBits]++;
			}
			else {
			
				stats->nNegative++;
				stats->nNonZeroBitsNormalizedNegative[meta.nNonZeroMantissaBits * width + meta.nNonZeroExponentBits]++;
			}
			break;
			
		case Denormalized:
			
			stats->nDenormalized++;
			
			if (meta.sign == Positive) {
				
				stats->nPositive++;
				stats->nNonZeroBitsDenormalizedPositive[meta.nNonZeroMantissaBits]++;
			}
			else {
				
				stats->nNegative++;
				stats->nNonZeroBitsDenormalizedNegative[meta.nNonZeroMantissaBits]++;
			}
			break;
			
		case NaN:
			
			stats->nNaN++;
			break;
			
		case Infinity:
			
			stats->nInf++;
			if (meta.sign == Positive) {
				
				stats->nPositive++;
			}
			else {
				
				stats->nNegative++;
			}
			break;
	}
}

void 
FloatInspectorStatisticsPrint(const FloatInspectorStatisticsRef stats,
							  FILE *restrict stream) {
	
	fprintf(stream, "--- Statistics ---\n\n");
	
	switch (stats->type) {
		case Float:
			fprintf(stream, "Type: float\n");
			break;
			
		case Double:
			fprintf(stream, "Type: double\n");
			break;
			
		case LongDouble:
			fprintf(stream, "Type: long double\n");
			break;
	}
	
	/* Coarse grained statistics.  */
	fprintf(stream,
			"%u entries overall,\n\n"
			"%u normalized numbers,\n"
			"%u denormalized numbers,\n"
			"%u positive numbers,\n"
			"%u negatvie numbers,\n"
			"%u NaNs,\n"
			"%u times infinity.\n",
			stats->nEntries,
			stats->nNormalized,
			stats->nDenormalized,
			stats->nPositive,
			stats->nNegative,
			stats->nNaN,
			stats->nInf);
	
	/* Fine grained statistics.  */
	fprintf(stream, 
			"%u exponent bits,\n"
			"%u mantissa bits.\n\n",
			stats->nExponentBits,
			stats->nMantissaBits);
	
	fprintf(stream, "Non-zero bits of positive normalized numbers:\n");
	
	for (unsigned int row = 0; row < stats->nMantissaBits + 1; row++) {
		for (unsigned int col = 0; col < stats->nExponentBits + 1; col++) {
			
			const unsigned int idx = row * (stats->nExponentBits + 1) + col;
			fprintf(stream, 
					"%u\t", 
					stats->nNonZeroBitsNormalizedPositive[idx]);
		}
		fprintf(stream, "\n");
	}
	fprintf(stream, "\n\n");
	
	fprintf(stream, "Non-zero bits of negative normalized numbers:\n");
	
	for (unsigned int row = 0; row < stats->nMantissaBits + 1; row++) {
		for (unsigned int col = 0; col < stats->nExponentBits + 1; col++) {
			
			const unsigned int idx = row * (stats->nExponentBits + 1) + col;
			fprintf(stream, 
					"%u\t", 
					stats->nNonZeroBitsNormalizedNegative[idx]);
		}
		fprintf(stream, "\n");
	}
	fprintf(stream, "\n\n");
	
	fprintf(stream, "Non-zero bits of positive denormalized numbers:\n");
	
	for (unsigned int row = 0; row < stats->nMantissaBits + 1; row++) {
	
		fprintf(stream, 
				"%u\n", 
				stats->nNonZeroBitsDenormalizedPositive[row]);
	}
	fprintf(stream, "\n\n");
	
	fprintf(stream, "Non-zero bits of negative denormalized numbers:\n");
	
	for (unsigned int row = 0; row < stats->nMantissaBits + 1; row++) {
		
		fprintf(stream, 
				"%u\n", 
				stats->nNonZeroBitsDenormalizedNegative[row]);
	}
	fprintf(stream, "\n\n");
	
}

