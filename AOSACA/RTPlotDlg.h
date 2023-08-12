#pragma once
#include "afxwin.h"


// CRTPlotDlg dialog

class CRTPlotDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRTPlotDlg)

private:	
	void		fill_PlotRgn();
	double		m_HighSlopeLimit;
	double		m_LowSlopeLimit;
	CPen		*pen_rms;
	CPen		*pen_strehl;
	Pen			*pen_divisonsdashdot;
	Pen			*pen_divisons;
	SolidBrush	*brush_zmodes;
	HDC			*hDc;
	Graphics	*plotarea;
	Graphics	*pOffscreenGraphics;
	bool		m_bInitPlot;
	CPoint		m_LastPointRms;
	CPoint		m_CurrentPointRms;
	CPoint		m_LastPointStrehl;
	CPoint		m_CurrentPointStrehl;
	RECT		m_slopetime_rect;
	double		*m_RmsData;
	double		*m_StrehlData;
	int			m_SlopeDataCounter;
	Bitmap		*offscreenBitmap;

public:
	CRTPlotDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRTPlotDlg();
	BOOL		m_BUpdate;
	void		Set_SlopePlotData(double rms, double strehl);
	void		Reset_SlopePlotData();

// Dialog Data
	enum { IDD = IDD_RTPLOT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnUpdateWindow(WPARAM wParam, LPARAM lParam);
public:
	CStatic m_PlotArea;
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};