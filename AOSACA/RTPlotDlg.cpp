// RTPlotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "RTPlotDlg.h"
#include "afxdialogex.h"

extern AOSACAParams *g_AOSACAParams;
extern COptCalc		*g_optcalc;

// CRTPlotDlg dialog

IMPLEMENT_DYNAMIC(CRTPlotDlg, CDialogEx)

CRTPlotDlg::CRTPlotDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRTPlotDlg::IDD, pParent)
{

}

CRTPlotDlg::~CRTPlotDlg()
{
	OnClose();
}

void CRTPlotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDE_RTPLOTS_PLOT, m_PlotArea);
}


BEGIN_MESSAGE_MAP(CRTPlotDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_UPDATE_WINDOW, &CRTPlotDlg::OnUpdateWindow)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CRTPlotDlg message handlersshiftData

void CRTPlotDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	//free up allocated memory
	delete [] m_RmsData;
	delete [] m_StrehlData;
	pen_rms->DeleteObject();
	delete pen_rms;
	pen_strehl->DeleteObject();
	delete pen_strehl;
	CDialogEx::OnClose();
}


BOOL CRTPlotDlg::OnEraseBkgnd(CDC* pDC)
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
BOOL CRTPlotDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
//	SetIcon(m_hIcon, true);
	m_HighSlopeLimit = 1.00;
	m_LowSlopeLimit =  0.00;
	m_SlopeDataCounter = 0;
	// Set these values on the plot window
	pen_rms = new CPen();
	pen_rms->CreatePen(PS_SOLID,1,RGB(255,255,0));
	pen_strehl = new CPen();
	pen_strehl->CreatePen(PS_SOLID,1,RGB(0,255,255));
	pen_divisonsdashdot = new Pen(Color(255,172,157,72), 1.);
	pen_divisonsdashdot->SetDashStyle(DashStyleDashDot);	
	pen_divisons = new Pen(Color(255,172,157,72), 1.);
	pen_divisons->SetDashStyle(DashStyleSolid);
	brush_zmodes = new SolidBrush(Color(128,65,209,61));
	m_PlotArea.MoveWindow(40,40,610,235, true);
	m_PlotArea.GetClientRect(&m_slopetime_rect);
	offscreenBitmap = new Bitmap(m_slopetime_rect.right-m_slopetime_rect.left, m_slopetime_rect.bottom-m_slopetime_rect.top);
	pOffscreenGraphics = Graphics::FromImage(offscreenBitmap);
	m_slopetime_rect.bottom -= PLOT_YOFFSET;
	m_slopetime_rect.left += PLOT_XOFFSET;
	m_slopetime_rect.right -= PLOT_XOFFSET;
	m_slopetime_rect.top += PLOT_YOFFSET;		
	m_RmsData = new double[m_slopetime_rect.right];
	m_StrehlData = new double[m_slopetime_rect.right];
	CWnd *Ctrl = GetDlgItem(IDE_RTPLOTS_PLOT);
	Ctrl->GetClientRect(&m_slopetime_rect);
	plotarea = new Graphics((Ctrl->GetDC())->GetSafeHdc());	
	m_bInitPlot = false;
	m_BUpdate = true;
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

void CRTPlotDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	if(m_BUpdate)
	{
		double rms = g_optcalc->retn_RMS();
		double strehl = g_optcalc->retn_Strehl();
		CWnd *Ctrl = GetDlgItem(IDE_RTPLOTS_PLOT);
		if(!m_bInitPlot)
		{
			Set_SlopePlotData( rms, strehl);	
			CDialogEx::OnPaint();
			Ctrl->InvalidateRect(NULL,true);
			Ctrl->UpdateWindow();		
			fill_PlotRgn();
		}
		else
		{			
			if(m_LastPointRms.x < (m_slopetime_rect.right-2))
			{
				double temp_no;
				bool bRms = false, bStrehl = false;
				// Store the slope data for later retrieval		
				m_CurrentPointRms.x = m_LastPointRms.x + 1;
				m_CurrentPointStrehl.x = m_LastPointStrehl.x + 1;
				if (rms <= m_HighSlopeLimit && rms >= m_LowSlopeLimit)
				{
					temp_no = 1.0  - ((rms - m_LowSlopeLimit) / (m_HighSlopeLimit - m_LowSlopeLimit));
					m_CurrentPointRms.y = (int)((double)(m_slopetime_rect.bottom - m_slopetime_rect.top) * temp_no);
					bRms = true;
				}
				if (strehl <= m_HighSlopeLimit && strehl >= m_LowSlopeLimit)
				{
					temp_no = 1.0  - ((strehl - m_LowSlopeLimit) / (m_HighSlopeLimit - m_LowSlopeLimit));
					m_CurrentPointStrehl.y = (int)((double)(m_slopetime_rect.bottom - m_slopetime_rect.top) * temp_no);
					bStrehl = true;
				}

				m_RmsData[m_SlopeDataCounter] = rms;
				m_StrehlData[m_SlopeDataCounter++] = strehl;
				if (bRms || bStrehl) 
				{
					CDC *cdc = Ctrl->GetDC();
					HGDIOBJ original = NULL;
					original = cdc->SelectObject(GetStockObject(DC_PEN));
					if (bRms)
					{
						cdc->SelectObject(pen_rms);
						cdc->MoveTo(m_LastPointRms);
						cdc->LineTo(m_CurrentPointRms);
					}
					if (bStrehl)
					{
						cdc->SelectObject(pen_strehl);
						cdc->MoveTo(m_LastPointStrehl);
						cdc->LineTo(m_CurrentPointStrehl);		
					}
					cdc->SelectObject(original);
					Ctrl->ReleaseDC(cdc);
				}
				m_LastPointRms = m_CurrentPointRms;
				m_LastPointStrehl = m_CurrentPointStrehl;
			}
			else
				Reset_SlopePlotData();
		}
	}
	// Do not call CDialogEx::OnPaint() for painting messages
}


