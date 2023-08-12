#pragma once


// CPSFDlg dialog

class CPSFDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPSFDlg)

public:
	CPSFDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPSFDlg();
	BOOL		m_BUpdate;
	double		*m_dData;
	void		InitParam();

	
private:
	CWnd		*Display;	
	LONG		DIBSecWidth;
	LONG		DIBSecHeight;
	LONG		BitsPerPixel;
	LONG		Stride;
	BITMAPINFO	*bmi;
	BYTE		*pDIBSectionBits;
	HBITMAP		hbm;
	CDC			memDC;
	long		m_msgID;
	double		*m_dDatatemp;
	unsigned char	*m_ubData;

// Dialog Data
	enum { IDD = IDD_PSFMAP };

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