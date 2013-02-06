/*
Author: Brant Lewis (brantlew@yahoo.com) - July 2012 

piefreespace (PFS) is a proxy dll that is used to translate complex freespacelib 
calls into a simple API for easy linking by the FreePIE FreeSpacePlugin.  It sets up
the device connection, handles I/O reads, and converts quaternion vectors into
Euler angles.  This simplifies the code in FreePIE instead of implementing and calling
a full C# libfreespace interface

It was developed against libfreespace-0.6 

*/

#include "piefreespace.h"
#include "freespace/freespace.h"
#include <string.h>
#include <math.h>
#include <time.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <stdio.h>

#define RADIANS_TO_DEGREES(rad) ((float) rad * (float) (180.0 / PI))

FreespaceDeviceId DeviceID = -1;
FreespaceDeviceInfo Device;
bool bRawFeed = false;

#define SMOOTHING_WINDOW_SIZE 3
float YawWindow[SMOOTHING_WINDOW_SIZE];
float PitchWindow[SMOOTHING_WINDOW_SIZE];
float RollWindow[SMOOTHING_WINDOW_SIZE];
int CurSample = -1;
int NumSamples = 0;

#define PI    3.141592654f
#define ANGLE_UNDEFINED  100
const float MAX_ANGULAR_CHANGE = (PI / 180.f) * 1.0f;  // 1 degree per sample max
float Pitch = ANGLE_UNDEFINED;                         // the last reported pitch

//----------------------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	   case DLL_PROCESS_ATTACH:
	   case DLL_THREAD_ATTACH:
	   case DLL_THREAD_DETACH:
	   case DLL_PROCESS_DETACH:
		   break;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
int Connect() {

   //printf("Starting piefreespace 1.1\n");

   // Initialize the freespace library
   int err = freespace_init();
   if (err)
      return err;

   // Look for a single tracker
   int num_devices;
   err = freespace_getDeviceList(&DeviceID, 1, &num_devices);
   if (err)
      return err;

   if (num_devices == 0)
      return FREESPACE_ERROR_NO_DEVICE;

   // Initialize variables
   CurSample = -1;
   NumSamples = 0;
   Pitch = ANGLE_UNDEFINED;

   // Attach to the tracker
   err = freespace_openDevice(DeviceID);
   if (err) {
      DeviceID = -1;
      return err;
   }

   err = freespace_getDeviceInfo(DeviceID, &Device);

   // Flush the message stream
   err = freespace_flush(DeviceID);
   if (err)
      return err;

   // Turn on the orientation message stream
   freespace_message msg;
   memset(&msg, 0, sizeof(msg));

   if (Device.hVer >= 2) {
      // version 2 protocol
      msg.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
      msg.dataModeControlV2Request.packetSelect = 3;    // User Frame (orientation)
      msg.dataModeControlV2Request.modeAndStatus = 0;   // operating mode == full motion
      //msg.dataModeControlV2Request.modeAndStatus = 0x4 << 1;   // operating mode == full motion on
   }
   else {
      // version 1 protocol
      msg.messageType = FREESPACE_MESSAGE_DATAMODEREQUEST;
      msg.dataModeRequest.enableUserPosition = 1;
      msg.dataModeRequest.inhibitPowerManager = 1;
   }

   err = freespace_sendMessage(DeviceID, &msg);
   if (err)
      return err;

   // Now the tracker is ready to read
   return 0;
}

//-----------------------------------------------------------------------------
void Close() {

   if (DeviceID >= 0) {
      
      // Shut off the data stream
      freespace_message msg;
      memset(&msg, 0, sizeof(msg));

      if (Device.hVer >= 2) {
         msg.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
         msg.dataModeControlV2Request.packetSelect = 0;        // No output
         msg.dataModeControlV2Request.modeAndStatus = 1 << 1;  // operating mode == sleep  
      }
      else {
         msg.messageType = FREESPACE_MESSAGE_DATAMODEREQUEST;
         msg.dataModeRequest.enableUserPosition = 0;
         msg.dataModeRequest.inhibitPowerManager = 0;
      }

      int err = freespace_sendMessage(DeviceID, &msg);
      
      // Detach from the tracker
      freespace_closeDevice(DeviceID);
      DeviceID = -1;
   }

   freespace_exit();
}

