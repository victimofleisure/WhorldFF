// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19apr06	initial version
		01		25apr06	add RotateHue
        02      30dec06	support 16-bit mode

		whorld freeframe server
 
*/

#ifndef CMAINFRAME_INCLUDED
#define CMAINFRAME_INCLUDED

#include "FreeFrame.h"
#include "Patch.h"
#include "WhorldView.h"
#include "SortStringArray.h"
#include "NormVal.h"
#include "RealTimer.h"

struct VideoInfoStructTag;

class CMainFrame {
public:
// Construction
	CMainFrame();
	~CMainFrame();
	bool	Init(const VideoInfoStructTag& videoInfo);

// Constants
	enum {	// origin motion types
		OM_PARK,	// no motion
		OM_DRAG,	// cursor drag mode
		OM_RANDOM	// random jump mode
	};

// Attributes
	int		GetWidth() const;
	int		GetHeight() const;
	int		GetPatchCount() const;
	int		GetCurPatch() const;
	void	SetSpeed(double Speed);
	double	GetSpeed() const;
	void	SetSpeedNorm(double Speed);
	double	GetSpeedNorm() const;
	void	SetZoom(double Zoom);
	double	GetZoom() const;
	void	SetZoomNorm(double Zoom);
	double	GetZoomNorm() const;
	void	SetTargetZoomNorm(double Zoom);
	double	GetTargetZoomNorm() const;
	void	SetDamping(double Damping);
	double	GetDamping() const;
	void	SetDampingNorm(double Damping);
	double	GetDampingNorm() const;
	void	SetTrail(double Trail);
	double	GetTrail() const;
	void	SetTrailNorm(double Trail);
	double	GetTrailNorm() const;
	void	SetRings(double Rings);
	double	GetRings() const;
	void	SetRingsNorm(double Rings);
	double	GetRingsNorm() const;
	void	SetTempo(double Tempo);
	double	GetTempo() const;
	void	SetTempoNorm(double Tempo);
	double	GetTempoNorm() const;
	void	SetHueLoopLength(double HueLoopLength);
	double	GetHueLoopLength() const;
	void	SetHueLoopLengthNorm(double HueLoopLength);
	double	GetHueLoopLengthNorm() const;
	void	SetCanvasScale(double CanvasScale);
	double	GetCanvasScale() const;
	void	SetCanvasScaleNorm(double CanvasScale);
	double	GetCanvasScaleNorm() const;
	void	SetHue(double Hue);
	double	GetHue() const;
	void	SetHueNorm(double Hue);
	double	GetHueNorm() const;
	void	RotateHue(double Rot);
	void	RotateHueNorm(double Rot);
	void	SetDrawMode(int Mode);
	int		GetDrawMode() const;
	void	SetOrgMotion(int Motion);
	int		GetOrgMotion() const;
	void	Mirror(bool Enable);
	bool	IsMirrored() const;
	void	LoopHue(bool Enable);
	bool	IsHueLooped() const;
	void	InvertColor(bool Enable);
	bool	IsColorInverted() const;
	void	SetReverse(bool Enable);
	bool	GetReverse() const;
	void	SetConvex(bool Enable);
	bool	GetConvex() const;
	void	SetZoomCenter(bool Enable);
	bool	GetZoomCenter() const;
	void	SetOriginNorm(const DPOINT& Origin);
	void	GetOriginNorm(DPOINT& Origin) const;
	void	SetTargetOriginNorm(const DPOINT& Origin);
	void	GetTargetOriginNorm(DPOINT& Origin) const;

// Operations
	DWORD	processFrame(LPVOID pFrame);
	bool	SelectPatch(int PatchIdx);

private:
// Types
	typedef	CArray<CPatch, CPatch&>	PATCH_LIST;
	typedef struct tagMYBITMAPINFO : BITMAPINFO {
		// The bmiColors array allocates a single DWORD, but in 16-bit mode,
		// bmiColors needs to contain three DWORDs: one DWORD each for the red,
		// green and blue color masks.  So we inherit from BITMAPINFO and add 
		// space for the green and blue masks; the red mask is bmiColors[0].
		DWORD	GreenMask;
		DWORD	BlueMask;
	} MYBITMAPINFO;

// Constants
	static const LPCSTR	m_PatchFolder;
	static const CNormVal		m_NormZoom;
	static const CNormVal		m_NormSpeed;
	static const CPosNormVal	m_NormDamping;
	static const CPosNormVal	m_NormTrail;
	static const CPosNormVal	m_NormRings;
	static const CNormVal		m_NormTempo;
	static const CNormVal		m_NormHueLoop;
	static const CNormVal		m_NormCanvas;
	static const CNormVal		m_NormHue;

// Member data
	VideoInfoStruct	m_VideoInfo;	// copy of video info passed to Init
	CWhorldView	m_View;			// our view instance
	PATCH_LIST	m_Patch;		// array of patches, in same order as m_PatchPath
	CSortStringArray	m_PatchPath;	// sorted array of patch paths
	CParmInfo	m_ParmInfo;		// current view parameters
	CRealTimer	m_TempoTimer;	// self-correcting timer based on performance counter
	MYBITMAPINFO	m_bmi;		// frame DIB info
	HDC		m_hDC;				// frame DIB device context
	HBITMAP	m_hDib;				// frame DIB handle
	void	*m_DibBits;			// frame DIB data
	HGDIOBJ	m_PrevBm;			// DC's previous bitmap
	LONG	m_FrameBytes;		// size of frame in bytes
	LONG	m_BytesPerPixel;	// number of bytes per pixel
	int		m_FrameRate;		// frame rate in frames per second
	int		m_PatchIdx;			// index of currently selected patch
	double	m_Speed;			// normalized speed: 0 = nominal, 1 = max, -1 = min
	double	m_Zoom;				// normalized zoom: 0 = nominal, 1 = max, -1 = min
	double	m_TargetZoom;		// zoom target value; may differ from m_Zoom when damped
	double	m_Damping;			// damping for origin motion and zoom; 1 = none, 0 = max
	double	m_Rings;			// maximum number of rings
	double	m_Tempo;			// tempo in beats per minute
	DPoint	m_TargetOrg;		// target origin, in normalized coordinates
	int		m_GrowDir;			// ring growth direction: 1 = outward, -1 = inward
	int		m_OrgMotion;		// origin motion; see enum above
	volatile UINT	m_Clock;	// CRealTimer thread's callback increments this
	UINT	m_PrevClock;		// previous value of m_Clock, for detecting change
	double	m_PrevHue;			// previous hue: 360 degrees mapped to 0..1

// Helpers
	bool	LoadPatches();
	static	void	TimerCallback(LPVOID Cookie);
	static	void	Mirror24(LPVOID pFrame, int w, int h);
	static	void	Mirror32(LPVOID pFrame, int w, int h);
	static	void	Mirror16(LPVOID pFrame, int w, int h);
};

