// ControlPanelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "ControlPanelDlg.h"
#include "afxdialogex.h"

extern AOSACAParams	*g_AOSACAParams;
extern CWFSImageDlg	*g_wfsimg;
extern CCamera		*g_camera;
extern CCentroid	*g_centroids;
extern COptCalc		*g_optcalc;
// CControlPanelDlg dialog

IMPLEMENT_DYNAMIC(CControlPanelDlg, CDialogEx)

CControlPanelDlg::CControlPanelDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CControlPanelDlg::IDD, pParent)
{	
	m_pParent=(CAOSACADlg *)pParent;
	m_bCPenalizeTilt = FALSE;
	m_bCSubBkgndinLive = FALSE;
	m_bCRenewPmat = FALSE;
	m_bCAutoMeasure = FALSE;
	m_bCAOtoggle = FALSE;
	m_bCPreCorrApply = FALSE;
}

CControlPanelDlg::~CControlPanelDlg()
{
	OnClose();
}

void CControlPanelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
	DDX_Check(pDX, IDC_CONTROL_CAM_SUBBKGND, m_bCSubBkgndinLive);
	DDX_Check(pDX, IDC_CONTROL_ANALYSIS_PENALIZETIPTILT, m_bCPenalizeTilt);
	DDX_Check(pDX, IDC_CONTROL_ANALYSIS_RENEWPMAT, m_bCRenewPmat);
	DDX_Check(pDX, IDC_CONTROL_ANALYSIS_AUTO, m_bCAutoMeasure);
	DDX_Check(pDX, IDC_CONTROL_DM_AOONOFF, m_bCAOtoggle);
	DDX_Control(pDX, IDB_CONTROL_CAM_LIVE, m_CamLive);
	DDX_Control(pDX, IDB_CONTROL_CAM_SNAP, m_CamSnap);
	DDX_Control(pDX, IDB_CONTROL_CAM_SAVEBKGND, m_SaveBkgnd);
	DDX_Control(pDX, IDB_CONTROL_ANALYSIS_SUBBKGND, m_SubBkgnd);
	DDX_Control(pDX, IDB_CONTROL_ANALYSIS_CENTROIDS, m_FindCentroids);
	DDX_Control(pDX, IDB_CONTROL_ANALYSIS_OPTPERF, m_Refraction);
	DDX_Control(pDX, IDB_CONTROL_DM_CLOOP, m_CLoopToggle);
	DDX_Control(pDX, IDB_CONTROL_DM_CLOOPSTOP, m_CLoopToggleStop);
	DDX_Control(pDX, IDB_CONTROL_DM_FLAT, m_FlatDM);
	DDX_Control(pDX, IDB_CONTROL_ICANDI_LIVE, m_ICANDILive);
	DDX_Control(pDX, IDB_CONTROL_ICANDI_RECORD, m_ICANDIRec);
	DDX_Control(pDX, IDS_CONTROL_CAM_EXP, m_SCamExp);
	DDX_Control(pDX, IDS_CONTROL_CAM_GAIN, m_SCamGain);
	DDX_Control(pDX, IDS_CONTROL_ANALYSIS_THRESHOLD, m_SThreshold);
	DDX_Control(pDX, IDS_CONTROL_CLOOPPARAMS_MINCENT, m_SMinCent);
	DDX_Control(pDX, IDS_CONTROL_CLOOPPARAMS_INTGGAIN, m_SIntGain);
	DDX_Control(pDX, IDS_CONTROL_CLOOPPARAMS_ZEROMODES, m_SModestoZero);
	DDX_Control(pDX, IDS_CONTROL_CLOOPPARAMS_PUPILSIZE, m_SPupilSize);
	DDX_Control(pDX, IDS_CONTROL_REFRACTION_SPHERE, m_SPreCorrection_Def);
	DDX_Control(pDX, IDS_CONTROL_REFRACTION_CYL, m_SPreCorrection_Cyl);
	DDX_Control(pDX, IDS_CONTROL_REFRACTION_AXIS, m_SPreCorrection_Axis);
	DDX_Control(pDX, IDS_CONTROL_DEFOCUS_FIXEDDEF, m_SFixedDef);
	DDX_Control(pDX, IDC_CONTROL_ANALYSIS_AUTO, m_CAutoRefraction);
	DDX_Control(pDX, IDC_CONTROL_DM_AOONOFF, m_CAOtoggle);
	DDX_Check(pDX, IDC_CONTROL_REFRACTION_APPLY, m_bCPreCorrApply);
	DDX_Control(pDX, IDC_CONTROL_REFRACTION_APPLY, m_CApplyRefCorr);
}


BEGIN_MESSAGE_MAP(CControlPanelDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDB_CONTROL_CAM_LIVE, &CControlPanelDlg::OnBnClickedControlCamLive)
	ON_BN_CLICKED(IDB_CONTROL_CAM_SNAP, &CControlPanelDlg::OnBnClickedControlCamSnap)
	ON_BN_CLICKED(IDC_CONTROL_CAM_SUBBKGND, &CControlPanelDlg::OnBnClickedControlCamSubBkgndinLive)
	ON_BN_CLICKED(IDB_CONTROL_CAM_SAVEBKGND, &CControlPanelDlg::OnBnClickedControlCamSavebkgnd)
	ON_BN_CLICKED(IDB_CONTROL_ANALYSIS_SUBBKGND, &CControlPanelDlg::OnBnClickedControlAnalysisSubbkgnd)
	ON_BN_CLICKED(IDB_CONTROL_ANALYSIS_CENTROIDS, &CControlPanelDlg::OnBnClickedControlAnalysisCentroids)
	ON_BN_CLICKED(IDB_CONTROL_ANALYSIS_OPTPERF, &CControlPanelDlg::OnBnClickedControlAnalysisOptperf)
	ON_BN_CLICKED(IDB_CONTROL_DM_CLOOP, &CControlPanelDlg::OnBnClickedControlDmCloop)
	ON_BN_CLICKED(IDB_CONTROL_DM_FLAT, &CControlPanelDlg::OnBnClickedControlDmFlat)
	ON_BN_CLICKED(IDB_CONTROL_ICANDI_LIVE, &CControlPanelDlg::OnBnClickedControlIcandiLive)
	ON_BN_CLICKED(IDB_CONTROL_ICANDI_RECORD, &CControlPanelDlg::OnBnClickedControlIcandiRecord)
	ON_BN_CLICKED(IDB_CONTROL_ICANDI_STAB_START, &CControlPanelDlg::OnBnClickedControlIcandiStabStart)
	ON_BN_CLICKED(IDB_CONTROL_ICANDI_STAB_RESET, &CControlPanelDlg::OnBnClickedControlIcandiStabReset)
	ON_BN_CLICKED(IDC_CONTROL_ANALYSIS_PENALIZETIPTILT, &CControlPanelDlg::OnBnClickedControlAnalysisPenalizetiptilt)
	ON_BN_CLICKED(IDC_CONTROL_ANALYSIS_RENEWPMAT, &CControlPanelDlg::OnBnClickedControlAnalysisRenewpmat)
	ON_BN_CLICKED(IDC_CONTROL_ANALYSIS_AUTO, &CControlPanelDlg::OnBnClickedControlAnalysisAuto)
	ON_BN_CLICKED(IDC_CONTROL_DM_AOONOFF, &CControlPanelDlg::OnBnClickedControlDmAoonoff)
	ON_BN_CLICKED(IDB_CONTROL_DM_CLOOPSTOP, &CControlPanelDlg::OnBnClickedControlDmCloopstop)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CONTROL_REFRACTION_APPLY, &CControlPanelDlg::OnBnClickedControlRefractionApply)
