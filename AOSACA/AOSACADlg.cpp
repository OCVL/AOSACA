
// AOSACADlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "AOSACADlg.h"
#include "afxdialogex.h"
#include <fstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Global object for storing/accessing applicaiton parameters
AOSACAParams		*g_AOSACAParams;

CCamera				*g_camera;
CDMirror			*g_dmirror;
CCentroid			*g_centroids;
COptCalc			*g_optcalc;
CControlPanelDlg	*g_controlpanel;
CWFSImageDlg		*g_wfsimg;
CDMMapDlg			*g_dmmap;
CWFMapDlg			*g_wfmap;
CPSFDlg				*g_psfmap;
CSVMapDlg			*g_svmap;
CRTPlotDlg			*g_rtplot;
CPupilView			*g_pupilview;
CThreadClass		*g_dmaothread;
CProgressDlg		*g_progressdlg;
CAOSACAParamsDlg	*g_aosacaparamsdlg;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAOSACADlg dialog




CAOSACADlg::CAOSACADlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAOSACADlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	g_AOSACAParams	= NULL;
	g_centroids		= NULL;
	g_optcalc		= NULL;
	g_camera		= NULL;
	g_dmirror		= NULL;
	g_controlpanel	= NULL;
	g_wfsimg		= NULL;
	g_dmmap			= NULL;
	g_wfmap			= NULL;
	g_psfmap		= NULL;
	g_svmap			= NULL;
	g_rtplot		= NULL;
	g_pupilview		= NULL;
	
	// Initialize a new parameters object
	g_AOSACAParams = new AOSACAParams();
	// Centroid system inits
	g_centroids = new CCentroid();
	g_optcalc = new COptCalc();
	m_eMouse_mode = MOUSE_NORMAL;

	m_bMenuFlags	= new bool[9];	
	m_bUpdatePupilCenter = false;
}

void CAOSACADlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAOSACADlg, CDialogEx)
	ON_MESSAGE(WM_UPDATE_WINDOW, OnUpdateWindow)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILE_SAVEIMAGE, &CAOSACADlg::OnFileSaveimage)
	ON_COMMAND(ID_FILE_EXIT, &CAOSACADlg::OnFileExit)
	ON_COMMAND(ID_EDIT_UPDATEPUPILCENTER, &CAOSACADlg::OnEditUpdatepupilcenter)
	ON_COMMAND(ID_EDIT_AOSACAPARAMETERS, &CAOSACADlg::OnEditAosacaparameters)
	ON_COMMAND(ID_TOOLS_GENERATEPOKEMATRIX, &CAOSACADlg::OnToolsGeneratepokematrix)
	ON_COMMAND(ID_TOOLS_ZERODM, &CAOSACADlg::OnToolsZerodm)
	ON_COMMAND(ID_TOOLS_TESTDMACTUATORS, &CAOSACADlg::OnToolsTestdmactuators)
	ON_COMMAND(ID_VIEW_DRAWCENTROIDS, &CAOSACADlg::OnViewDrawcentroids)
	ON_COMMAND(ID_VIEW_SLOPEVECTORMAP, &CAOSACADlg::OnViewSlopevectormap)
	ON_COMMAND(ID_VIEW_WAVEFRONTMAP, &CAOSACADlg::OnViewWavefrontmap)
	ON_COMMAND(ID_VIEW_PSFMAP, &CAOSACADlg::OnViewPsfmap)
	ON_COMMAND(ID_VIEW_DMMAP, &CAOSACADlg::OnViewDmmap)
	ON_COMMAND(ID_VIEW_REALTIMEPLOTS, &CAOSACADlg::OnViewRealtimeplots)
	ON_COMMAND(ID_TOOLS_ALIGNDM, &CAOSACADlg::OnToolsAligndm)
	ON_COMMAND(ID_EDIT_PUPILCAMERASETTINGS, &CAOSACADlg::OnEditPupilcamerasettings)
	ON_COMMAND(ID_HELP_ABOUTAOSACA, &CAOSACADlg::OnHelpAboutaosaca)
	ON_COMMAND(ID_TOOLS_SAVEMIRRORSHAPE, &CAOSACADlg::OnToolsSavemirrorshape)
END_MESSAGE_MAP()


// CAOSACADlg message handlers

