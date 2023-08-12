#pragma once


typedef enum {POKEMAT, CLOSELOOP} Thread_ID;
// CThreadClass

class CThreadClass : public CWinThread
{
	DECLARE_DYNCREATE(CThreadClass)

public:
	CThreadClass();           // protected constructor used by dynamic creation
	virtual ~CThreadClass();
	bool		InfMatrixSteps();
	Thread_ID	cur_ID;
	HANDLE		m_ehThreadExit;
	HANDLE		m_ehThreadClose;

private:
	CWinThread	*thread;

public:
	CAOSACADlg	*m_pParent;

public:
	virtual BOOL InitInstance(CAOSACADlg *parent);
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
};


