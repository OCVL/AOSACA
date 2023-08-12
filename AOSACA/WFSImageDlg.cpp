// WFSImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "WFSImageDlg.h"
#include "afxdialogex.h"

//Optivec library headers
#include "VecALL.h"
#include "MatALL.h"

extern AOSACAParams	*g_AOSACAParams;
extern CCentroid	*g_centroids;

// CWFSImageDlg dialog

IMPLEMENT_DYNAMIC(CWFSImageDlg, CDialogEx)

CWFSImageDlg::CWFSImageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWFSImageDlg::IDD, pParent)
{
	m_pParent=(CAOSACADlg *)pParent;
	m_ubData	= NULL;
	offscreenBitmap = NULL;
	bmi = NULL;
	pDIBSectionBits = NULL;
	hbm = NULL;
	m_bDrawSearchBoxes = false;
	m_bDrawCentroids = false;
}

CWFSImageDlg::~CWFSImageDlg()
{
}

void CWFSImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDH_WFSIMAGE, m_WFImageDisp);
	DDX_Control(pDX, IDL_CMAPBARG, m_CMapBargray);
	DDX_Control(pDX, IDL_CMAPBARS, m_CMapBarsummer);
}


BEGIN_MESSAGE_MAP(CWFSImageDlg, CDialogEx)
	ON_MESSAGE(WM_UPDATE_WINDOW, OnUpdateWindow)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CWFSImageDlg message handlers

void CWFSImageDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	delete m_gdiPenRed;
	delete m_gdiPenWhite;
	delete m_gdiPenYellow;
	if (m_ubData) delete [] m_ubData;
	delete bmi;
	delete matrix;

	CDialogEx::OnClose();
}

BOOL CWFSImageDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
    GetClientRect(&rect);
    CBrush *pOld = pDC->SelectObject(&g_AOSACAParams->m_bkgndbrush);
    BOOL bRes  = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOld);    // restore old brush
    return bRes;
//	return CDialogEx::OnEraseBkgnd(pDC);
}


BOOL CWFSImageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CFont *fnt1;
	fnt1 = new CFont();
	fnt1->CreateFont(20,8,0,0,600,0,1,0,0,0,0,0,0,_T("Arial"));
	// create pens	
	m_gdiPenRed = new Pen(Color(255, 0, 0), 1);
	m_gdiPenWhite = new Pen(Color(255, 255, 255), 1);
	m_gdiPenYellow = new Pen(Color(128, 128, 0), 1);

	DIBSecWidth = g_AOSACAParams->IMAGE_WIDTH_PIX;
	DIBSecHeight = g_AOSACAParams->IMAGE_HEIGHT_PIX;
	BitsPerPixel = 8;	
	m_ubData = new unsigned char [g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX];
	ZeroMemory(m_ubData, g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX);
	
	Stride = ((DIBSecWidth * BitsPerPixel + 31L) & (~31L)) / 8L;

	m_BUpdate = false;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CWFSImageDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (point.x > m_slx && point.x < m_slx+g_AOSACAParams->IMAGE_WIDTH_PIX && point.y > m_sly && point.y < m_sly+g_AOSACAParams->IMAGE_HEIGHT_PIX)
	{
		point.x -= m_slx;
		point.y -= m_sly;
		CDPoint pt;
		pt.x = point.x;
		pt.y = point.y;
		m_pParent->lBtnUp_ImageDlg(pt);
	}

	//CDialogEx::OnLButtonUp(nFlags, point);
}

void CWFSImageDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (point.x > m_slx && point.x < m_slx+g_AOSACAParams->IMAGE_WIDTH_PIX && point.y > m_sly && point.y < m_sly+g_AOSACAParams->IMAGE_HEIGHT_PIX)
	{
		point.x -= m_slx;
		point.y -= m_sly;
		m_pParent->mouseMove_ImageDlg(point);
	}

	//CDialogEx::OnMouseMove(nFlags, point);
}

void CWFSImageDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	if (m_BUpdate)
	{
		BYTE *pCurRowPixel = (BYTE *)(pDIBSectionBits);
		if (g_AOSACAParams->g_last_click == LIVE || g_AOSACAParams->g_last_click == SNAP)
			memcpy(pCurRowPixel, g_AOSACAParams->g_pImgBuffPrc, DIBSecWidth*DIBSecHeight);
		else
			memcpy(pCurRowPixel, g_AOSACAParams->g_pLocalImgBuff, DIBSecWidth*DIBSecHeight);
	
		imagedisp->DrawImage(offscreenBitmap, 0, 0, DIBSecWidth, DIBSecHeight);

		//draw searchboxes
		if(m_bDrawSearchBoxes)
		{
			double* dBoxCenters;
			unsigned short* usSearchArray;
			dBoxCenters = g_centroids->get_SearchBoxCenters();
			usSearchArray = g_centroids->get_SearchArray();
			int nTotalBoxes = VUS_sum(usSearchArray, (g_AOSACAParams->LENSLETGRID)*(g_AOSACAParams->LENSLETGRID));
			RectF* pRects;
			pRects = new RectF[nTotalBoxes];
			int nboxcount = 0;
			
			for (short indexx=0; indexx<g_AOSACAParams->LENSLETGRID; indexx++)
				for (short indexy=0; indexy<g_AOSACAParams->LENSLETGRID; indexy++)
					if (usSearchArray[indexy*g_AOSACAParams->LENSLETGRID+indexx] == 1)
					pRects[nboxcount++] = RectF(dBoxCenters[indexx<<1] - g_AOSACAParams->SEARCHBOX_SIZE/2,
												g_AOSACAParams->IMAGE_WIDTH_PIX - dBoxCenters[(indexy<<1)+1] - g_AOSACAParams->SEARCHBOX_SIZE/2,
												g_AOSACAParams->SEARCHBOX_SIZE, g_AOSACAParams->SEARCHBOX_SIZE);


			imagedisp->DrawRectangles(m_gdiPenYellow, pRects, nboxcount);
			delete [] pRects;
			m_bDrawSearchBoxes = false;
		}		
		//draw centroids
		if (m_bDrawCentroids)
		{
			double* dCentroids;
			bool*	bCentFound;
			bool*	bUseCentroid;
			short	sTotalCentroids;
			short	sGeoCentind;
			sTotalCentroids = g_centroids->get_TotalCentCount();
			dCentroids = g_centroids->get_Centroids(sGeoCentind);
			bCentFound = g_centroids->get_CentFound();
			bUseCentroid = g_centroids->get_UseCentroids();

			CDC *cdc;
			cdc=Display->GetDC();		
			long x,y;
			//update centroids color
			for (int i=0; i<sTotalCentroids; i++)
			{		
				x = (long) (dCentroids[i]);
				y = (long) (dCentroids[i+sTotalCentroids]);

				if (bUseCentroid[i])
				{
					if (bCentFound[i])
					{
						if (i == sGeoCentind)
						{
							cdc->SetPixelV(x, y, RGB(30,144,255));
							cdc->SetPixelV(x, y-3, RGB(30,144,255));
							cdc->SetPixelV(x, y+3, RGB(30,144,255));
							cdc->SetPixelV(x-3, y, RGB(30,144,255));
							cdc->SetPixelV(x+3, y, RGB(30,144,255));
						}
						else
						{
							cdc->SetPixelV(x, y, RGB(255,255,255));
							cdc->SetPixelV(x, y-3, RGB(255,255,255));
							cdc->SetPixelV(x, y+3, RGB(255,255,255));
							cdc->SetPixelV(x-3, y, RGB(255,255,255));
							cdc->SetPixelV(x+3, y, RGB(255,255,255));
						}
					}
					else
					{
						cdc->SetPixelV(x, y, RGB(255,0,0));
						cdc->SetPixelV(x-2, y-2, RGB(255,0,0));
						cdc->SetPixelV(x-2, y+2, RGB(255,0,0));
						cdc->SetPixelV(x+2, y-2, RGB(255,0,0));
						cdc->SetPixelV(x+2, y+2, RGB(255,0,0));
					}
				}
			}
			Display->ReleaseDC(cdc);
		
			m_bDrawCentroids = false;
		}
		m_BUpdate = false;
	}
	// Do not call CDialogEx::OnPaint() for painting messages
}

