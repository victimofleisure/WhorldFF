// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19apr06	initial version

		whorld freeframe plugin
 
*/

#ifndef __WHORLDPLUG_H__
#define __WHORLDPLUG_H__

#include "FreeFrame.h"
#include "MainFrm.h"

class WhorldPlug : public CMainFrame {
public:
// Constants
	enum {
		PARAM_PATCH,
		PARAM_SPEED,
		PARAM_ZOOM,
		PARAM_HUE,
		PARAM_DAMPING,
		PARAM_TRAIL,
		PARAM_RINGS,
		PARAM_TEMPO,
		PARAM_XPOS,
		PARAM_YPOS,
		NUM_PARAMS
	};
	enum {
		NUM_INPUTS = 1,
		MAX_STRING = 16
	};

// Types
	typedef struct ParamConstsStructTag {
		float defaultValue;
		char name[MAX_STRING + 1];
	} ParamConstantsStruct;
	typedef struct ParamDynamicDataStructTag {
		float value;
		char displayValue[MAX_STRING];
	} ParamDynamicDataStruct;

// Construction
	WhorldPlug();
	~WhorldPlug();

// Attributes
	char*	getParameterDisplay(DWORD index);			
	DWORD	setParameter(SetParameterStruct* pParam);		
	float	getParameter(DWORD index);

// Operations
	// processFrame is inherited from base class
	DWORD	processFrameCopy(ProcessFrameCopyStruct* pFrameData);

private:
// Member data
	ParamDynamicDataStruct m_Param[NUM_PARAMS];
};

PlugInfoStruct*	getInfo();							
DWORD	initialise();								
DWORD	deInitialise();								
DWORD	getNumParameters();							
char*	getParameterName(DWORD index);				
float	getParameterDefault(DWORD index);			
DWORD	getPluginCaps(DWORD index);	
LPVOID	instantiate(VideoInfoStruct* pVideoInfo);
DWORD	deInstantiate(LPVOID instanceID);	
LPVOID	getExtendedInfo();		

#endif