BOOL CAOSACADlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	ghDlg = m_hWnd;
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	//Font for closed loop values/refraction measurements
	CFont *fnt_cloop, *fnt_refrac;	
	fnt_cloop = new CFont();
	fnt_cloop->CreateFont(25,8,0,0,500,0,0,0,0,0,0,0,0,_T("Arial"));
	CWnd *EdtBox;
    EdtBox = GetDlgItem(IDE_NUMCENTROIDS);
	EdtBox->SetFont(fnt_cloop,true);
    EdtBox = GetDlgItem(IDE_RMS);
	EdtBox->SetFont(fnt_cloop,true);
	EdtBox = GetDlgItem(IDE_FREQUENCY);
	EdtBox->SetFont(fnt_cloop,true);
	fnt_refrac = new CFont();
	fnt_refrac->CreateFont(20,8,0,0,600,0,0,0,0,0,0,0,0,_T("Arial"));
	EdtBox = GetDlgItem(IDE_SPHERE);
	EdtBox->SetFont(fnt_refrac,true);
	EdtBox = GetDlgItem(IDE_CYLINDER);
	EdtBox->SetFont(fnt_refrac,true);
	EdtBox = GetDlgItem(IDE_AXIS);
	EdtBox->SetFont(fnt_refrac,true);

	//Setting the Main Dialog to the size of the screen
	short X0 = 4;
	short Y0 = 4;
	MoveWindow(X0,Y0,MAIN_WINDOW_WIDTH,MAIN_WINDOW_HEIGHT,false);

	// Creating the Camera object
	g_camera = new CCamera(this);
	if (!g_AOSACAParams->g_bCamReady)
	{		
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "Camera/Framegrabber not connected/detected, Application will exit";
		g_AOSACAParams->ShowError(MB_ICONINFORMATION);
		OnClose();
	}

	//Create DM object
	g_dmirror = new CDMirror();
	if (!g_AOSACAParams->g_bDMReady)
	{//add mirror details to caption
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "DM not connected/detected, you wont be able to run AO";
		g_AOSACAParams->ShowError(MB_ICONINFORMATION);
	}
	else
	{
		g_optcalc->UpdateDMMaxDeflection(g_AOSACAParams->DM_MAX_DEFLECTION);
		g_optcalc->Send_Voltages(BIAS_BIT);
	}
	
	// Create Wavefront Sensor Image dialog and initialize its params
	RECT mainrect = CRect(5,15, 0, 0);
	mainrect.right = mainrect.left+519;
	mainrect.bottom = mainrect.top+550;
	g_wfsimg = new CWFSImageDlg(this);
	g_wfsimg->Create(IDD_WFSIMAGE,this);
	g_wfsimg->MoveWindow(&mainrect, true);	
	g_wfsimg->ShowWindow(SW_SHOW);
	g_wfsimg->InitParam();

	//Control Panel Dialog
	mainrect.left = mainrect.right + 7;
	mainrect.top +=110;
	mainrect.right +=382;
	g_controlpanel = new CControlPanelDlg(this);
	g_controlpanel->Create(IDD_CONTROLPANEL,this);
	g_controlpanel->MoveWindow(&mainrect, true);
	g_controlpanel->ShowWindow(SW_SHOW);	

	//Slope Vector Map Dialog
	mainrect.left = mainrect.right + 7;
	mainrect.top -= 110;	
	mainrect.right = mainrect.left + 354;	
	mainrect.bottom = mainrect.top + 354;
	g_svmap = new CSVMapDlg(this);
	g_svmap->Create(IDD_SVMAP,this);
	g_svmap->MoveWindow(&mainrect, true);
	g_svmap->ShowWindow(SW_SHOW);

	//g_wfmapDialog
	mainrect.right = mainrect.left + 180;
	mainrect.top = mainrect.bottom + 7;
	mainrect.bottom = mainrect.top + 188;
	g_wfmap = new CWFMapDlg(this);
	g_wfmap->Create(IDD_WFMAP,this);
	g_wfmap->MoveWindow(&mainrect, true);
	g_wfmap->InitParam();
	g_wfmap->SetDataPoints(g_optcalc->InitWaveMatrix(g_wfmap->m_bData));
	g_wfmap->ShowWindow(SW_SHOW);

	//PDF Dialog
	mainrect.left = mainrect.right + 7;
	mainrect.right = mainrect.left + 167;
	g_psfmap = new CPSFDlg(this);
	g_psfmap->Create(IDD_PSFMAP,this);
	g_psfmap->MoveWindow(&mainrect, true);
	g_psfmap->ShowWindow(SW_SHOW);		
	g_psfmap->InitParam();

	//DM Map Dialog
	mainrect.left = 5;
	mainrect.top = 572;
	mainrect.right = mainrect.left+250;
	mainrect.bottom = mainrect.top + 310;
	g_dmmap = new CDMMapDlg(this);
	g_dmmap->Create(IDD_DMMAP,this);
	g_dmmap->MoveWindow(&mainrect, true);
	g_dmmap->InitParam();
	g_dmmap->ShowWindow(SW_SHOW);

	//Pupil Camera View Dialog
	mainrect.left = mainrect.right + 7;
	mainrect.right = mainrect.left+336;
	mainrect.bottom = mainrect.top + 310;
	g_pupilview = new CPupilView(this);
