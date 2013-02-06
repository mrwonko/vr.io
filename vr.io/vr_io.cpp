// trackio.cpp : Defines the exported functions for the DLL application.
/*

#include "vr_io.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>

#define TIODATA_ORIENTATION 1

struct OrientData
{
   unsigned char Type;
   float Yaw;
   float Pitch;
   float Roll;
};

static bool Stopped = false;
static HANDLE Pipe = INVALID_HANDLE_VALUE;
static BOOL Connected = false;

#ifdef _USRDLL

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	   case DLL_PROCESS_ATTACH:
	   case DLL_THREAD_ATTACH:
	   case DLL_THREAD_DETACH:
	   case DLL_PROCESS_DETACH:
		   break;
	}
	return TRUE;
}

#endif

//-----------------------------------------------------------------------------
int TIO_StartServer()
{
   if (Pipe != INVALID_HANDLE_VALUE)
      return 0;   // already created
   
   LPTSTR pipe_name = TEXT("\\\\.\\pipe\\virtual_io");

   Pipe = CreateNamedPipe(pipe_name,
                     PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE,
                     PIPE_TYPE_MESSAGE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS,
                     1, 
                     1024,
                     1024,
                     1000,
                     NULL);

   if (Pipe == INVALID_HANDLE_VALUE) {
      int err = (int)GetLastError();
      if (err == ERROR_ACCESS_DENIED || err == ERROR_PIPE_BUSY)
         return TIOERR_PIPE_IN_USE;
      else
         return err;
   }
   else
      return 0;
}

//-----------------------------------------------------------------------------
int TIO_AcceptClient()
{
   if (Connected)
      return 0;

   // Set the pipe to asynchronous mode until connected
   DWORD mode = PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
   SetNamedPipeHandleState(Pipe, &mode, NULL, NULL);
   
   int err = ERROR_PIPE_LISTENING;
   while (!Connected && err == ERROR_PIPE_LISTENING) {
      Connected = ConnectNamedPipe(Pipe, NULL);
      if (!Connected) {
         err = GetLastError();
         if (err == ERROR_PIPE_LISTENING) {
            Sleep(500);  // sleep and then check periodically for another connecting client
         }
         else if (err == ERROR_PIPE_CONNECTED) {
            err = 0;  // client connected before call so connection is ok
            Connected = true;
         }
      }
   }

   if (Connected) {
      // Set the pipe to synchronous mode for I/O
      mode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
      SetNamedPipeHandleState(Pipe, &mode, NULL, NULL);
   }

   return err;
}

//-----------------------------------------------------------------------------
int TIO_SendOrientation(float yaw, float pitch, float roll)
{
   if (Pipe != INVALID_HANDLE_VALUE) {

      OrientData packet;
      packet.Type = TIODATA_ORIENTATION;
      packet.Yaw = yaw;
      packet.Pitch = pitch;
      packet.Roll = roll;
     
      DWORD bytes = 0;
      BOOL ok = WriteFile(Pipe, &packet, sizeof(OrientData), &bytes, NULL);
      if (!ok) {
         int err = GetLastError();
         if (err == TIOERR_CLIENT_DISCONNECTED) { // alias for ERROR_NO_DATA
            DisconnectNamedPipe(Pipe);
            Connected = FALSE;
         }
         return err;
      }
   }

   return 0;
}

//-----------------------------------------------------------------------------
void TIO_StopServer()
{
   if (Pipe != INVALID_HANDLE_VALUE) {
      
      // Disconnect client
      if (Connected) {
         DisconnectNamedPipe(Pipe);
         Connected = FALSE;
      }

      CloseHandle(Pipe);
      Pipe = INVALID_HANDLE_VALUE;
   }
}

//-----------------------------------------------------------------------------
int TIO_Connect()
{
   LPTSTR pipe_name = TEXT("\\\\.\\pipe\\virtual_io");
   Pipe = CreateFile(pipe_name, GENERIC_READ | FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);

   if (Pipe == INVALID_HANDLE_VALUE) {
      int err = GetLastError();
      return err;
   }

   DWORD mode = PIPE_READMODE_MESSAGE;
   BOOL ok = SetNamedPipeHandleState(Pipe, &mode, 0, 0);
   if (ok)
      return 0;
   else {
      int err = GetLastError();
      TIO_Close();
      return err;
   }
}

//-----------------------------------------------------------------------------
int TIO_GetOrientation(float* yaw, float* pitch, float* roll)
{
   if (Pipe == INVALID_HANDLE_VALUE) {
      return -1;
   }
   else {

      unsigned char buff[sizeof(OrientData)];
      DWORD bytes = 0;

      BOOL ok = ReadFile(Pipe, buff, 16, &bytes, NULL); 
      if (ok) {
         if (bytes == 0) {
            int err = GetLastError();
            return -1;
         }
         else {
            OrientData* packet = (OrientData*)buff;
            *yaw = packet->Yaw;
            *pitch = packet->Pitch;
            *roll = packet->Roll;
            return 0;
         }
      }
      else {
         int err = GetLastError();
         return err;
      }
   }
}

//-----------------------------------------------------------------------------
void TIO_Close()
{
   if (Pipe != INVALID_HANDLE_VALUE) {
      CloseHandle(Pipe);
      Pipe = INVALID_HANDLE_VALUE;
   }
}

*/