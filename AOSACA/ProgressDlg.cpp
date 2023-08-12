// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "ProgressDlg.h"
#include "afxdialogex.h"

extern AOSACAParams		*g_AOSACAParams;
extern CControlPanelDlg	*g_controlpanel;
extern CCentroid		*g_centroids;

// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialogEx)

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProgressDlg::IDD, pParent)
{
	m_pParent = (CAOSACADlg *)pParent;
}

CProgressDlg::~CProgressDlg()
{
	OnClose();
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDP_PROGRESSBAR, m_pProgress);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_UPDATE_WINDOW, &CProgressDlg::OnUpdateWindow)
	ON_BN_CLICKED(IDCANCEL, &CProgressDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CProgressDlg message handlers
void CProgressDlg::SetBarParameters(short max)
{
	m_pProgress.SetRange(0, max);
	m_pProgress.SetPos(0);
	m_pProgress.SetStep(1);
	m_pProgress.SetState(PBST_NORMAL);
}

void CProgressDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	g_AOSACAParams->g_bPokeMatGeneration = false;
	m_pParent->UpdateMouseMode(MOUSE_NORMAL);
	g_controlpanel->EnableWindow(TRUE);
	EndDialog(IDOK);
//	CDialogEx::OnClose();
}

void CProgressDlg::Close() 
{
	OnClose();
}

afx_msg LRESULT CProgressDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
	switch (lParam) {
	case UPDATE_WINDOW:
		m_pProgress.StepIt();
		//OnPaint();
		//Invalidate(FALSE);
		break;
	default:
		break;
	}
	return 0;
}


void CProgressDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	m_pParent->OnUpdateGeneratePokeMatrixMenu();
	OnClose();
}

CString CProgressDlg::OnDoneGeneration() 
{
	// TODO: Add your message handler code here and/or call default
	CString ret;
	ret.Empty();
	SetDlgItemText(IDCANCEL,_T("Done"));
	g_AOSACAParams->g_stAppErrBuff = _T("Do you want to save newly generated Poke Matrix??"); 
	if (g_AOSACAParams->ShowError(MB_ICONQUESTION) == IDYES)
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		//Ask for a new filename
		TCHAR szFilters[]= _T("Poke Matrix Files (*.pmat)|*.pmat|");

		CString val;
		SYSTEMTIME systime;
		GetLocalTime(&systime);
		val.Format(_T("%i_%i_%i_%i_%i"), systime.wMonth, systime.wDay, systime.wYear, systime.wHour,systime.wMinute);
	   // Create an Open dialog; the default file name extension is ".my".
	   CFileDialog fileDlg(FALSE, _T("pmat"), val, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilters);

	   fileDlg.m_ofn.lpstrInitialDir  = g_AOSACAParams->g_stAppHomePath+"utils";
	   fileDlg.m_ofn.lpstrTitle  = _T("New Poke matrix file name");

		if( fileDlg.DoModal() == IDOK )
		{
			CString fileName = fileDlg.GetPathName();
			if (!fileName.IsEmpty())
			{
				if (!g_centroids->SavePmat(fileName))
					AfxMessageBox(_T("Can not open Pmat file to write, check folder permissions") ,MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
				else
					ret = fileName;
			}
		}
	}
	return ret;
}