END_MESSAGE_MAP()


// CControlPanelDlg message handlers
BOOL CControlPanelDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString text;
	RECT rect;

	m_editbkgndbrush.CreateSolidBrush(EDITBACKGROUNDCOLOR);	
	m_frgndbrush.CreateSolidBrush(FOREGROUNDCOLOR);

	m_bImaging = false;
	m_bSubBkgnd = false;
	m_DefocusStepSize = 0.025;
	m_FixedDefocusValue = 0.;	
	m_MinCent = 0;
	m_IntGain = g_optcalc->get_Gain();
	m_ModestoZero = 12;
	m_PupilSize = g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS;
	m_fMinCentPercent = 1.;
	m_SvBkgndExp = g_AOSACAParams->EXPOSURE_MS;
	m_Duration = 15.;
	m_nGrabCount = 0;
	m_PreDefocusValue=m_PreCylinderValue=m_PreAxisValue=0.;
	m_bFlatMirror = true;

	m_CamLive.LoadBitmaps(IDB_LIVEU, IDB_LIVED, NULL, IDB_LIVEX);
	m_CamSnap.LoadBitmaps(IDB_SNAPU, IDB_SNAPD, NULL, IDB_SNAPX);
	m_SaveBkgnd.LoadBitmaps(IDB_SAVEBKGNDU, IDB_SAVEBKGNDD, NULL, IDB_SAVEBKGNDX);
	m_SubBkgnd.LoadBitmaps(IDB_SUBBKGNDU, IDB_SUBBKGNDD, NULL, IDB_SUBBKGNDX);
	m_FindCentroids.LoadBitmaps(IDB_CENTROIDSU, IDB_CENTROIDSD, NULL, IDB_CENTROIDSX);
	m_Refraction.LoadBitmaps(IDB_REFRACTIONU, IDB_REFRACTIOND, NULL, IDB_REFRACTIONX);
	m_CLoopToggle.LoadBitmaps(IDB_CLOOPU, IDB_CLOOPD, NULL, IDB_CLOOPX);
	m_CLoopToggleStop.LoadBitmaps(IDB_CLCLOSEDU, IDB_CLCLOSEDD, NULL, NULL);
	m_FlatDM.LoadBitmaps(IDB_FLATDMU, IDB_FLATDMD, NULL, NULL);
	m_FlatDM.EnableWindow(g_AOSACAParams->g_bDMReady);
	m_ICANDILive.LoadBitmaps(IDB_LIVEU, IDB_LIVED, NULL, IDB_LIVEX);
	m_ICANDIRec.LoadBitmaps(IDB_RECORDU, IDB_RECORDD, NULL, IDB_RECORDX);
	
	rect.left = 31;	rect.right  = rect.left+ 40;
	rect.top  = 48; rect.bottom = rect.top + 20;
	m_eCamExp.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_CAM_EXP);

	rect.left = 56;	rect.right  = rect.left+ 30;
	rect.top  = rect.bottom+79; rect.bottom = rect.top + 20;
	m_eCamGain.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | !WS_VISIBLE,  
					 rect, this, IDE_CONTROL_CAM_GAIN);

	rect.left = 211;	rect.right  = rect.left+ 24;
	rect.top  = 120; rect.bottom = rect.top + 20;
	m_eCentThreshold.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_ANALYSIS_THRESHOLD);

	rect.left = 280;	rect.right  = rect.left+ 55;
	rect.top  = 64; rect.bottom = rect.top + 20;
	m_eMinCent.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_CLOOPPARAMS_MINCENT);

	rect.top  = rect.bottom+30; rect.bottom = rect.top + 20;
	m_eIntGain.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_CLOOPPARAMS_INTGGAIN);

	rect.top  = rect.bottom+30; rect.bottom = rect.top + 20;
	m_eModestoZero.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_CLOOPPARAMS_ZEROMODES);

	rect.top  = rect.bottom+30; rect.bottom = rect.top + 20;
	m_ePupilSize.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_CLOOPPARAMS_PUPILSIZE);

	rect.left = 10;	rect.right  = rect.left+ 45;
	rect.top  = 225; rect.bottom = rect.top + 20;
	m_ePreCorrection_Def.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_REFRACTION_SPHERE);

	rect.left = rect.right+24;	rect.right  = rect.left+ 42;	
	m_ePreCorrection_Cyl.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_REFRACTION_CYL);

	rect.left  = rect.right+25; rect.right = rect.left + 32;
	m_ePreCorrection_Axis.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_REFRACTION_AXIS);	
		
	rect.left = 12;	rect.right  = rect.left+ 45;
	rect.top  = rect.bottom+50; rect.bottom = rect.top + 20;
	m_eFixedDef.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_DEFOCUS_FIXEDDEF);

	rect.left = rect.right+28;	rect.right  = rect.left+ 45;
	m_eDefStep.Create(WS_CHILD | WS_VISIBLE | ES_RIGHT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_DEFOCUS_STEPSIZE);
	
					 
	rect.left = 168;	rect.right  = rect.left+ 65;
	rect.top  = 368; rect.bottom = rect.top + 20;
	m_ePrefix.Create(WS_CHILD | WS_VISIBLE | ES_CENTER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_CONTROL_ICANDI_RECORD_PREFIX);
	
	rect.top  = rect.bottom+3; rect.bottom = rect.top + 20;
	m_eLength.Create(WS_CHILD | WS_VISIBLE | ES_CENTER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_NUMBER,  
					 rect, this, IDE_CONTROL_ICANDI_RECORD_DURATION);

	m_eCamExp.SetValidation(MYEDIT_INTEGER, g_AOSACAParams->EXPOSURE_MS, 15, 180, 3, -1, false);
	m_eCamGain.SetValidation(MYEDIT_INTEGER, g_AOSACAParams->CAMGAIN_DB, 8, 30, 2, -1, false);
	m_eCentThreshold.SetValidation(MYEDIT_INTEGER, g_AOSACAParams->THRESHOLD, 1, 255, 3, -1, false);
	m_eMinCent.SetValidation(MYEDIT_INTEGER, m_MinCent, 0, 0, 3, false) ;
	m_eIntGain.SetValidation(MYEDIT_DOUBLE, m_IntGain, 0, 1, 1, 2, false);
	m_eModestoZero.SetValidation(MYEDIT_INTEGER, m_ModestoZero, 0, 50, 2, 0, false);
	m_ePupilSize.SetValidation(MYEDIT_DOUBLE, m_PupilSize/1000., 4, g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS/1000., 1, 1, false);
	m_ePreCorrection_Def.SetValidation(MYEDIT_DOUBLE, m_PreDefocusValue, -9, 9, 1, 2, true);
	m_ePreCorrection_Cyl.SetValidation(MYEDIT_DOUBLE, m_PreCylinderValue, -9, 9, 1, 2, true);
	m_ePreCorrection_Axis.SetValidation(MYEDIT_INTEGER, m_PreAxisValue, 0, 179, 3, 0, false);
	m_eFixedDef.SetValidation(MYEDIT_DOUBLE, m_FixedDefocusValue, -2, 2, 1, 3, true);
	m_eDefStep.SetValidation(MYEDIT_DOUBLE, m_DefocusStepSize, 0.005, 0.1, 1, 3, false);
	m_ePrefix.SetValidation(MYEDIT_ALPHANUMERIC);
	m_eLength.SetValidation(MYEDIT_INTEGER, 15, 1, 120, 3, -1, false);

	m_bCSubBkgndinLive	= FALSE;
	m_bCPenalizeTilt	= FALSE;
	m_bCRenewPmat		= FALSE;
	m_bCAutoMeasure		= FALSE;
	m_bCAOtoggle		= FALSE;
	m_bSvBkgndExpChange = FALSE;
	m_bCPreCorrApply	= FALSE;
	
	m_bFlags	= new bool[13];
	// Flags for setting icons on the various buttons in this dialog box
	m_bFlags[0] = true;		//Liveshow / Snapshot / Exposure / SaveBackground
	m_bFlags[1] = false; 	//Subtract Background Button
	m_bFlags[2] = false;	//Centroids Button
	m_bFlags[3] = false;	//Optical Performance Button
	m_bFlags[4] = false; 	//AutoCorrection CheckBox
	m_bFlags[5] = g_AOSACAParams->g_bDMReady;	//Min Cent
	m_bFlags[6] = false;		//Pre def,cyl,axis
	m_bFlags[7] = g_AOSACAParams->g_bDMReady;	//fixed def
	m_bFlags[8] = g_AOSACAParams->g_bDMReady;	//Closed loop
	m_bFlags[9] = g_AOSACAParams->g_bDMReady;	//AO On/Off toggle
	m_CLoopToggleStop.ShowWindow(false);
	m_CLoopToggle.ShowWindow(true);
	m_CLoopToggle.EnableWindow( g_AOSACAParams->g_bDMReady );
	m_CApplyRefCorr.EnableWindow( g_AOSACAParams->g_bDMReady );

	m_bRunning = true;
	m_ehUpdateDialog = ::CreateEvent(NULL, false, true, NULL);
	m_ehCamSnap = ::CreateEvent(NULL, false, false, NULL);
	m_ehCPanelThreadClose = ::CreateEventA(NULL, false, false, NULL);
	m_ehCPanelThreadClosed = ::CreateEventA(NULL, false, false, NULL);
	Thread_Dialog_Controls = CreateThread(NULL,0,ControlPanelThread,this,0,&ThreadId_Dialog_Controls);
	SetThreadPriority(Thread_Dialog_Controls, THREAD_PRIORITY_LOWEST);
	g_AOSACAParams->g_last_click = LIVE;

	UpdateData(false);	
	return true;
}

void CControlPanelDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	SetEvent(m_ehCPanelThreadClose);
	::WaitForSingleObject(m_ehCPanelThreadClosed, INFINITE);
	CloseHandle(m_ehUpdateDialog);
	CloseHandle(m_ehCamSnap);
	CloseHandle(m_ehCPanelThreadClose);
	CloseHandle(m_ehCPanelThreadClosed);
	CDialogEx::OnClose();
}


void CControlPanelDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	int		ScrollID;
	CString text;
	BOOL	direction = -1;

	ScrollID=((CWnd *)(pScrollBar))->GetDlgCtrlID();
	if (nSBCode == SB_LINEUP)
		direction = 1;
	else if (nSBCode == SB_LINEDOWN)
		direction = 0;
	if (direction != -1)
	{
		switch(ScrollID)
		{
		case IDS_CONTROL_CAM_EXP:
			g_AOSACAParams->EXPOSURE_MS = (short)(m_eCamExp.UpdateValue(direction, 1));
			g_camera->UpdateExposureTime();
			g_AOSACAParams->g_last_click != LIVE?Get_SnapShot():0;
			break;
		case IDS_CONTROL_CAM_GAIN:
			g_AOSACAParams->CAMGAIN_DB = (BYTE)(m_eCamGain.UpdateValue(direction, 1));
			g_camera->UpdateCameraGain();
			break;
		case IDS_CONTROL_ANALYSIS_THRESHOLD:
			g_AOSACAParams->THRESHOLD = (short)(m_eCentThreshold.UpdateValue(direction, 1));			
			break;
		case IDS_CONTROL_CLOOPPARAMS_MINCENT:
			m_MinCent = (short)(m_eMinCent.UpdateValue(direction, 1));
			m_fMinCentPercent = (float)m_MinCent/(float)g_centroids->get_TotalCentCount();
			g_centroids->set_MinCentroids(m_MinCent);
			break;
		case IDS_CONTROL_CLOOPPARAMS_INTGGAIN:
			m_IntGain = m_eIntGain.UpdateValue(direction, 0.01);
			g_optcalc->set_Gain(m_IntGain);
			break;
		case IDS_CONTROL_CLOOPPARAMS_ZEROMODES:
			m_ModestoZero = (short)m_eModestoZero.UpdateValue(direction, 1.);
			g_centroids->set_ModestoZero(m_ModestoZero);
			break;
		case IDS_CONTROL_CLOOPPARAMS_PUPILSIZE:
			m_PupilSize = m_ePupilSize.UpdateValue(direction, 0.1)*1000.;
			m_MinCent = g_centroids->Update_bUse_centroid(m_PupilSize);
			g_optcalc->UpdateUserParameters(m_PupilSize);
			SetDlgItemInt(IDE_CONTROL_CLOOPPARAMS_MINCENT, m_MinCent, false);
			g_centroids->set_MinCentroids(m_MinCent);
			break;
		case IDS_CONTROL_REFRACTION_SPHERE:
			m_PreDefocusValue=m_ePreCorrection_Def.UpdateValue(direction, 0.05);
			send_DM_update_PreCorrection();
			break;
		case IDS_CONTROL_REFRACTION_CYL:
			m_PreCylinderValue=m_ePreCorrection_Cyl.UpdateValue(direction, 0.05);
			send_DM_update_PreCorrection();
			break;
		case IDS_CONTROL_REFRACTION_AXIS:
			m_PreAxisValue=m_ePreCorrection_Axis.UpdateValue(direction, 5);
			send_DM_update_PreCorrection();
			break;
		case IDS_CONTROL_DEFOCUS_FIXEDDEF:			
			m_FixedDefocusValue=m_eFixedDef.UpdateValue(direction, m_DefocusStepSize);
			send_DM_update_Defocus(m_FixedDefocusValue);
			break;		
		}
	}

	//CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}


