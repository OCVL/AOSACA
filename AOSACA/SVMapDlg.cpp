// SVMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "SVMapDlg.h"
#include "afxdialogex.h"

extern CCentroid	*g_centroids;
extern AOSACAParams	*g_AOSACAParams;
// CSVMapDlg dialog

IMPLEMENT_DYNAMIC(CSVMapDlg, CDialogEx)

CSVMapDlg::CSVMapDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSVMapDlg::IDD, pParent)
{
	// Allocations for ref points and actual points
	ref_points = NULL;
	act_points = NULL;
	delta_points = NULL;
	m_bFound = NULL;
}

CSVMapDlg::~CSVMapDlg()
{
	OnClose();
}

void CSVMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSVMapDlg, CDialogEx)
	ON_MESSAGE(WM_UPDATE_WINDOW, OnUpdateWindow)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSVMapDlg message handlers
void CSVMapDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (ref_points) delete [] ref_points;
	if (act_points) delete [] act_points;
	if (delta_points) delete [] delta_points;
	if (m_bFound) delete [] m_bFound;
	CDialogEx::OnClose();
}

HBRUSH CSVMapDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CSVMapDlg::OnEraseBkgnd(CDC* pDC)
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

BOOL CSVMapDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	Display = GetDlgItem(IDL_SVMAP);
	Display->SetWindowPos(NULL, 
						  5, 
						  5, 
						  340, 
						  340, 
						  SWP_FRAMECHANGED); 
	m_BUpdate = false;
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


void CSVMapDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages
	if(m_BUpdate)
	{
		Display->InvalidateRect(NULL,true);
		Display->UpdateWindow();
		Display->GetClientRect(&m_OrigDrawArea);
		CDC *cdc=Display->GetDC();			
		Draw_SLVectors(cdc);
		Display->ReleaseDC(cdc);
	}	
}

LRESULT CSVMapDlg::OnUpdateWindow(WPARAM wParam, LPARAM lParam)
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

void CSVMapDlg::det_DirAndDrawArrow(CDC *dc,CPoint ref, CPoint act)
{
	CPoint arrow_pts[5];
	// There are 8 directions to draw the arrow	
	// If it points North
	if(act.x == ref.x && act.y > ref.y)
	{
		arrow_pts[0] = ref;
		arrow_pts[1].x = ref.x - 1; arrow_pts[1].y = ref.y + 1;
		arrow_pts[2].x = ref.x - 2; arrow_pts[2].y = ref.y + 2;
		arrow_pts[3].x = ref.x + 1; arrow_pts[3].y = ref.y + 1;
		arrow_pts[4].x = ref.x + 2; arrow_pts[4].y = ref.y + 2;
	}
	// South
	if(act.x == ref.x && act.y < ref.y)
	{
		arrow_pts[0] = ref;
		arrow_pts[1].x = ref.x - 1; arrow_pts[1].y = ref.y - 1;
		arrow_pts[2].x = ref.x - 2; arrow_pts[2].y = ref.y - 2;
		arrow_pts[3].x = ref.x + 1; arrow_pts[3].y = ref.y - 1;
		arrow_pts[4].x = ref.x + 2; arrow_pts[4].y = ref.y - 2;
	}
	// East
	if(act.y == ref.y && act.x < ref.x)
	{
		arrow_pts[0] = ref;
		arrow_pts[1].x = ref.x - 1; arrow_pts[1].y = ref.y - 1;
		arrow_pts[2].x = ref.x - 2; arrow_pts[2].y = ref.y - 2;
		arrow_pts[3].x = ref.x - 1; arrow_pts[3].y = ref.y + 1;
		arrow_pts[4].x = ref.x - 2; arrow_pts[4].y = ref.y + 2;
	}
	// West
	if(act.y == ref.y && act.x > ref.x)
	{
		arrow_pts[0] = ref;
		arrow_pts[1].x = ref.x + 1; arrow_pts[1].y = ref.y - 1;
		arrow_pts[2].x = ref.x + 2; arrow_pts[2].y = ref.y - 2;
		arrow_pts[3].x = ref.x + 1; arrow_pts[3].y = ref.y + 1;
		arrow_pts[4].x = ref.x + 2; arrow_pts[4].y = ref.y + 2;
	}
	//	North - East
	if(act.y > ref.y && act.x < ref.x)
	{
		arrow_pts[0] = ref;
		arrow_pts[1].x = ref.x ; arrow_pts[1].y = ref.y + 1;
		arrow_pts[2].x = ref.x ; arrow_pts[2].y = ref.y + 2;
		arrow_pts[3].x = ref.x - 1; arrow_pts[3].y = ref.y;
		arrow_pts[4].x = ref.x - 2; arrow_pts[4].y = ref.y;
	}
	// South - East
	if(act.y < ref.y && act.x < ref.x)
	{
		arrow_pts[0] = ref;
		arrow_pts[1].x = ref.x; arrow_pts[1].y = ref.y - 1;
		arrow_pts[2].x = ref.x; arrow_pts[2].y = ref.y - 2;
		arrow_pts[3].x = ref.x - 1; arrow_pts[3].y = ref.y;
		arrow_pts[4].x = ref.x - 2; arrow_pts[4].y = ref.y;
	}
	// South - West
	if(act.y < ref.y && act.x > ref.x)
	{
		arrow_pts[0] = ref;
		arrow_pts[1].x = ref.x; arrow_pts[1].y = ref.y - 1;
		arrow_pts[2].x = ref.x; arrow_pts[2].y = ref.y - 2;
		arrow_pts[3].x = ref.x + 1; arrow_pts[3].y = ref.y;
		arrow_pts[4].x = ref.x + 2; arrow_pts[4].y = ref.y;
	}
	// North - West
	if(act.y > ref.y && act.x > ref.x)
	{
		arrow_pts[0] = ref;
		arrow_pts[1].x = ref.x; arrow_pts[1].y = ref.y + 1;
		arrow_pts[2].x = ref.x; arrow_pts[2].y = ref.y + 2;
		arrow_pts[3].x = ref.x + 1; arrow_pts[3].y = ref.y;
		arrow_pts[4].x = ref.x + 2; arrow_pts[4].y = ref.y;
	}

	// If slope vector is 0
	if(act.y == ref.y && act.x == ref.x)
	{
        dc->SetPixelV(act,RGB(255,255,255));        
		return;
	}

	// Draw the arrow
	for(int i = 0; i < 5;i++)
		dc->SetPixelV(arrow_pts[i],RGB(255,0,0));
}

