// WFMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "WFMapDlg.h"
#include "afxdialogex.h"

extern AOSACAParams	*g_AOSACAParams;
// CWFMapDlg dialog

IMPLEMENT_DYNAMIC(CWFMapDlg, CDialogEx)

CWFMapDlg::CWFMapDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWFMapDlg::IDD, pParent)
{
	colormap	= NULL;
	m_dData		= NULL;
	m_dDatatemp	= NULL;
	m_usDataGrid = NULL;
	m_bData		= NULL;
}

CWFMapDlg::~CWFMapDlg()
{
	OnClose();
}

void CWFMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWFMapDlg, CDialogEx)
	ON_MESSAGE(WM_UPDATE_WINDOW, OnUpdateWindow)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CWFMapDlg message handlers

void CWFMapDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (colormap) delete [] colormap;
	if (m_dData) delete [] m_dData;
	if (m_dDatatemp) delete [] m_dDatatemp;
	if (m_usDataGrid) delete [] m_usDataGrid;
	if (m_bData) delete [] m_bData;
	CDialogEx::OnClose();
}

HBRUSH CWFMapDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
	return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());
}

BOOL CWFMapDlg::OnEraseBkgnd(CDC* pDC)
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

BOOL CWFMapDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	Display = GetDlgItem(IDL_WFMAP);
	Display->SetWindowPos(NULL, 
						  85-(g_AOSACAParams->WAVE_WIDTH_PIX/2), 
						  90-(g_AOSACAParams->WAVE_WIDTH_PIX/2), 
						  g_AOSACAParams->WAVE_WIDTH_PIX+6, 
						  g_AOSACAParams->WAVE_WIDTH_PIX+6, 
						  SWP_FRAMECHANGED);
	m_BUpdate = true;
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


void CWFMapDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages
	if (m_BUpdate)
	{
		VD_subC(m_dDatatemp, m_dData, m_nDataSize, m_dMin);
		VD_mulC(m_dDatatemp, m_dDatatemp, m_nDataSize, (511.0/(m_dMax-m_dMin)));
		VD_limit(m_dDatatemp, m_dDatatemp, m_nDataSize, 0, 511);
		CDC *dc = Display->GetDC();
		HGDIOBJ holdbm;
		short idx;
		RGBTRIPLE *pCurRowPixel = (RGBTRIPLE *)(pDIBSectionBits);
		short i=-1;
		int x,y;
		for (y=0; y<DIBSecHeight;y++)
			for (x=0; x<DIBSecWidth; x++)
			{
				idx = x * DIBSecHeight + y;
				if (m_bData[idx])
					m_usDataGrid[idx] = (unsigned short) m_dDatatemp[++i];
			}
		// The order for both 'for' loops below needs to be updated based upon WFMap orientation wrt Wavefront sensor
		for (y =DIBSecHeight-1; y>=0 ; --y)
		{
			for (x = 0; x < DIBSecWidth; ++x)
			{
				idx = x * DIBSecHeight + y;	
				if (m_bData[idx])
				{
					pCurRowPixel[x].rgbtBlue = GetBValue(colormap[m_usDataGrid[idx]]);
					pCurRowPixel[x].rgbtGreen = GetGValue(colormap[m_usDataGrid[idx]]);
					pCurRowPixel[x].rgbtRed = GetRValue(colormap[m_usDataGrid[idx]]);
				}
				else
				{					
					pCurRowPixel[x].rgbtBlue = 0;
					pCurRowPixel[x].rgbtGreen = 0;
					pCurRowPixel[x].rgbtRed = 0;
				}				
			}
			pCurRowPixel = (RGBTRIPLE *)((BYTE*)pCurRowPixel + Stride);
		}
		
		holdbm = memDC.SelectObject(hbm);
		dc->BitBlt(0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight,&memDC,0,0,SRCCOPY);
		memDC.SelectObject(holdbm);		
		ReleaseDC(dc);
	}
}


LRESULT CWFMapDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
	m_msgID = lParam;

	switch (m_msgID) {
	case UPDATE_WINDOW:	
		if (m_BUpdate) {
		OnPaint();
//		Invalidate(false);
		}
		break;
	default:
		break;
	}
	
	return 0;
}

////////////////////////////
// User defined functions //
////////////////////////////

void CWFMapDlg::InitParam() 
{
	m_dMax = 1.0;
	m_dMin = -1.0;
	BitsPerPixel = 24;
	DIBSecWidth = g_AOSACAParams->WAVE_WIDTH_PIX;
	DIBSecHeight = g_AOSACAParams->WAVE_WIDTH_PIX;

	m_bData = new bool [DIBSecWidth*DIBSecHeight];	
	ZeroMemory(m_bData,(DIBSecWidth*DIBSecHeight)*sizeof(bool));
	
	Stride = ((DIBSecWidth * BitsPerPixel + 31L) & (~31L)) / 8L;	
	
	memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = DIBSecWidth;
	bmi.bmiHeader.biHeight = DIBSecHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = (WORD)BitsPerPixel;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = Stride * abs(DIBSecHeight);
	hbm = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDIBSectionBits, NULL, 0);
	memDC.CreateCompatibleDC(NULL);

	colormap = new COLORREF[512];
	memcpy(colormap, g_AOSACAParams->dmcolormap, 512*sizeof(COLORREF));
}

void CWFMapDlg::SetDataPoints(short count)
{
	m_nDataSize = count;
	if (m_dData != NULL)
		delete [] m_dData;
	m_dData = new double [m_nDataSize];
	if (m_dDatatemp != NULL)
		delete [] m_dDatatemp;
	m_dDatatemp = new double [m_nDataSize];
	if (m_usDataGrid != NULL)
		delete [] m_usDataGrid;
	m_usDataGrid = new unsigned short[DIBSecWidth*DIBSecHeight];
	memset(m_usDataGrid, 0, DIBSecWidth*DIBSecHeight*sizeof(unsigned short));
}