BOOL CControlPanelDlg::OnEraseBkgnd(CDC* pDC)
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


void CControlPanelDlg::OnBnClickedControlCamLive()
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_WAIT)));
	if ( g_AOSACAParams->g_last_click != LIVE )
		SetEvent(g_AOSACAParams->g_ehCamLive);
	g_AOSACAParams->g_last_click = g_AOSACAParams->g_cur_click = LIVE;
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_ARROW)));
}

void CControlPanelDlg::OnBnClickedControlCamSnap()
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_WAIT)));
	g_AOSACAParams->g_cur_click = SNAP;
	SetEvent(m_ehCamSnap);
}

void CControlPanelDlg::OnBnClickedControlCamSubBkgndinLive()
{
	// TODO: Add your control notification handler code here	
	UpdateData(true);
	g_camera->UpdateSubBkgndinLiveflag(m_bCSubBkgndinLive);
}


void CControlPanelDlg::OnBnClickedControlCamSavebkgnd()
{
	// TODO: Add your control notification handler code here
	g_AOSACAParams->g_stAppErrBuff.Empty();
	g_AOSACAParams->g_stAppErrBuff = _T("Would you like to save current image as background?");
	int ret = g_AOSACAParams->ShowError(MB_ICONQUESTION);
	if (ret == 6)
	{
		SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_WAIT)));
		switch(g_AOSACAParams->g_last_click)
		{
		case LIVE:
		case SUBBKGND:		
		case SELCENT:		
		case CENTROIDS:
		case OPTPERFORM:
			m_SvBkgndExp = g_AOSACAParams->EXPOSURE_MS;
			m_bSvBkgndExpChange = false;
			Get_SnapShot();
		case SNAP:
			break;
		}
		CStringA filename = "\\utils\\background.tif";
		if (g_camera->SaveImage(filename, true))
		{		
			g_AOSACAParams->g_stAppErrBuff.Empty();
			g_AOSACAParams->g_stAppErrBuff = _T("Current Image saved as background");
			g_AOSACAParams->ShowError(MB_ICONINFORMATION);
		}
		m_pParent->UpdateMouseMode(MOUSE_NORMAL);
		SetEvent(m_ehUpdateDialog);
	}
}


void CControlPanelDlg::OnBnClickedControlAnalysisSubbkgnd()
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_WAIT)));
	if (g_AOSACAParams->g_last_click == SNAP)
	{		
		SubbkgndButton();
		SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_ARROW)));
	}
	else
	{
		g_AOSACAParams->g_cur_click = SUBBKGND;
		SetEvent(m_ehCamSnap);	
	}
}

void CControlPanelDlg::SubbkgndButton()
{
	SubBkgnd();
	if (g_AOSACAParams->PUPIL_CENTER.x > 120)
	{
		m_pParent->UpdateMouseMode(MOUSE_SELECT_CENTER);
		m_pParent->lBtnUp_ImageDlg(g_AOSACAParams->PUPIL_CENTER);
	}
	else
	{		
		m_pParent->SendMessage(WM_UPDATE_WINDOW, 0, UPDATE_WFSIMG_WINDOW);			
		m_pParent->UpdateMouseMode(MOUSE_NORMAL);
		m_bFlags[2] = false; // Disable centroids button
	}
	m_pParent->m_bMenuFlags[1] = true;//Update Pupil center menu id
	m_pParent->OnUpdateMenu();
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_ARROW)));
	SetEvent(m_ehUpdateDialog);
}


void CControlPanelDlg::OnBnClickedControlAnalysisCentroids()
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_WAIT)));
	if (g_AOSACAParams->g_last_click == LIVE || g_AOSACAParams->g_last_click == CENTROIDS || g_AOSACAParams->g_last_click == OPTPERFORM || g_AOSACAParams->g_last_click == AUTO)
	{		
		g_AOSACAParams->g_cur_click = CENTROIDS;
		SetEvent(m_ehCamSnap);
	}
	else
	{
		CentroidsButton();
	}
}

void CControlPanelDlg::CentroidsButton()
{
	switch(g_AOSACAParams->g_last_click)
		{
		case SNAP:
			SubBkgnd();
		case SUBBKGND:
			break;
		case CENTROIDS:
			break;
		}
	if (!FindCentroids())
		return;
	m_pParent->m_bMenuFlags[4] = g_AOSACAParams->g_bDMReady;//Generate poke matrix menu id
	m_pParent->OnUpdateMenu();
	m_pParent->UpdateMouseMode(MOUSE_NORMAL);				
	m_bFlags[3] = true;//Optical Performance Button
	g_AOSACAParams->g_last_click = CENTROIDS;
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_ARROW)));
	SetEvent(m_ehUpdateDialog);
}


void CControlPanelDlg::OnBnClickedControlAnalysisOptperf()
{
	// TODO: Add your control notification handler code here
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_WAIT)));
	if (g_AOSACAParams->g_last_click == LIVE || g_AOSACAParams->g_last_click == OPTPERFORM || g_AOSACAParams->g_last_click == AUTO)
	{
		g_AOSACAParams->g_cur_click = OPTPERFORM;
		SetEvent(m_ehCamSnap);
	}
	else
	{
		OptPerfButton();
		SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_ARROW)));	
		SetEvent(m_ehUpdateDialog);
	}
}

