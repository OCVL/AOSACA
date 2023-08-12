// Centroid.h: interface for the CCentroid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CENTROID_H__579FEB40_6522_11D6_8008_006008A334DD__INCLUDED_)
#define AFX_CENTROID_H__579FEB40_6522_11D6_8008_006008A334DD__INCLUDED_

#pragma once

#include <windows.h>
#include <stdio.h>
#include <math.h>

#define MAX_TERM_NUMBER  65

////////////////////////////////////////////////////////////////////////

class CMasterCentroid
{
public:
	CMasterCentroid();
	CMasterCentroid(short size);
	~CMasterCentroid();
	double	*m_dCentroidV;
	double	*m_dRef_centersV;
	bool	*m_bUse_centroid;		// 1 to use centroid for computations, 0 if not.
	bool	*m_bFound;				// 1 if centroid is found, 0 if not.
	unsigned short	*m_sPeakintensity;		// peak pixel intensity in the search boxes
	double	*m_dAverageintensity;	// average intensity in the search boxes
	unsigned short	m_sFound_cent_count;		// total centroids found
	unsigned short	m_sUse_cent_count;	//# of centroids to use for given active pupil size
	short	m_sGeo_cent_ind;
};


class CCentroid  
{

public:
	// Member functions
	CCentroid();
	virtual ~CCentroid();

	// External Initialization Functions
	void	SetUserCenterPoint(float pupil_dia);
	bool	InitializeCentroidSystemParameters();
	void	UpdateOptParams(double,double,int);
	void	UpdateComParams(int,int,int,int);

	// Retrieval Functions		
	short	GetCentroidCount();
	short	get_TotalCentCount(){return m_nTotalCentroids;}	
	short	get_UseCentCount();	
	bool	FetchSlopeVector(double *dest, bool *found);
	void	get_RefPoints(double *dest);
	bool	get_MinCentReady(){return m_bMinCent;}
	bool	get_Reconflag(){return m_bRecon;}
	bool	get_CLoopReady(){return m_bMinCent&&m_bPMatReady;}
	int		get_NormalizedCenters(double*,double);
	double*	get_Centroids(short &Geoind){Geoind = m_centroid_matrix->m_sGeo_cent_ind; return m_centroid_matrix->m_dCentroidV;}
	bool*	get_CentFound(){return m_centroid_matrix->m_bFound;}	
	bool*	get_UseCentroids(){return m_centroid_matrix->m_bUse_centroid;}
	double* get_SearchBoxCenters(){return m_dBox_center;}
	unsigned short* get_SearchArray(){return m_usSearch_array;}
	void	set_sensorlessAOvec(char *buff){memcpy(m_dRefPhiV_sensorlessAO, buff, (m_nTotalCentroids<<1)*sizeof(double));}
	double*	get_sensorlessAOvec(){return m_dRefPhiV_sensorlessAO;}

	// File Operations
	CString SaveCentroidTextFile(CString centtextfile);

	// Computations
	bool	LocateAllCentroid(unsigned char *imgbuf);
	void	LocateOneCentroid(unsigned char *imgbuf, short index);
	void	CheckThreshold(int value);
	void	Estimate_Center(bool reset_user_center=false);
	
	//Drawing functions
	void	Make_Search_Array (float pupil_dia);
	void	Make_Search_Array_Act (float pupil_dia);
	
	// Centroid adjustment functions
	short	Add_Delete_Centroid(double x, double y); 
	bool	SavePmat(CString);	
	void	set_Pmat(double*);
	void	set_ModestoZero(short nummodes);
	void	CalcSlopes(double*,bool);
	bool	Initialize_Phi();
	double	*m_dErrV;
	double	*m_dPhiVZ;
	double	*m_dReconM;
	double	*m_dZM;
	double	*m_dZAbber;
	bool	Generate_Reconstructor();
	void	Initialize_Matrices();
	void	UnInitialize_Matrices();
	bool	set_MirrorChoice(short choice );
	void	set_MinCentroids(short centcount);
	bool	Load_Pmat();
	void	Generate_PTPmat();

	double	*m_dXYSlopeV;
	bool	*m_bWFSMask;
	short	*m_nWFS_index;
	double	*m_dReconM_fullpupil;  
	double	*m_dWFS_Lens_Position;
	bool	m_bRenewPmat;
	bool	m_bPenalizeTipTilt;
	double	*m_dPmat_Zernike;
	double	*m_dReconM_Zer;
	bool	m_bReconM_Zer_ready;
	bool	m_bPmat_Zernike_ready;
	void	Remove_Rotation(double *);
	double	Measure_Rotation(double *);
	void	Generate_Reconstructor_Zernike();
	void	set_Pmat_Zernike(double *);
	void	Remove_TipTilt(double *tempPhi4);
	void	Generate_Reconstructor_New();
	void	Update_Pmat();
	void	Remove_BadSpots(double *tempPhi3);
	void	Pinv_SVD(double *MA_pinv, double *MA, unsigned long len, unsigned long ht, unsigned short CutTermNum );
	unsigned short Update_bUse_centroid(double pupildia);
	double	pupil_usedforcorrection;

private:
	double	*m_dPmat;
	double	*m_dPmat_rcZero; //Pmat with rows and columns set to zero by m_bUse_centroid's and m_bUse_actuator's zero positions
	double	*m_dUmat_S;
	double	*m_dVmat_S;
	double	*m_dWImat_S;
	double	*m_dWImat_S_cpy;
	double	*m_dRefPhiV;
	double	*m_dRefPhiV_sensorlessAO;
	short	m_nModestoZero;
	short	m_sMinCent;
	bool	m_bPMatReady;
	bool	m_bMinCent;

	/////////////////////
	double	*m_dPmat_new;
	double	*m_dPmatT;
	////////////////////

		
	// Computed Variables	
	CMasterCentroid *m_centroid_matrix;  
	bool	m_bRecon;
	short	m_nTotalCentroids;
	double	*m_dBox_center;
	unsigned short *m_usSearch_array;

	double	*m_dActMask; // actuator mask for certain pupil size (always of size g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID)
	double	*m_dActMaskFull; // actuator mask for full pupil size (always of size g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID)
	double	*m_dAct_Positions_um_squared; // positions^2 in um of actuators on DM grid (always of size g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID)
	bool	*m_bUse_actuator; // boolean list of actuators to use (always of size g_AOSACAParams->NUMACTS)

	// Local operations
	bool	max_in_box(CDPoint *pcenter, short lengthx, short lengthy, unsigned char *imgbuf, unsigned short *peak);
	void	Reset_Master_Centroid();
	bool	find_centroid(int boxindex, unsigned char *imgbuf); 
	double	centroid_of_max(CDPoint *pcenter, short lengthx, short lengthy, unsigned char *imgbuf, bool retVariance);
};






#endif // !defined(AFX_CENTROID_H__579FEB40_6522_11D6_8008_006008A334DD__INCLUDED_)
