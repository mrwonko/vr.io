#define IS_DLL
#include <stdio.h>
#include "vr_io.h"
#include "trackers\freespace_tracker.h"


class InProcessClient : IVRIOClient
{
 
public:
	InProcessClient( void );
	~InProcessClient( void );

	int initialize( void );
	int think( void );
	int getOrientation( VRIO_Channel channel, VRIO_Message& message );//todo: need another exported type for this... data includes positional and orientation data for now...
	void dispose( void );

protected:
	FreespaceTracker* _freespace;


};


InProcessClient::InProcessClient( void )
{
	// TODO: ...
}

InProcessClient::~InProcessClient( void )
{
	// TODO: ...
}

int InProcessClient::initialize( void )
{
	_freespace = new FreespaceTracker();
	return 0;
}

void InProcessClient::dispose( void ) // this should really get 
{
	delete _freespace;
}

int InProcessClient::think( void )
{
	// TODO: initialize everything.... 
	return 0;

	// read out and precache each of the active sensors...
}

int InProcessClient::getOrientation( VRIO_Channel channel, VRIO_Message& message ) // this should really get 
{
	message.pitch += 1;
	message.yaw += 4;
	message.roll = 0;
	
	return 0;
}



// Factory function definitions...
// TODO: Add Named Pipe Client and Named Pipe Server

IVRIOClient* APIENTRY _vrio_getInProcessClient()
{
	InProcessClient* client = new InProcessClient();
	return (IVRIOClient*) client;
}