void CControlPanelDlg::OptPerfButton()
{
	switch(g_AOSACAParams->g_last_click)
	{
	case SNAP:
		SubBkgnd();
	case SUBBKGND:
		FindCentroids();		
	case CENTROIDS:
		break;
	}
	m_pParent->Opt_Perform();
	m_pParent->Update_Displays();
	m_bFlags[4] = true; 	//AutoCorrection CheckBox
	m_bFlags[6] = true; 	//Refraction correction
	m_bFlags[8] = true;		//Closed loop
	m_bFlags[9] = true;		//AO On/Off toggle
	if (!g_centroids->get_Reconflag() 
		&& !g_centroids->get_MinCentReady() )
		m_bFlags[8] = false;

	g_AOSACAParams->g_last_click = OPTPERFORM;	
	SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_ARROW)));
	SetEvent(m_ehUpdateDialog);	
}

void CControlPanelDlg::OnBnClickedControlDmCloop()
{
	// TODO: Add your control notification handler code here
	// Hide CLoop button
	m_CLoopToggle.ShowWindow(FALSE);
	// Unhide CLoop stop button
	m_CLoopToggleStop.ShowWindow(TRUE);
	g_AOSACAParams->g_bControlON = true;
	// Update dialog controls
	m_bFlags[0] = false;	//Liveshow / Snapshot / Exposure / SaveBackground
	m_bFlags[1] = false; 	//Subtract Background Button
	m_bFlags[2] = false;	//Centroids Button
	m_bFlags[3] = false;	//Optical Performance Button
	m_bFlags[4] = false; 	//AutoCorrection CheckBox
	m_bFlags[5] = true;		//Min Cent
	m_bFlags[6] = false;	//Pre def,cyl,axis
	m_bFlags[7] = true;		//fixed def
	m_bFlags[8] = true;		//Closed loop
	m_bFlags[9] = false;	//AO On/Off toggle
	SetEvent(m_ehUpdateDialog);
	// Start closed loop thread
	m_pParent->StartCLoopThread();
}


void CControlPanelDlg::OnBnClickedControlDmCloopstop()
{
	// TODO: Add your control notification handler code here
	// Stop closed loop thread
	m_pParent->StopCLoopThread();
	// Hide CLoop stop button
	m_CLoopToggleStop.ShowWindow(FALSE);
	// Unhide CLoop button
	m_CLoopToggle.ShowWindow(TRUE);
	// Update dialog controls
	m_bFlags[0] = true;		//Liveshow / Snapshot / Exposure / SaveBackground
	m_bFlags[1] = true; 	//Subtract Background Button
	m_bFlags[2] = true;		//Centroids Button
	m_bFlags[3] = true;		//Optical Performance Button
	m_bFlags[4] = false; 	//AutoCorrection CheckBox
	m_bFlags[5] = true;		//Min Cent
	m_bFlags[6] = true;		//Pre def,cyl,axis
	m_bFlags[7] = true;		//fixed def
	m_bFlags[8] = false;	//Closed loop
	m_bFlags[9] = true;		//AO On/Off toggle
	SetEvent(m_ehUpdateDialog);
}


void CControlPanelDlg::OnBnClickedControlDmFlat()
{
	// TODO: Add your control notification handler code here
//	(m_CZeroVolts.GetCheck())?m_CZeroVolts.SetCheck(false):0;
	if (g_AOSACAParams->g_bControlON)
		g_AOSACAParams->g_bResetMirror = true;	
	
	g_optcalc->Send_Voltages(BIAS_BIT);		
	m_pParent->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
	m_pParent->ResetZeroDMMenu();
}


void CControlPanelDlg::OnBnClickedControlIcandiLive()
{
	// TODO: Add your control notification handler code here
}


void CControlPanelDlg::OnBnClickedControlIcandiRecord()
{
	// TODO: Add your control notification handler code here
}


void CControlPanelDlg::OnBnClickedControlIcandiStabStart()
{
	// TODO: Add your control notification handler code here
}


void CControlPanelDlg::OnBnClickedControlIcandiStabReset()
{
	// TODO: Add your control notification handler code here
}


void CControlPanelDlg::OnBnClickedControlAnalysisPenalizetiptilt()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	g_centroids-> m_bPenalizeTipTilt= m_bCPenalizeTilt?true:false;
	g_centroids->Generate_PTPmat();
}


void CControlPanelDlg::OnBnClickedControlAnalysisRenewpmat()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if(m_bCRenewPmat)
	{ 
		g_centroids-> m_bRenewPmat= true;
	}
	else
	{		
		g_centroids-> m_bRenewPmat= false;
		g_centroids->Generate_PTPmat();
	}
}


void CControlPanelDlg::OnBnClickedControlAnalysisAuto()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if(m_bCAutoMeasure)
	{
		m_bFlags[0] = m_bFlags[1] = m_bFlags[2] = m_bFlags[3] = m_bFlags[8] = m_bFlags[9] = false;
		m_bFlags[4] = m_bFlags[5] = m_bFlags[6] = m_bFlags[7] = true;
		g_AOSACAParams->g_bControlON = false;
		g_AOSACAParams->g_last_click = AUTO;
		m_pParent->StartCLoopThread();
	}
	else
	{
		g_AOSACAParams->g_last_click = OPTPERFORM;		
		m_pParent->StopCLoopThread();
		m_bFlags[0] = m_bFlags[1] = m_bFlags[2] = m_bFlags[3] = m_bFlags[4] = m_bFlags[5] = m_bFlags[6] = m_bFlags[7] = m_bFlags[8] = m_bFlags[9] = true;	
	}
	
	SetEvent(m_ehUpdateDialog);
}


void CControlPanelDlg::OnBnClickedControlRefractionApply()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	g_AOSACAParams->g_bCPreCorrApply = m_bCPreCorrApply;
	g_optcalc->UpdateDMFlatData();
	
	m_pParent->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
}


void CControlPanelDlg::OnBnClickedControlDmAoonoff()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (m_bCAOtoggle)
		g_optcalc->AO_Off(true);
	else
		g_optcalc->AO_Off(false);
	m_pParent->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
}

void CControlPanelDlg::createDirectory()
{
	CString val, initials;
	GetDlgItemText(IDE_CONTROL_ICANDI_RECORD_PREFIX, initials);
	initials.TrimLeft();
	initials.TrimRight();	
	SYSTEMTIME systime;
	GetLocalTime(&systime);
}

bool CControlPanelDlg::FindCentroids()
{	
	m_pParent->init_Centroid_Detection();
	g_AOSACAParams->g_last_click = CENTROIDS;
	g_AOSACAParams->g_bDrawCentroids?g_wfsimg->m_bDrawCentroids = true:g_wfsimg->m_bDrawCentroids = false;
	g_wfsimg->SendMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
	return true;
}

