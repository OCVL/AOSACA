
// AOSACA.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

// ========== define message handlers for child dialogs ==============
#define WM_UPDATE_WINDOW		WM_USER+1
#define UPDATE_WINDOW			-103
#define UPDATE_WFSIMG_WINDOW	-104
#define UPDATE_WFMAP_PSF_WINDOW	-105
#define UPDATE_DMVMAP_WINDOW	-106
#define UPDATE_RTP_WINDOW		-107
#define UPDATE_PGB_WINDOW		-108
#define UPDATE_PGBD_WINDOW		-109
#define UPDATE_PGBC_WINDOW		-110

#include "resource.h"		// main symbols

//GDI+ related variables
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
//End GDI+ initialization

//ALPAO DM library initialization
#pragma comment(lib, "delayimp")
#pragma comment(lib, "ASDK")

//Application parameters header
#include "AOSACAParams.h"

//Include any other supporting header files
#include "Camera.h"
#include "DMirror.h"
#include "Centroid.h"
#include "OptCalc.h"
#include "ControlPanelDlg.h"
#include "WFSImageDlg.h"
#include "DMMapDlg.h"
#include "WFMapDlg.h"
#include "PSFDlg.h"
#include "SVMapDlg.h"
#include "PupilView.h"
#include "RTPlotDlg.h"
#include "ThreadClass.h"
#include "ProgressDlg.h"
#include "AOSACAParamsDlg.h"

// CAOSACAApp:
// See AOSACA.cpp for the implementation of this class
//

class CAOSACAApp : public CWinApp
{
public:
	CAOSACAApp();	
	void Restart();

	//Variables to limit to only one instace of AOSACA
	HANDLE	m_Mutex_h;
	UINT	m_WinMsg_ui; 

protected:
	//GDI+ variables
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	bool	m_bRestartAppOnExit;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
};

extern CAOSACAApp theApp;