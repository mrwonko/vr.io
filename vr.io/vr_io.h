#ifndef _VRIO_H
#define _VRIO_H

#define VRIOERR_NO_DATA_SOURCE        2
#define VRIOERR_SERVER_STOPPED        6
#define VRIOERR_SERVER_DISCONNECTED 109
#define VRIOERR_PIPE_IN_USE         231
#define VRIOERR_CLIENT_DISCONNECTED 232


//todo: compilation in this proj should export, everywhere else import...

//#if defined(VRIO_EXPORT)	// inside DLL
#	define VRIO_API   __declspec(dllexport)
//#else						// outside DLL define
//#define VRIO_API   __declspec(dllimport)
//#endif  

// Define exported interfaces
struct IVRIOClient
{
	virtual int initialize( void ) = 0;
	virtual int getOrientation( void ) = 0;
	virtual void dispose( void ) = 0;
};

// Export the factory functions for each type...
extern "C" VRIO_API IVRIOClient* _vrio_getInProcessClient();

#endif