inline int CMainFrame::GetWidth() const
{
	return(m_VideoInfo.frameWidth);
}

inline int CMainFrame::GetHeight() const
{
	return(m_VideoInfo.frameHeight);
}

inline int CMainFrame::GetPatchCount() const
{
	return(m_PatchPath.GetSize());
}

inline int CMainFrame::GetCurPatch() const
{
	return(m_PatchIdx);
}

inline void CMainFrame::SetSpeed(double Speed)
{
	m_Speed = Speed;
}

inline double CMainFrame::GetSpeed() const
{
	return(m_Speed);
}

inline void CMainFrame::SetSpeedNorm(double Speed)
{
	SetSpeed(m_NormSpeed.Denorm(Speed));
}

inline double CMainFrame::GetSpeedNorm() const
{
	return(m_NormSpeed.Norm(GetSpeed()));
}

inline double CMainFrame::GetZoom() const
{
	return(m_NormZoom.Denorm(m_Zoom));
}

inline double CMainFrame::GetZoomNorm() const
{
	return(m_Zoom);
}

inline void CMainFrame::SetTargetZoomNorm(double Zoom)
{
	m_TargetZoom = Zoom;
}

inline double CMainFrame::GetTargetZoomNorm() const
{
	return(m_TargetZoom);
}

inline void CMainFrame::SetDamping(double Damping)
{
	m_Damping = Damping;
}

inline double CMainFrame::GetDamping() const
{
	return(m_Damping);
}

inline void CMainFrame::SetDampingNorm(double Damping)
{
	SetDamping(m_NormDamping.Denorm(m_NormDamping.GetScale() - Damping));
}

inline double CMainFrame::GetDampingNorm() const
{
	return(m_NormDamping.GetScale() - m_NormDamping.Norm(GetDamping()));
}

inline void CMainFrame::SetTrail(double Trail)
{
	m_View.SetTrail(Trail);
}

inline double CMainFrame::GetTrail() const
{
	return(m_View.GetTrail());
}

