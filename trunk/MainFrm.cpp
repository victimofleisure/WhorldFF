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
		02		02oct06	add Copies and Spread
        03      30dec06	support 16-bit mode
		04		30dec06	make top-down DIB conditional
		05		30mar08	pass global parameters to TimerHook

		whorld freeframe server
 
*/

#include <stdafx.h>
#include "FreeFrame.h"
#include "MainFrm.h"
#include "PathStr.h"
#include <math.h>

//												Offset	Scale	LogBase
const CNormVal		CMainFrame::m_NormSpeed(	.5,		2,		20);
const CNormVal		CMainFrame::m_NormZoom(		.5,		2,		10);
const CPosNormVal	CMainFrame::m_NormDamping(	0,		1.01,	20);
const CPosNormVal	CMainFrame::m_NormTrail(	0,		1,		10);
const CPosNormVal	CMainFrame::m_NormRings(	0,		1,		30);
const CNormVal		CMainFrame::m_NormTempo(	0,		250,	0);
const CNormVal		CMainFrame::m_NormHueLoop(	0,		360,	0);
const CNormVal		CMainFrame::m_NormCanvas(	0,		2,		0);
const CNormVal		CMainFrame::m_NormHue(		0,		360,	0);

CMainFrame::CMainFrame()
{
	ZeroMemory(&m_VideoInfo, sizeof(m_VideoInfo));
	m_ParmInfo.SetDefaults();
	ZeroMemory(&m_bmi, sizeof(m_bmi));
	m_hDC = NULL;
	m_hDib = NULL;
	m_DibBits = NULL;
	m_PrevBm = NULL;
	m_FrameBytes = 0;
	m_BytesPerPixel = 0;
	m_FrameRate = 25;	// how do we know our frame rate?
	m_PatchIdx = -1;
	m_Speed = 1;
	m_Zoom = 0;
	m_TargetZoom = 0;
	m_Damping = 1;
	m_Rings = CPatch::MAX_RINGS;
	m_Tempo = 100;
	m_TargetOrg = DPoint(.5, .5);
	m_GrowDir = 1;
	m_OrgMotion = 0;
	m_Clock = 0;
	m_PrevClock = 0;
	m_PrevHue = 0;
	m_TempoTimer.Launch(TimerCallback, this, THREAD_PRIORITY_TIME_CRITICAL);
	m_TempoTimer.Run(TRUE);
}

CMainFrame::~CMainFrame()
{
	if (m_PrevBm != NULL)
		SelectObject(m_hDC, m_PrevBm);	// restore DC's previous bitmap
	DeleteObject(m_hDib);
	DeleteObject(m_hDC);
}

void CMainFrame::SetZoom(double Zoom)
{
	m_View.SetZoom(Zoom);
	m_Zoom = m_NormZoom.Norm(Zoom);
	m_TargetZoom = m_Zoom;
}

void CMainFrame::SetZoomNorm(double Zoom)
{
	m_View.SetZoom(m_NormZoom.Denorm(Zoom));
	m_Zoom = Zoom;
	m_TargetZoom = Zoom;
}

void CMainFrame::SetRings(double Rings)
{
	m_Rings = Rings;
	int	Limit = round(Rings);
	if (Limit >= CPatch::MAX_RINGS)
		Limit = INT_MAX;	// no limit
	m_View.SetMaxRings(Limit);
}

void CMainFrame::SetTempo(double Tempo)
{
	m_TempoTimer.SetFreq(float(Tempo / 60.0), TRUE);
	m_Tempo = Tempo;
}

void CMainFrame::SetOrgMotion(int Motion)
{
	m_OrgMotion = Motion;
}

void CMainFrame::SetReverse(bool Enable)
{
	int	Dir = Enable ? -1 : 1;
	if (Dir == m_GrowDir)
		return;	// nothing to do
	m_GrowDir = Dir;
}

void CMainFrame::SetZoomCenter(bool Enable)
{
	m_View.SetZoomType(Enable ? 
		CWhorldView::ZT_WND_CENTER : CWhorldView::ZT_RING_ORIGIN);
}

