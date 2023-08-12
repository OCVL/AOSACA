#pragma once
#include "afxcmn.h"
#include "AOSACADlg.h"

// CProgressDlg dialog

class CProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProgressDlg();
	void	SetBarParameters(short max);
	CString	OnDoneGeneration();
	void	Close();

private:	
	CAOSACADlg	*m_pParent;

// Dialog Data
	enum { IDD = IDD_PROGRESSDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_pProgress;
	afx_msg void OnClose();
protected:
	afx_msg LRESULT OnUpdateWindow(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedCancel();
};
