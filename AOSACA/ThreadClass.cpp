// ThreadClass.cpp : implementation file
//

#include "stdafx.h"
#include "AOSACA.h"
#include "ThreadClass.h"
#include <MMSystem.h>

extern AOSACAParams		*g_AOSACAParams;
extern CCamera			*g_camera;
extern CCentroid		*g_centroids;
extern COptCalc			*g_optcalc;
extern CControlPanelDlg	*g_controlpanel;

// CThreadClass
UINT Run_DMAOLoop(LPVOID pParam); //Main Thread

IMPLEMENT_DYNCREATE(CThreadClass, CWinThread)

CThreadClass::CThreadClass()
{
	thread = NULL;
}

CThreadClass::~CThreadClass()
{
}

BOOL CThreadClass::InitInstance(CAOSACADlg *parent)
{
	// TODO:  perform and per-thread initialization here
	m_pParent = parent;
	m_ehThreadExit = CreateEvent(NULL, FALSE, FALSE, _T("THREAD_EXIT_EVENT"));
	m_ehThreadClose = CreateEvent(NULL, FALSE, FALSE, _T("CLOOP_THREAD_CLOSE_EVENT"));
	AfxBeginThread(Run_DMAOLoop, this, THREAD_PRIORITY_HIGHEST,0,0,NULL);
	return TRUE;
}

int CThreadClass::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	if (g_AOSACAParams->g_bPokeMatGeneration || g_AOSACAParams->g_bCloseLoopThread)
	{
		ResetEvent(g_AOSACAParams->g_ehPokeThread);
		ResetEvent(g_AOSACAParams->g_ehCLoopThread);
		g_AOSACAParams->g_bPokeMatGeneration = g_AOSACAParams->g_bCloseLoopThread = false;
	}
	SetEvent(m_ehThreadClose);
	::WaitForSingleObject(m_ehThreadExit, 1000);
	CloseHandle(m_ehThreadExit);
	CloseHandle(m_ehThreadClose);
	SetEvent(g_AOSACAParams->g_ehCLoopShutdown);
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CThreadClass, CWinThread)
END_MESSAGE_MAP()


// CThreadClass message handlers
bool CThreadClass::InfMatrixSteps()
{		
	::WaitForSingleObject(g_AOSACAParams->g_ehCamNewFrame, INFINITE);

	if(!m_pParent->Steps_in_finding_Centroids() && (g_AOSACAParams->g_bPokeMatGeneration || g_AOSACAParams->g_bControlON)) 
		return false;
	if (g_AOSACAParams->g_bControlON || g_AOSACAParams->g_bAutoMeasure)
		g_centroids->CalcSlopes(NULL, true);

	return true;
}