void CSVMapDlg::Draw_SLVectors(CDC *dc)
{
	int i;
	CPoint pt_ref, pt_act;

	CPen *pen = new CPen();
	pen->CreatePen(PS_SOLID,0,RGB(255,255,0));
	dc->SelectObject(pen);
	// Find half of the num of spots and determine xref and yref
	g_centroids->FetchSlopeVector(act_points, m_bFound);
	VD_mulC(act_points, act_points, m_num_of_spots<<1, SVMAP_SCALE);
	VDs_subV(delta_points, act_points, ref_points, m_num_of_spots<<1, 2.0);
	for(i=0;i<m_num_of_spots;i++)
	{
		if (m_bFound[i])
		{
			pt_act.x = (int)(act_points[i]+delta_points[i]) + X_OFFSET_SVM;
			pt_act.y = (int)(act_points[i+m_num_of_spots]+delta_points[i+m_num_of_spots]) + Y_OFFSET_SVM;
			pt_ref.x = (int)ref_points[i] + X_OFFSET_SVM;
			pt_ref.y = (int)ref_points[i+m_num_of_spots] + Y_OFFSET_SVM;			
			dc->MoveTo(pt_act);
			dc->LineTo(pt_ref);
			// Determine the direction of the vector and draw the arrow
			det_DirAndDrawArrow(dc,pt_ref,pt_act);
		}

	}
	pen->DeleteObject();
	delete pen;
}

void CSVMapDlg::Init_CenterPoints(short num_of_spots)
{
	m_num_of_spots = num_of_spots;
	ref_points = new double [m_num_of_spots<<1];
	memset(ref_points, 0, (m_num_of_spots<<1)*sizeof(double));
	act_points = new double [m_num_of_spots<<1];
	memset(act_points, 0, (m_num_of_spots<<1)*sizeof(double));
	delta_points = new double [m_num_of_spots<<1];
	memset(delta_points, 0, (m_num_of_spots<<1)*sizeof(double));
	m_bFound = new bool[num_of_spots];
	memset(m_bFound, 0, m_num_of_spots*sizeof(bool));
	g_centroids->get_RefPoints(ref_points);
	VD_mulC(ref_points, ref_points, m_num_of_spots<<1, SVMAP_SCALE);
	m_BUpdate = true;
}
