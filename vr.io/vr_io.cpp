#include "vr_io.h"

class InProcessClient : IVRIOClient
{
 
public:
	InProcessClient( void );
	~InProcessClient( void );

	int initialize( void );
	int think( void );
	int getOrientation( void ); //todo: need another exported type for this... data includes positional and orientation data for now...
	void dispose( void );
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
	// TODO: initialize everything....
	return 0;
}

int InProcessClient::think( void )
{
	// TODO: initialize everything....
	return 0;
}

int InProcessClient::getOrientation( void ) // this should really get 
{
	// TODO: initialize everything....
	return 0;
}

void InProcessClient::dispose( void ) // this should really get 
{
	// TODO: Do what must be done here...
}

IVRIOClient* _vrio_getInProcessClient()
{
	InProcessClient* client = new InProcessClient();
	return (IVRIOClient*) client;
}

// TODO: Add Named Pipe Client and Named Pipe Server

