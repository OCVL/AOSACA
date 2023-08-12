#pragma once


#define PUPIL_CAM_WIDTH		320
#define PUPIL_CAM_HEIGHT	240
#include "afxwin.h"
// CPupilView dialog

class CPupilView : public CDialogEx
{
	DECLARE_DYNAMIC(CPupilView)

public:
	CPupilView(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPupilView();
	HWND		hPupilCamWindow;	// handle for capture window
	bool		m_bHasVideoDlg;		// flag to identify whether a camera is present and if a video settingsdialog is present
	bool		OnEditPupilcamerasettings();// function to update camera settings
	
// Dialog Data
	enum { IDD = IDD_PUPILVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	CStatic m_ImageDisp;
};
