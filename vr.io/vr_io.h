/*
Author: Brant Lewis - October 2012 


#ifndef _TRACKIO_H
#define _TRACKIO_H


#define TIOERR_NO_DATA_SOURCE        2
#define TIOERR_SERVER_STOPPED        6
#define TIOERR_SERVER_DISCONNECTED 109
#define TIOERR_PIPE_IN_USE         231
#define TIOERR_CLIENT_DISCONNECTED 232


#ifdef _USRDLL
	#define TIO_DLL_DECL __declspec(dllexport)
#else
	#define TIO_DLL_DECL
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// Tracker Data Source / Server Interface
TIO_DLL_DECL int TIO_StartServer();
TIO_DLL_DECL int TIO_AcceptClient();
TIO_DLL_DECL int TIO_SendOrientation(float yaw, float pitch, float roll);
TIO_DLL_DECL void TIO_StopServer();

// Tracker Data Reader / Client Interface
TIO_DLL_DECL int TIO_Connect();

// TODO: need 
TIO_DLL_DECL int TIO_GetOrientation(float* yaw, float* pitch, float* roll);
TIO_DLL_DECL void TIO_Close();


#if defined(__cplusplus)
}
#endif

#endif



*/