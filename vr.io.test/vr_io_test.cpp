// trackiotest.cpp : A simple little console app to test out the vi.io library...

#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char* argv[])
{
	if (argc < 2) {
       PrintHelp();
       return 1;
   }

   //SetConsoleCtrlHandler(OnCtrlC, TRUE);

   client = _vrio_getInProcessClient();
   client->initialize();
   
   VRIO_Message message;
   message.init();

   int i = 0;

   while (i < 1000)
   {
		client->think();
		//printf("Currently active channels: %i\n", client->getChannelCount());
		
		system("cls");
		client->getOrientation(HEAD, message);
		
		printf("\n\n%i> HEAD Orientation pitch:%f yaw:%f roll:%f \n", i, message.pitch, message.yaw, message.roll);
		
		if ( client->getChannelCount() > 1 )
		{
			client->getOrientation(WEAPON, message);
			printf("\n\n%i> WEAPON Orientation pitch:%f yaw:%f roll:%f \n", i, message.pitch, message.yaw, message.roll);
			Sleep(5);
		}
			
		i++;
   }
            
	client->dispose();
	delete client;

	printf("Finished... \n");

	Sleep(2000);
	return 0;
}