inline void CMainFrame::SetTrailNorm(double Trail)
{
	SetTrail(m_NormTrail.Denorm(Trail));
}

inline double CMainFrame::GetTrailNorm() const
{
	return(m_NormTrail.Norm(GetTrail()));
}

inline double CMainFrame::GetRings() const
{
	return(m_Rings);
}

inline void CMainFrame::SetRingsNorm(double Rings)
{
	SetRings(m_NormRings.Denorm(Rings) * CPatch::MAX_RINGS);
}

inline double CMainFrame::GetRingsNorm() const
{
	return(m_NormRings.Norm(GetRings() / CPatch::MAX_RINGS));
}

inline double CMainFrame::GetTempo() const
{
	return(m_Tempo);
}

inline void CMainFrame::SetTempoNorm(double Tempo)
{
	SetTempo(m_NormTempo.Denorm(Tempo));
}

inline double CMainFrame::GetTempoNorm() const
{
	return(m_NormTempo.Norm(GetTempo()));
}

inline void CMainFrame::SetHueLoopLength(double HueLoopLength)
{
	m_View.SetHueLoopLength(HueLoopLength);
}

inline double CMainFrame::GetHueLoopLength() const
{
	return(m_View.GetHueLoopLength());
}

inline void CMainFrame::SetHueLoopLengthNorm(double HueLoopLength)
{
	SetHueLoopLength(m_NormHueLoop.Denorm(HueLoopLength));
}

inline double CMainFrame::GetHueLoopLengthNorm() const
{
	return(m_NormHueLoop.Norm(GetHueLoopLength()));
}

inline void CMainFrame::SetCanvasScale(double CanvasScale)
{
	m_View.SetCanvasScale(CanvasScale);
}

inline double CMainFrame::GetCanvasScale() const
{
	return(m_View.GetCanvasScale());
}

inline void CMainFrame::SetCanvasScaleNorm(double CanvasScale)
{
	SetCanvasScale(m_NormCanvas.Denorm(CanvasScale) + 1);
}

inline double CMainFrame::GetCanvasScaleNorm() const
{
	return(m_NormCanvas.Norm(GetCanvasScale() - 1));
}

inline void CMainFrame::SetHue(double Hue)
{
	m_View.SetHue(Hue);
}

inline double CMainFrame::GetHue() const
{
	return(m_View.GetHue());
}

inline void CMainFrame::SetHueNorm(double Hue)
{
	SetHue(m_NormHue.Denorm(Hue));
}

inline double CMainFrame::GetHueNorm() const
{
	return(m_NormHue.Norm(GetHue()));
}

inline void CMainFrame::RotateHueNorm(double Rot)
{
	RotateHue(m_NormHue.Denorm(Rot));
}

inline void CMainFrame::SetDrawMode(int Mode)
{
	m_View.SetDrawMode(Mode);
}

inline int CMainFrame::GetDrawMode() const
{
	return(m_View.GetDrawMode());
}

inline int CMainFrame::GetOrgMotion() const
{
	return(m_OrgMotion);
}

inline void CMainFrame::Mirror(bool Enable)
{
	m_View.Mirror(Enable);
}

inline bool CMainFrame::IsMirrored() const
{
	return(m_View.IsMirrored());
}

inline void CMainFrame::LoopHue(bool Enable)
{
	m_View.LoopHue(Enable);
}

inline bool	CMainFrame::IsHueLooped() const
{
	return(m_View.IsHueLooped());
}

inline void CMainFrame::InvertColor(bool Enable)
{
	m_View.InvertColor(Enable);
}

inline bool CMainFrame::IsColorInverted() const
{
	return(m_View.IsColorInverted());
}

inline void CMainFrame::SetConvex(bool Enable)
{
	m_View.SetConvex(Enable);
}

inline bool CMainFrame::GetReverse() const
{
	return(m_GrowDir < 0);
}

inline bool CMainFrame::GetConvex() const
{
	return(m_View.GetConvex());
}

inline bool CMainFrame::GetZoomCenter() const
{
	return(m_View.GetZoomType() == CWhorldView::ZT_WND_CENTER);
}

inline void CMainFrame::GetOriginNorm(DPOINT& Origin) const
{
	m_View.GetNormOrigin(Origin);
}

inline void CMainFrame::SetTargetOriginNorm(const DPOINT& Origin)
{
	m_TargetOrg = Origin;
}

inline void CMainFrame::GetTargetOriginNorm(DPOINT& Origin) const
{
	Origin = m_TargetOrg;
}

#endif
