// DMMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "DMMapDlg.h"
#include "afxdialogex.h"

extern AOSACAParams *g_AOSACAParams;
extern COptCalc		*g_optcalc;

// CDMMapDlg dialog

IMPLEMENT_DYNAMIC(CDMMapDlg, CDialogEx)

CDMMapDlg::CDMMapDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDMMapDlg::IDD, pParent)
{

}

CDMMapDlg::~CDMMapDlg()
{
	OnClose();
}

void CDMMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDL_DMCMAP, m_DMCmap);
}


BEGIN_MESSAGE_MAP(CDMMapDlg, CDialogEx)
	ON_MESSAGE(WM_UPDATE_WINDOW, OnUpdateWindow)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CDMMapDlg message handlers
void CDMMapDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default	
	delete [] colormap;
	delete [] m_DMVoltages;
	delete [] m_DMActRects;
	delete [] m_bDMSelection;
	CDialogEx::OnClose();
}

BOOL CDMMapDlg::OnEraseBkgnd(CDC* pDC)
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


BOOL CDMMapDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// TODO: Add extra initialization here	
	m_BUpdate = true;
	m_dMaxDef = 0.0;
	if( m_CMapDMbmp.m_hObject == 0 && m_CMapDMbmp.LoadBitmap( IDB_DMCMAP ) )  
	{
		m_DMCmap.SetBitmap( m_CMapDMbmp );
	}
	
	m_DMCmap.SetWindowPos(NULL, 5, 260, 236, 20, SWP_FRAMECHANGED);
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

void CDMMapDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!g_AOSACAParams->g_bControlON && !g_AOSACAParams->g_bPokeMatGeneration)
		for(int i=0; i<g_AOSACAParams->NUMACTS; i++)
		{
			if ((m_DMActRects[i].left<= point.x)
				&&(m_DMActRects[i].right>= point.x)
				&&(m_DMActRects[i].top<= point.y)
				&&(m_DMActRects[i].bottom>= point.y))
			{
				m_bDMSelection[i] = true;
				m_DMVoltages[i] = m_dMaxDef;
				UpdateSelectedActuators();
			}
		}

//	CDialogEx::OnLButtonDblClk(nFlags, point);
}

void CDMMapDlg::OnPaint()
{
//	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	m_BUpdate?color_AllActuators():0;
	// Do not call CDialogEx::OnPaint() for painting messages
}

void CDMMapDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!g_AOSACAParams->g_bControlON && !g_AOSACAParams->g_bPokeMatGeneration)
		for(int i=0; i<g_AOSACAParams->NUMACTS; i++)
		{
			if ((m_DMActRects[i].left<= point.x)
				&&(m_DMActRects[i].right>= point.x)
				&&(m_DMActRects[i].top<= point.y)
				&&(m_DMActRects[i].bottom>= point.y))
			{
				m_bDMSelection[i] = false;
				m_DMVoltages[i] = g_AOSACAParams->g_DMBiasDeflections[i];
				UpdateSelectedActuators();
			}
		}	

//	CDialogEx::OnRButtonUp(nFlags, point);
}

LRESULT CDMMapDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
	m_msgID = lParam;

	switch (m_msgID) {
	case UPDATE_WINDOW:
		if (m_BUpdate) 
		{
			ZeroMemory(m_bDMSelection, g_AOSACAParams->NUMACTS*sizeof(BOOL));
			g_optcalc->get_DMVoltages(m_DMVoltages);
			OnPaint();
			Invalidate(false);
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

void CDMMapDlg::color_AllActuators()
{
	double voltval;
	int i,j,l=0,climit;
	COLORREF color;

	CPaintDC dc(this);
	// Retrieve voltage for each actuator and apply necessary color
	// The order for both 'for' loops below needs to be updated based upon DM orientation wrt Wavefront sensor
	for (j=g_AOSACAParams->DMGRID-1; j>=0; j--)
	{
		for (i=0; i<g_AOSACAParams->DMGRID; i++)
		{
			if(g_AOSACAParams->g_bActuatorMask[j*g_AOSACAParams->DMGRID+i])
			{
				voltval = m_DMVoltages[l];
				climit=(int)((voltval-(m_dMaxDef))*511./(-(2.*m_dMaxDef)));
				if (climit<0)
					climit = 0;
				else if (climit>511)
					climit = 511;				
				color = colormap[climit];				
				CBrush tbrush(color);
				CBrush *pbrush = dc.SelectObject(&tbrush);
				dc.PatBlt(m_DMActRects[l].left,m_DMActRects[l].top,(m_DMActRects[l].right-m_DMActRects[l].left),(m_DMActRects[l].bottom-m_DMActRects[l].top),PATCOPY);
				pbrush->DeleteObject();
				l++;
			}
		}
	}
}
void CDMMapDlg::InitParam()
{
	colormap = new COLORREF[512];
	memcpy(colormap, g_AOSACAParams->dmcolormap, 512*sizeof(COLORREF));
	m_dMaxDef = g_AOSACAParams->DM_MAX_DEFLECTION;
	m_DMVoltages = new double[g_AOSACAParams->NUMACTS];
	m_bDMSelection = new BOOL[g_AOSACAParams->NUMACTS];	
	memcpy(m_DMVoltages, g_AOSACAParams->g_DMBiasDeflections, g_AOSACAParams->NUMACTS*sizeof(double));
	memset(m_bDMSelection, 0, g_AOSACAParams->NUMACTS*sizeof(BOOL));
	m_DMActRects = new RECT[g_AOSACAParams->NUMACTS];

	short i=0, j=0, l=0;
	for (j=g_AOSACAParams->DMGRID-1; j>=0; j--)
	{
		for (i=0; i<g_AOSACAParams->DMGRID; i++)
		{
			if(g_AOSACAParams->g_bActuatorMask[j*g_AOSACAParams->DMGRID+i])
			{				
				m_DMActRects[l].left = X_OFFSET_DM + (i*(ACTR_BOX_SIZE+BOX_GAP));
				m_DMActRects[l].right = m_DMActRects[l].left + ACTR_BOX_SIZE;
				m_DMActRects[l].top = Y_OFFSET_DM + (j*(ACTR_BOX_SIZE+BOX_GAP));
				m_DMActRects[l].bottom = m_DMActRects[l].top + ACTR_BOX_SIZE;
				l++;
			}
		}
	}
}

void CDMMapDlg::UpdateSelectedActuators()
{
	g_optcalc->SendVoltsTest(m_DMVoltages);
	m_BUpdate = true;
	color_AllActuators();
	Invalidate(false);
}