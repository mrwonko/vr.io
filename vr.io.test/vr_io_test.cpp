// trackiotest.cpp : A simple little console app to test out the vi.io library...

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freespace/freespace.h"
#include "mongoose.h"
#undef VRIO_EXPORT
#include "../vr.io/vr_io.h"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool stopped = false;
IVRIOClient* client;
struct mg_context *ctx;

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

	mg_stop(ctx);

	return TRUE;
}

static int begin_request_handler(struct mg_connection *conn) {
  const struct mg_request_info *request_info = mg_get_request_info(conn);
  char content[100];
    VRIO_Message message;
	message.init();
	client->getOrientation(HEAD, message);
  // Prepare the message we're going to send
  int content_length = sprintf_s(content, sizeof(content),
                                "%f %f %f \n\n", message.pitch, message.yaw, message.roll);

  // Send HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %d\r\n"        // Always set Content-Length
            "\r\n"
            "%s",
            content_length, content);

  return 1;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		PrintHelp();
		return 1;
	}

	SetConsoleCtrlHandler(OnCtrlC, TRUE);

	struct mg_callbacks callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = begin_request_handler;
	const char *options[] = { "listening_ports", "8081", "num_threads", "10", NULL };
	ctx = mg_start(&callbacks, NULL, options);

	client = _vrio_getInProcessClient();
	client->initialize();
   
	VRIO_Message message;
	message.init();

	while (!stopped)
	{
		client->think();
		client->getOrientation(HEAD, message);
		system("cls");
		printf("\n\nOrientation retrieved... pitch:%f yaw:%f roll:%f \n\n", message.pitch, message.yaw, message.roll);
	   
		Sleep(30);
	}
   
	printf("\n");
	return 0; 
}