void CControlPanelDlg::Get_SnapShot()
{	
	SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
	m_pParent->SetDlgItemText(IDE_FREQUENCY, _T("-"));
	g_AOSACAParams->g_bSubstractBkGnd = false;
	if (g_AOSACAParams->g_last_click == LIVE)
	{
		CEvent waitEvent;
		ResetEvent(g_AOSACAParams->g_ehCamLive);
		SetEvent(g_AOSACAParams->g_ehCamSnap);
		WaitForSingleObject(waitEvent, g_AOSACAParams->EXPOSURE_MS<<1);
	}
	else
		SetEvent(g_AOSACAParams->g_ehCamSnap);
	::WaitForSingleObject(g_AOSACAParams->g_ehCamNewFrame, g_AOSACAParams->EXPOSURE_MS<<1);
	ResetEvent(g_AOSACAParams->g_ehCamNewFrame);
	g_AOSACAParams->g_last_click = SNAP;
}

void CControlPanelDlg::send_DM_update_PreCorrection()
{
	//Check if the apply checkbox is checked
	if (!m_bCPreCorrApply)
	{
		g_AOSACAParams->g_bCPreCorrApply = m_bCPreCorrApply = TRUE;
		UpdateData(FALSE);
	}
	// Add the new defocus & cylinder to the mirror
	g_optcalc->PreCorrection(m_PreDefocusValue,m_PreCylinderValue, m_PreAxisValue, g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
	
	if (g_AOSACAParams->g_bCPreCorrApply)	// Refresh the DM Map to reflect new voltages
		m_pParent->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
}

void CControlPanelDlg::send_DM_update_Defocus(double def)
{
	// Add the new defocus to the mirror
	g_optcalc->Induce_Abberations(NULL, def, g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
	// Refresh the DM Map to reflect new voltages
	m_pParent->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
}

DWORD WINAPI CControlPanelDlg::ControlPanelThread (LPVOID param)
{
	CControlPanelDlg *parent = (CControlPanelDlg *)param;
	CString text;	
	CEvent waitEvent;
	HANDLE hCamEvents[3];
	hCamEvents[0] = parent->m_ehUpdateDialog;
	hCamEvents[1] = parent->m_ehCamSnap;
	hCamEvents[2] = parent->m_ehCPanelThreadClose;

	do
	{
		switch (::WaitForMultipleObjects(3, hCamEvents, FALSE, INFINITE)) 
		{
		case WAIT_OBJECT_0:
			parent->m_eCamExp.EnableWindow(parent->m_bFlags[0]);
			parent->m_SCamExp.EnableWindow(parent->m_bFlags[0]);		
			parent->m_CamLive.EnableWindow(parent->m_bFlags[0]);
			parent->m_CamSnap.EnableWindow(parent->m_bFlags[0]);
			parent->m_SaveBkgnd.EnableWindow(parent->m_bFlags[0]);
			parent->m_eCamGain.EnableWindow(parent->m_bFlags[0]);
			parent->m_SCamGain.EnableWindow(parent->m_bFlags[0]);
			parent->m_SubBkgnd.EnableWindow(parent->m_bFlags[1]);
			parent->m_FindCentroids.EnableWindow(parent->m_bFlags[2]);
			parent->m_Refraction.EnableWindow(parent->m_bFlags[3]);
			parent->m_CAutoRefraction.EnableWindow(parent->m_bFlags[4]);
			if (!parent->m_bFlags[4] && parent->m_bCAutoMeasure)
				parent->m_CAutoRefraction.SetCheck(FALSE);
			parent->m_eMinCent.EnableWindow(parent->m_bFlags[5] && g_AOSACAParams->g_bDMReady);
			parent->m_SMinCent.EnableWindow(parent->m_bFlags[5] && g_AOSACAParams->g_bDMReady);
			parent->m_ePreCorrection_Def.EnableWindow(parent->m_bFlags[6] && g_AOSACAParams->g_bDMReady && parent->m_MinCent);
			parent->m_SPreCorrection_Def.EnableWindow(parent->m_bFlags[6] && g_AOSACAParams->g_bDMReady && parent->m_MinCent);
			parent->m_ePreCorrection_Cyl.EnableWindow(parent->m_bFlags[6] && g_AOSACAParams->g_bDMReady && parent->m_MinCent);
			parent->m_SPreCorrection_Cyl.EnableWindow(parent->m_bFlags[6] && g_AOSACAParams->g_bDMReady && parent->m_MinCent);
			parent->m_ePreCorrection_Axis.EnableWindow(parent->m_bFlags[6] && g_AOSACAParams->g_bDMReady && parent->m_MinCent);
			parent->m_SPreCorrection_Axis.EnableWindow(parent->m_bFlags[6] && g_AOSACAParams->g_bDMReady && parent->m_MinCent);
			parent->m_CApplyRefCorr.EnableWindow(parent->m_bFlags[6] && g_AOSACAParams->g_bDMReady && parent->m_MinCent);
			parent->m_eFixedDef.EnableWindow(parent->m_bFlags[7] && g_AOSACAParams->g_bDMReady);
			parent->m_SFixedDef.EnableWindow(parent->m_bFlags[7] && g_AOSACAParams->g_bDMReady);
			if ( g_AOSACAParams->g_bDMReady) 
			{
				if (parent->m_bFlags[8] && !g_AOSACAParams->g_bControlON) //enable closed loop button
				{
					parent->m_CLoopToggleStop.ShowWindow(false);
					parent->m_CLoopToggle.ShowWindow(true);
					parent->m_CLoopToggle.EnableWindow(parent->m_bFlags[8]);
				}
				else if (parent->m_bFlags[8] && g_AOSACAParams->g_bControlON)
				{			
					parent->m_CLoopToggle.ShowWindow(false);
					parent->m_CLoopToggleStop.ShowWindow(true);
					parent->m_CLoopToggleStop.EnableWindow(parent->m_bFlags[8]);
				}
				else if (!parent->m_bFlags[8] && !g_AOSACAParams->g_bControlON)
				{
					parent->m_CLoopToggleStop.ShowWindow(false);
					parent->m_CLoopToggle.ShowWindow(true);
					parent->m_CLoopToggleStop.EnableWindow(parent->m_bFlags[8]);
				}
			}
			parent->m_FlatDM.EnableWindow( g_AOSACAParams->g_bDMReady);
			parent->m_CAOtoggle.EnableWindow(parent->m_bFlags[9] && g_AOSACAParams->g_bDMReady);
			break;
		case WAIT_OBJECT_0+1: // Get a new snap shot from the camera
			{
				SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
				parent->m_pParent->SetDlgItemText(IDE_FREQUENCY, _T("-"));
				g_AOSACAParams->g_bSubstractBkGnd = false;
				if (g_AOSACAParams->g_last_click == LIVE)
				{					
					g_AOSACAParams->g_Logfile<<LPCTSTR(g_AOSACAParams->GetTimeStamp())<<"REL\n";
					ResetEvent(g_AOSACAParams->g_ehCamLive);
				}				
				////////////////
				SetEvent(g_AOSACAParams->g_ehCamSnap);
				::WaitForSingleObject(g_AOSACAParams->g_ehCamNewFrame, INFINITE);
				////////////////
				g_AOSACAParams->g_last_click = SNAP;
				switch (g_AOSACAParams->g_cur_click)
				{//SUBBKGND, CENTROIDS, OPTPERFORM, AUTO
				case SNAP:
					parent->m_pParent->SendMessage(WM_UPDATE_WINDOW, 0, UPDATE_WFSIMG_WINDOW);
					parent->m_pParent->UpdateMouseMode(MOUSE_NORMAL);		
					parent->m_bFlags[1] = true;
					parent->m_pParent->m_bMenuFlags[0] = true;
					parent->m_pParent->OnUpdateMenu();
					SetEvent(parent->m_ehUpdateDialog);
					break;
				case SUBBKGND:
					parent->SubbkgndButton();
					break;
				case CENTROIDS:
					parent->CentroidsButton();
					break;
				case OPTPERFORM:
					parent->OptPerfButton();
					break;
				case AUTO:
					break;
				default:
					;
				}				
			}
			break;
		case WAIT_OBJECT_0+2:
			parent->m_bRunning = false;
			break;
		}
	}while(parent->m_bRunning);
	
	SetEvent(parent->m_ehCPanelThreadClosed);

	return 0;
}

void CControlPanelDlg::UpdateMinCent()
{
	m_MinCent = (int)(g_centroids->get_TotalCentCount()*m_fMinCentPercent);		
	g_centroids->set_MinCentroids(m_MinCent);
	UpdateCloopmodeStatus();
	m_eMinCent.SetValidation(MYEDIT_INTEGER, m_MinCent, 0.6*g_centroids->get_TotalCentCount(), g_centroids->get_TotalCentCount(), 3, -1, false);
}

BOOL CControlPanelDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
			{
				CString text;
				CWnd *pWnd = GetFocus();
				if (pWnd == &m_eCamExp)
				{
					m_eCamExp.CheckValidity();
					g_AOSACAParams->EXPOSURE_MS = (short)(GetDlgItemInt(IDE_CONTROL_CAM_EXP,NULL,false));
					g_camera->UpdateExposureTime();
				}				
				else if (pWnd == &m_eCentThreshold)
				{
					m_eCentThreshold.CheckValidity();
					g_AOSACAParams->THRESHOLD = (short)(GetDlgItemInt(IDE_CONTROL_ANALYSIS_THRESHOLD,NULL, false));
				}
				else if (pWnd == &m_eMinCent)
				{
					m_eMinCent.CheckValidity();
					m_MinCent = (short)(GetDlgItemInt(IDE_CONTROL_CLOOPPARAMS_MINCENT,NULL,false));					
					m_fMinCentPercent = (float)m_MinCent/(float)g_centroids->get_TotalCentCount();
					g_centroids->set_MinCentroids(m_MinCent);
				}
				else if (pWnd == &m_eIntGain)
				{
					m_eIntGain.CheckValidity();
					GetDlgItemText(IDE_CONTROL_CLOOPPARAMS_INTGGAIN,text);
					m_IntGain = _ttof(text);
					g_optcalc->set_Gain(m_IntGain);
				}
				else if (pWnd == &m_eModestoZero)
				{
					m_eModestoZero.CheckValidity();
					GetDlgItemText(IDE_CONTROL_CLOOPPARAMS_ZEROMODES,text);
					m_ModestoZero = _ttoi(text);
					g_centroids->set_ModestoZero(m_ModestoZero);
				}
				else if (pWnd == &m_ePupilSize)
				{
					m_ePupilSize.CheckValidity();
					GetDlgItemText(IDE_CONTROL_CLOOPPARAMS_PUPILSIZE,text);
					m_PupilSize = _ttof(text)*1000.;
					m_MinCent = g_centroids->Update_bUse_centroid(m_PupilSize);
					g_optcalc->UpdateUserParameters(m_PupilSize);
					SetDlgItemInt(IDE_CONTROL_CLOOPPARAMS_MINCENT, m_MinCent, false);
					g_centroids->set_MinCentroids(m_MinCent);
				}
				else if (pWnd == &m_eFixedDef)
				{
					m_eFixedDef.CheckValidity();
					GetDlgItemText(IDE_CONTROL_DEFOCUS_FIXEDDEF,text);
					m_FixedDefocusValue = _ttof(text);					
					send_DM_update_Defocus(m_FixedDefocusValue);
				}
				else if (pWnd == &m_eDefStep)
				{
					m_eDefStep.CheckValidity();
					GetDlgItemText(IDE_CONTROL_DEFOCUS_STEPSIZE,text);
					m_DefocusStepSize = _ttof(text);
				}				
				else if (pWnd == &m_ePreCorrection_Def)
				{
					m_ePreCorrection_Def.CheckValidity();
					GetDlgItemText(IDE_CONTROL_REFRACTION_SPHERE,text);
					m_PreDefocusValue = _ttof(text);
					send_DM_update_PreCorrection();
				}
				else if (pWnd == &m_ePreCorrection_Cyl)
				{
					m_ePreCorrection_Cyl.CheckValidity();
					GetDlgItemText(IDE_CONTROL_REFRACTION_CYL,text);
					m_PreCylinderValue = _ttof(text);
					send_DM_update_PreCorrection();
				}
				else if (pWnd == &m_ePreCorrection_Axis)
				{
					m_ePreCorrection_Axis.CheckValidity();
					GetDlgItemText(IDE_CONTROL_REFRACTION_AXIS,text);
					m_PreAxisValue = _ttof(text);
					send_DM_update_PreCorrection();
				}
				text = '\0';
				pWnd = GetNextDlgTabItem(GetFocus());
				if (pWnd)
					GotoDlgCtrl(pWnd);
				return false;
			}
			break;
		case 83:
			m_bFlags[0]?OnBnClickedControlCamSnap():0;
			break;
		case 79:
			m_bFlags[3]?OnBnClickedControlAnalysisOptperf():0;
			break;
		case 66:
			OnBnClickedControlDmFlat();
			break;
		case 67:
			if (m_bFlags[8] && !g_AOSACAParams->g_bControlON)
				OnBnClickedControlDmCloop();
			else if(m_bFlags[8] && g_AOSACAParams->g_bControlON)
				OnBnClickedControlDmCloopstop();
			break;
		case 82: // Recording
		case 114:
			break;
		case 107: //VK_NUMPAD_ADD - Add defocus
		case 187:
			{
				CWnd *pWnd = GetFocus();
				if (pWnd != &m_eCamExp && pWnd != &m_eCamGain && pWnd != &m_eCentThreshold && pWnd != &m_eMinCent && pWnd != &m_eIntGain && pWnd != &m_eModestoZero 
					&& pWnd != &m_ePupilSize && pWnd != &m_eFixedDef && pWnd != &m_eDefStep && pWnd != &m_ePreCorrection_Def && pWnd != &m_ePreCorrection_Cyl && pWnd != &m_ePreCorrection_Axis && 
					pWnd != &m_ePrefix && pWnd != &m_eLength)
				{
				Update_Defocus(true);
				}
			}
			break;
		case 109: //VK_NUMPAD_SUBTRACT - Substract defocus
		case 189:
			{
				CWnd *pWnd = GetFocus();
				if (pWnd != &m_eCamExp && pWnd != &m_eCamGain && pWnd != &m_eCentThreshold && pWnd != &m_eMinCent && pWnd != &m_eIntGain && pWnd != &m_eModestoZero 
					&& pWnd != &m_ePupilSize && pWnd != &m_eFixedDef && pWnd != &m_eDefStep && pWnd != &m_ePreCorrection_Def && pWnd != &m_ePreCorrection_Cyl && pWnd != &m_ePreCorrection_Axis && 
					pWnd != &m_ePrefix && pWnd != &m_eLength)
				{
				Update_Defocus(false);
				}
			}
			break;
		case VK_ESCAPE:
			break;
		}		
	}

	return false;

