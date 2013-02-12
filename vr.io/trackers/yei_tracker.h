#include "imotionsensor.h"
#include "../sensor_fusion.h"
#include "../utils/thread_helpers.h"

class YEITracker : IMotionSensor {

public:
	
	YEITracker();
	~YEITracker();
 
	void	getOrientation( int deviceIndex, QAngle &angle );
	bool	initialized();
	void	update() {};
 	bool	hasOrientation();
 	int 	deviceCount() { return _deviceCount; }
	

protected:
	struct	InputThreadState _threadState;
	bool	_initialized;
	int 	_deviceCount;
};
