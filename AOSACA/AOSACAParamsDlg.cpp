// AOSACAParamsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "AOSACAParamsDlg.h"
#include "afxdialogex.h"
#include <iterator>

extern AOSACAParams	*g_AOSACAParams;
extern CCentroid	*g_centroids;

// CAOSACAParamsDlg dialog

IMPLEMENT_DYNAMIC(CAOSACAParamsDlg, CDialogEx)

CAOSACAParamsDlg::CAOSACAParamsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAOSACAParamsDlg::IDD, pParent)
	, m_WFSImgColor(0)
{

}

CAOSACAParamsDlg::~CAOSACAParamsDlg()
{
}

void CAOSACAParamsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDR_PARAMETERS_WFSIMAGE_GREYSCALE, m_WFSImgColor);
}


BEGIN_MESSAGE_MAP(CAOSACAParamsDlg, CDialogEx)
	ON_BN_CLICKED(IDB_PARAMETERS_AOLOOP_INFMAT_SEL, &CAOSACAParamsDlg::OnBnClickedParametersAoloopInfmatSel)
END_MESSAGE_MAP()


// CAOSACAParamsDlg message handlers


BOOL CAOSACAParamsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here	
	m_bAppUpdate[0] = m_bAppUpdate[1] = m_bAppUpdate[2] = m_bAppUpdate[3] = m_bAppUpdate[4] = m_bAppUpdate[5] = m_bAppUpdate[6] = false;
	RECT rect;
	//System parameters
	rect.left = 185;	rect.right  = rect.left+ 65;
	rect.top  = 47; rect.bottom = rect.top + 20;
	m_eWFmapWidth.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_SYSTEM_WFMAP);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_ePSFmapWidth.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_SYSTEM_PSFMAP);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eWavelength.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_SYSTEM_WFS);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eLensletDia.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_SYSTEM_LENSLETDIA);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eLensletFL.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_SYSTEM_LENSLETFL);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eLensletGrid.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_SYSTEM_LENSLETGRID);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eMagnification.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_SYSTEM_MAG);
	//User parameters
	rect.top  = rect.bottom + 34; rect.bottom = rect.top + 20;
	m_ePupilSizemax.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_USER_PUPIL);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eSearchboxSize.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_USER_SEARCHBOX);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eCentMaxPass1.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_USER_CENTMAX1);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eCentMaxPass2.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_USER_CENTMAX2);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eCentMaxPass3.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_USER_CENTMAX3);
	//Camera parameters	
	rect.left = 185;	rect.right  = rect.left+ 65;
	rect.top  = rect.bottom + 76; rect.bottom = rect.top + 20;
	m_eCamPitch.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_CAMERA_PITCH);
	//DM parameters	
	rect.top  = rect.bottom + 31; rect.bottom = rect.top + 20;
	m_eDMSerial.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_DM_SERIAL);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eDMDefPMG.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_DM_MAXDEFPMG);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eDMmaxDef.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_DM_MAXDEFAO);					 
	//AO Loop parameters	
	rect.top  = rect.bottom + 31; rect.bottom = rect.top + 20;
	m_ePMGIter.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_AOLOOP_PMGITER);
	rect.top  = rect.bottom + 5; rect.bottom = rect.top + 20;
	m_eAOCycles.Create(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,  
					 rect, this, IDE_PARAMETERS_AOLOOP_CYCLES);

	m_dWavelength = g_AOSACAParams->WAVELENGTH_MICRONS;
	m_dLensletDia = g_AOSACAParams->LENSLET_DIAMETER_MICRONS;
	m_dLensletFL = g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS;
	m_dMagnification = g_AOSACAParams->MAGNIFICATION;
	m_shPupilSizemax = (short)g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS;
	m_dCamPitch = g_AOSACAParams->MICRONS_PER_PIXEL;
	m_dDMdefPMG = g_AOSACAParams->DM_POKE_MAT_MAX_DEF;
	m_dDMmaxDef = g_AOSACAParams->DM_MAX_DEFLECTION;

	m_shWFmapWidth = g_AOSACAParams->WAVE_WIDTH_PIX;
	m_shPSFmapWidth = g_AOSACAParams->PSF_MTF_WIDTH_PIX;
	m_shLensletGrid = g_AOSACAParams->LENSLETGRID;
	m_shSearchboxSize = g_AOSACAParams->SEARCHBOX_SIZE;
	m_shCentMaxPass1 = g_AOSACAParams->CENT_MAX_PASS1;
	m_shCentMaxPass2 = g_AOSACAParams->CENT_MAX_PASS2;
	m_shCentMaxPass3 = g_AOSACAParams->CENT_MAX_PASS3;
	m_shPMGIter = g_AOSACAParams->DM_POKE_MAT_ITER;
	m_shAOCycles = g_AOSACAParams->CYCLES_BELOW_SETCENTROIDNO;

	m_stDMSerial = g_AOSACAParams->DM_SERIAL_NO;
	m_stPokeMatrixFile = g_AOSACAParams->POKE_MATRIX_FILE;
	//Initialize all vlaues
	m_eWFmapWidth.SetValidation(MYEDIT_INTEGER, m_shWFmapWidth, 100, 120, 3, -1, false);
	m_ePSFmapWidth.SetValidation(MYEDIT_INTEGER, m_shPSFmapWidth, 110, 130, 3, -1, false);
	m_eWavelength.SetValidation(MYEDIT_DOUBLE, m_dWavelength, 0.543, 0.940, 1, 3, false);
	m_eLensletDia.SetValidation(MYEDIT_DOUBLE, m_dLensletDia, 180.0, 190.0, 3, 1, false) ;
	m_eLensletFL.SetValidation(MYEDIT_INTEGER, m_dLensletFL, 7000, 9000, 4, -1, false);
	m_eLensletGrid.SetValidation(MYEDIT_INTEGER, m_shLensletGrid, 25, 30, 2, -1, false);
	m_eMagnification.SetValidation(MYEDIT_DOUBLE, m_dMagnification, 0.5, 1.5, 1, 2, false);
	m_ePupilSizemax.SetValidation(MYEDIT_INTEGER, m_shPupilSizemax, 5000, 7200, 4, -1, false);
	m_eSearchboxSize.SetValidation(MYEDIT_INTEGER, m_shSearchboxSize, 10, 20, 2, -1, false);
	m_eCentMaxPass1.SetValidation(MYEDIT_INTEGER, m_shCentMaxPass1, 10, 20, 2, -1, false);
	m_eCentMaxPass2.SetValidation(MYEDIT_INTEGER, m_shCentMaxPass2, 5, 15, 2, -1, false);
	m_eCentMaxPass3.SetValidation(MYEDIT_INTEGER, m_shCentMaxPass3, 2, 10, 2, -1, false);
	m_eCamPitch.SetValidation(MYEDIT_DOUBLE, m_dCamPitch, 5.0, 15.0, 2, 2, false);
	m_eDMSerial.SetValidation(MYEDIT_ALPHANUMERIC);
	m_eDMDefPMG.SetValidation(MYEDIT_DOUBLE, m_dDMdefPMG, -0.5, 0.5, 1, 2, true);
	m_eDMmaxDef.SetValidation(MYEDIT_DOUBLE, m_dDMmaxDef, -0.9, 0.9, 1, 2, true);
	m_ePMGIter.SetValidation(MYEDIT_INTEGER, m_shPMGIter, 1, 5, 1, -1, false);
	m_eAOCycles.SetValidation(MYEDIT_INTEGER, m_shAOCycles, 10, 50, 2, -1, false);

	SetDlgItemTextW(IDE_PARAMETERS_DM_SERIAL, m_stDMSerial);
	SetDlgItemTextW(IDL_PARAMETERS_AOLOOP_INFMAT_PATH, m_stPokeMatrixFile);
	m_WFSImgColor = g_AOSACAParams->USECOLORMAP?1:0;
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CAOSACAParamsDlg::PreTranslateMessage(MSG* pMsg)
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
				if (pWnd == &m_eWFmapWidth)
				{
					m_eWFmapWidth.CheckValidity();
					m_shWFmapWidth = (short)(GetDlgItemInt(IDE_PARAMETERS_SYSTEM_WFMAP,NULL,false));				
				}				
				else if (pWnd == &m_ePSFmapWidth)
				{
					m_ePSFmapWidth.CheckValidity();
					m_shPSFmapWidth = (short)(GetDlgItemInt(IDE_PARAMETERS_SYSTEM_PSFMAP,NULL, false));
				}
				else if (pWnd == &m_eWavelength)
				{
					m_eWavelength.CheckValidity();
					GetDlgItemText(IDE_PARAMETERS_SYSTEM_WFS,text);
					m_dWavelength = _ttof(text);
				}
				else if (pWnd == &m_eLensletDia)
				{
					m_eLensletDia.CheckValidity();
					GetDlgItemText(IDE_PARAMETERS_SYSTEM_LENSLETDIA,text);
					m_dLensletDia = _ttof(text);
				}
				else if (pWnd == &m_eLensletFL)
				{
					m_eLensletFL.CheckValidity();
					GetDlgItemText(IDE_PARAMETERS_SYSTEM_LENSLETFL,text);
					m_dLensletFL = _ttof(text);
				}
				else if (pWnd == &m_eLensletGrid)
				{
					m_eLensletGrid.CheckValidity();
					m_shLensletGrid = (short)(GetDlgItemInt(IDE_PARAMETERS_SYSTEM_LENSLETGRID,NULL, false));
				}
				else if (pWnd == &m_eMagnification)
				{
					m_eMagnification.CheckValidity();
					GetDlgItemText(IDE_PARAMETERS_SYSTEM_MAG,text);
					m_dMagnification = _ttof(text);
				}
				else if (pWnd == &m_ePupilSizemax)
				{
					m_ePupilSizemax.CheckValidity();
					m_shPupilSizemax = (short)(GetDlgItemInt(IDE_PARAMETERS_USER_PUPIL,NULL, false));
				}			
				else if (pWnd == &m_eSearchboxSize)
				{
					m_eSearchboxSize.CheckValidity();
					m_shSearchboxSize = (short)(GetDlgItemInt(IDE_PARAMETERS_USER_SEARCHBOX,NULL, false));
				}
				else if (pWnd == &m_eCentMaxPass1)
				{
					m_eCentMaxPass1.CheckValidity();
					m_shCentMaxPass1 = (short)(GetDlgItemInt(IDE_PARAMETERS_USER_CENTMAX1,NULL, false));
				}
				else if (pWnd == &m_eCentMaxPass2)
				{
					m_eCentMaxPass2.CheckValidity();
					m_shCentMaxPass2 = (short)(GetDlgItemInt(IDE_PARAMETERS_USER_CENTMAX2,NULL, false));
				}
				else if (pWnd == &m_eCentMaxPass3)
				{
					m_eCentMaxPass3.CheckValidity();
					m_shCentMaxPass3 = (short)(GetDlgItemInt(IDE_PARAMETERS_USER_CENTMAX3,NULL, false));
				}
				else if (pWnd == &m_eCamPitch)
				{
					m_eCamPitch.CheckValidity();
					GetDlgItemText(IDE_PARAMETERS_CAMERA_PITCH,text);
					m_dCamPitch = _ttof(text);
				}
				else if (pWnd == &m_eDMSerial)
				{
					GetDlgItemText(IDE_PARAMETERS_CAMERA_PITCH,text);
					text.Trim();
					m_stDMSerial = text;
				}
				else if (pWnd == &m_eDMDefPMG)
				{
					m_eDMDefPMG.CheckValidity();
					GetDlgItemText(IDE_PARAMETERS_DM_MAXDEFPMG,text);
					m_dDMdefPMG = _ttof(text);
				}
				else if (pWnd == &m_eDMmaxDef)
				{
					m_eDMmaxDef.CheckValidity();
					GetDlgItemText(IDE_PARAMETERS_DM_MAXDEFAO,text);
					m_dDMmaxDef = _ttof(text);
				}
				else if (pWnd == &m_ePMGIter)
				{
					m_ePMGIter.CheckValidity();
					m_shPMGIter = (short)(GetDlgItemInt(IDE_PARAMETERS_AOLOOP_PMGITER, false));
				}
				else if (pWnd == &m_eAOCycles)
				{
					m_eAOCycles.CheckValidity();
					m_shAOCycles = (short)(GetDlgItemInt(IDE_PARAMETERS_AOLOOP_CYCLES, false));
				}
				text = '\0';
				pWnd = GetNextDlgTabItem(GetFocus());
				if (pWnd)
					GotoDlgCtrl(pWnd);
				return false;
			}
			break;
		case VK_ESCAPE:
			break;
		}		
	}
	
	return false;

