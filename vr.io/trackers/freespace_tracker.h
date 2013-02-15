#include "imotionsensor.h"
#include "../sensor_fusion.h"
#include "freespace.h"
#include "../utils/thread_helpers.h"

#define MAX_SENSORS 2

class FreespaceTracker : IMotionSensor {

public:
	
	FreespaceTracker();
	~FreespaceTracker();
 
	void	getOrientation( int deviceIndex, QAngle &angle );
	bool	initialized();
	void	update() {};
 	bool	hasOrientation();
 	int 	deviceCount() { return _deviceCount; }
	void	think( );

	// These ideally would be protected
	void	_initDevice(FreespaceDeviceId id);
	void	_removeDevice(FreespaceDeviceId id);
	
protected:
	struct	InputThreadState _threadState;
	bool	_initialized;
	int 	_deviceCount;
	int		_thinkCount;
};
