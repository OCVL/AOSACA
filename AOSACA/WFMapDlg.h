#pragma once


// CWFMapDlg dialog

class CWFMapDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWFMapDlg)

public:
	CWFMapDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWFMapDlg();
	LONG		DIBSecWidth;
	LONG		DIBSecHeight;
	BOOL		m_BUpdate;
	double		*m_dData;
	bool		*m_bData;
	void		InitParam();
	void		SetDataPoints(short);

private:
	CWnd		*Display;
	LONG		BitsPerPixel;
	LONG		Stride;
	BITMAPINFO	bmi;
	BYTE		*pDIBSectionBits;
	HBITMAP		hbm;
	CDC			memDC;
	COLORREF	*colormap;
	long		m_msgID;
	short		m_nDataSize;
	double		m_dMax;
	double		m_dMin;
	double		*m_dDatatemp;
	unsigned short *m_usDataGrid;

// Dialog Data
	enum { IDD = IDD_WFMAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT	OnUpdateWindow(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};