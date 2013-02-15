 
#include "../utils/math_helpers.h"

#define Interface class
#define implements public

#define DeclareInterface(name) __interface actual_##name {
 
#define DeclareBasedInterface(name, base) \
        __interface actual_##name \
     : public actual_##base {
 
#define EndInterface(name) };                \
     Interface name : public actual_##name { \
     public:                                 \
        virtual ~name() {}                   \
     };
			

#ifndef IMOTION_SENSOR_H
#define IMOTION_SENSOR_H
DeclareInterface(IMotionSensor)
	//todo: fill out interface...
	void	getOrientation( int deviceIndex, QAngle &angle );
	void	think( void );
EndInterface(IMotionSensor)
#endif 