/*	g_pupilview->Create(IDD_PUPILVIEW,this);
	g_pupilview->MoveWindow(&mainrect, true);
	g_pupilview->ShowWindow(SW_SHOW);
	*/
	//Real-Time Plot dialog
	mainrect.left = mainrect.right + 7;
	mainrect.right = mainrect.left+663;
	g_rtplot = new CRTPlotDlg(this);
	g_rtplot->Create(IDD_RTPLOT,this);
	g_rtplot->MoveWindow(&mainrect, true);
	g_rtplot->ShowWindow(SW_SHOW);

	//Create an object for thread class
	g_dmaothread = new CThreadClass();
	g_dmaothread->InitInstance(this);

	// Initialize menu flags
	m_bMenuFlags[0] = false; // save image
	m_bMenuFlags[1] = false;  // update pupil center
	m_bMenuFlags[2] = true;  // update AOSACA parameters
	m_bMenuFlags[3] = g_pupilview->m_bHasVideoDlg; //Pupil Cam Settings
	m_bMenuFlags[4] = false; // Generate poke matrix
	m_bMenuFlags[5] = g_AOSACAParams->g_bDMReady; // Set all DM actuators to '0'
	m_bMenuFlags[6] = g_AOSACAParams->g_bDMReady; //Test all DM actuators one by one
	m_bMenuFlags[7] = g_AOSACAParams->g_bDMReady; //Align DM
	m_bMenuFlags[8] = g_AOSACAParams->g_bDMReady; //Save mirror shape
	OnUpdateMenu();
	g_AOSACAParams->g_bDrawCentroids = true;
	
	m_Statusbar.Create(WS_CHILD|WS_VISIBLE|CCS_BOTTOM|SBARS_SIZEGRIP, CRect(0,0,0,0), this, IDS_STATUSBAR);

	// Perform initial setup of all matrices to aid in changing pupil size, adding defocus etc prior to doing refraction atleast once
	// Load wfs template image
	CStringA filename = "utils\\wfs_temp.bimg";	
	ifstream templateFile;
	templateFile.open(filename, ios::in | ios::binary);
	if (!templateFile.read ((char*)g_AOSACAParams->g_pImgBuffPrc, g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX*sizeof(unsigned char)))
		ZeroMemory(g_AOSACAParams->g_pImgBuffPrc, g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX*sizeof(unsigned char));
	else
	{
		g_camera->SubtractBackground();
		if (g_AOSACAParams->PUPIL_CENTER.x > 120)
		{
			g_centroids->SetUserCenterPoint(2000.);
			LocateCentroids();
			g_centroids->Estimate_Center(false);   // This updates the actual pupil center
			g_centroids->Make_Search_Array((float)g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
			g_centroids->Make_Search_Array_Act((float)g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
			g_centroids->Load_Pmat();
			g_centroids->Estimate_Center();
			g_optcalc->set_ZReconReady(false);
			g_optcalc->set_ZOpenLoopReady(false);
			g_optcalc->Initialize_Zernike_Coeffs(g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
			g_optcalc->set_ZReconReady(false);
			g_centroids->Update_bUse_centroid(g_optcalc->get_currentPupilSize());
			g_controlpanel->UpdateMinCent();
			g_controlpanel->m_bFlags[6] = true;
			SetEvent(g_controlpanel->m_ehUpdateDialog);
		}
		init_Centroid_Detection();
		g_centroids->CalcSlopes(NULL, false);
		// Opt_Perform	
		g_optcalc->Init_Mahajan();  
		g_optcalc->Init_Southwell();   
		g_optcalc->Compute_RMS(g_controlpanel->m_FixedDefocusValue);	
		g_optcalc->Compute_RMS_Southwell();
		g_centroids-> m_bPenalizeTipTilt = true;
		g_centroids->Generate_PTPmat();
	}	
	templateFile.close();
	
	SetEvent(g_AOSACAParams->g_ehCamLive);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

LRESULT CAOSACADlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
	CString fileName;

	switch (lParam) {
	case UPDATE_WINDOW:		
		Update_Displays();
		break;
	case UPDATE_WFSIMG_WINDOW:		
		g_wfsimg->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
		break;
	case UPDATE_DMVMAP_WINDOW:
		g_dmmap->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
		break;
	case UPDATE_PGB_WINDOW:
		g_progressdlg->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
		break;
	case UPDATE_PGBD_WINDOW:
		fileName = g_progressdlg->OnDoneGeneration();
		if (!fileName.IsEmpty())
		{
			//Ask if the user wants to load the new poke matrix
			g_AOSACAParams->g_stAppErrBuff = "Would you like to use newly saved Poke Matrix";
			if (g_AOSACAParams->ShowError(MB_ICONQUESTION) == MB_OK)
			{ //Save this new filename into the parameters file
				g_AOSACAParams->POKE_MATRIX_FILE = fileName;
				g_AOSACAParams->g_stAppErrBuff = "Application will close now, restart your application to take effect of your updated values";
				g_AOSACAParams->ShowError(MB_ICONINFORMATION);
				OnClose();
			}
		}
		break;
	case UPDATE_PGBC_WINDOW:
		g_progressdlg->Close();
		break;
	default:
		break;
	}
	
	return 0;
}

void CAOSACADlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAOSACADlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAOSACADlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

////////////////////////////////////////////////////////////////
// INITIALIZATIONS
////////////////////////////////////////////////////////////////
void CAOSACADlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if(g_dmaothread) 
	{
		if (g_AOSACAParams->g_bPokeMatGeneration || g_AOSACAParams->g_bCloseLoopThread)
		{
			g_AOSACAParams->g_stAppErrBuff.Empty();
			g_AOSACAParams->g_stAppErrBuff = "Stop current running operation(s) before closing";
			g_AOSACAParams->ShowError(MB_ICONINFORMATION);
			return;
		}
		g_dmaothread->ExitInstance();
	}
	delete g_dmaothread;
	if(g_controlpanel) delete g_controlpanel;
	if(g_wfsimg) delete g_wfsimg;
	if(g_dmmap) delete g_dmmap;
	if(g_psfmap) delete g_psfmap;
	if(g_wfmap) delete g_wfmap;
	if(g_svmap) delete g_svmap;
	if(g_pupilview) delete g_pupilview;
	if(g_rtplot) delete g_rtplot;
	if(g_optcalc) delete g_optcalc;	
	if(g_camera) delete g_camera;
	if(g_centroids) delete g_centroids;
	if(g_AOSACAParams) delete g_AOSACAParams;
	EndDialog(IDOK);

	CDialogEx::OnClose();
}


//////////////////////////////////
// Mouse update functions/Handlers
void CAOSACADlg::lBtnUp_ImageDlg(CDPoint point)
{
	CDC *cdc;
	CString cstrAux = _T("");
	CDPoint pt;
	
	if (m_eMouse_mode==MOUSE_NORMAL) 
		return;
	
	else
	{
		switch(m_eMouse_mode)
		{
		case MOUSE_SELECT_CENTER:
			//center is selected and boxes are drawn after the mouse button is released
			pt = g_AOSACAParams->PUPIL_CENTER;
			g_AOSACAParams->PUPIL_CENTER = point;
			g_centroids->SetUserCenterPoint(2000.);
			if(LocateCentroids()==false && g_centroids->GetCentroidCount()<=10)
			{				
				g_wfsimg->setCursor_ImageDlg(ARROW);
				g_AOSACAParams->g_stAppErrBuff.Empty();
				g_AOSACAParams->g_stAppErrBuff = "Center Selection Failed, are you sure you are not trying to find on background image?"; 
				g_AOSACAParams->ShowError(MB_ICONERROR);
				g_AOSACAParams->PUPIL_CENTER = pt;
				return;
			}
			//no error!! save the center			
			g_centroids->Estimate_Center(m_bUpdatePupilCenter);   // This updates the actual pupil center
			m_bUpdatePupilCenter = false;
			g_controlpanel->m_bFlags[2] = true;
			SetEvent(g_controlpanel->m_ehUpdateDialog);
			g_centroids->Make_Search_Array((float)g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
			g_centroids->Make_Search_Array_Act((float)g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
			if (!g_centroids->Load_Pmat() && g_AOSACAParams->g_bDMReady)
			{				
				g_wfsimg->setCursor_ImageDlg(ARROW);
				g_AOSACAParams->g_stAppErrBuff.Empty();
				g_AOSACAParams->g_stAppErrBuff = "Unable to load Poke matrix!!!, generate poke matrix before performing AO correction"; 
				g_AOSACAParams->ShowError(MB_ICONERROR);
			}
			g_svmap->Init_CenterPoints(g_centroids->get_TotalCentCount());
			g_centroids->Estimate_Center();			
			g_wfsimg->m_bDrawSearchBoxes = true;
			PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WFSIMG_WINDOW);			
			g_optcalc->set_ZReconReady(false);
			g_optcalc->set_ZOpenLoopReady(false);
			g_optcalc->Initialize_Zernike_Coeffs(g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
			g_optcalc->set_ZReconReady(false);
			g_centroids->Update_bUse_centroid(g_optcalc->get_currentPupilSize());

			cstrAux.Format(_T("(%d, %d) selected as new center."), (int) pt.x, (int) pt.y);

			g_wfsimg->releaseCapture_ImageDlg(); //restore mouse input processing to normal
			g_wfsimg->setCursor_ImageDlg(ARROW);
			g_controlpanel->UpdateMinCent();
			m_eMouse_mode = MOUSE_NORMAL;
			break;			
		case POKE_MATRIX_GENERATION:
			g_progressdlg->SetFocus();
			break;			
		} // end switch/case on mouse mode
		
		// Update the text window on the graphic window
		cdc = (get_DisplayHandle())->GetDC();
		cdc->TextOut(0, 0, cstrAux, cstrAux.GetLength());
		(get_DisplayHandle())->ReleaseDC(cdc);
	}	
}

CWnd* CAOSACADlg::get_DisplayHandle(void)
{
	CWnd *Ctrl;
	
	Ctrl = g_wfsimg->GetDlgItem(IDH_WFSIMAGE);
	return Ctrl;
}

void CAOSACADlg::mouseMove_ImageDlg(CPoint point)
{
	CDC *cdc;
	CString cstrAux=_T("");
	CWnd *hCentroidDisplayArea = get_DisplayHandle();

	if (!(point.x > g_AOSACAParams->SEARCHBOX_SIZE && point.x < (g_AOSACAParams->IMAGE_HEIGHT_PIX-g_AOSACAParams->SEARCHBOX_SIZE) && point.y > g_AOSACAParams->SEARCHBOX_SIZE && point.y < (g_AOSACAParams->IMAGE_WIDTH_PIX-g_AOSACAParams->SEARCHBOX_SIZE)))
	{
		g_wfsimg->releaseCapture_ImageDlg(); //restore mouse input processing to normal
		g_wfsimg->setCursor_ImageDlg(ARROW);
	}
	else if (m_eMouse_mode == MOUSE_SELECT_CENTER) 
	{
		cstrAux.Format(_T("Current Position: (%d, %d) "), (int) point.x, (int) point.y);
		cdc = hCentroidDisplayArea->GetDC();
		cdc->TextOut(0, 0, cstrAux, cstrAux.GetLength());
		hCentroidDisplayArea->ReleaseDC(cdc);
		g_wfsimg->setCapture_ImageDlg(); // redirect all mouse input to this win
		g_wfsimg->setCursor_ImageDlg(CROSS);
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CAOSACADlg::LocateOneCentroid (short id) 
{
	bool status=false; // assume false until everything is ok
	
	RECT CentDispRect;
	CWnd *hCentroidDisplayArea=get_DisplayHandle();
	hCentroidDisplayArea->GetWindowRect(&CentDispRect);
	memcpy(g_AOSACAParams->g_pLocalImgBuff, g_AOSACAParams->g_pImgBuffPrc, (g_AOSACAParams->IMAGE_WIDTH_PIX)*(g_AOSACAParams->IMAGE_HEIGHT_PIX));
	g_centroids->LocateOneCentroid(g_AOSACAParams->g_pLocalImgBuff, id);
	g_wfsimg->m_bDrawSearchBoxes = true;
	g_wfsimg->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
	return status;
}

bool CAOSACADlg::LocateCentroids (void)
{
	bool centstatus = false, imagestatus=true;
	memcpy(g_AOSACAParams->g_pLocalImgBuff, g_AOSACAParams->g_pImgBuffPrc, (g_AOSACAParams->IMAGE_WIDTH_PIX)*(g_AOSACAParams->IMAGE_HEIGHT_PIX));
	centstatus = g_centroids->LocateAllCentroid(g_AOSACAParams->g_pLocalImgBuff);

	if (g_AOSACAParams->g_bPokeMatGeneration && g_centroids->GetCentroidCount()<g_centroids->get_TotalCentCount())
		centstatus = false;
		
	return (centstatus && imagestatus);
}

bool CAOSACADlg::init_Centroid_Detection(void)
{
	if(Steps_in_finding_Centroids() == false)
		return false;
	UpdateMouseMode(MOUSE_NORMAL);
	return true;
}

bool CAOSACADlg::Steps_in_finding_Centroids(void)
{
	bool status = false;
	status = LocateCentroids();
	//Once the centroids have been found, image is not fresh anymore
	SetDlgItemInt(IDE_NUMCENTROIDS, g_centroids->GetCentroidCount());		
	
	if (!g_AOSACAParams->g_bControlON || g_AOSACAParams->g_bPokeMatGeneration)
	{
		g_AOSACAParams->g_bDrawCentroids?g_wfsimg->m_bDrawCentroids = true:g_wfsimg->m_bDrawCentroids = false;
		g_wfsimg->SendMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
	}
	
	return status;
}

BOOL CAOSACADlg::OnEraseBkgnd(CDC* pDC)
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

void CAOSACADlg::Opt_Perform()
{
	UpdateMouseMode(MOUSE_NORMAL);

	if (g_AOSACAParams->g_bControlON) //while in closed loop, update WFS image and draw centroids here
	{
		g_wfsimg->m_bDrawCentroids = g_AOSACAParams->g_bDrawCentroids;
		g_wfsimg->SendMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
		SetEvent(g_AOSACAParams->g_ehCamSnap);
	}

	// Compute the zernikes
	g_optcalc->Init_Mahajan();
	g_optcalc->Init_Southwell();
///////////////////////////////////////////////
	g_optcalc->Compute_RMS(g_controlpanel->m_FixedDefocusValue);	
	g_optcalc->Compute_RMS_Southwell();

	// Drawing the Wave Map
	if(g_wfmap->m_BUpdate)
	{
		g_optcalc->Compute_WAVE (g_wfmap->m_dData);
		g_wfmap->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
	}

	if (!g_AOSACAParams->g_bPokeMatGeneration)
	{
		// Drawing the PSF
		if(g_psfmap->m_BUpdate)
		{
			g_optcalc->Compute_PSF_MTF (g_psfmap->m_dData);		
			g_psfmap->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
		}
		// Drawing the slope vectors
		if(g_svmap->m_BUpdate) 
			g_svmap->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
		// Check if the window is visible
		if(g_rtplot->m_BUpdate)// g_AOSACAParams->g_bControlON && 
			g_rtplot->PostMessageW(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
	}	
	// Refresh the Wavefront Correction Image to reflect new voltages
	if (g_AOSACAParams->g_bUpdateDMVMap && g_dmmap->m_BUpdate)
		g_dmmap->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
}

void CAOSACADlg::Update_Displays()
{
	CString result;
	double d,c,a;
	result.Format(_T("%4.4f"),g_optcalc->retn_RMS());
	SetDlgItemText(IDE_RMS,result);	
	// For the refractive measurement
    g_optcalc->get_DefCylAxis(&d,&c,&a);
	result.Format(_T("%4.3f"),d);
	SetDlgItemText(IDE_SPHERE,result);
	result.Format(_T("%4.3f"),c);
	SetDlgItemText(IDE_CYLINDER,result);
	result.Format(_T("%4.2f"),a);
	SetDlgItemText(IDE_AXIS,result);    
}

void CAOSACADlg::Initialize_CLoop()
{	
	!g_AOSACAParams->g_bResetMirror?g_rtplot->SendMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW),g_rtplot->Reset_SlopePlotData():0;
	g_optcalc->reset_Voltages();
	g_centroids->Initialize_Phi();
	g_AOSACAParams->g_bResetMirror = false;
}

// AOSACA Application Menu handler functions
void CAOSACADlg::OnFileSaveimage()
{
	// TODO: Add your command handler code here
	CString file_types = _T("TIFF Files|*.tif;*.tiff|");
	file_types += "TIFF Files (*.tif)|*.tif;*.tiff|";
	file_types += "|";
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,(LPCTSTR)file_types,NULL);
	
	if (dlg.DoModal() == IDOK)
	{
		CString pathname = dlg.GetPathName();
		CString ext = dlg.GetFileExt();
		if (ext.CompareNoCase(_T("tif")) !=0)
			pathname += ".tif";
		CStringA filename;
		filename = pathname;
		if (!g_camera->SaveImage(filename, false))
		{
			g_AOSACAParams->g_stAppErrBuff.Empty();
			g_AOSACAParams->g_stAppErrBuff = "Failed to Save Image, check your folder permissions and try again";
			g_AOSACAParams->ShowError(MB_ICONERROR);
		}
	}
}


void CAOSACADlg::OnFileExit()
{
	// TODO: Add your command handler code here
	OnClose();
}


void CAOSACADlg::OnEditUpdatepupilcenter()
{
	// TODO: Add your command handler code here
	m_bUpdatePupilCenter = true;
	m_eMouse_mode = MOUSE_SELECT_CENTER;
}


void CAOSACADlg::OnEditAosacaparameters()
{
	// TODO: Add your command handler code here
	CAOSACAParamsDlg *g_aosacaparamsdlg = new CAOSACAParamsDlg(this);
	if (g_aosacaparamsdlg->DoModal() == IDOK)
		{
			bool res[7];
			memcpy(res, g_aosacaparamsdlg->m_bAppUpdate, 7*sizeof(bool));
			if (res[0])
			{// Applicaiton restart		 
				delete g_aosacaparamsdlg;
				g_AOSACAParams->g_stAppErrBuff.Empty();
				g_AOSACAParams->g_stAppErrBuff = "Application will close now, restart your application to take effect of your updated values";
				g_AOSACAParams->ShowError(MB_ICONINFORMATION);
				OnClose();
			}
			if (res[1])
			{
				// recreate both Centroid and Optcalc objects
				if (g_optcalc != NULL)
				{
					delete g_optcalc;
					delete g_centroids;
				}
				g_centroids = new CCentroid();
				g_optcalc = new COptCalc();
				g_wfmap->SetDataPoints(g_optcalc->InitWaveMatrix(g_wfmap->m_bData));
			}
			else if(res[2])
			{// recreate OptCalc object
				if (g_optcalc != NULL)
				{
					delete g_optcalc;
				}
				g_optcalc = new COptCalc();
				g_wfmap->SetDataPoints(g_optcalc->InitWaveMatrix(g_wfmap->m_bData));
			}
			if (res[3])
			{// Update pupil size
				if (g_optcalc != NULL)
				{
					delete g_optcalc;
				}
				g_optcalc = new COptCalc();
				g_wfmap->SetDataPoints(g_optcalc->InitWaveMatrix(g_wfmap->m_bData));
				g_controlpanel->Update_PupilSize();
			}
			if (res[4])
			{// recreate DM object
				if (g_dmirror != NULL)
					delete g_dmirror;
				g_dmirror = new CDMirror();
				g_controlpanel->FlatDM();
			}
			if (res[5])
			{// Update WFS image color
				g_wfsimg->SetWFSImageColor();
			}
			if (res[6])
			{
				g_optcalc->UpdateDMMaxDeflection(g_AOSACAParams->DM_MAX_DEFLECTION);
			}
	}

	delete g_aosacaparamsdlg;
}


void CAOSACADlg::OnToolsGeneratepokematrix()
{
	// TODO: Add your command handler code here
	// Update AOSACA Menu
	m_bMenuFlags[0] = false; // save image
	m_bMenuFlags[1] = false; // update pupil center
	m_bMenuFlags[2] = false; // update AOSACA parameters
	m_bMenuFlags[3] = false; // Pupil Cam Settings
	m_bMenuFlags[4] = false; // Generate poke matrix
	m_bMenuFlags[5] = false; // Set all DM actuators to '0'
	m_bMenuFlags[6] = false; // Test all DM actuators one by one
	m_bMenuFlags[8] = false; // Save mirror shape
	OnUpdateMenu();
	UpdateMouseMode(POKE_MATRIX_GENERATION);
	g_controlpanel->EnableWindow(false);
	g_controlpanel->Invalidate(TRUE);
	g_progressdlg = new CProgressDlg(this);
	g_progressdlg->Create(IDD_PROGRESSDLG,NULL);
	g_progressdlg->MoveWindow(MAIN_WINDOW_WIDTH/2-152, MAIN_WINDOW_HEIGHT/2-35, 475, 140);
	g_progressdlg->ShowWindow(SW_SHOW);
	g_progressdlg->SetBarParameters(((g_AOSACAParams->NUMACTS)*g_AOSACAParams->DM_POKE_MAT_ITER + 2));
	g_AOSACAParams->g_bSubstractBkGnd = true;
	g_AOSACAParams->g_bPokeMatGeneration = true;
	SetEvent(g_AOSACAParams->g_ehPokeThread);
}


void CAOSACADlg::OnToolsAligndm()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(2);
	submenu->CheckMenuItem(ID_TOOLS_ZERODM, MF_UNCHECKED | MF_BYCOMMAND);
	UINT menuState = submenu->GetMenuState(ID_TOOLS_ALIGNDM,MF_BYCOMMAND);
	ASSERT(menuState != 0xFFFFFFFF);
	if (menuState & MF_CHECKED)
	{		
		submenu->CheckMenuItem(ID_TOOLS_ALIGNDM, MF_UNCHECKED | MF_BYCOMMAND);
		g_optcalc->Send_Voltages(BIAS_BIT);
	}
	else
	{
		submenu->CheckMenuItem(ID_TOOLS_ALIGNDM, MF_CHECKED | MF_BYCOMMAND);
		g_optcalc->Send_Voltages(ALLIGN_BIT);
	}	
	// Update AOSACA Menu
	m_bMenuFlags[0] = true; // save image
	m_bMenuFlags[1] = true; // update pupil center
	m_bMenuFlags[2] = true; // update AOSACA parameters
	m_bMenuFlags[4] = false; // Generate poke matrix
	m_bMenuFlags[5] = g_AOSACAParams->g_bDMReady; // Set all DM actuators to '0'
	m_bMenuFlags[6] = g_AOSACAParams->g_bDMReady; // Test all DM actuators one by one	
	m_bMenuFlags[8] = g_AOSACAParams->g_bDMReady; // Save mirror shape
	OnUpdateMenu();
	g_dmmap->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
}


void CAOSACADlg::OnToolsZerodm()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(2);	
	submenu->CheckMenuItem(ID_TOOLS_ALIGNDM, MF_UNCHECKED | MF_BYCOMMAND);
	UINT menuState = submenu->GetMenuState(ID_TOOLS_ZERODM,MF_BYCOMMAND);
	ASSERT(menuState != 0xFFFFFFFF);
	if (menuState & MF_CHECKED)
	{
		submenu->CheckMenuItem(ID_TOOLS_ZERODM, MF_UNCHECKED | MF_BYCOMMAND);
		g_optcalc->Send_Voltages(BIAS_BIT);
	}
	else
	{
		submenu->CheckMenuItem(ID_TOOLS_ZERODM, MF_CHECKED | MF_BYCOMMAND);
		g_optcalc->Send_Voltages(ABS_ZERO_BIT);
	}	
	// Update AOSACA Menu
	m_bMenuFlags[0] = true; // save image
	m_bMenuFlags[1] = true; // update pupil center
	m_bMenuFlags[2] = true; // update AOSACA parameters
	m_bMenuFlags[4] = false; // Generate poke matrix
	m_bMenuFlags[5] = g_AOSACAParams->g_bDMReady; // Set all DM actuators to '0'
	m_bMenuFlags[6] = g_AOSACAParams->g_bDMReady; // Test all DM actuators one by one
	m_bMenuFlags[8] = g_AOSACAParams->g_bDMReady; // Save mirror shape
	OnUpdateMenu();
	g_dmmap->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
}


void CAOSACADlg::OnToolsTestdmactuators()
{
	// TODO: Add your command handler code here
}


void CAOSACADlg::OnToolsSavemirrorshape()
{
	// TODO: Add your command handler code here
	CString file_types = _T("TXT Files|*.txt;*.txt|");
	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,(LPCTSTR)file_types,NULL);
	
	if (dlg.DoModal() == IDOK)
	{
		CString pathname = dlg.GetPathName();
		CString ext = dlg.GetFileExt();
		if (ext.CompareNoCase(_T("txt")) !=0)
			pathname += ".txt";
		CStringA filename;
		filename = pathname;
		if (!g_optcalc->SaveVoltages(filename))
		{
			g_AOSACAParams->g_stAppErrBuff.Empty();
			g_AOSACAParams->g_stAppErrBuff = "Failed to Save mirror shape, check your folder permissions and try again";
			g_AOSACAParams->ShowError(MB_ICONERROR);
		}
	}
}


void CAOSACADlg::OnViewDrawcentroids()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(3);
	UINT menuState = submenu->GetMenuState(ID_VIEW_DRAWCENTROIDS,MF_BYCOMMAND);
	ASSERT(menuState != 0xFFFFFFFF);
	if (menuState & MF_CHECKED)
	{
		submenu->CheckMenuItem(ID_VIEW_DRAWCENTROIDS, MF_UNCHECKED | MF_BYCOMMAND);		
		g_AOSACAParams->g_bDrawCentroids = false;
	}
	else
	{
		submenu->CheckMenuItem(ID_VIEW_DRAWCENTROIDS, MF_CHECKED | MF_BYCOMMAND);
		g_AOSACAParams->g_bDrawCentroids = true;
	}
}


