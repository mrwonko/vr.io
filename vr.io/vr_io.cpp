#define IS_DLL
#include <stdio.h>
#include "vr_io.h"
#include "trackers\freespace_tracker.h"
#include "trackers\yei_tracker.h"

class InProcessClient : IVRIOClient
{
 
public:
	InProcessClient( void );
	~InProcessClient( void );

	int initialize( void );
	int think( void );
	int getOrientation( VRIO_Channel channel, VRIO_Message& message );//todo: need another exported type for this... data includes positional and orientation data for now...
	int getChannelCount( );

	void dispose( void );

protected:
	FreespaceTracker* _freespace;
	YEITracker* _yei;
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
	_yei = new YEITracker();
	return 0;
}

void InProcessClient::dispose( void ) // this should really get 
{
	delete _yei;
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
	QAngle angle;
	
	if (_yei->deviceCount() > 0)
	{
		_yei->getOrientation(channel, angle);
		message.pitch = angle[PITCH];
		message.yaw = angle[YAW];
		message.roll = angle[ROLL];
		
		return 0;
	}

	// TODO: tons of cleanup....




	if ( _freespace->deviceCount() <= 0 ) {
		return 0;
	}
	
	
	_freespace->getOrientation(channel, angle);

	message.pitch = angle[PITCH];
	message.yaw = angle[YAW];
	message.roll = angle[ROLL];

	return 0;
}

int InProcessClient::getChannelCount( )
{
	return _freespace->deviceCount() + _yei->deviceCount();
}



// Factory function definitions...
// TODO: Add Named Pipe Client and Named Pipe Server

IVRIOClient* APIENTRY _vrio_getInProcessClient()
{
	InProcessClient* client = new InProcessClient();
	return (IVRIOClient*) client;
}