UINT Run_DMAOLoop(LPVOID pParam)
{
	CThreadClass *parent = (CThreadClass *)pParam;
	long cent_unmatch = 0;
	double telapse = 0.;
	CString text;
	CString errBuff;
	HANDLE hThreadEvents[3];
	hThreadEvents[0] = g_AOSACAParams->g_ehPokeThread;
	hThreadEvents[1] = g_AOSACAParams->g_ehCLoopThread;
	hThreadEvents[2] = parent->m_ehThreadClose;	
	
	LARGE_INTEGER time1;
	LARGE_INTEGER time2;
	CEvent WaitEvent;
	short waitTimePMG = 120;
	short waitTimeCL = 20;

	bool bRunThread = true;
	//__int64 tstart;
	do
	{
		switch(::WaitForMultipleObjects(3, hThreadEvents, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0: // Influence matrix computation
			{				
				bool status = true;
				int CentSize, i, j, k;
				double *m_dvTempPhi1, *m_dvTempPhi2;
				double	**m_dmTempInfMat;
				double *m_dvTempWaveZ1, *m_dvTempWaveZ2;  //for Zernike P_mat 10/20/2015
				double **m_dmTempInfMat_Zernike;  //for Zernike P_mat 10/20/2015
				status = true;
				CentSize = g_centroids->get_TotalCentCount()<<1;
				m_dvTempPhi1 = new double[CentSize];
				memset(m_dvTempPhi1, 0, CentSize*sizeof(double));
				m_dvTempPhi2 = new double[CentSize];
				memset(m_dvTempPhi2, 0, CentSize*sizeof(double));		
				m_dmTempInfMat = new double*[g_AOSACAParams->DM_POKE_MAT_ITER];
				//for Zernike P_mat
				m_dvTempWaveZ1 = new double[MAX_TERM_NUMBER];
				memset(m_dvTempWaveZ1, 0, MAX_TERM_NUMBER*sizeof(double));
				m_dvTempWaveZ2 = new double[MAX_TERM_NUMBER];
				memset(m_dvTempWaveZ2, 0, MAX_TERM_NUMBER*sizeof(double));
				m_dmTempInfMat_Zernike = new double*[g_AOSACAParams->DM_POKE_MAT_ITER];
				//end

				g_optcalc->UpdateDMMaxDeflection(g_AOSACAParams->DM_POKE_MAT_MAX_DEF);

				for (i=0;i<g_AOSACAParams->DM_POKE_MAT_ITER;i++)
				{
					m_dmTempInfMat[i] = new double[g_AOSACAParams->NUMACTS*CentSize];
					memset(m_dmTempInfMat[i], 0, (g_AOSACAParams->NUMACTS*CentSize)*sizeof(double));

					m_dmTempInfMat_Zernike[i] = new double[g_AOSACAParams->NUMACTS*MAX_TERM_NUMBER];  //for Zernike P_mat
					memset(m_dmTempInfMat_Zernike[i], 0, (g_AOSACAParams->NUMACTS*MAX_TERM_NUMBER)*sizeof(double));  //for Zernike P_mat

				}

				//Check if refraction is applied
				if (g_AOSACAParams->g_bCPreCorrApply)
				{
					g_AOSACAParams->g_bCPreCorrApply = false;
					g_optcalc->UpdateDMFlatData();
					(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
					g_AOSACAParams->g_bCPreCorrApply = true;
				}

				//g_optcalc->Send_Voltages(BIAS_BIT);

				for (k=0; k<g_AOSACAParams->DM_POKE_MAT_ITER && status && g_AOSACAParams->g_bPokeMatGeneration; k++)
				{
					//do poke matrix generation steps		
					for(j=0;j<g_AOSACAParams->NUMACTS && g_AOSACAParams->g_bPokeMatGeneration && status;j++)
					{// - calculate phi for each actuator									
						g_optcalc->SetActuator(j,MAX_BIT);	// pull					
						::WaitForSingleObject(WaitEvent, waitTimePMG);
						(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
						SetEvent(g_AOSACAParams->g_ehCamSnap);
						if ( status = parent->InfMatrixSteps())
							g_centroids->CalcSlopes(m_dvTempPhi1, true);
						(parent->m_pParent)->Opt_Perform();	
						g_optcalc->get_ZCoeffs_all(m_dvTempWaveZ1, MAX_TERM_NUMBER) ;   //for Zernike P_mat

						g_optcalc->SetActuator(j,COMP_BIT);	// - apply bias to DM
						::WaitForSingleObject(WaitEvent, waitTimePMG);					
						g_optcalc->SetActuator(j,MIN_BIT);	// push					
						::WaitForSingleObject(WaitEvent, waitTimePMG);
						(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
						SetEvent(g_AOSACAParams->g_ehCamSnap);
						if (status = parent->InfMatrixSteps())
							g_centroids->CalcSlopes(m_dvTempPhi2, true);
						(parent->m_pParent)->Opt_Perform();

						g_optcalc->get_ZCoeffs_all(m_dvTempWaveZ2, MAX_TERM_NUMBER) ;  //for Zernike P_mat
						g_optcalc->SetActuator(j,COMP_BIT);	// - apply bias to DM
						::WaitForSingleObject(WaitEvent, waitTimePMG);
						VD_subV( &m_dmTempInfMat[k][j*CentSize], m_dvTempPhi2, m_dvTempPhi1, CentSize );
						VD_divC( &m_dmTempInfMat[k][j*CentSize], &m_dmTempInfMat[k][j*CentSize], CentSize, 2.*g_optcalc->get_MinBit());

						VD_subV( &m_dmTempInfMat_Zernike[k][j*MAX_TERM_NUMBER], m_dvTempWaveZ2, m_dvTempWaveZ1, MAX_TERM_NUMBER ); //for Zernike P_mat
						VD_divC( &m_dmTempInfMat_Zernike[k][j*MAX_TERM_NUMBER], &m_dmTempInfMat_Zernike[k][j*MAX_TERM_NUMBER], MAX_TERM_NUMBER, 2.*g_optcalc->get_MinBit());//for Zernike P_mat

						(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_PGB_WINDOW);
					}
				}

				//Check if refraction is applied and revert back to refraction deflections
				if (g_AOSACAParams->g_bCPreCorrApply)
				{
					g_optcalc->UpdateDMFlatData();
					(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_DMVMAP_WINDOW);
				}

				//g_controlpanel->FlatDM();
				g_optcalc->UpdateDMMaxDeflection(g_AOSACAParams->DM_MAX_DEFLECTION);

				if (status && g_AOSACAParams->g_bPokeMatGeneration && k == g_AOSACAParams->DM_POKE_MAT_ITER)
				{			
					for (i=1;i<g_AOSACAParams->DM_POKE_MAT_ITER;i++)
					{
						VD_addV(m_dmTempInfMat[0], m_dmTempInfMat[0], m_dmTempInfMat[i], g_AOSACAParams->NUMACTS*CentSize);
						VD_addV(m_dmTempInfMat_Zernike[0], m_dmTempInfMat_Zernike[0], m_dmTempInfMat_Zernike[i], g_AOSACAParams->NUMACTS*MAX_TERM_NUMBER);  //for Zernike P_mat
					}	

					(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_PGB_WINDOW);
					VD_divC(m_dmTempInfMat[0], m_dmTempInfMat[0], g_AOSACAParams->NUMACTS*CentSize, g_AOSACAParams->DM_POKE_MAT_ITER);
					g_centroids->set_Pmat(m_dmTempInfMat[0]);

					VD_divC(m_dmTempInfMat_Zernike[0], m_dmTempInfMat_Zernike[0], g_AOSACAParams->NUMACTS*MAX_TERM_NUMBER, g_AOSACAParams->DM_POKE_MAT_ITER); //for Zernike P_mat
					g_centroids->set_Pmat_Zernike(m_dmTempInfMat_Zernike[0]); //for Zernike P_mat

					(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_PGB_WINDOW);
					g_AOSACAParams->g_bPokeMatGeneration = false;
					PlaySound(_T("utils\\Balloon.wav"),NULL,SND_FILENAME);
					(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_PGBD_WINDOW);
				}
				else if(!status && g_AOSACAParams->g_bPokeMatGeneration)
				{	
					g_AOSACAParams->g_stAppErrBuff.Empty();
					g_AOSACAParams->g_stAppErrBuff = "Error Occured while generating Poke Matrix!!!";
					g_AOSACAParams->ShowError(MB_ICONERROR);
					(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_PGBC_WINDOW);
				}
				for (i=0;i<g_AOSACAParams->DM_POKE_MAT_ITER;i++)
				{
					delete [] m_dmTempInfMat[i];
					delete [] m_dmTempInfMat_Zernike[i];
				}
				delete [] m_dmTempInfMat;
				delete [] m_dvTempPhi1;
				delete [] m_dvTempPhi2;
				delete [] m_dmTempInfMat_Zernike;
				delete [] m_dvTempWaveZ1;
				delete [] m_dvTempWaveZ2;
			}
			break;
		case WAIT_OBJECT_0+1: // Closed loop 				
			QueryPerformanceCounter(&time1);
			if(parent->InfMatrixSteps() && g_AOSACAParams->g_bCloseLoopThread)
			{			
				if ((cent_unmatch > g_AOSACAParams->CYCLES_BELOW_SETCENTROIDNO && g_AOSACAParams->g_bControlON) || g_AOSACAParams->g_bResetMirror)
					(parent->m_pParent)->Initialize_CLoop();
				cent_unmatch = -1;

				//(parent->m_pParent)->Opt_Perform(); // step 5  //this code should run first before calculating voltage for Zernike control
				if (g_AOSACAParams->g_bControlON)
				{			
					g_optcalc->Compute_Voltages();								
					g_optcalc->Send_Voltages(COMP_BIT);
					::WaitForSingleObject(WaitEvent, waitTimeCL);
					// Update the wavefront correction image
					g_AOSACAParams->g_bUpdateDMVMap = true;						
				}	
				SetEvent(g_AOSACAParams->g_ehCamSnap);
				// Updating the WFS image(incl.Centroids), wave map, psf, slope vectors and DM map
				(parent->m_pParent)->Opt_Perform(); // step 5
				(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);		
				// Update Frequency
				QueryPerformanceCounter(&time2);
				telapse = double(g_AOSACAParams->m_lnFreq/(time2.QuadPart-time1.QuadPart));
				text.Format(_T("%2.2f"),telapse);
				(parent->m_pParent)->SetDlgItemText(IDE_FREQUENCY, text);
			}
			else if (g_AOSACAParams->g_bControlON)
			{
				++cent_unmatch;				
				::WaitForSingleObject(WaitEvent, waitTimeCL);
				SetEvent(g_AOSACAParams->g_ehCamSnap);
				// Updating the wave map, psf and slope vectors			
				(parent->m_pParent)->Opt_Perform(); // step 5
				(parent->m_pParent)->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WINDOW);
			}
			break;
		case WAIT_OBJECT_0+2: // Terminate Thread			
			bRunThread = false;
			SetEvent(parent->m_ehThreadExit);
			break;
		default:
			break;
		}
	}while(bRunThread);

	return 0;
}