void CAOSACADlg::OnViewSlopevectormap()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(3);
	UINT menuState = submenu->GetMenuState(ID_VIEW_SLOPEVECTORMAP,MF_BYCOMMAND);
	ASSERT(menuState != 0xFFFFFFFF);
	if (menuState & MF_CHECKED)
	{
		submenu->CheckMenuItem(ID_VIEW_SLOPEVECTORMAP, MF_UNCHECKED | MF_BYCOMMAND);
		g_svmap->m_BUpdate = false;
	}
	else
	{
		submenu->CheckMenuItem(ID_VIEW_SLOPEVECTORMAP, MF_CHECKED | MF_BYCOMMAND);
		g_svmap->m_BUpdate = true;
	}
}


void CAOSACADlg::OnViewWavefrontmap()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(3);
	UINT menuState = submenu->GetMenuState(ID_VIEW_WAVEFRONTMAP,MF_BYCOMMAND);
	ASSERT(menuState != 0xFFFFFFFF);
	if (menuState & MF_CHECKED)
	{
		submenu->CheckMenuItem(ID_VIEW_WAVEFRONTMAP, MF_UNCHECKED | MF_BYCOMMAND);
		g_wfmap->m_BUpdate = false;
	}
	else
	{
		submenu->CheckMenuItem(ID_VIEW_WAVEFRONTMAP, MF_CHECKED | MF_BYCOMMAND);
		g_wfmap->m_BUpdate = true;
	}
}


