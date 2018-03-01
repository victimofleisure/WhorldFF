// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19apr06	initial version
		01		25apr06	map Hue to RotateHue instead of SetHue
		02		01may06	change patch default to zero
		03		07jul06	deInstantiate wasn't deleting instance
		04		02oct06	bump plugin version number
        05      30dec06	support 16-bit mode
        06      30mar08	bump plugin version number

		whorld freeframe plugin
 
*/

#include <stdafx.h>
#include "WhorldPlug.h"
#include <math.h>

const PlugInfoStruct PlugInfo = {
	1,	// API major version
	0,	// API minor version
	{'W', 'H', 'O', 'R'},	// plugin identifier
	{'W', 'h', 'o', 'r', 'l', 'd', 'F', 'F',
	 ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',},  // plugin title
	1	// source plugin
};

const PlugExtendedInfoStruct PlugExtInfo = {
	1,		// plugin major version
	300,	// plugin minor version
	"Whorld Freeframe plugin",	// description
	"Copyleft 2006 Chris Korda",	// about text
	0,		// extended data size
	NULL	// extended data block
};

const WhorldPlug::ParamConstantsStruct paramConstants[WhorldPlug::NUM_PARAMS] = {
//			 1234567890123456
	{ 0,	"Patch           "},
	{.5,	"Speed           "},
	{.5,	"Zoom            "},
	{.5,	"Hue             "},
	{.5,	"Damping         "},
	{.5,	"Trail           "},
	{.5,	"Rings           "},
	{.5,	"Tempo           "},
	{.5,	"X Position      "},
	{.5,	"Y Position      "},
};

PlugInfoStruct* getInfo() 
{
	return const_cast<PlugInfoStruct *>(&PlugInfo);
}

DWORD initialise()
{
	return FF_SUCCESS;
}

DWORD deInitialise()
{
	return FF_SUCCESS;
}

DWORD getNumParameters()
{
	return WhorldPlug::NUM_PARAMS;  
}

char* getParameterName(DWORD index)
{
	if (index >= 0 && index < WhorldPlug::NUM_PARAMS)
		return const_cast<char *>(paramConstants[index].name);
	return "                ";
}

float getParameterDefault(DWORD index)
{
	if (index >= 0 && index < WhorldPlug::NUM_PARAMS)
		return paramConstants[index].defaultValue;
	return 0;
}

WhorldPlug::WhorldPlug()
{
	for (int i = 0; i < NUM_PARAMS; i++) {
		m_Param[i].value = .5;
		memset(m_Param[i].displayValue, ' ', MAX_STRING);
	}
}

WhorldPlug::~WhorldPlug()
{
}

char* WhorldPlug::getParameterDisplay(DWORD index)
{
	memset(m_Param[index].displayValue, ' ', MAX_STRING);
	if (index >= 0 && index < NUM_PARAMS) {
		CString	s;
		switch (index) {
		case PARAM_PATCH:
			s.Format("%d", GetCurPatch());
			break;
		default:
			s.Format("%g", m_Param[index].value);
			break;
		}
		memcpy(m_Param[index].displayValue, s, min(s.GetLength(), MAX_STRING));
	}
	return m_Param[index].displayValue;
}

DWORD WhorldPlug::setParameter(SetParameterStruct* pParam)
{
	int	index = pParam->index;
	if (index >= 0 && index < NUM_PARAMS) {
		float	val = pParam->value;
		m_Param[index].value = val;
		switch (index) {
		case PARAM_PATCH:
			{
				int	patches = GetPatchCount();
				if (patches) {
					int sel = int(val * patches);
					sel = min(sel, patches - 1);
					SelectPatch(sel);
				}
			}
			break;
		case PARAM_SPEED:
			SetSpeedNorm(val);
			break;
		case PARAM_ZOOM:
			SetTargetZoomNorm(val);
			break;
		case PARAM_HUE:
			RotateHueNorm(val);
			break;
		case PARAM_DAMPING:
			SetDampingNorm(val);
			break;
		case PARAM_TRAIL:
			SetTrailNorm(val);
			break;
		case PARAM_RINGS:
			SetRingsNorm(val);
			break;
		case PARAM_TEMPO:
			SetTempoNorm(val);
			break;
		case PARAM_XPOS:
			if (GetOrgMotion() == CMainFrame::OM_DRAG) {
				DPOINT	p;
				GetTargetOriginNorm(p);
				p.x = val;
				SetTargetOriginNorm(p);
			}
			break;
		case PARAM_YPOS:
			if (GetOrgMotion() == CMainFrame::OM_DRAG) {
				DPOINT	p;
				GetTargetOriginNorm(p);
				p.y = val;
				SetTargetOriginNorm(p);
			}
			break;
		}
		return FF_SUCCESS;
	}
	return FF_FAIL;
}

float WhorldPlug::getParameter(DWORD index)
{
	if (index >= 0 && index < NUM_PARAMS)
		return m_Param[index].value;
	return 0;
}

DWORD WhorldPlug::processFrameCopy(ProcessFrameCopyStruct* pFrameData)
{
	return FF_FAIL;
}

DWORD getPluginCaps(DWORD index)
{
	switch (index) {

	case FF_CAP_16BITVIDEO:
		return FF_TRUE;

	case FF_CAP_24BITVIDEO:
		return FF_TRUE;

	case FF_CAP_32BITVIDEO:
		return FF_TRUE;

	case FF_CAP_PROCESSFRAMECOPY:
		return FF_FALSE;

	case FF_CAP_MINIMUMINPUTFRAMES:
		return WhorldPlug::NUM_INPUTS;

	case FF_CAP_MAXIMUMINPUTFRAMES:
		return WhorldPlug::NUM_INPUTS;

	case FF_CAP_COPYORINPLACE:
		return FF_FALSE;

	default:
		return FF_FALSE;
	}
}

LPVOID instantiate(VideoInfoStruct* pVideoInfo)
{
	// this shouldn't happen if the host is checking the capabilities properly
	if (pVideoInfo->bitDepth < 0 || pVideoInfo->bitDepth > 2)
		return (LPVOID) FF_FAIL;

	WhorldPlug *pPlugObj = new WhorldPlug;

	if (!pPlugObj->Init(*pVideoInfo)) {
		delete pPlugObj;
		return NULL;
	}

	return (LPVOID) pPlugObj;
}

DWORD deInstantiate(LPVOID instanceID)
{
	WhorldPlug *pPlugObj = (WhorldPlug*) instanceID;
	delete pPlugObj;	// delete first, THEN set to null (duh!)
	pPlugObj = NULL;	// mark instance deleted
	return FF_SUCCESS;
}

LPVOID getExtendedInfo()
{
	return (LPVOID) &PlugExtInfo;
}