//	return CDialogEx::PreTranslateMessage(pMsg);
}

void CControlPanelDlg::SubBkgnd()
{	
	g_camera->SubtractBackground();
	g_AOSACAParams->g_last_click = SUBBKGND;
}

void CControlPanelDlg::Update_Defocus(BOOL direction)
{
	CString text;
	GetDlgItemText(IDE_CONTROL_DEFOCUS_STEPSIZE,text);				
	m_DefocusStepSize = _ttof(text);
	if (direction == 1)
		m_FixedDefocusValue += m_DefocusStepSize;
	else if (direction == 0)
		m_FixedDefocusValue -= m_DefocusStepSize;
	direction = -1;
	text.Format(_T("%2.3f"),m_FixedDefocusValue);
	SetDlgItemText(IDE_CONTROL_DEFOCUS_FIXEDDEF,text);
	send_DM_update_Defocus(m_FixedDefocusValue);
}

void CControlPanelDlg::UpdateCloopmodeStatus()
{
	m_bFlags[19] = g_centroids->get_CLoopReady();
	m_bFlags[20] = true;
	m_bFlags[21] = g_centroids->get_CLoopReady();
	
	SetEvent(m_ehUpdateDialog);
}

void CControlPanelDlg::FlatDM()
{
	OnBnClickedControlDmFlat();
}