void CAOSACADlg::OnViewPsfmap()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(3);
	UINT menuState = submenu->GetMenuState(ID_VIEW_PSFMAP,MF_BYCOMMAND);
	ASSERT(menuState != 0xFFFFFFFF);
	if (menuState & MF_CHECKED)
	{
		submenu->CheckMenuItem(ID_VIEW_PSFMAP, MF_UNCHECKED | MF_BYCOMMAND);
		g_psfmap->m_BUpdate = false;
	}
	else
	{
		submenu->CheckMenuItem(ID_VIEW_PSFMAP, MF_CHECKED | MF_BYCOMMAND);
		g_psfmap->m_BUpdate = true;
	}
}


void CAOSACADlg::OnViewDmmap()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(3);
	UINT menuState = submenu->GetMenuState(ID_VIEW_DMMAP,MF_BYCOMMAND);
	ASSERT(menuState != 0xFFFFFFFF);
	if (menuState & MF_CHECKED)
	{
		submenu->CheckMenuItem(ID_VIEW_DMMAP, MF_UNCHECKED | MF_BYCOMMAND);
		g_dmmap->m_BUpdate = false;
	}
	else
	{
		submenu->CheckMenuItem(ID_VIEW_DMMAP, MF_CHECKED | MF_BYCOMMAND);
		g_dmmap->m_BUpdate = true;
	}
}


