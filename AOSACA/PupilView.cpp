// PupilView.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "PupilView.h"
#include "afxdialogex.h"
#include <Vfw.h>

extern AOSACAParams*g_AOSACAParams;
// CPupilView dialog

IMPLEMENT_DYNAMIC(CPupilView, CDialogEx)

CPupilView::CPupilView(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPupilView::IDD, pParent)
{

}

CPupilView::~CPupilView()
{
	OnClose();
}

void CPupilView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDL_PUPILVIEW, m_ImageDisp);
}


BEGIN_MESSAGE_MAP(CPupilView, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CPupilView message handlers


void CPupilView::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	capPreview(hPupilCamWindow, FALSE);
	CDialogEx::OnClose();
}

bool CPupilView::OnEditPupilcamerasettings()
{
	// TODO: Add your command handler code here
	CAPDRIVERCAPS CapDriverCaps = { }; 
	capDriverGetCaps(hPupilCamWindow, &CapDriverCaps, sizeof(CAPDRIVERCAPS)); 

	// Video source dialog box. 
	if (CapDriverCaps.fHasDlgVideoSource)
	{
		capDlgVideoSource(hPupilCamWindow); 
		return true;
	}
	return false;
}

BOOL CPupilView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
    GetClientRect(&rect);
    CBrush *pOld = pDC->SelectObject(&g_AOSACAParams->m_bkgndbrush);
    BOOL bRes  = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOld);    // restore old brush
    return bRes;
//	return CDialogEx::OnEraseBkgnd(pDC);
}

BOOL CPupilView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_bHasVideoDlg = false;
	m_ImageDisp.MoveWindow(5,5,PUPIL_CAM_WIDTH,PUPIL_CAM_HEIGHT, 1);
	hPupilCamWindow = capCreateCaptureWindow(_T("PupilCamView Handle"), WS_CHILD | WS_VISIBLE, 0, 0, PUPIL_CAM_WIDTH, PUPIL_CAM_HEIGHT, m_ImageDisp, 1);
    // connect the driver with the camera 
	if (capDriverConnect(hPupilCamWindow,0))
	{	 
		LPBITMAPINFO lpbi;
		DWORD dwSize;
		dwSize = capGetVideoFormatSize(hPupilCamWindow);
		lpbi = (LPBITMAPINFO)malloc (dwSize);
		capGetVideoFormat(hPupilCamWindow, lpbi, dwSize); 
		lpbi->bmiHeader.biHeight = PUPIL_CAM_HEIGHT;
		lpbi->bmiHeader.biWidth = PUPIL_CAM_WIDTH;
		lpbi->bmiHeader.biSizeImage = PUPIL_CAM_HEIGHT*PUPIL_CAM_WIDTH*(lpbi->bmiHeader.biBitCount/8);
		capSetVideoFormat( hPupilCamWindow,lpbi, dwSize);
		CAPTUREPARMS CaptureParms;
		capCaptureGetSetup(hPupilCamWindow, &CaptureParms, sizeof(CAPTUREPARMS));
		CaptureParms.dwAudioBufferSize = 0;
		CaptureParms.fCaptureAudio = FALSE;
		capCaptureSetSetup(hPupilCamWindow, &CaptureParms, sizeof (CAPTUREPARMS));
		CAPDRIVERCAPS CapDriverCaps = { }; 
		capDriverGetCaps(hPupilCamWindow, &CapDriverCaps, sizeof(CAPDRIVERCAPS)); 
		m_bHasVideoDlg = CapDriverCaps.fHasDlgVideoSource?true:false;
		capPreviewRate(hPupilCamWindow, 24);     // rate, in milliseconds
		capPreview(hPupilCamWindow, TRUE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