HBRUSH CControlPanelDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	
//	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	switch (nCtlColor)
	{
	case CTLCOLOR_EDIT:
		pDC->SetBkColor (EDITBACKGROUNDCOLOR); 
		pDC->SetTextColor (EDITTEXTCOLOR); // White text
		return (HBRUSH)(m_editbkgndbrush.GetSafeHandle());
		break;
	case CTLCOLOR_BTN:
		pDC->SetBkColor (BACKGROUNDCOLOR); 
		pDC->SetTextColor (TEXTCOLOR); // White text
		return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());
		break;
	case CTLCOLOR_SCROLLBAR:
		pDC->SetBkColor (BACKGROUNDCOLOR); 
		pDC->SetTextColor (TEXTCOLOR); // White text
		return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());
		break;
	case CTLCOLOR_DLG:		
		pDC->SetBkColor (BACKGROUNDCOLOR); 
		pDC->SetTextColor (TEXTCOLOR); // White text
		return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());
		break;
	case CTLCOLOR_STATIC:			
		pDC->SetTextColor (TEXTCOLOR); // White text
		pDC->SetBkColor (BACKGROUNDCOLOR);  
		if( (pWnd->GetDlgCtrlID() == IDC_CONTROL_ANALYSIS_PENALIZETIPTILT) || (pWnd->GetDlgCtrlID() == IDC_CONTROL_ANALYSIS_RENEWPMAT) || (pWnd->GetDlgCtrlID() == IDC_CONTROL_ANALYSIS_AUTO) 
			|| (pWnd->GetDlgCtrlID() == IDC_CONTROL_REFRACTION_APPLY) || (pWnd->GetDlgCtrlID() == IDC_CONTROL_DM_AOONOFF) || (pWnd->GetDlgCtrlID() == IDG_CONTROL_CAMERA)
			|| (pWnd->GetDlgCtrlID() == IDG_CONTROL_ANALYSIS) || (pWnd->GetDlgCtrlID() == IDG_CONTROL_CLOOPPARAMS) || (pWnd->GetDlgCtrlID() == IDG_CONTROL_REFRACTION)
			|| (pWnd->GetDlgCtrlID() == IDG_CONTROL_DEFOCUS) || (pWnd->GetDlgCtrlID() == IDG_CONTROL_DM) || (pWnd->GetDlgCtrlID() == IDG_CONTROL_ICANDI) || (pWnd->GetDlgCtrlID() == IDG_CONTROL_ICANDI_RECORD)
			|| (pWnd->GetDlgCtrlID() == IDG_CONTROL_ICANDI_STAB))
			return (HBRUSH)(m_frgndbrush.GetSafeHandle());
		else
			return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());
		break;
	default:
		break;

	}

	// TODO:  Return a different brush if the default is not desired
	return (HBRUSH)(m_frgndbrush.GetSafeHandle());
}

void CControlPanelDlg::Update_PupilSize()
{	
	if (m_PupilSize > g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS)
	{
		m_PupilSize = g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS;

		CString text;
		text.Format(_T("%1.1f"), m_PupilSize);
		SetDlgItemText(IDE_CONTROL_CLOOPPARAMS_PUPILSIZE, text);
		m_MinCent = g_centroids->Update_bUse_centroid(m_PupilSize);
		g_optcalc->UpdateUserParameters(m_PupilSize);
		SetDlgItemInt(IDE_CONTROL_CLOOPPARAMS_MINCENT, m_MinCent, false);
		g_centroids->set_MinCentroids(m_MinCent);
	}
}