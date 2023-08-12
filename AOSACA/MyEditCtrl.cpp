// MyEditCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "AOSACA.h"
#include "MyEditCtrl.h"
#include <Math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyEditCtrl

CMyEditCtrl::CMyEditCtrl()
{
}

CMyEditCtrl::~CMyEditCtrl()
{
}

BEGIN_MESSAGE_MAP(CMyEditCtrl, CEdit)
	//{{AFX_MSG_MAP(CMyEditCtrl)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyEditCtrl message handlers

void CMyEditCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CString text;
    GetWindowText(text);
	short length = text.GetLength();
	int ind = text.Find(_T("."));
	bool isNegPresent = false;
    bool isValid=false;
	(m_nType == MYEDIT_DOUBLE && length > 0 && ind >-1)?m_bDecimal = true, ind += 1:m_bDecimal = false;
	isNegPresent = text.Find(_T("-"))==-1?false:true;
	switch (nChar)
	{
	case VK_UP:
		{
			GetParent()->PostMessage(WM_NEXTDLGCTL,true,0);
			return;
		}
	case VK_DOWN:
		{
			GetParent()->PostMessage(WM_NEXTDLGCTL,false,0);
			return;
		}
	case VK_BACK: //always allow
		isValid = true;
		break;
	case VK_RETURN:
		//validate the value, read in the whole text and check
		CheckValidity();
		isValid = true;
		return;
	default: //check for the rest of input for validity
		if (m_nType == MYEDIT_ALPHANUMERIC)
		{
			if ((nChar >= 48 && nChar <= 57) || (nChar >= 65 && nChar <= 90) || (nChar >= 97 && nChar <= 122))
				isValid = true;
		}
		else if (m_nType == MYEDIT_INTEGER && length < m_nManLength)
		{			
			if (nChar >= 48 && nChar <= 57)
			{
				if ((length < 1 && nChar != 48) || (length >=1) || (length < 1 && nChar == 48))
					isValid = true;
			}
		}
		else if (m_nType == MYEDIT_DOUBLE)
		{
			if (m_bNegative)//negative number allowed
			{
				if ((length <1 && nChar == 45) || (!m_bDecimal && length <= (isNegPresent?m_nManLength+1:m_nManLength) && nChar == 46))
					isValid = true;
				else if (nChar >= 48 && nChar <= 57)
				{
					if (!m_bDecimal && length<(isNegPresent?m_nManLength+1:m_nManLength))
						isValid = true;
					else if (m_bDecimal)
					{
						text = text.Right(length - ind);
						if (text.GetLength() < m_nPrecLength)
							isValid = true;
					}
				}
			}
			else //only positive numbers
			{
				if (!m_bDecimal && length<=m_nManLength && nChar == 46) //true
					isValid = true;	
				else if (nChar >= 48 && nChar <= 57)
				{
					if (!m_bDecimal && length<m_nManLength)
						isValid = true;
					else if (m_bDecimal)
					{
						text = text.Right(length - ind);
						if (text.GetLength() < m_nPrecLength)
							isValid = true;
					}
				}
			}
		}
		else //do nothing
		{
		}
	}
    if (isValid)
        CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CMyEditCtrl::SetValidation(int type, double defval, double min, double max, short manlen, short preclen, bool negflg)
{
	CString text;

	m_nType = type;
	m_dDefault = defval;
	m_dMax = max;	
	negflg?m_dMin = min:m_dMin = fabs(min);
	if (m_dMin > m_dMax)
		m_dMin = m_dMax;
	m_nPrecLength = preclen;
	m_nManLength = manlen;
	m_bNegative = negflg;
	switch (m_nType)
	{
	case MYEDIT_ALPHANUMERIC:
		text = "PrefixR";
		m_bDecimal = false;
		break;
	case MYEDIT_INTEGER:
		text.Format(_T("%d"), (int)(m_dDefault));	
		m_bDecimal = false;
		break;
	case MYEDIT_DOUBLE:
		text.Format(_T("%*.*f"), m_nManLength, m_nPrecLength, m_dDefault);
		m_bDecimal = true;
		break;
	}
	SetWindowText(text);
}

void CMyEditCtrl::CheckValidity()
{
	CString text;
    GetWindowText(text);
	short length = text.GetLength();
	switch (m_nType)
	{
	case MYEDIT_ALPHANUMERIC:
		if (length < 1)
			SetWindowText(_T("PrefixR"));
		break;
	case MYEDIT_INTEGER:
		if (_wtoi(text) > (int)(m_dMax))
		{
			text.Format(_T("%d"), (int)(m_dMax));
			SetWindowText(text);
		}
		else if (_wtoi(text) < (int)(m_dMin))
		{
			text.Format(_T("%d"), (int)(m_dMin));
			SetWindowText(text);
		}
		else if (length < 1)
		{
			text.Format(_T("%d"), (int)(m_dDefault));
			SetWindowText(text);
		}
		break;
	case MYEDIT_DOUBLE:
		if (_wtof(text) > m_dMax)
		{
			text.Format(_T("%*.*f"), m_nManLength, m_nPrecLength,m_dMax);
			SetWindowText(text);
		}
		else if (_wtof(text) < m_dMin)
		{
			text.Format(_T("%*.*f"), m_nManLength, m_nPrecLength,m_dMin);
			SetWindowText(text);
		}			
		else if (length < 1)
		{
			text.Format(_T("%*.*f"), m_nManLength, m_nPrecLength,m_dDefault);
			SetWindowText(text);
		}
		break;
	}
}

double CMyEditCtrl::UpdateValue(BOOL incr, double incrval)
{
	if (m_nType == MYEDIT_INTEGER || m_nType == MYEDIT_DOUBLE)
	{		
		CString text;
		GetWindowText(text);
		double value;
		value = _wtof(text);
		if (incr == 1)
			value += incrval;
		else if (incr == 0)
			value -= incrval;
		if (m_nType == MYEDIT_INTEGER)
			text.Format(_T("%d"), (int)(value));
		else if (m_nType == MYEDIT_DOUBLE)
			text.Format(_T("%*.*f"), m_nManLength, m_nPrecLength,value);
		SetWindowText(text);
		CheckValidity();
		GetWindowText(text);
		value = _wtof(text);
		return value;
	}
	return 0;
}