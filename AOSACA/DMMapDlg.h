#pragma once
#include "afxwin.h"


// CDMMapDlg dialog

class CDMMapDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDMMapDlg)

public:
	CDMMapDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDMMapDlg();
	void		InitParam();
	BOOL		m_BUpdate;	
	double		*m_DMVoltages;
	CBitmap		m_CMapDMbmp;

private:
	long		m_msgID;
	double		m_dMaxDef;
	COLORREF	*colormap;
	RECT		*m_DMActRects;
	BOOL		*m_bDMSelection;
	void		UpdateSelectedActuators();
	void		color_AllActuators();

// Dialog Data
	enum { IDD = IDD_DMMAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT	OnUpdateWindow(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CStatic m_DMCmap;
};
