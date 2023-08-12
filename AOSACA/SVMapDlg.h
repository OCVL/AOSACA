#pragma once


// CSVMapDlg dialog

class CSVMapDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSVMapDlg)

public:
	CSVMapDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSVMapDlg();
	void		Init_CenterPoints(short num_of_spots);
	BOOL		m_BUpdate;
	RECT		m_OrigDrawArea;

private:
	CWnd		*Display;
	double		*ref_points;
	double		*act_points;	
	double		*delta_points;
	bool		*m_bFound;
	short		m_num_of_spots;
	void		det_DirAndDrawArrow(CDC *dc,CPoint ref, CPoint act);
	void		Draw_SLVectors(CDC *dc);	

// Dialog Data
	enum { IDD = IDD_SVMAP };

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