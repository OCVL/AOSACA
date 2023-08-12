// DMirror.h: interface for the CDMirror class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DMIRROR_H__6C9E5690_8E93_4ECB_9709_7D160A0FDC5A__INCLUDED_)
#define AFX_DMIRROR_H__6C9E5690_8E93_4ECB_9709_7D160A0FDC5A__INCLUDED_

#pragma once


class CDMirror 
{
public:
	CDMirror();	//Constructor
	~CDMirror();//Destructor
	BOOL	SendVoltages(double*);
	BOOL	TestActuators();
	bool	isDMReady();	

private:
	long	m_lCurDev;	
    double	*m_dOffs; 
	double	*m_dDeflections;
	HANDLE	m_eDMTest;
	HANDLE	Thread_DM_Test;
	DWORD	ThreadId_DM_Test;
	static DWORD WINAPI DMTestThread(LPVOID param);
    int m_dmId;
	int m_nAct;    

	//Operations
	bool	DMInitialize();
	void	DMTerminate();
};

#endif // !defined(AFX_DMIRROR_H__6C9E5690_8E93_4ECB_9709_7D160A0FDC5A__INCLUDED_)