afx_msg LRESULT CRTPlotDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
{
	switch (lParam) {
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
void CRTPlotDlg::fill_PlotRgn()
{
	//update Y scale values independent of plots mode	
	int step = (m_slopetime_rect.bottom-m_slopetime_rect.top)/4;
	//draw plot divison lines
	plotarea->Clear(Color(255,0,0,0));
	plotarea->DrawLine(pen_divisonsdashdot, m_slopetime_rect.left, m_slopetime_rect.top, m_slopetime_rect.right, m_slopetime_rect.top);
	plotarea->DrawLine(pen_divisonsdashdot, m_slopetime_rect.left, m_slopetime_rect.top+step, m_slopetime_rect.right, m_slopetime_rect.top+step);
	plotarea->DrawLine(pen_divisonsdashdot, m_slopetime_rect.left, m_slopetime_rect.top+step*2, m_slopetime_rect.right, m_slopetime_rect.top+step*2);
	plotarea->DrawLine(pen_divisonsdashdot, m_slopetime_rect.left, m_slopetime_rect.top+step*3, m_slopetime_rect.right, m_slopetime_rect.top+step*3);
	plotarea->DrawLine(pen_divisonsdashdot, m_slopetime_rect.left, m_slopetime_rect.bottom, m_slopetime_rect.right, m_slopetime_rect.bottom);
}

void CRTPlotDlg::Reset_SlopePlotData()
{
	// Initialize flag is set to false
	m_bInitPlot = false;
	// Update the last point variable
	m_LastPointRms.x = m_LastPointRms.y = 0;
	m_LastPointStrehl.x = m_LastPointStrehl.y = 0;
	// Update the slope data counter
	m_SlopeDataCounter = 0;
}

void CRTPlotDlg::Set_SlopePlotData(double rms, double strehl)
{
	CString temp;
	double temp_no;
	// Initialize flag is set to true
	m_bInitPlot = true;
	// Keep track of the first slope value for retrieval later
	m_RmsData[m_SlopeDataCounter] = rms;
	m_StrehlData[m_SlopeDataCounter++] = strehl;
	m_LastPointRms.x = m_LastPointStrehl.x = m_slopetime_rect.left;
	temp_no = 1.0  - ((rms - m_LowSlopeLimit) / (m_HighSlopeLimit - m_LowSlopeLimit));
	m_LastPointRms.y = (int)((double)(m_slopetime_rect.bottom - m_slopetime_rect.top) * temp_no);
	temp_no = 1.0  - ((strehl - m_LowSlopeLimit) / (m_HighSlopeLimit - m_LowSlopeLimit));
	m_LastPointStrehl.y = (int)((double)(m_slopetime_rect.bottom - m_slopetime_rect.top) * temp_no);
}


HBRUSH CRTPlotDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	//	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
		pDC->SetTextColor (TEXTCOLOR); // White text
		pDC->SetBkColor (BACKGROUNDCOLOR);
		if ((pWnd->GetDlgCtrlID() == IDL_RTPLOTS_LEGEND_RMS))
			pDC->SetTextColor(0x0000FFFF);
		else if ((pWnd->GetDlgCtrlID() == IDL_RTPLOTS_LEGEND_STREHL))
			pDC->SetTextColor(0x00FFFF00);
		return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());
		break;
	default:
		break;

	}

	// TODO:  Return a different brush if the default is not desired
	return (HBRUSH)(g_AOSACAParams->m_bkgndbrush.GetSafeHandle());
}
