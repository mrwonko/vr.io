/* trackiotest.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string.h>
#include "../vr.io/vr_io.h"
#include "piefreespace.h"
#include "freespace/freespace.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

bool Stopped = false;

//-----------------------------------------------------------------------------
void RunSender() {

   // First test for the Hillcrest device
   int err = PFS_Connect();

   if (err) {
      printf("Hillcrest connect errored with %d\n", err);
      return;
   }
   
   // Make sure we can read data
   float yaw = 0;
   float pitch = 0;
   float roll = 0;
   err = PFS_GetOrientation(&yaw, &pitch, &roll);
   PFS_Close();   // close the tracker until a client connects

   if (err) {
      printf("Read errored with %d\n", err);
      return;
   }
 
   // Start the server
   printf("Starting server\n");
   err = TIO_StartServer();
   if (err) {
      if (err == TIOERR_PIPE_IN_USE)
         printf("Start Server errored: pipe already in use");
      else
         printf("Start Server errored with %d\n", err);
      return;
   }

   while (!err && !Stopped) {

      // Wait for client
      printf("Waiting for client\n");
      err = TIO_AcceptClient();
      if (err == 0) {

         printf("Connecting to Hillcrest\n");
         err = PFS_Connect();
         if (err) {
            printf("Hillcrest connect errored with %d\n", err);
         }
         else {
            
            while (!err && !Stopped) {
               // Poll the sensor and feed the data to the server   
               err = PFS_GetOrientation(&yaw, &pitch, &roll);
               if (err) {
                  if (err == FREESPACE_ERROR_TIMEOUT) {
                     //printf("wait timeout\n");
                     err = 0;  // not a real error so keep looping
                  }
                  else
                     printf("Read errored with %d\n", err); 
               }
               else {
                  printf("Sending yaw=%f, pitch=%f, roll=%f\n", yaw, pitch, roll);
                  err = TIO_SendOrientation(yaw, pitch, roll);
                  
               }
            }

            if (err == TIOERR_CLIENT_DISCONNECTED) {
               printf("Client disconnected\n");
               err = 0;  // cancel the error to keep listening for another client
            }

            printf("Detaching Hillcrest\n");
            PFS_Close();
         }
      }
      else {
         if (err == TIOERR_SERVER_STOPPED)
            printf("Shutting down server\n");  // normal shutdown
         else
            printf("Accept errored with %i\n", err);
      }
   }

   TIO_StopServer();
}

//-----------------------------------------------------------------------------
void RunReceiver()
{
   printf("Connecting to tracker data source\n");
   int err = TIO_Connect();
   if (err) {
      if (err == TIOERR_NO_DATA_SOURCE)
         printf("Data source not found");
      else 
         printf("Error %i connecting to tracker data source\n", err);
      return;
   }

   printf("Polling tracker data\n");
   while (!err && !Stopped) {
      float yaw = 0;
      float pitch = 0;
      float roll = 0;
      err = TIO_GetOrientation(&yaw, &pitch, &roll);

      if (err == 0)
         printf("Received yaw=%f, pitch=%f, roll=%f\n", yaw, pitch, roll);
      else {
         if (err == TIOERR_SERVER_DISCONNECTED) {
            printf("Tracker data source disconnected\n");
         }
         else
            printf("Error %i during receive\n", err);
      }
   }
   
   TIO_Close();
}

//-----------------------------------------------------------------------------
void PrintUsage()
{
   printf("trackiotest [recv|send]\n\n");
}

//-----------------------------------------------------------------------------
BOOL WINAPI OnCtrlC(DWORD type)
{
   // For all control events, shut down this application
   Stopped = true;
   TIO_Close();
   TIO_StopServer();
   return TRUE;
}
*/
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	/* if (argc < 2) {
       PrintUsage();
       return 1;
   }

   SetConsoleCtrlHandler(OnCtrlC, TRUE);

   if (strcmp(argv[1], "recv") == 0)
      RunReceiver();
   else if (strcmp(argv[1], "send") == 0)
      RunSender();
   else {
      PrintUsage();
      return 1;
   }

   printf("\n");
   
   return 0; */
	return 0;
}

/* */