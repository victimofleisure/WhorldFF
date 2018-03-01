// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      18apr06	initial version
		01		30dec06	add freeframe orientations
		02		30mar08	add macro to define parameters

        standard include files
 
*/

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__67434E6E_C31F_4514_855E_7C34DD868F75__INCLUDED_)
#define AFX_STDAFX_H__67434E6E_C31F_4514_855E_7C34DD868F75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _WIN32_WINNT 0x0500	// for GDI functions SetDCPenColor, SetDCBrushColor
#define WINVER	0x0500		// for monitor API; may cause NT 5.0 beta warning

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// trig macros
#define PI 3.141592653589793
#define DTR(x) (x * PI / 180)	// degrees to radians
#define RTD(x) (x * 180 / PI)	// radians to degrees

// clamp a value to a range
#define CLAMP(x, lo, hi) (min(max((x), (lo)), (hi)))

// load string from resource via temporary object
#define LDS(x) CString((LPCSTR)x)

// optimized FPU rounding
inline int round(double x)
{
	int		temp;
	__asm	fld		x		// load real
	__asm	fistp	temp	// store integer and pop stack
	return(temp);
}

// double-precision coordinate
#include "DPoint.h"

// minimal base for non-CObject classes
#include "WObject.h"

#define WHORLDFF

#undef PARMDEF
#define PARMDEF(Name, Tag, MinVal, MaxVal, Steps, Scale) IDS_VP_##Tag,

enum {
#include "ParmDef.h"
};

#define IDS_DOC_BAD_FORMAT "Bad format"

// optionally redefine RGB macro to convert from RGB to BGR
#ifdef COLOR_SPACE_BGR	// define in Project/Settings/Preprocessor definitions
#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))
#endif

// freeframe orientations
#define	FF_ORIGIN_TOP_LEFT			1
#define	FF_ORIGIN_BOTTOM_LEFT		2

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__67434E6E_C31F_4514_855E_7C34DD868F75__INCLUDED_)