//	return CDialogEx::PreTranslateMessage(pMsg);
}


void CAOSACAParamsDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData(TRUE);
	if (g_AOSACAParams->WAVE_WIDTH_PIX != m_shWFmapWidth || g_AOSACAParams->PSF_MTF_WIDTH_PIX != m_shPSFmapWidth)
	{
		g_AOSACAParams->WAVE_WIDTH_PIX = m_shWFmapWidth;
		g_AOSACAParams->PSF_MTF_WIDTH_PIX = m_shPSFmapWidth;
		m_bAppUpdate[0] = true;
	}
	g_AOSACAParams->DM_POKE_MAT_MAX_DEF = m_dDMdefPMG;
	if (g_AOSACAParams->DM_MAX_DEFLECTION != m_dDMmaxDef)
	{
		g_AOSACAParams->DM_MAX_DEFLECTION = m_dDMmaxDef;
		m_bAppUpdate[6] = true;
	}		
	g_AOSACAParams->CYCLES_BELOW_SETCENTROIDNO = m_shAOCycles;
	g_AOSACAParams->DM_POKE_MAT_ITER = m_shPMGIter;
	g_AOSACAParams->SEARCHBOX_SIZE = m_shSearchboxSize;
	g_AOSACAParams->CENT_MAX_PASS1 = m_shCentMaxPass1;
	g_AOSACAParams->CENT_MAX_PASS2 = m_shCentMaxPass2;
	g_AOSACAParams->CENT_MAX_PASS3 = m_shCentMaxPass3;
	if (g_AOSACAParams->LENSLET_DIAMETER_MICRONS != m_dLensletDia
		|| g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS != m_dLensletFL
		|| g_AOSACAParams->LENSLETGRID != m_shLensletGrid
		|| g_AOSACAParams->MAGNIFICATION != m_dMagnification
		|| g_AOSACAParams->MICRONS_PER_PIXEL != m_dCamPitch)
	{
		g_AOSACAParams->LENSLET_DIAMETER_MICRONS = m_dLensletDia;
		g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS = m_dLensletFL;
		g_AOSACAParams->LENSLETGRID = m_shLensletGrid;
		g_AOSACAParams->MAGNIFICATION = m_dMagnification;
		g_AOSACAParams->MICRONS_PER_PIXEL = m_dCamPitch;
		m_bAppUpdate[1] = true;
	}
	if(g_AOSACAParams->WAVELENGTH_MICRONS != m_dWavelength)
	{//update COptcalc
		g_AOSACAParams->WAVELENGTH_MICRONS = m_dWavelength;
		m_bAppUpdate[2] = true;
	}
	if (g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS != m_shPupilSizemax)
	{
		m_bAppUpdate[3] = true;
		g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS = m_shPupilSizemax;
	}
	if (g_AOSACAParams->DM_SERIAL_NO != m_stDMSerial )
	{
		g_AOSACAParams->DM_SERIAL_NO = m_stDMSerial;
		m_bAppUpdate[4] = true;
	}
	if (g_AOSACAParams->USECOLORMAP != m_WFSImgColor?true:false)
	{
		g_AOSACAParams->USECOLORMAP = m_WFSImgColor?true:false;
		m_bAppUpdate[5] = true;
	}
	if (g_AOSACAParams->POKE_MATRIX_FILE != m_stPokeMatrixFile )
	{
		g_AOSACAParams->POKE_MATRIX_FILE = m_stPokeMatrixFile;
		m_bAppUpdate[0] = true;
	}

	CDialogEx::OnOK();
}


void CAOSACAParamsDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialogEx::OnCancel();
}


void CAOSACAParamsDlg::OnBnClickedParametersAoloopInfmatSel()
{
	// TODO: Add your control notification handler code here
	this->UpdateData();	
	TCHAR szFilters[]= _T("Poke Matrix Files (*.pmat)|*.pmat|");

   // Create an Open dialog; the default file name extension is ".my".
   CFileDialog fileDlg(TRUE, _T("pmat"), g_AOSACAParams->POKE_MATRIX_FILE,
      OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

   fileDlg.m_ofn.lpstrInitialDir  = g_AOSACAParams->g_stAppHomePath+"utils";
   fileDlg.m_ofn.lpstrTitle  = _T("Select a Poke matrix file");

	if( fileDlg.DoModal() == IDOK )
	{
		CString pathName = fileDlg.GetPathName();
		CFileStatus filestatus;
		CFile::GetStatus( pathName, filestatus );
		std::ifstream   file(pathName);
		int count = std::distance(std::istream_iterator<double>(file),
									std::istream_iterator<double>());
		if (count != g_AOSACAParams->NUMACTS*(g_centroids->get_TotalCentCount()<<1))
		{//wrong file size
			g_AOSACAParams->g_stAppErrBuff = _T("Wrong Poke Matrix file size, try another file"); 
			g_AOSACAParams->ShowError(MB_ICONERROR);
		}
		else
		{
			CString folderPath = pathName.Left(pathName.ReverseFind('\\'));
			if (folderPath.Compare(g_AOSACAParams->g_stAppHomePath+"utils")) //not in the same folder
				CopyFile(pathName, g_AOSACAParams->g_stAppHomePath+"utils\\"+fileDlg.GetFileName(), TRUE);
			m_stPokeMatrixFile = fileDlg.GetFileTitle();			
			SetDlgItemTextW(IDL_PARAMETERS_AOLOOP_INFMAT_PATH, m_stPokeMatrixFile);
		}
	}
	else
		return;
	this->UpdateData(FALSE);
}