void CMainFrame::SetOriginNorm(const DPOINT& Origin)
{
	m_View.SetNormOrigin(Origin);
	m_TargetOrg = Origin;
}

void CMainFrame::RotateHue(double Rot)
{
	m_View.RotateHue(Rot - m_PrevHue);
	m_PrevHue = Rot;
}

bool CMainFrame::Init(const VideoInfoStruct& videoInfo)
{
	m_VideoInfo = videoInfo;
	m_hDC = CreateCompatibleDC(NULL);
	if (m_hDC == NULL)
		return(FALSE);
	ZeroMemory(&m_bmi, sizeof(m_bmi));
	m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmi.bmiHeader.biWidth = m_VideoInfo.frameWidth;
	m_bmi.bmiHeader.biHeight = LONG(m_VideoInfo.frameHeight);
	if (videoInfo.orientation != FF_ORIGIN_BOTTOM_LEFT)
		m_bmi.bmiHeader.biHeight = -m_bmi.bmiHeader.biHeight;	// top-down DIB
	m_bmi.bmiHeader.biPlanes = 1;
	switch (m_VideoInfo.bitDepth) {
	case FF_CAP_16BITVIDEO:
		m_bmi.bmiHeader.biBitCount = 16;
		m_bmi.bmiHeader.biCompression = BI_BITFIELDS;	// must be 5-6-5
		*(DWORD *)m_bmi.bmiColors = 0xF800;	// red color mask
		m_bmi.GreenMask = 0x07E0;	// green color mask
		m_bmi.BlueMask = 0x001F;	// blue color mask
		m_BytesPerPixel = 2;
		break;
	case FF_CAP_24BITVIDEO:
		m_bmi.bmiHeader.biBitCount = 24;
		m_BytesPerPixel = 3;
		break;
	case FF_CAP_32BITVIDEO:
		m_bmi.bmiHeader.biBitCount = 32;
		m_BytesPerPixel = 4;
		break;
	default:
		return(FALSE);
	}
	m_hDib = CreateDIBSection(GetDC(NULL), &m_bmi, DIB_RGB_COLORS, &m_DibBits, NULL, 0);
	if (m_hDib == NULL)
		return(FALSE);
	BITMAP	bm;	// check bitmap's actual size in bytes, just in case
	if (!GetObject(m_hDib, sizeof(bm), &bm))
		return(FALSE);
	if (bm.bmWidthBytes != LONG(m_VideoInfo.frameWidth) * m_BytesPerPixel)
		return(FALSE);
	m_PrevBm = SelectObject(m_hDC, m_hDib);
	if (m_PrevBm == NULL)
		return(FALSE);
	m_FrameBytes = m_VideoInfo.frameWidth * m_VideoInfo.frameHeight * m_BytesPerPixel;
	m_View.SetWndSize(CSize(m_VideoInfo.frameWidth, m_VideoInfo.frameHeight));
	m_View.SetTimerFreq(m_FrameRate);
	LoadPatches();
	SelectPatch(0);	// select first patch
	return(TRUE);
}

bool CMainFrame::LoadPatches()
{
	CString	PatchFolder;
	CPathStr	Path;
	char	*p = Path.GetBuffer(MAX_PATH);
	bool	retc = SUCCEEDED(SHGetSpecialFolderPath(NULL, p, CSIDL_PERSONAL, 0));
	if (!retc)
		return(FALSE);
	Path.ReleaseBuffer();
	Path.Append("WhorldFF\\*.*");
	CFileFind	ff;
	BOOL bWorking = ff.FindFile(Path);
	CPatch	Patch;
	while (bWorking) {
		bWorking = ff.FindNextFile();
		if (!ff.IsDirectory())
			m_PatchPath.Add(ff.GetFilePath());
	}
	m_PatchPath.Sort();
	CStdioFile	fp;
	int	i = 0;
	while (i < m_PatchPath.GetSize()) {
		if (fp.Open(m_PatchPath[i], CFile::modeRead | CFile::shareDenyWrite)
		&& Patch.Read(fp)) {	// this overload doesn't display any messages
			m_Patch.Add(Patch);
			i++;
		} else
			m_PatchPath.RemoveAt(i);
	}
	return(TRUE);
}