void CAOSACADlg::OnViewRealtimeplots()
{
	// TODO: Add your command handler code here
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(3);
	UINT menuState = submenu->GetMenuState(ID_VIEW_REALTIMEPLOTS,MF_BYCOMMAND);
	ASSERT(menuState != 0xFFFFFFFF);
	if (menuState & MF_CHECKED)
	{
		submenu->CheckMenuItem(ID_VIEW_REALTIMEPLOTS, MF_UNCHECKED | MF_BYCOMMAND);
		g_rtplot->m_BUpdate = false;
	}
	else
	{
		submenu->CheckMenuItem(ID_VIEW_REALTIMEPLOTS, MF_CHECKED | MF_BYCOMMAND);
		g_rtplot->m_BUpdate = true;
	}
}

void CAOSACADlg::ResetZeroDMMenu()
{
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(2);
	submenu->CheckMenuItem(ID_TOOLS_ZERODM, MF_UNCHECKED | MF_BYCOMMAND);
}

void CAOSACADlg::OnUpdateGeneratePokeMatrixMenu()
{
	// Update AOSACA Menu
	m_bMenuFlags[0] = false; // save image
	m_bMenuFlags[1] = false; // update pupil center
	m_bMenuFlags[2] = true;  // update AOSACA parameters
	m_bMenuFlags[4] = false; // Generate poke matrix
	m_bMenuFlags[5] = g_AOSACAParams->g_bDMReady;  // Set all DM actuators to '0'
	m_bMenuFlags[6] = g_AOSACAParams->g_bDMReady;  // Test all DM actuators one by one
	m_bMenuFlags[8] = g_AOSACAParams->g_bDMReady;  // Save mirror shape
	OnUpdateMenu();
}

