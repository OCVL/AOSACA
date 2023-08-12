// BmcMirror.cpp: implementation of the CDMirror class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AOSACA.h"
#include "DMirror.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern AOSACAParams	*g_AOSACAParams;

#include "asdkDM.h"
using namespace acs;

DM	*dm;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDMirror::CDMirror()
{	
	m_lCurDev	= -1;	
	if (DMInitialize())
	{
		;
	}
}

CDMirror::~CDMirror()
{
	DMTerminate();
	delete [] m_dOffs;
	delete [] m_dDeflections;
}

bool CDMirror::DMInitialize()
{
	dm = NULL;
	// Convert a TCHAR string to a LPCSTR
	CT2CA pszConvertedAnsiString (g_AOSACAParams->DM_SERIAL_NO);
	// construct a std::string using the LPCSTR input
	String serial (pszConvertedAnsiString);
	dm = new DM(serial.c_str());
	
	if (dm == NULL)
    {
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "ALPAO device not found for initialization";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}
	else 
	{
		m_lCurDev = 1;
		m_nAct = (int)dm->Get( "NbOfActuator" );
		if (m_nAct != g_AOSACAParams->NUMACTS)
		{			
			m_lCurDev = 0;
			g_AOSACAParams->g_stAppErrBuff.Empty();
			g_AOSACAParams->g_stAppErrBuff = "Connected DM's total # of actuators does not match with the # in ini file";
			g_AOSACAParams->ShowError(MB_ICONERROR);
			return false;
		}
		m_dOffs = new double[m_nAct];
		m_dDeflections = new double[m_nAct];
		memcpy(m_dDeflections, g_AOSACAParams->g_DMBiasDeflections, g_AOSACAParams->NUMACTS*sizeof(double));
		dm->Send( m_dDeflections );
		g_AOSACAParams->g_bDMReady = true;
	}
	
	return dm->Check();
}

void CDMirror::DMTerminate()
{	
	g_AOSACAParams->g_bDMReady = false;
	dm->Stop();
	dm->Reset();
}

BOOL CDMirror::SendVoltages(double *nCurDeflections)
{
	//send updates deflections
	memcpy(m_dDeflections, nCurDeflections, m_nAct*sizeof(double));

	if (dm->Send(m_dDeflections) == FAILURE)
    {
		AfxMessageBox(_T("Sending Voltages error: device Error"), MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
            // If send failed, view error message
        dm->PrintLastError();
        return FALSE;
    }
	return TRUE;
}

BOOL CDMirror::TestActuators()
{
	SetEvent(m_eDMTest);
	return TRUE;
}

bool CDMirror::isDMReady()
{
	if (m_lCurDev>=0)
		return true;
	else
		return false;
}

DWORD WINAPI CDMirror::DMTestThread (LPVOID param)
{
	CDMirror *parent = (CDMirror *)param;
	CEvent waitEvent;
	
	while(1)
	{
		::WaitForSingleObject(parent->m_eDMTest, INFINITE);
		ZeroMemory(parent->m_dDeflections, parent->m_nAct*sizeof(double));
						
		for (int i=0; i<parent->m_nAct; i++)
		{
			// fill one value with 0.2
			parent->m_dDeflections[i] = 0.5;			
		
			::WaitForSingleObject(waitEvent, 500);
			
			parent->m_dDeflections[i] = 0;
		}
	}
	return 0;
}