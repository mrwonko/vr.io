// trackiotest.cpp : A simple little console app to test out the vi.io library...

#include <stdio.h>
#include <string.h>
#include "freespace/freespace.h"
#undef VRIO_EXPORT
#include "../vr.io/vr_io.h"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool stopped = false;
IVRIOClient* client;

void PrintHelp()
{
	printf("trackiotest [recv|send]\n\n");
}

BOOL WINAPI OnCtrlC(DWORD type)
{
	// For all control events, shut down this application
	stopped = true;
   
	client->dispose();
	delete client;
	return TRUE;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
       PrintHelp();
       return 1;
   }

   SetConsoleCtrlHandler(OnCtrlC, TRUE);

   client = _vrio_getInProcessClient();
   client->initialize();
   
   VRIO_Message message;
   message.init();

   while (!stopped)
   {
		client->think();
		client->getOrientation(HEAD, message);
		printf("Orientation retrieved... %f %f %f \n", message.pitch, message.yaw, message.roll);
	   
		Sleep(500);
   }
   
	printf("\n");
	return 0; 
	return 0;
}

