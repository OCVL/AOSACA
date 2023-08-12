#pragma once
#include "afxwin.h"


#define MOUSE_ADJUST_X	5
#define MOUSE_ADJUST_Y	5

#include "AOSACADlg.h"

// CWFSImageDlg dialog

class CWFSImageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWFSImageDlg)

public:
	CWFSImageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWFSImageDlg();
	// User defined functions & variables
	bool		m_bDrawSearchBoxes;
	bool		m_bDrawCentroids;
	void		setCapture_ImageDlg(void);
	void		releaseCapture_ImageDlg(void);
	void		setCursor_ImageDlg(int val);
	void		InitParam();
	void		SetWFSImageColor();

private:
	CAOSACADlg	*m_pParent;
	BOOL		m_BUpdate;	
	LONG		DIBSecWidth;
	LONG		DIBSecHeight;
	LONG		BitsPerPixel;
	LONG		Stride;
	BITMAPINFO*	bmi;
	BYTE*		pDIBSectionBits;
	unsigned char*	m_ubData;
	short		m_slx;
	short		m_sly;
	HBITMAP		hbm;
	long		m_msgID;
	Graphics*	imagedisp;
	Bitmap*		offscreenBitmap;
	RECT		m_imagedisp_rect;
	Matrix*		matrix;
	CWnd*		Display;	
	Pen*		m_gdiPenRed;
	Pen*		m_gdiPenWhite;
	Pen*		m_gdiPenYellow;
	CBitmap		m_CMapBargraybmp; 
	CBitmap		m_CMapBarsummerbmp; 

	void		ClearImageArea();

// Dialog Data
	enum { IDD = IDD_WFSIMAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	afx_msg LRESULT	OnUpdateWindow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CStatic m_WFImageDisp;
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CStatic m_CMapBargray;
	CStatic m_CMapBarsummer;
};