LRESULT CWFSImageDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
	m_msgID = lParam;

	switch (m_msgID) {
	case UPDATE_WINDOW:	
		m_BUpdate = true;
		OnPaint();
//		Invalidate(false);
		break;
	default:
		break;
	}
	
	return 0;
}

////////////////////////////
// User defined functions //
////////////////////////////

void CWFSImageDlg::InitParam() 
{	
	SetWFSImageColor();	

	Display = GetDlgItem(IDH_WFSIMAGE);
	this->GetClientRect(&m_imagedisp_rect);
	m_slx = 2;
	m_sly = 30;
	Display->SetWindowPos(NULL, m_slx, m_sly, g_AOSACAParams->IMAGE_HEIGHT_PIX, g_AOSACAParams->IMAGE_WIDTH_PIX, SWP_FRAMECHANGED);	
	Display->GetClientRect(&m_imagedisp_rect);
	m_WFImageDisp.GetClientRect(&m_imagedisp_rect);
	imagedisp = new Graphics(m_WFImageDisp.GetDC()->GetSafeHdc());
	matrix = new Matrix(1,0,0,-1,0,0); // Method1
	matrix->Translate(0,g_AOSACAParams->IMAGE_HEIGHT_PIX,MatrixOrderAppend); // Method1
	imagedisp->SetTransform(matrix); // Method1	
 
	if( m_CMapBargraybmp.m_hObject == 0 && m_CMapBargraybmp.LoadBitmap( IDB_GRAYCMAP ) )  
	{
		m_CMapBargray.SetBitmap( m_CMapBargraybmp );
	}
	if( m_CMapBarsummerbmp.m_hObject == 0 && m_CMapBarsummerbmp.LoadBitmap( IDB_SUMCMAP ) )  
	{
		m_CMapBarsummer.SetBitmap( m_CMapBarsummerbmp );
	}
	m_CMapBargray.SetWindowPos(NULL, m_slx, 5, 512, 20, SWP_FRAMECHANGED);
	m_CMapBarsummer.SetWindowPos(NULL, m_slx, 5, 512, 20, SWP_FRAMECHANGED);
}

void CWFSImageDlg::setCapture_ImageDlg(void)
{
	SetCapture();
}

void CWFSImageDlg::setCursor_ImageDlg(int val)
{
	if(val == ARROW)
		SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_ARROW)));
	else
		SetCursor(LoadCursor(NULL,  MAKEINTRESOURCE(IDC_CROSS)));
}

void CWFSImageDlg::releaseCapture_ImageDlg(void)
{
	ReleaseCapture();
}

void CWFSImageDlg::SetWFSImageColor()
{
	if (g_AOSACAParams->g_last_click == LIVE)
	{
		CEvent waitEvent;
		ResetEvent(g_AOSACAParams->g_ehCamLive);
		::WaitForSingleObject(waitEvent, g_AOSACAParams->EXPOSURE_MS<<1);
	}
	if (bmi != NULL)
	{
		delete bmi;
		bmi = NULL;
	}
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
	memcpy(bmi->bmiColors, g_AOSACAParams->USECOLORMAP?g_AOSACAParams->summercmap:g_AOSACAParams->greyscale, 256*sizeof(RGBQUAD));
	if (hbm != NULL)
		DeleteObject(hbm);
	hbm = ::CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, (void**)&pDIBSectionBits, NULL, 0);	
	if (offscreenBitmap != NULL)
	{
		delete offscreenBitmap;
		offscreenBitmap = NULL;
	}
	offscreenBitmap = new Bitmap(bmi, pDIBSectionBits);
	if (g_AOSACAParams->g_last_click == LIVE)
		SetEvent(g_AOSACAParams->g_ehCamLive);
	m_CMapBarsummer.ShowWindow(g_AOSACAParams->USECOLORMAP);
	m_CMapBargray.ShowWindow(!g_AOSACAParams->USECOLORMAP);
}