//-----------------------------------------------------------------------------
int GetOrientation(float* yaw, float* pitch, float* roll) {

   
   int duration = 0;
   int timeout = 250;       // 1/4 second max time in this function

   long start = clock();

   freespace_message msg;
   while (duration < timeout) {
   
      int err = freespace_readMessage(DeviceID, &msg, timeout - duration);
      if (err == 0) {
         // Check if this is a user frame message.
         if (msg.messageType == FREESPACE_MESSAGE_USERFRAME) {
            // Convert from quaternion to Euler angles

            // Get the quaternion vector
            float w = msg.userFrame.angularPosA;
            float x = msg.userFrame.angularPosB;
            float y = msg.userFrame.angularPosC;
            float z = msg.userFrame.angularPosD;

            // normalize the vector
            float len = sqrtf((w*w) + (x*x) + (y*y) + (z*z));
            w /= len;
            x /= len;
            y /= len;
            z /= len;

            // The Freespace quaternion gives the rotation in terms of
            // rotating the world around the object. We take the conjugate to
            // get the rotation in the object's reference frame.
            w = w;
            x = -x;
            y = -y;
            z = -z;
      
            // Convert to angles in radians
            float m11 = (2.0f * w * w) + (2.0f * x * x) - 1.0f;
            float m12 = (2.0f * x * y) + (2.0f * w * z);
            float m13 = (2.0f * x * z) - (2.0f * w * y);
            float m23 = (2.0f * y * z) + (2.0f * w * x);
            float m33 = (2.0f * w * w) + (2.0f * z * z) - 1.0f;

            if (bRawFeed) {
               // Just return the raw samples
               *roll = atan2f(m23, m33);
               *pitch = asinf(-m13);
               *yaw = atan2f(m12, m11);
               return 0;
            }
            else {
               // Run the new sample through the smoothing filter

               // Find the index into the rotating sample window
               CurSample++;
               if (CurSample >= SMOOTHING_WINDOW_SIZE)
                  CurSample = 0;

               // Add the new sample
               RollWindow[CurSample] = atan2f(m23, m33);
               PitchWindow[CurSample] = asinf(-m13);
               YawWindow[CurSample] = atan2f(m12, m11);

               if (NumSamples < SMOOTHING_WINDOW_SIZE) {
                  // At startup, don't return a value until the window is full
                  NumSamples++;
               }
               else {
                  // Average all the samples in the sample window

                  float roll_sum = RollWindow[CurSample];
                  float pitch_sum = PitchWindow[CurSample];
                  float yaw_sum = YawWindow[CurSample];

                  int i = CurSample+1;
                  if (i >= SMOOTHING_WINDOW_SIZE)
                     i = 0;
                  
                  while (i != CurSample) {
                     roll_sum += RollWindow[i];
                     pitch_sum += PitchWindow[i];

                     if (fabs(YawWindow[CurSample] - YawWindow[i]) > PI) {
                        // If the yaw samples cross a discontinuity, then make the angles
                        // all positive or all negative so averaging works
                        if (YawWindow[CurSample] > 0)
                           yaw_sum += (YawWindow[i] + (2 * PI));
                        else
                           yaw_sum += (YawWindow[i] + (-2 * PI));
                     }
                     else
                        yaw_sum += YawWindow[i];

                     i++;
                     if (i >= SMOOTHING_WINDOW_SIZE)
                        i = 0;
                  }

                  // Compute the smoothed values
                  float next_roll = roll_sum / SMOOTHING_WINDOW_SIZE;
                  float next_pitch = pitch_sum / SMOOTHING_WINDOW_SIZE;
                  float next_yaw = yaw_sum / SMOOTHING_WINDOW_SIZE;

                  // correct yaw angles
                  if (next_yaw > PI)
                     next_yaw -= (2 * PI);
                  else if (next_yaw < -PI)
                     next_yaw += (2 * PI);

                  // Put a speed limit on pitch changes to minimize violent
                  // pitch recalibrations
                  if (Pitch == ANGLE_UNDEFINED) {
                     Pitch = next_pitch;    // initial value
                  }
                  else {
                     // Limit any large angular changes
                     if (next_pitch < Pitch)
                        Pitch = max(next_pitch, Pitch - MAX_ANGULAR_CHANGE);
                     else if (next_pitch > Pitch)
                        Pitch = min(next_pitch, Pitch + MAX_ANGULAR_CHANGE);
                  }

                  // return the adjusted values
                  *roll = next_roll;
                  *pitch = Pitch;
                  *yaw = next_yaw;        
                  return 0;
               }
            }
         }

         // any other message types will just fall through and keep looping until the timeout is reached
      }
      else
         return err;  // return on timeouts or serious errors

      duration += clock() - start;
   }

   return FREESPACE_ERROR_TIMEOUT;  // The function returns gracefully without values
}

//-----------------------------------------------------------------------------
void PFS_EnableRawFeed(int val) {
   bRawFeed = (val != 0);
}

//-----------------------------------------------------------------------------
int PFS_Connect() {

   int err = Connect();
   if (err)
      Close();   // Shutdown on error

   return err;
}

//-----------------------------------------------------------------------------
void PFS_Close() {
   Close();
}

//-----------------------------------------------------------------------------
int PFS_GetOrientation(float* yaw, float* pitch, float* roll) {
   
   int err = GetOrientation(yaw, pitch, roll);
   if (err && (err != FREESPACE_ERROR_TIMEOUT))
      Close();  // Shutdown on true error

   return err;
}
/*
//-----------------------------------------------------------------------------
#ifndef _USRDLL

// This section is strictly for development and testing as an EXE and is not 
// compiled as part of the dll
#include <conio.h>


int main(int argc, char* argv[]) {

   //PFS_EnableRawFeed(1);
   int err = PFS_Connect();

   if (err)
      printf("Connect errored with %d", err);
   else {

      clock_t start = clock();
      float yaw, pitch, roll;
      float prev_yaw = 0, prev_pitch = 0, prev_roll = 0;
      while (!err && !_kbhit()) {
         
         err = PFS_GetOrientation(&yaw, &pitch, &roll);
         if (err) {
            if (err == FREESPACE_ERROR_TIMEOUT) {
               printf("wait timeout\n");
               err = 0;  // not a real error so keep looping
            }
         }
         else {
            yaw = RADIANS_TO_DEGREES(yaw);
            pitch = RADIANS_TO_DEGREES(pitch);
            roll = RADIANS_TO_DEGREES(roll);
            clock_t now = clock();
            long duration = now - start;
            start = now;
            
            printf("%d\tyaw=%g, pitch=%g, roll=%g\n", duration, yaw, pitch, roll);

            
            //Sleep(0);
         }
      }

      if (err)
         printf("Read errored with %d", err);
      else
         _getch();
   
      PFS_Close();
   }

   printf("press any key to quit\n");
   while (!_kbhit()) {
      Sleep(10);
   }
}

#endif
*/