void CAOSACADlg::OnUpdateMenu()
{
	CMenu* mmenu = GetMenu();
	CMenu* submenu = mmenu->GetSubMenu(0);
	m_bMenuFlags[0] && g_AOSACAParams->g_last_click != LIVE?submenu->EnableMenuItem(ID_FILE_SAVEIMAGE, MF_ENABLED | MF_BYCOMMAND)
					:submenu->EnableMenuItem(ID_FILE_SAVEIMAGE, MF_GRAYED | MF_BYCOMMAND);	//Save Image
	submenu = mmenu->GetSubMenu(1);
	(m_bMenuFlags[1] && g_AOSACAParams->g_last_click == SUBBKGND)?submenu->EnableMenuItem(ID_EDIT_UPDATEPUPILCENTER, MF_ENABLED | MF_BYCOMMAND)
					:submenu->EnableMenuItem(ID_EDIT_UPDATEPUPILCENTER, MF_GRAYED | MF_BYCOMMAND);	//Update Pupil center	
	(m_bMenuFlags[2] && !g_AOSACAParams->g_bControlON)?submenu->EnableMenuItem(ID_EDIT_AOSACAPARAMETERS, MF_ENABLED | MF_BYCOMMAND)
					:submenu->EnableMenuItem(ID_EDIT_AOSACAPARAMETERS, MF_GRAYED | MF_BYCOMMAND);	//AOSACA Parameters
	(m_bMenuFlags[3])?submenu->EnableMenuItem(ID_EDIT_PUPILCAMERASETTINGS, MF_ENABLED | MF_BYCOMMAND)
					:submenu->EnableMenuItem(ID_EDIT_PUPILCAMERASETTINGS, MF_GRAYED | MF_BYCOMMAND);	//Pupil Cam Settings
	submenu = mmenu->GetSubMenu(2);
	(m_bMenuFlags[4] && (g_AOSACAParams->g_last_click == OPTPERFORM || g_AOSACAParams->g_last_click == CENTROIDS) && !g_AOSACAParams->g_bControlON)?
					submenu->EnableMenuItem(ID_TOOLS_GENERATEPOKEMATRIX, MF_ENABLED | MF_BYCOMMAND)
					:submenu->EnableMenuItem(ID_TOOLS_GENERATEPOKEMATRIX, MF_GRAYED | MF_BYCOMMAND);	//Generate poke matrix
	(m_bMenuFlags[5] && !g_AOSACAParams->g_bControlON)?submenu->EnableMenuItem(ID_TOOLS_ZERODM, MF_ENABLED | MF_BYCOMMAND)
					:submenu->EnableMenuItem(ID_TOOLS_ZERODM, MF_GRAYED | MF_BYCOMMAND);	//Zero DM
	(m_bMenuFlags[6] && !g_AOSACAParams->g_bControlON)?submenu->EnableMenuItem(ID_TOOLS_TESTDMACTUATORS, MF_ENABLED | MF_BYCOMMAND)
					:submenu->EnableMenuItem(ID_TOOLS_TESTDMACTUATORS, MF_GRAYED | MF_BYCOMMAND);	//Test DM
	(m_bMenuFlags[8] && !g_AOSACAParams->g_bControlON)?submenu->EnableMenuItem(ID_TOOLS_SAVEMIRRORSHAPE, MF_ENABLED | MF_BYCOMMAND)
					:submenu->EnableMenuItem(ID_TOOLS_SAVEMIRRORSHAPE, MF_GRAYED | MF_BYCOMMAND);	//Test DM
}
// Done Menu handler functions

