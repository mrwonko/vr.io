#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <sys/timeb.h>

#include "yei_threespace_api.h"
#include "yei_tracker.h"
#include "../sensor_fusion.h"

#define MAX_YEI_SENSORS 2



DWORD YEI_Sensor_Thread(LPVOID params)
{
	InputThreadState* state = (InputThreadState*)params;
	TSS_Error err;
	TSS_Euler euler;
	TSS_AxisAngle ang;
	int id = 0;
	Quaternion q;
	float raw_data[9];
	SensorData data;
	TSS_Quaternion quat;
	
	unsigned int counter = 0;

	int i = 0;
	while (!state->quit)
	{
		i = ++i % MAX_SENSORS;
		id = state->deviceIds[i];
				
		if (id < 0)
			continue;
		
		//lots of options here....
		//err = getAllSensorsNormalizedf9(state->deviceIds[i], raw_data);
		//todo: you can poll for the actual update rate to tweak the sensor fusion algorithm
		// or maybe not even necessary to use raw data.. err = getAllSensorsRawf9(state->deviceIds[i], raw_data);
		//err= getFiltTaredOrientQuat(state->deviceIds[i], &quat);
		err=getFiltOrientEuler(state->deviceIds[i], &euler);
		if( !err ){
		
			// TODO: for maximum performance I should be able to process the raw data... but not at 10:30 on a work night...
			/*
			
			state->sensorFusion[i].MahonyAHRSupdate(
													raw_data[0], raw_data[1], raw_data[2],  // gyro data
													raw_data[0], raw_data[1], raw_data[2],  // accelerometer
													raw_data[0], raw_data[1], raw_data[2]); // magnetometer
		

			state->sensorFusion[i].MahonyAHRSupdateIMU(
													raw_data[0], raw_data[1], raw_data[2],  // gyro data
													raw_data[0], raw_data[1], raw_data[2]);  // accelerometer
												
			
			q.w = quat.w;
			q.x = quat.x;
			q.y = quat.y;
			q.z = quat.z;			

			//q = state->sensorFusion[i].Read();
			
			// convert quaternion to euler angles
			float m11 = (2.0f * q[0] * q[0]) + (2.0f * q[1] * q[1]) - 1.0f;
			float m12 = (2.0f * q[1] * q[2]) + (2.0f * q[0] * q[3]);
			float m13 = (2.0f * q[1] * q[3]) - (2.0f * q[0] * q[2]);
			float m23 = (2.0f * q[2] * q[3]) + (2.0f * q[0] * q[1]);
			float m33 = (2.0f * q[0] * q[0]) + (2.0f * q[3] * q[3]) - 1.0f;

			float roll = RADIANS_TO_DEGREES(atan2f(m23, m33)) + 180;
			float pitch = RADIANS_TO_DEGREES(asinf(-m13));
			float yaw = RADIANS_TO_DEGREES(atan2f(m12, m11));
			*/
			
			float pitch = RADIANS_TO_DEGREES(euler.x);
			float yaw = RADIANS_TO_DEGREES(euler.y);
			float roll = RADIANS_TO_DEGREES(euler.z);
						
			state->deviceAngles[i][ROLL]  = roll;
			state->deviceAngles[i][PITCH] = pitch;
			state->deviceAngles[i][YAW]   = yaw;

			state->pitch[i] = pitch;
			state->roll[i] = roll;
			state->yaw[i] = yaw;

			state->sampleCount[i]++;
		}
		else{
			printf("YEI Error: %s\n", TSS_Error_String[err]);
			state->errorCount[i]++;
		}

	}

	return 0;
}

YEITracker::YEITracker() 
{
	printf("Initializing YEI drivers\n");
	int rc = 0;
	_deviceCount = 0;
	_threadState.Init();
		
	for (int i=0; i<MAX_YEI_SENSORS; i++) {
		_threadState.deviceAngles[i].Init();
	}

	// TODO: initialize YEI Device...
	TSS_ID deviceId = NO_DEVICE_ID;
    TSS_ID dongle = NO_DEVICE_ID;
    ComPort comport;

 	if(!getFirstAvailableTSSComPort(&comport, TSS_FIND_ALL^TSS_FIND_DNG))
	{
		deviceId = createTSDevice(comport);
		
		if( deviceId == NO_DEVICE_ID){
			printf("Failed to create a YEI sensor on %s\n",comport.com_port);
			return;
		}
	}
	else
	{
		printf("No YEI Dongle found\n");
		return;
	}
    
	printf("YEI device  initialized on com port %s \n", comport.com_port);
	    
	DWORD threadId;
	_threadState.deviceIds[0] = deviceId;

	_threadState.handle = CreateThread(
		NULL,
		0, // Default stack size
		(LPTHREAD_START_ROUTINE)&YEI_Sensor_Thread, 
		(LPVOID)&_threadState, 
		0, 
		&threadId);

	_deviceCount++;
	printf("YEI device initialized successfully");		
	_initialized = true;
}


YEITracker::~YEITracker() 
{
	_threadState.quit = true;	
	int i = 0;
	if (WaitForSingleObject(_threadState.handle, 1000)) {
		printf("YEI input thread shut down successfully...\n");
	} else {
		printf("YEI input thread join timed out, releasing thread handle...\n");
		DWORD exitCode = 0;
		TerminateThread(_threadState.handle, exitCode);
	}
    printf("Shutting down YEI devices\n");

	// TODO: kill off any YEI device...
	
	for (int idx=0; idx < MAX_SENSORS; idx++) {
		if (_threadState.deviceIds[idx] >= 0) {

			printf("Shutting down YEI device %i\n", idx);

			closeTSDevice(_threadState.deviceIds[idx]);
		}
	}
}

void YEITracker::getOrientation(int deviceIndex, QAngle& angle)
{
	printf("Device Orientation %i (%i samples, %i errors, %i returned)\n", deviceIndex, _threadState.sampleCount[deviceIndex], _threadState.errorCount[deviceIndex], _threadState.lastReturnCode[deviceIndex]);
	
	if (_threadState.deviceIds[deviceIndex] == -1) {
		angle.Init();
		return;
	}

	angle[PITCH] = _threadState.pitch[deviceIndex];
	angle[ROLL] = _threadState.roll[deviceIndex];
	angle[YAW] = _threadState.yaw[deviceIndex];
}

bool YEITracker::initialized()
{
	return _initialized;
}

bool YEITracker::hasOrientation()
{
	return true;
}
