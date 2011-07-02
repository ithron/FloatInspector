//
//  FloatInspectorTest.c
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
#include <float.h>


int main(int, char **);

int
main(int argc, char **argv) {
	
#pragma unused(argc, argv)
	
	const float fvals[] = {
		
		.0f,
		1.f,
		-1.f,
		3.1415e-20f,
		3.1415e+20f,
		5e-45f,
		FLT_MAX,
		FLT_MIN
	};
	
	const double dvals[] = {
		
		.0,
		1.,
		-1.,
		3.1415e-300,
		3.1415e+300,
		5e-245,
		DBL_MAX,
		DBL_MIN
	};
	
	const long double ldvals[] = {
		
		.0l,
		1.l,
		-1.l,
		3.1415e-500l,
		3.1415e+500l,
		5e-445l,
		LDBL_MAX,
		LDBL_MIN
	};
	
	FloatInspectorStatisticsRef restrict statsF = 
		FloatInspectorStatisticsCreateFloat();
	
	FloatInspectorStatisticsRef restrict statsD = 
		FloatInspectorStatisticsCreateDouble();
	
	FloatInspectorStatisticsRef restrict statsLD = 
		FloatInspectorStatisticsCreateLongDouble();
	
	for (int i = 0; i < sizeof(fvals) / sizeof(float); i++) {
		
		FloatInspectorMetaInformation meta = 
			FloatInspectorMetaInformationCreateWithFloat(fvals[i]);
		
		FloatInspectorStatisticsUpdateWithMetaInformation(statsF, 
														  meta);
		
		const char *descr = FloatInspectorMetaInformationDescription(meta);
		
		printf("Float value:\t\t\t\t\t\t%e\n%s\n", fvals[i], descr);
		
		FloatInspectorMetaInformationFree(meta);
	}
	
	for (int i = 0; i < sizeof(dvals) / sizeof(double); i++) {
		
		FloatInspectorMetaInformation meta = 
			FloatInspectorMetaInformationCreateWithDouble(dvals[i]);
		
		FloatInspectorStatisticsUpdateWithMetaInformation(statsD, 
														  meta);
		
		const char *descr = FloatInspectorMetaInformationDescription(meta);
		
		printf("Double value:\t\t\t\t\t\t%e\n%s\n", dvals[i], descr);
		
		FloatInspectorMetaInformationFree(meta);
	}
	
	for (int i = 0; i < sizeof(ldvals) / sizeof(long double); i++) {
		
		FloatInspectorMetaInformation meta = 
			FloatInspectorMetaInformationCreateWithLongDouble(ldvals[i]);
		
		FloatInspectorStatisticsUpdateWithMetaInformation(statsLD, 
														  meta);
		
		const char *descr = FloatInspectorMetaInformationDescription(meta);
		
		printf("Long Double value:\t\t\t\t\t%Le\n%s\n", ldvals[i], descr);
		
		FloatInspectorMetaInformationFree(meta);
	}
	
	FloatInspectorStatisticsPrint(statsF, stdout);
	FloatInspectorStatisticsPrint(statsD, stdout);
	FloatInspectorStatisticsPrint(statsLD, stdout);
	
	FloatInspectorStatisticsFree(statsF);
	FloatInspectorStatisticsFree(statsD);
	FloatInspectorStatisticsFree(statsLD);
	
	
	return EXIT_SUCCESS;
}
