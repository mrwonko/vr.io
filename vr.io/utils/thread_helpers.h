#pragma once

#include "math_helpers.h"
#include "../sensor_fusion.h"

#define MAX_SENSORS 2

struct InputThreadState 
{
	void* handle;
	QAngle deviceAngles[MAX_SENSORS];
	float wtf[10];
	int deviceIds[MAX_SENSORS];
	float wtf2[10];
	int sampleCount[MAX_SENSORS];
	float wtf3[10];
	int errorCount[MAX_SENSORS];
	float wtf5[10];
	int lastReturnCode[MAX_SENSORS];
	
	float pitch[MAX_SENSORS];
	float roll[MAX_SENSORS];
	float yaw[MAX_SENSORS];

	SensorFusion sensorFusion[MAX_SENSORS];
	
	bool quit;

	void Init()
	{
		for ( int i=0; i<MAX_SENSORS; i++ )
		{
			deviceIds[i] = -1;
			sampleCount[i] = 0;
			errorCount[i] = 0;
			lastReturnCode[i] = 0;
			deviceAngles[i].Init();
			quit = false;
		}
	}
};
