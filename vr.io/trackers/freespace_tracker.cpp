#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <sys/timeb.h>

#include "freespace.h"
#include "freespace_tracker.h"
#include "../sensor_fusion.h"


static void hotplug_Callback(enum freespace_hotplugEvent evnt, FreespaceDeviceId id, void* params) {
	FreespaceTracker* sensor = (FreespaceTracker*) params;

	if (evnt == FREESPACE_HOTPLUG_REMOVAL) { 
        sensor->_removeDevice(id);
    } else if (evnt == FREESPACE_HOTPLUG_INSERTION) {
        sensor->_initDevice(id);
    }
}

DWORD Freespace_Sensor_Thread(LPVOID params)
{
	InputThreadState* state = (InputThreadState*)params;
	int rc;
	FreespaceDeviceId id;
	struct freespace_message message;
	static uint16_t lastseq[MAX_SENSORS];
	Quaternion q;
	
	for (int i = 0; i<MAX_SENSORS; i++){
		lastseq[i] = 0;
		state->sampleCount[i] = 0;
		state->errorCount[i] = 0;
		state->lastReturnCode[i] = 0;
		state->pitch[i] = 0;
		state->roll[i] = 0;
		state->yaw[i] = 0;
	}

	int i = 0;
	while (!state->quit)
	{
		i = ++i % MAX_SENSORS;
		id = state->deviceIds[i];
				
		if (id < 0)
			continue;

		rc = freespace_readMessage(id, &message, 5);
		state->lastReturnCode[i] = rc;
		if (rc == FREESPACE_ERROR_TIMEOUT || rc == FREESPACE_ERROR_INTERRUPTED) {
			state->errorCount[i]++;
			continue;
		}

		if (message.messageType == FREESPACE_MESSAGE_BODYFRAME && message.bodyFrame.sequenceNumber != lastseq[i]) {

			lastseq[i] = message.bodyFrame.sequenceNumber;

			state->sensorFusion[i].MahonyAHRSupdateIMU(
				message.bodyFrame.angularVelX / 1000.0f, 
				message.bodyFrame.angularVelY / 1000.0f,
				message.bodyFrame.angularVelZ / 1000.0f,
				message.bodyFrame.linearAccelX ,
				message.bodyFrame.linearAccelY ,
				message.bodyFrame.linearAccelZ );

			q = state->sensorFusion[i].Read();

			// convert quaternion to euler angles
			float m11 = (2.0f * q[0] * q[0]) + (2.0f * q[1] * q[1]) - 1.0f;
			float m12 = (2.0f * q[1] * q[2]) + (2.0f * q[0] * q[3]);
			float m13 = (2.0f * q[1] * q[3]) - (2.0f * q[0] * q[2]);
			float m23 = (2.0f * q[2] * q[3]) + (2.0f * q[0] * q[1]);
			float m33 = (2.0f * q[0] * q[0]) + (2.0f * q[3] * q[3]) - 1.0f;

			float roll = RADIANS_TO_DEGREES(atan2f(m23, m33)) + 180;
			float pitch = RADIANS_TO_DEGREES(asinf(-m13));
			float yaw = RADIANS_TO_DEGREES(atan2f(m12, m11));

			
			state->deviceAngles[i][ROLL]  = roll;
			state->deviceAngles[i][PITCH] = pitch;
			state->deviceAngles[i][YAW]   = yaw;

			state->pitch[i] = pitch;
			state->roll[i] = roll;
			state->yaw[i] = yaw;

			state->sampleCount[i]++;
		}
	}

	return 0;
}

FreespaceTracker* _freespaceSensor;

FreespaceTracker::FreespaceTracker() 
{
	printf("Initializing freespace drivers\n");
	int rc = 0;
	_deviceCount = 0;
	_threadState.quit = false;
	
	for (int i=0; i<MAX_SENSORS; i++) {
		_threadState.deviceAngles[i].Init();
		_threadState.deviceIds[i] = -1;
	}
				
	if ( freespace_init() != FREESPACE_SUCCESS ) {
		printf("Freespace initialization error. rc=%d\n", rc);
		return;
	}
	
	freespace_setDeviceHotplugCallback(hotplug_Callback, this);
	freespace_perform();
	DWORD threadId;

	_threadState.handle = CreateThread(
		NULL,
		0, // Default stack size
		(LPTHREAD_START_ROUTINE)&Freespace_Sensor_Thread, 
		(LPVOID)&_threadState, 
		0, 
		&threadId);
	
	

	_freespaceSensor = this;
	_initialized = true;
}


FreespaceTracker::~FreespaceTracker() 
{
	_threadState.quit = true;	
	int i = 0;
	if (WaitForSingleObject(_threadState.handle, 1000)) {
		printf("Freespace input thread shut down successfully...\n");
	} else {
		printf("Freespace input thread join timed out, releasing thread handle...\n");
		DWORD exitCode = 0;
		TerminateThread(_threadState.handle, exitCode);
	}
    printf("Shutting down Freespace devices\n");

	for (int idx=0; idx < MAX_SENSORS; idx++) {
		if (_threadState.deviceIds[idx] >= 0) {
			_removeDevice(_threadState.deviceIds[idx]);
		}
	}

	freespace_exit();
}

void FreespaceTracker::_initDevice(FreespaceDeviceId id) 
{
	if (_deviceCount >= MAX_SENSORS) {
		printf("Too many devices, can't add new freespace device %i\n", id);
		return;
    }

	printf("Opening freespace device %i\n", id);
    int rc = freespace_openDevice(id);
    if (rc != 0) {
    printf("Error opening device.\n");
        return;
    }

    rc = freespace_flush(id);
    if (rc != 0) {
    printf("Error flushing device.\n");
        return;
    }
		
    struct freespace_message message;
    memset(&message, 0, sizeof(message));
	message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
	message.dataModeControlV2Request.packetSelect = 2;
	message.dataModeControlV2Request.modeAndStatus |= 0 << 1;
    
    if (freespace_sendMessage(id, &message) != FREESPACE_SUCCESS) {
    printf("Freespace unable to send message: returned %d.\n", rc);
		return;
    }

	_threadState.deviceAngles[_deviceCount].Init();
	_threadState.deviceIds[_deviceCount] = id;
    _deviceCount++;
	
	printf("Freespace sensor %i initialized (id: %i)", _deviceCount, id);
}

void FreespaceTracker::_removeDevice(FreespaceDeviceId id) {
    struct freespace_message message;
	int rc = 0;

	// Remove the device from our list.
    for (int i = 0; i < MAX_SENSORS; i++) {
        if (_threadState.deviceIds[i] == id) {
            _threadState.deviceIds[i] = -1;
			break;
        }
    }
	    
	printf("%d> Sending message to enable mouse motion data.\n", id);
    memset(&message, 0, sizeof(message));

    if (freespace_isNewDevice(id)) {
        message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
        message.dataModeControlV2Request.packetSelect = 1;
    } else {
        message.messageType = FREESPACE_MESSAGE_DATAMODEREQUEST;
        message.dataModeRequest.enableMouseMovement = 1;
    }
    
    if (freespace_sendMessage(id, &message) != FREESPACE_SUCCESS) {
    printf("Could not send message: %d.\n", rc);
    } else {
        freespace_flush(id);
	}

   printf("%d> Cleaning up freespace device...\n", id);
    freespace_closeDevice(id);
	_deviceCount--;
}

void FreespaceTracker::getOrientation(int deviceIndex, QAngle& angle)
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

bool FreespaceTracker::initialized()
{
	return _initialized;
}

bool FreespaceTracker::hasOrientation()
{
	return true;
}