bool CMainFrame::SelectPatch(int PatchIdx)
{
	if (PatchIdx < 0 || PatchIdx >= m_PatchPath.GetSize())
		return(FALSE);
	if (PatchIdx == m_PatchIdx)	// if patch is already selected
		return(TRUE);	// nothing to do
	const CPatch *p = &m_Patch[PatchIdx];
	m_ParmInfo = *p;
	// set master attributes
	SetSpeed(p->m_Master.Speed);
	SetZoom(p->m_Master.Zoom);
	SetDamping(p->m_Master.Damping);
	SetTrail(p->m_Master.Trail);
	SetRings(p->m_Master.Rings);
	SetTempo(p->m_Master.Tempo);
	SetHueLoopLength(p->m_Master.HueLoopLength);
	SetCanvasScale(p->m_Master.CanvasScale);
	// set main attributes
	SetOriginNorm(p->m_Main.Origin);
	SetDrawMode(p->m_Main.DrawMode);
	SetOrgMotion(p->m_Main.OrgMotion);
	SetHue(p->m_Main.Hue);
	Mirror(p->m_Main.Mirror);
	SetReverse(p->m_Main.Reverse);
	SetConvex(p->m_Main.Convex);
	InvertColor(p->m_Main.InvertColor);
	LoopHue(p->m_Main.LoopHue);
	SetZoomCenter(p->m_Main.ZoomCenter);
	m_View.SetCopyCount(round(p->m_Master.Copies));
	m_View.SetCopySpread(round(p->m_Master.Spread));
	m_View.FlushHistory();	// prevent glitching
	m_PatchIdx = PatchIdx;
	return(TRUE);
}

DWORD CMainFrame::processFrame(LPVOID pFrame)
{
	double	delta = m_Zoom - m_TargetZoom;
	if (fabs(delta) > 1e-3) {
		m_Zoom -= delta * GetDamping();	// update normalized zoom
		DPoint	p1, p2;
		m_View.GetNormOrigin(p1);
		m_View.SetZoom(m_NormZoom.Denorm(m_Zoom));	// pass zoom to view
		m_View.GetNormOrigin(p2);
		m_TargetOrg -= (p1 - p2);	// zoom can change origin, so compensate target
	}
	switch (m_OrgMotion) {
	case OM_DRAG:
		{
			DPoint	org;
			m_View.GetNormOrigin(org);
			org += (m_TargetOrg - org) * GetDamping();
			m_View.SetNormOrigin(org);
		}
		break;
	case OM_RANDOM:
		{
			if (m_Clock != m_PrevClock) {	// time for a new random target
				m_TargetOrg.x = double(rand()) / RAND_MAX;
				m_TargetOrg.y = double(rand()) / RAND_MAX;
			}
			m_PrevClock = m_Clock;
			DPoint	org;
			m_View.GetNormOrigin(org);
			org += (m_TargetOrg - org) * GetDamping();	// update normalized origin
			m_View.SetNormOrigin(org);	// pass origin to view
		}
		break;
	}
	CParmInfo	Info = m_ParmInfo;
	for (int i = 0; i < CParmInfo::ROWS; i++) {
		Info.m_Row[i].Freq *= m_Speed;	// compensate frequency for master speed
	}
	static const CWhorldView::PARMS	GlobParm;
	m_View.TimerHook(Info, GlobParm, m_Speed * m_GrowDir);
	m_View.Draw(m_hDC);
	// the following assumes pFrame layout matches DIB layout
	if (IsMirrored()) {
		memcpy(pFrame, m_DibBits, m_FrameBytes / 2);	// skip bottom half
		switch (m_VideoInfo.bitDepth) {
		case FF_CAP_16BITVIDEO:
			Mirror16(pFrame, GetWidth(), GetHeight());
			break;
		case FF_CAP_24BITVIDEO:
			Mirror24(pFrame, GetWidth(), GetHeight());
			break;
		case FF_CAP_32BITVIDEO:
			Mirror32(pFrame, GetWidth(), GetHeight());
			break;
		}
	} else
		memcpy(pFrame, m_DibBits, m_FrameBytes);
	return(FF_SUCCESS);
}

