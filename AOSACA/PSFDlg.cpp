// PSFDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "PSFDlg.h"
#include "afxdialogex.h"

extern AOSACAParams *g_AOSACAParams;

#include "VecAll.h"
#include "MatALL.h"


// CPSFDlg dialog

IMPLEMENT_DYNAMIC(CPSFDlg, CDialogEx)

CPSFDlg::CPSFDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPSFDlg::IDD, pParent)
{
	m_dData		= NULL;
	m_dDatatemp	= NULL;
	m_ubData	= NULL;
}

CPSFDlg::~CPSFDlg()
{
	OnClose();
}

void CPSFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPSFDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_UPDATE_WINDOW, &CPSFDlg::OnUpdateWindow)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CPSFDlg message handlers

void CPSFDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (m_dData) delete [] m_dData;
	if (m_dDatatemp) delete [] m_dDatatemp;	
	if (m_ubData) delete [] m_ubData;
	delete bmi;
	CDialogEx::OnClose();
}

HBRUSH CPSFDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
//	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
		pDC->SetBkColor (BACKGROUNDCOLOR);
			return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());
		break;
	default:
		break;

	}

	// TODO:  Return a different brush if the default is not desired
	return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());;
}

BOOL CPSFDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
    GetClientRect(&rect);
    CBrush myBrush(RGB(0, 0, 0));    // dialog background color
    CBrush *pOld = pDC->SelectObject(&myBrush);
    BOOL bRes  = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOld);    // restore old brush
    return bRes;
//	return CDialogEx::OnEraseBkgnd(pDC);
}

BOOL CPSFDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here	
	Display = GetDlgItem(IDL_PSFMAP);
	Display->SetWindowPos(NULL, 
						  78-(g_AOSACAParams->PSF_MTF_WIDTH_PIX/2), 
						  90-(g_AOSACAParams->PSF_MTF_WIDTH_PIX/2), 
						  g_AOSACAParams->PSF_MTF_WIDTH_PIX+6, 
						  g_AOSACAParams->PSF_MTF_WIDTH_PIX+6, 
						  SWP_FRAMECHANGED); 
	m_BUpdate = true;
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

void CPSFDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages
	if (m_BUpdate)
	{
		// Determine max and min in image for scaling PSF
		double max,min;
		unsigned int size = DIBSecWidth*DIBSecHeight;
		max = VD_max(m_dData, size);
		min = VD_min(m_dData, size);
		if (max-min >0)
		{
			VD_subC(m_dDatatemp, m_dData, size, min);
			VD_mulC(m_dDatatemp, m_dDatatemp, size, (UCHAR_MAX/(max-min)));
		}
		else if(max && !(max-min))
		{
			VD_subV(m_dDatatemp, m_dData, m_dData, size);
			VD_addC(m_dDatatemp, m_dDatatemp, size, UCHAR_MAX);
		}
		VD_roundtoUB(m_ubData, m_dDatatemp, size);
		MUB_rotate270(&m_ubData, &m_ubData, DIBSecHeight, DIBSecWidth);
		
		CDC *dc = Display->GetDC();
		HGDIOBJ holdbm;
		BYTE *pCurRowPixel = (BYTE *)(pDIBSectionBits);
		memcpy(pCurRowPixel, m_ubData, size);			
		holdbm = memDC.SelectObject(hbm);
		HGDIOBJ hbmResult = dc->SelectObject(hbm);	
		BitBlt(dc->m_hDC,0, 0,bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight, memDC.m_hDC, 0, 0, SRCCOPY );	
		::SelectObject( memDC.m_hDC, (HBITMAP)::SelectObject( memDC.m_hDC, holdbm ));	
		::SelectObject( dc->m_hDC, (HBITMAP)::SelectObject( dc->m_hDC, hbmResult ));
		ReleaseDC(dc);
	}
}

LRESULT CPSFDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
	m_msgID = lParam;

	switch (m_msgID) {
	case UPDATE_WINDOW:	
		if (m_BUpdate) {
		OnPaint();
	//	Invalidate(false);
		}
		break;
	default:
		break;
	}
	
	return 0;
}

///////////////////////////
// User defined function //
///////////////////////////

void CPSFDlg::InitParam() 
{
	DIBSecWidth = g_AOSACAParams->PSF_MTF_WIDTH_PIX;
	DIBSecHeight = g_AOSACAParams->PSF_MTF_WIDTH_PIX;
	BitsPerPixel = 8;
	m_dData = new double [DIBSecWidth*DIBSecHeight];
	m_dDatatemp = new double [DIBSecWidth*DIBSecHeight];
	m_ubData = new unsigned char [DIBSecWidth*DIBSecHeight];

	
	Stride = ((DIBSecWidth * BitsPerPixel + 31L) & (~31L)) / 8L;	
	
	bmi = (BITMAPINFO *)new BYTE[sizeof(BITMAPINFO) + UCHAR_MAX * sizeof(RGBQUAD)];
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = DIBSecWidth;
	bmi->bmiHeader.biHeight = DIBSecHeight;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = (WORD)BitsPerPixel;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0;//Stride * abs(DIBSecHeight);
	bmi->bmiHeader.biXPelsPerMeter = 0;
	bmi->bmiHeader.biYPelsPerMeter = 0;
	bmi->bmiHeader.biClrUsed = 0;
	bmi->bmiHeader.biClrImportant = 0;
	memcpy(bmi->bmiColors, g_AOSACAParams->greyscale, 256*sizeof(RGBQUAD));
	hbm = ::CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, (void**)&pDIBSectionBits, NULL, 0);
	memDC.CreateCompatibleDC(NULL);
}