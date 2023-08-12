#if !defined(AFX_MYEDITCTRL_H__D02C45B2_2644_4AD1_800A_2C4D8C5AC764__INCLUDED_)
#define AFX_MYEDITCTRL_H__D02C45B2_2644_4AD1_800A_2C4D8C5AC764__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyEditCtrl.h : header file
//
#define MYEDIT_ALPHANUMERIC 0
#define MYEDIT_INTEGER		1
#define MYEDIT_DOUBLE		2

/////////////////////////////////////////////////////////////////////////////
// CMyEditCtrl window

class CMyEditCtrl : public CEdit
{
// Construction
public:
	CMyEditCtrl();

// Attributes
private:
	int m_nType;
	bool m_bDecimal;
	bool m_bNegative;
	double m_dDefault;
	double m_dMin;
	double m_dMax;
	short m_nManLength;
	short m_nPrecLength;

// Operations
public:	
	void SetValidation(int, double defval=0.0, double min=0.0, double max=0.0, short manlen=-1, short preclength=-1, bool negflg=false);
	void CheckValidity();
	double UpdateValue(BOOL incr, double incrval);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyEditCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyEditCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyEditCtrl)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYEDITCTRL_H__D02C45B2_2644_4AD1_800A_2C4D8C5AC764__INCLUDED_)
