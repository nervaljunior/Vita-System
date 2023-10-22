#include "alarm.h"
#include "global.h"
#include "pressure.h"
#include "ihm.h"
#include "watchdog.h"

#if DEBUG_CODE != 0
#include "SendOnlySoftwareSerial.h"
#endif

//Please note that this function calcules the SAMPLE std deviation (thus, N-1 in the denominator)
void calcSampleMeanStdDevError(long *samples, int size, float *mean, float *sampleStdDev, float *error)
{
	double sampleSum = 0;
	for(int i = 0; i < size; i++) {
		sampleSum += samples[i];
	}
	*mean = sampleSum/((float)size);

	double sqDevSum = 0.0;

	for(int i = 0; i < size; i++) {
		// pow(x, 2) is x squared.
		sqDevSum += (*mean - (float)samples[i])*(*mean - (float)samples[i]);//^2
	}

	//Please note that this function calcules the SAMPLE std deviation (thus, N-1 in the denominator)
	*sampleStdDev = sqrt(sqDevSum/((float)size-1));

	*error = (*sampleStdDev)/sqrt(size);
}