BOOL CAOSACADlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
			return false;
		case VK_ESCAPE:
			return false;
		}		
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CAOSACADlg::StartCLoopThread()
{				
	UpdateMouseMode(MOUSE_NORMAL);
	g_AOSACAParams->g_bSubstractBkGnd = true;
	g_AOSACAParams->g_bCloseLoopThread = true;
	g_AOSACAParams->g_bControlON?Initialize_CLoop():g_AOSACAParams->g_bAutoMeasure = true;
	g_optcalc->set_CloopFlag(true);	
	ResetEvent(g_AOSACAParams->g_ehCamNewFrame);
	// Update AOSACA Menu
	m_bMenuFlags[0] = false; // save image
	m_bMenuFlags[1] = false; // update pupil center
	m_bMenuFlags[2] = false; // update AOSACA parameters
	m_bMenuFlags[3] = false; // Pupil Cam settings
	m_bMenuFlags[4] = false; // Generate poke matrix
	m_bMenuFlags[5] = false; // Set all DM actuators to '0'
	m_bMenuFlags[6] = false; // Test all DM actuators one by one
	m_bMenuFlags[8] = false; // Save mirror shape
	OnUpdateMenu();
	SetEvent(g_AOSACAParams->g_ehCamSnap);
	SetEvent(g_AOSACAParams->g_ehCLoopThread);
}

void CAOSACADlg::StopCLoopThread()
{	
	if (g_AOSACAParams->g_bCloseLoopThread)
	{
		CEvent waitEvent;
		g_AOSACAParams->g_bControlON?g_AOSACAParams->g_bControlON = false:g_AOSACAParams->g_bAutoMeasure = false;
		g_optcalc->set_CloopFlag(false);
		ResetEvent(g_AOSACAParams->g_ehCLoopThread);
		::WaitForSingleObject(waitEvent, g_AOSACAParams->EXPOSURE_MS<<5);
		g_AOSACAParams->g_bCloseLoopThread = false;			
		g_AOSACAParams->g_bSubstractBkGnd = false;
		// Update AOSACA Menu
		m_bMenuFlags[0] = true; // save image
		m_bMenuFlags[1] = true; // update pupil center
		m_bMenuFlags[2] = true; // update AOSACA parameters
		m_bMenuFlags[3] = g_pupilview->m_bHasVideoDlg; // Pupil Cam settings
		m_bMenuFlags[4] = g_AOSACAParams->g_bDMReady;  // Generate poke matrix
		m_bMenuFlags[5] = g_AOSACAParams->g_bDMReady;  // Set all DM actuators to '0'
		m_bMenuFlags[6] = g_AOSACAParams->g_bDMReady;  // Test all DM actuators one by one
		m_bMenuFlags[8] = g_AOSACAParams->g_bDMReady;  // Save mirror shape
		OnUpdateMenu();
	//	SetEvent(g_AOSACAParams->g_ehCamLive);
	}
}


void CAOSACADlg::OnEditPupilcamerasettings()
{
	// TODO: Add your command handler code here
	g_pupilview->OnEditPupilcamerasettings();
}


void CAOSACADlg::OnHelpAboutaosaca()
{
	// TODO: Add your command handler code here
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