void CMainFrame::TimerCallback(LPVOID Cookie)
{
	((CMainFrame *)Cookie)->m_Clock++;	// trigger random jump
}

void CMainFrame::Mirror32(LPVOID pFrame, int w, int h)
{
	int	hw = w >> 1;	// assume width and height are even
	int	hh = h >> 1;
	int	hspan = hw * 4;
	int	whw = w + hw;
	int	x, y;
	// upper right
	DWORD	*src;
	DWORD	*dst;
	src = (DWORD *)pFrame;
	dst = (DWORD *)pFrame + w;
	for (y = 0; y < hh; y++) {
		for (x = 0; x < hw; x++)
			*--dst = *src++;
		src += hw;
		dst += whw;
	}
	// lower left
	src = (DWORD *)pFrame;
	dst = (DWORD *)pFrame + w * h;
	for (y = 0; y < hh; y++) {
		dst -= w;
		memcpy(dst, src, hspan);
		src += w;
	}
	src = (DWORD *)pFrame;
	dst = (DWORD *)pFrame + w * h;
	// lower right
	for (y = 0; y < hh; y++) {
		for (x = 0; x < hw; x++)
			*--dst = *src++;
		src += hw;
		dst -= hw;
	}
}

void CMainFrame::Mirror24(LPVOID pFrame, int w, int h)
{
	int	hw = w >> 1;	// assume width and height are even
	int	hh = h >> 1;
	int	hspan = hw * 3;
	int	whw = w + hw;
	struct PIX24 {
		char	b[3];
	};
	int	x, y;
	// upper right
	PIX24	*src;
	PIX24	*dst;
	src = (PIX24 *)pFrame;
	dst = (PIX24 *)pFrame + w;
	for (y = 0; y < hh; y++) {
		for (x = 0; x < hw; x++)
			*--dst = *src++;
		src += hw;
		dst += whw;
	}
	// lower left
	src = (PIX24 *)pFrame;
	dst = (PIX24 *)pFrame + w * h;
	for (y = 0; y < hh; y++) {
		dst -= w;
		memcpy(dst, src, hspan);
		src += w;
	}
	src = (PIX24 *)pFrame;
	dst = (PIX24 *)pFrame + w * h;
	// lower right
	for (y = 0; y < hh; y++) {
		for (x = 0; x < hw; x++)
			*--dst = *src++;
		src += hw;
		dst -= hw;
	}
}

void CMainFrame::Mirror16(LPVOID pFrame, int w, int h)
{
	int	hw = w >> 1;	// assume width and height are even
	int	hh = h >> 1;
	int	hspan = hw * 2;
	int	whw = w + hw;
	int	x, y;
	// upper right
	WORD	*src;
	WORD	*dst;
	src = (WORD *)pFrame;
	dst = (WORD *)pFrame + w;
	for (y = 0; y < hh; y++) {
		for (x = 0; x < hw; x++)
			*--dst = *src++;
		src += hw;
		dst += whw;
	}
	// lower left
	src = (WORD *)pFrame;
	dst = (WORD *)pFrame + w * h;
	for (y = 0; y < hh; y++) {
		dst -= w;
		memcpy(dst, src, hspan);
		src += w;
	}
	src = (WORD *)pFrame;
	dst = (WORD *)pFrame + w * h;
	// lower right
	for (y = 0; y < hh; y++) {
		for (x = 0; x < hw; x++)
			*--dst = *src++;
		src += hw;
		dst -= hw;
	}
}
