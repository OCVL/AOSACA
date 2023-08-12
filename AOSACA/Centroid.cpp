// Centroid.cpp: implementation of the CCentroid class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AOSACA.h"
#include "Centroid.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//Optivec library headers
#include "VecALL.h"
#include "MatALL.h"

extern AOSACAParams	*g_AOSACAParams;
//**********************************************************************************************************
// sub-class CMasterCentroid
//**********************************************************************************************************
CMasterCentroid::~CMasterCentroid()
{
	delete [] m_dCentroidV;
	delete [] m_dRef_centersV;
	delete [] m_bUse_centroid;
	delete [] m_bFound;
	delete [] m_dAverageintensity;
	delete [] m_sPeakintensity;
	m_sFound_cent_count = 0;
	m_sUse_cent_count = 0;
}

CMasterCentroid::CMasterCentroid()
{
}

CMasterCentroid::CMasterCentroid(short size)
{
	m_dCentroidV = new double[size<<1];
	memset(m_dCentroidV, 0, (size<<1)*sizeof(double));
	m_dRef_centersV = new double[size<<1];
	memset(m_dRef_centersV, 0, (size<<1)*sizeof(double));
	m_bUse_centroid = new bool[size];
	memset(m_bUse_centroid, 0, size*sizeof(bool));
	m_bFound = new bool[size];
	memset(m_bFound, 0, size*sizeof(bool));
	m_dAverageintensity = new double[size];
	memset(m_dAverageintensity, 0, size*sizeof(double));
	m_sPeakintensity = new unsigned short[size];
	memset(m_sPeakintensity, 0, size*sizeof(unsigned short));
	m_sFound_cent_count = 0;
	m_sUse_cent_count = 0;
	m_sGeo_cent_ind = -1;
}

//**********************************************************************************************************
// Construction/Destruction
//**********************************************************************************************************

CCentroid::CCentroid()
{
	//////////////////////////////////////
	m_centroid_matrix		= NULL;
	m_dPmat					= NULL;
	m_dPmat_rcZero			= NULL;
	m_dUmat_S				= NULL;
	m_dVmat_S				= NULL;
	m_dWImat_S				= NULL;
	m_dWImat_S_cpy			= NULL;
	m_bRecon				= false;
	m_dReconM				= NULL;
	m_dErrV					= NULL;
	m_dRefPhiV				= NULL;
	m_dRefPhiV_sensorlessAO = NULL;
	m_dPhiVZ				= NULL;
	m_dZM					= NULL;
	m_dZAbber				= NULL;
	m_nModestoZero			= 12;
	m_dBox_center			= NULL;
	m_usSearch_array		= NULL;
	m_sMinCent				= 0;
	m_bMinCent				= false;
	m_bPMatReady			= false;	
	m_dXYSlopeV				= NULL;
	m_bWFSMask				= NULL;
	m_nWFS_index			= NULL;
	m_dReconM_fullpupil		= NULL;  
	m_dWFS_Lens_Position	= NULL;
	m_bRenewPmat = false;
	m_bPenalizeTipTilt = false;
	m_dPmat_Zernike			= NULL;
	m_dReconM_Zer			= NULL;	
	m_dPmat_new				= NULL;
	m_dPmatT				= NULL;

	m_dActMask				= NULL;
	m_dActMaskFull			= NULL;
	m_dAct_Positions_um_squared = NULL;
	m_bUse_actuator			= NULL;

	InitializeCentroidSystemParameters();

	m_dXYSlopeV				= NULL;
	m_bReconM_Zer_ready=false;
	m_bRenewPmat=false;
	m_bPenalizeTipTilt=false;
	m_bPmat_Zernike_ready=false;
	m_dReconM_fullpupil=NULL;
	pupil_usedforcorrection=g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS;
}

CCentroid::~CCentroid()
{
	if (m_centroid_matrix != NULL) delete m_centroid_matrix;
	if (m_usSearch_array != NULL) delete [] m_usSearch_array;
	if (m_dBox_center != NULL) delete [] m_dBox_center;
	if (m_dUmat_S != NULL) delete [] m_dUmat_S;
	if (m_dVmat_S != NULL) delete [] m_dVmat_S;
	if (m_dWImat_S != NULL) delete [] m_dWImat_S;
	if (m_dWImat_S_cpy != NULL) delete [] m_dWImat_S_cpy;
	if (m_dErrV != NULL) delete [] m_dErrV;
	if (m_dZAbber != NULL) delete [] m_dZAbber, m_dZAbber=NULL;

}

//**********************************************************************************************************
//Parameter setting routines
//**********************************************************************************************************
void CCentroid::SetUserCenterPoint(float pupil_dia)
{
	Reset_Master_Centroid();
	Make_Search_Array(pupil_dia);
}

//**********************************************************************************************************

bool CCentroid::InitializeCentroidSystemParameters()
{	
	// The following two arrays are initialized only once and even if the optical parameters are changed,
	// the system parameters can be re-initialized time and again without re-allocating these 
	// resources.
	m_dBox_center = new double[g_AOSACAParams->LENSLETGRID<<1];
	memset(m_dBox_center, 0, (g_AOSACAParams->LENSLETGRID<<1)*sizeof(double));
	m_dErrV = new double[(g_AOSACAParams->NUMACTS)];
	memset(m_dErrV, 0, ((g_AOSACAParams->NUMACTS))*sizeof(double));
	m_usSearch_array = new unsigned short [(g_AOSACAParams->LENSLETGRID)*(g_AOSACAParams->LENSLETGRID)];
	memset(m_usSearch_array, 0, ((g_AOSACAParams->LENSLETGRID)*(g_AOSACAParams->LENSLETGRID))*sizeof(unsigned short));
	m_dZAbber = new double[MAX_TERM_NUMBER+1];
	memset(m_dZAbber, 0, (MAX_TERM_NUMBER+1)*sizeof(double));	
	return true;
}

void CCentroid::UnInitialize_Matrices()
{
	if (m_dPmat != NULL) delete [] m_dPmat, m_dPmat=NULL;
	if (m_dPmat_rcZero != NULL) delete [] m_dPmat_rcZero, m_dPmat_rcZero=NULL;
	if (m_dReconM != NULL) delete [] m_dReconM, m_dReconM=NULL;
	if (m_dRefPhiV != NULL) delete [] m_dRefPhiV, m_dRefPhiV=NULL;	
	if (m_dRefPhiV_sensorlessAO != NULL) delete [] m_dRefPhiV_sensorlessAO, m_dRefPhiV_sensorlessAO=NULL;	
	if (m_dPhiVZ != NULL) delete [] m_dPhiVZ, m_dPhiVZ=NULL;
	if (m_dZM != NULL) delete [] m_dZM, m_dZM=NULL;	

	//for Southwell
	if (m_dXYSlopeV != NULL) delete [] m_dXYSlopeV;
	if (m_bWFSMask != NULL) delete [] m_bWFSMask;
	if (m_nWFS_index != NULL) delete [] m_nWFS_index;
	if (m_dPmat_new != NULL) delete [] m_dPmat_new;
	if (m_dPmatT != NULL) delete [] m_dPmatT;
	if (m_dWFS_Lens_Position != NULL) delete [] m_dWFS_Lens_Position;

	//for Zernike control
	if (m_dPmat_Zernike != NULL) delete [] m_dPmat_Zernike, m_dPmat_Zernike=NULL;
	if (m_dReconM_Zer != NULL) delete [] m_dReconM_Zer, m_dReconM_Zer=NULL;
	if (m_dReconM_fullpupil != NULL) delete [] m_dReconM_fullpupil, m_dReconM_fullpupil=NULL;

	// for actuators control
	if (m_dActMask != NULL) delete [] m_dActMask;
	if (m_dActMaskFull != NULL) delete [] m_dActMaskFull;
	if (m_dAct_Positions_um_squared != NULL) delete [] m_dAct_Positions_um_squared;
	if (m_bUse_actuator != NULL) delete [] m_bUse_actuator;
}

void CCentroid::Initialize_Matrices()
{
	short size = m_nTotalCentroids<<1;
	m_dPmat = new double[size*(g_AOSACAParams->NUMACTS)];
	memset(m_dPmat, 0, (size*(g_AOSACAParams->NUMACTS))*sizeof(double));
	m_dPmat_rcZero = new double[size*(g_AOSACAParams->NUMACTS)]; 
	memset(m_dPmat_rcZero, 0, (size*(g_AOSACAParams->NUMACTS))*sizeof(double)); 
	m_dReconM = new double[(g_AOSACAParams->NUMACTS)*size];
	memset(m_dReconM, 0, ((g_AOSACAParams->NUMACTS)*size)*sizeof(double));
	m_dRefPhiV = new double[size];
	memset(m_dRefPhiV, 0, size*sizeof(double));	
	m_dRefPhiV_sensorlessAO = new double[size];
	memset(m_dRefPhiV_sensorlessAO, 0, size*sizeof(double));	
	m_dPhiVZ = new double[size+1];
	memset(m_dPhiVZ, 0, (size+1)*sizeof(double));
	m_dZM = new double[size*(MAX_TERM_NUMBER+1)];
	memset(m_dZM, 0, (size*(MAX_TERM_NUMBER+1))*sizeof(double));
	//for Southwell
	m_dXYSlopeV = new double[size];
	memset(m_dXYSlopeV, 0, (size)*sizeof(double));
	m_bWFSMask	= new bool[g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID];
	memset(m_bWFSMask, 0, ( g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID)*sizeof(bool));
	m_nWFS_index	= new short[g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID];
	memset(m_nWFS_index, 0, ( g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID)*sizeof(short));
	m_dPmat_new = new double[size*(g_AOSACAParams->NUMACTS)];
	memset(m_dPmat_new, 0, (size*(g_AOSACAParams->NUMACTS))*sizeof(double));
	m_dPmatT = new double[size*(g_AOSACAParams->NUMACTS)];
	memset(m_dPmatT, 0, (size*(g_AOSACAParams->NUMACTS))*sizeof(double));
	m_dWFS_Lens_Position=new double[size];
	memset(m_dWFS_Lens_Position, 0, (m_nTotalCentroids)*sizeof(double));
	m_dReconM_fullpupil = new double[(g_AOSACAParams->NUMACTS)*size];
	memset(m_dReconM_fullpupil, 0, ((g_AOSACAParams->NUMACTS)*size)*sizeof(double));
	short sizeDM_Grid_Squared = g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID;
	m_dActMask = new double[sizeDM_Grid_Squared]; 
	memset(m_dActMask, 0, (sizeDM_Grid_Squared)*sizeof(double));
	m_dActMaskFull = new double[sizeDM_Grid_Squared];
	memset(m_dActMaskFull, 0, (sizeDM_Grid_Squared)*sizeof(double));
	m_dAct_Positions_um_squared = new double[sizeDM_Grid_Squared];
	memset(m_dAct_Positions_um_squared, 0, (sizeDM_Grid_Squared)*sizeof(double));
	m_bUse_actuator = new bool[g_AOSACAParams->NUMACTS];
	memset(m_bUse_actuator, 0, (g_AOSACAParams->NUMACTS)*sizeof(bool));
	
	//////for Zernike control
	m_dPmat_Zernike = new double[MAX_TERM_NUMBER*(g_AOSACAParams->NUMACTS)];
	memset(m_dPmat_Zernike, 0, (MAX_TERM_NUMBER*(g_AOSACAParams->NUMACTS))*sizeof(double));
	m_dReconM_Zer = new double[(g_AOSACAParams->NUMACTS)*MAX_TERM_NUMBER];
	memset(m_dReconM_Zer, 0, ((g_AOSACAParams->NUMACTS)*MAX_TERM_NUMBER)*sizeof(double));
}

bool CCentroid::Load_Pmat()
{

	m_bPMatReady = false;
	m_bRecon = false;	
	CStringA filename;
	FILE *fp;
	// load Zernike Pmat
	fopen_s(&fp, "utils\\P_mat_Zernike.txt","r");
	if(fp)
	{	
		MD_read(&m_dPmat_Zernike, MAX_TERM_NUMBER, (g_AOSACAParams->NUMACTS), fp);		
		fclose(fp);	
		m_bPmat_Zernike_ready=true;
	}

	fopen_s(&fp, (_T("utils\\")+g_AOSACAParams->POKE_MATRIX_FILE+".pmat"),"r");
	if(fp)
	{	
		MD_read(&m_dPmat, m_nTotalCentroids<<1, (g_AOSACAParams->NUMACTS), fp);		
		fclose(fp);
		Generate_PTPmat();
	}

	return m_bPMatReady;
}

bool CCentroid::Generate_Reconstructor()
{
	m_bRecon = false;
	double *temp;
	short i;
	// SVD(P_mat'*P_mat)	
	temp = new double[(g_AOSACAParams->NUMACTS)*(g_AOSACAParams->NUMACTS)];
	memcpy(m_dWImat_S, m_dWImat_S_cpy, (g_AOSACAParams->NUMACTS)*(g_AOSACAParams->NUMACTS)*sizeof(double));
	for (i=1; i<m_nModestoZero; i++)
		m_dWImat_S[((g_AOSACAParams->NUMACTS)-i)*(g_AOSACAParams->NUMACTS)+((g_AOSACAParams->NUMACTS)-i)] = 0;

	MD_mulM(&temp, &m_dVmat_S, &m_dWImat_S, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS));
	MD_mulMT(&temp, &temp, &m_dUmat_S,(g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS));
	MD_mulMT(&m_dReconM, &temp, &m_dPmat_rcZero, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS), (m_nTotalCentroids<<1));
	
	///////////used for open loop
	if (pupil_usedforcorrection==g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) 
	{
		memcpy(m_dWImat_S, m_dWImat_S_cpy, (g_AOSACAParams->NUMACTS)*(g_AOSACAParams->NUMACTS)*sizeof(double));
		for (i=1; i<10/*modes to ignore for open loop correction*/; i++)
			m_dWImat_S[((g_AOSACAParams->NUMACTS)-i)*(g_AOSACAParams->NUMACTS)+((g_AOSACAParams->NUMACTS)-i)] = 0;
		MD_mulM(&temp, &m_dVmat_S, &m_dWImat_S, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS));
		MD_mulMT(&temp, &temp, &m_dUmat_S,(g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS));
		MD_mulMT(&m_dReconM_fullpupil, &temp, &m_dPmat_rcZero, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS), (m_nTotalCentroids<<1));
	} 

	m_bRecon = true;
	delete [] temp, temp = NULL;
	return (m_bRecon);
}

void CCentroid::Generate_PTPmat()
{
	// SVD(P_mat'*P_mat)
	unsigned long len=(g_AOSACAParams->NUMACTS);
	unsigned long ht=(g_AOSACAParams->NUMACTS);
	double *temp0,*m_dPmat_Copy;	
	temp0 = new double[len*ht];
	memset(temp0, 0, (len*ht)*sizeof(double));
	m_dPmat_Copy = new double[(m_nTotalCentroids<<1)*(g_AOSACAParams->NUMACTS)];
	memset(m_dPmat_Copy, 0, ((m_nTotalCentroids<<1)*(g_AOSACAParams->NUMACTS))*sizeof(double));

	memcpy(m_dPmat_Copy, m_dPmat, (m_nTotalCentroids<<1)*(g_AOSACAParams->NUMACTS)*sizeof(double));

	int count_bUse_actuator;
	count_bUse_actuator = 0;
	for (int col3=0; col3<(g_AOSACAParams->NUMACTS); col3++)
	{
		if (m_bUse_actuator[col3] == 0)
			count_bUse_actuator++;
	}

	int boxindex, col;
	boxindex=0;
	for (boxindex=0;boxindex<m_nTotalCentroids;boxindex++)
	{
		if (m_centroid_matrix->m_bUse_centroid[boxindex] == 0)
		{
			for (col=0; col<(g_AOSACAParams->NUMACTS); col++)
			{
				m_dPmat_Copy[boxindex*(g_AOSACAParams->NUMACTS)+col]=0;
				m_dPmat_Copy[(m_nTotalCentroids+boxindex)*(g_AOSACAParams->NUMACTS)+col]=0;
			}				
		}
		else 
		{
			if (count_bUse_actuator != g_AOSACAParams->NUMACTS) // Make sure only to do this is m_bUse_actuator has been calculated before at least once and is not all false
			{
				for (col=0; col<(g_AOSACAParams->NUMACTS); col++)
				{
					if (m_bUse_actuator[col] == 0)
					{
						m_dPmat_Copy[boxindex*(g_AOSACAParams->NUMACTS)+col]=0;
						m_dPmat_Copy[(m_nTotalCentroids+boxindex)*(g_AOSACAParams->NUMACTS)+col]=0;
					}
				}	
			}
		}
	}

	memcpy(m_dPmat_rcZero, m_dPmat_Copy, (m_nTotalCentroids<<1)*(g_AOSACAParams->NUMACTS)*sizeof(double));

	MD_TmulM( &temp0, &m_dPmat_rcZero, &m_dPmat_rcZero, (m_nTotalCentroids<<1), (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS) );
	double u;
	u=0.01;
	if (m_bPmat_Zernike_ready && m_bPenalizeTipTilt)
	{
		//tip/tilt penalization
		double *P_tiptilt;	
		P_tiptilt = new double[len*ht];
		memset(P_tiptilt, 0, (len*ht)*sizeof(double));
		MD_TmulM( &P_tiptilt, &m_dPmat_Zernike, &m_dPmat_Zernike, 2, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS) );
		//
		MD_mulC( &P_tiptilt, &P_tiptilt, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS), u ) ;
		MD_addM( &temp0, &temp0, &P_tiptilt, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS) );
		delete [] P_tiptilt,P_tiptilt=NULL;
	}

	if (m_dUmat_S == NULL)
	{
		m_dUmat_S = new double[ht*len];
		m_dWImat_S = new double[len*len];
		m_dWImat_S_cpy = new double[len*len];
		m_dVmat_S = new double[ len*len];
	}
	double *W, *MOne;
	W = new double[len];
	MOne = new double[ len*len];
	MD_equ1( &MOne, len);

	MD_SVdecompose( &m_dUmat_S, &m_dVmat_S, W, &temp0, ht, len);	
	MD_SVsort(&m_dUmat_S, &m_dVmat_S, W, &m_dUmat_S, &m_dVmat_S, W, ht, len);

	//any singular value less than tolerance are treated as zero
	unsigned int i;
	for (i=0; i<len; i++)
	{
		if (W[i]<0.0000000001)
		{
			W[i]=0;
		}
		else
		{
			W[i]=1/W[i];
		}
	}
	MD_mulMdia( &m_dWImat_S, &MOne, W, len, len);
	memcpy(m_dWImat_S_cpy, m_dWImat_S, len*len*sizeof(double));

	delete [] W, W = NULL;
	delete [] MOne, MOne = NULL;
	delete [] temp0, temp0 = NULL;
	delete [] m_dPmat_Copy, m_dPmat_Copy=NULL;
	
	m_bPMatReady = true;	
	Generate_Reconstructor();
}

void CCentroid::set_ModestoZero(short nummodes)
{
	m_nModestoZero = nummodes;
	m_bRecon&&m_bPMatReady?Generate_Reconstructor():0;
}

//**********************************************************************************************************
//Parameter retreival routines
//**********************************************************************************************************
short CCentroid::GetCentroidCount()
{
	return m_centroid_matrix->m_sFound_cent_count;
}

//**********************************************************************************************************
void CCentroid::Make_Search_Array (float pupil_dia)
{	
	double xcenterpoint, ycenterpoint;
	double *x, *y, *X, *Y;
	x = new double[g_AOSACAParams->LENSLETGRID];
	memset(x, 0, g_AOSACAParams->LENSLETGRID*sizeof(double));
	y = new double[g_AOSACAParams->LENSLETGRID];
	memset(y, 0, g_AOSACAParams->LENSLETGRID*sizeof(double));
	X = new double[g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID];
	memset(X, 0, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID*sizeof(double));
	Y = new double[g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID];
	memset(Y, 0, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID*sizeof(double));
	
	MD_Col_extract( x, &m_dBox_center, g_AOSACAParams->LENSLETGRID, 2, 1 );
	MD_transpose( &y, &x, g_AOSACAParams->LENSLETGRID, 1);
	MD_Col_extract( x, &m_dBox_center, g_AOSACAParams->LENSLETGRID, 2, 0 );
	//Mesh grid
	double *ones;
	ones = new double[g_AOSACAParams->LENSLETGRID];
	memset(ones, 0, g_AOSACAParams->LENSLETGRID*sizeof(double));
	VD_equC(ones, g_AOSACAParams->LENSLETGRID, 1.0);
	MD_mulM(&X, &ones, &x, g_AOSACAParams->LENSLETGRID, 1, g_AOSACAParams->LENSLETGRID);
	MD_mulMT(&Y, &y, &ones, g_AOSACAParams->LENSLETGRID, 1, g_AOSACAParams->LENSLETGRID);
	delete [] x, x=NULL;
	delete [] y, y = NULL;
	delete [] ones, ones = NULL;
	
	xcenterpoint = (g_AOSACAParams->PUPIL_CENTER).x;
	ycenterpoint = (g_AOSACAParams->PUPIL_CENTER).y;
	//compute reference centroids (searchbox_center(s))
	double pixelradius = (pupil_dia/2.)*g_AOSACAParams->MAGNIFICATION/g_AOSACAParams->MICRONS_PER_PIXEL;
	double *dSearch_array, *X2, *Y2;
	
	dSearch_array = new double[g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID];
	memset(dSearch_array, 0, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID*sizeof(double));
	X2 = new double[g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID];
	memset(X2, 0, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID*sizeof(double));
	Y2 = new double[g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID];	
	memset(Y2, 0, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID*sizeof(double));
	VDx_square( X2, X, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID, 1.0, -xcenterpoint );
	VDx_square( Y2, Y, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID, 1.0, -ycenterpoint );
	VD_addV( dSearch_array, X2, Y2, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID );

	VD_cmp_leC( dSearch_array, dSearch_array, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID, pow(pixelradius,2));
	m_nTotalCentroids = (short)VD_sum(dSearch_array, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID);
	VD_roundtoUS(m_usSearch_array, dSearch_array, g_AOSACAParams->LENSLETGRID*g_AOSACAParams->LENSLETGRID);
	delete [] X, X=NULL;
	delete [] Y, Y=NULL;
	delete [] dSearch_array, dSearch_array=NULL;
	delete [] X2, X2=NULL;
	delete [] Y2, Y2=NULL;
	UnInitialize_Matrices();
	Initialize_Matrices();	
	if (m_centroid_matrix != NULL) delete m_centroid_matrix;
	m_centroid_matrix = new CMasterCentroid(m_nTotalCentroids);

	for (short indexx=0; indexx<g_AOSACAParams->LENSLETGRID; indexx++)
	{
		for (short indexy=0; indexy<g_AOSACAParams->LENSLETGRID; indexy++)
		{
			m_bWFSMask[indexy*g_AOSACAParams->LENSLETGRID+indexx]=m_usSearch_array[indexy*g_AOSACAParams->LENSLETGRID+indexx]; 
			if (m_usSearch_array[indexy*g_AOSACAParams->LENSLETGRID+indexx] == 1)
			{ 
				m_nWFS_index[indexy+g_AOSACAParams->LENSLETGRID*indexx]=m_centroid_matrix->m_sFound_cent_count; 
				m_centroid_matrix->m_dCentroidV[m_centroid_matrix->m_sFound_cent_count] = m_centroid_matrix->m_dRef_centersV[m_centroid_matrix->m_sFound_cent_count] = m_dBox_center[indexx<<1];
				m_centroid_matrix->m_dCentroidV[m_centroid_matrix->m_sFound_cent_count+m_nTotalCentroids] = m_centroid_matrix->m_dRef_centersV[m_centroid_matrix->m_sFound_cent_count+m_nTotalCentroids] = m_dBox_center[(indexy<<1)+1];
				m_centroid_matrix->m_bFound[m_centroid_matrix->m_sFound_cent_count] = m_centroid_matrix->m_bUse_centroid[m_centroid_matrix->m_sFound_cent_count] = true;
				m_centroid_matrix->m_dAverageintensity[m_centroid_matrix->m_sFound_cent_count] = m_centroid_matrix->m_sPeakintensity[m_centroid_matrix->m_sFound_cent_count] = 0;
				m_dWFS_Lens_Position[m_centroid_matrix->m_sFound_cent_count] = sqrt(double((indexy - (g_AOSACAParams->LENSLETGRID-1)/2) * (indexy - (g_AOSACAParams->LENSLETGRID-1)/2) + (indexx - (g_AOSACAParams->LENSLETGRID-1)/2) * (indexx - (g_AOSACAParams->LENSLETGRID-1)/2)))*g_AOSACAParams->LENSLET_DIAMETER_MICRONS;
				if (m_dBox_center[indexx<<1] == (g_AOSACAParams->PUPIL_CENTER).x && m_dBox_center[(indexy<<1)+1] == (g_AOSACAParams->PUPIL_CENTER).y)
					m_centroid_matrix->m_sGeo_cent_ind = m_centroid_matrix->m_sFound_cent_count;
				m_centroid_matrix->m_sFound_cent_count++;				
			}
		}
	}
	m_centroid_matrix->m_sUse_cent_count = m_centroid_matrix->m_sFound_cent_count;
	m_bMinCent = true;
}

void CCentroid::Make_Search_Array_Act (float pupil_dia)
{	
	double xcenterpoint, ycenterpoint;
	double *x, *y, *X, *Y, actPitch;
	x = new double[g_AOSACAParams->DMGRID];
	memset(x, 0, g_AOSACAParams->DMGRID*sizeof(double));
	y = new double[g_AOSACAParams->DMGRID];
	memset(y, 0, g_AOSACAParams->DMGRID*sizeof(double));
	X = new double[g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID];
	memset(X, 0, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID*sizeof(double));
	Y = new double[g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID];
	memset(Y, 0, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID*sizeof(double));
	actPitch = 800; // Hard coded, actuator pitch = 800 um for Alpao 97 actuator DM
	xcenterpoint = (g_AOSACAParams->DMGRID)/double(2)*actPitch; // Should be 4400 um
	ycenterpoint = xcenterpoint;

	short index;
	for (index=0; index<g_AOSACAParams->DMGRID; index++)
	{
		x[index] = actPitch/2 + index*actPitch;
		y[index] = actPitch/2 + index*actPitch;
	}
	
	//Mesh grid
	double *ones;
	ones = new double[g_AOSACAParams->DMGRID];
	memset(ones, 0, g_AOSACAParams->DMGRID*sizeof(double));
	VD_equC(ones, g_AOSACAParams->DMGRID, 1.0);
	MD_mulM(&X, &ones, &x, g_AOSACAParams->DMGRID, 1, g_AOSACAParams->DMGRID);
	MD_mulMT(&Y, &y, &ones, g_AOSACAParams->DMGRID, 1, g_AOSACAParams->DMGRID);

	delete [] x, x=NULL;
	delete [] y, y = NULL;
	delete [] ones, ones = NULL;

	double pupil_rad = (pupil_dia/2.0);
	double *X2, *Y2;

	X2 = new double[g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID];
	memset(X2, 0, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID*sizeof(double));
	Y2 = new double[g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID];	
	memset(Y2, 0, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID*sizeof(double));
	VDx_square( X2, X, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID, 1.0, -xcenterpoint );
	VDx_square( Y2, Y, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID, 1.0, -ycenterpoint );
	
	VD_addV( m_dAct_Positions_um_squared, X2, Y2, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID );
	VD_cmp_leC( m_dActMaskFull, m_dAct_Positions_um_squared, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID, pow(1.25*pupil_dia/2,2));

	delete [] X, X=NULL;
	delete [] Y, Y=NULL;
	delete [] X2, X2=NULL;
	delete [] Y2, Y2=NULL;
}

int CCentroid::get_NormalizedCenters(double *normalizedgridV, double pupil_dia)
{
	double scalefactor = (pupil_dia/2.)*g_AOSACAParams->MAGNIFICATION/g_AOSACAParams->MICRONS_PER_PIXEL;
	int j=0;
	for (int i=0; i<m_nTotalCentroids; i++)
	{
		if (m_centroid_matrix->m_bUse_centroid[i])
		{
			normalizedgridV[j] = m_centroid_matrix->m_dRef_centersV[i];
			normalizedgridV[j+m_centroid_matrix->m_sUse_cent_count] = m_centroid_matrix->m_dRef_centersV[i+m_nTotalCentroids];
			j++;
		}
	}
	VD_subvector_subC(normalizedgridV, m_centroid_matrix->m_sUse_cent_count, 1, m_centroid_matrix->m_dRef_centersV[m_centroid_matrix->m_sGeo_cent_ind]);
	VD_subvector_subC(&normalizedgridV[m_centroid_matrix->m_sUse_cent_count], m_centroid_matrix->m_sUse_cent_count, 1, m_centroid_matrix->m_dRef_centersV[m_centroid_matrix->m_sGeo_cent_ind+m_nTotalCentroids]);	
	VD_divC(normalizedgridV, normalizedgridV, m_centroid_matrix->m_sUse_cent_count<<1, scalefactor);
	VD_subvector_mulC( &normalizedgridV[m_centroid_matrix->m_sUse_cent_count], m_centroid_matrix->m_sUse_cent_count, 1, -1.);
	
	return m_centroid_matrix->m_sUse_cent_count;
}

bool CCentroid::LocateAllCentroid(unsigned char *imgbuf)
{
	m_centroid_matrix->m_sFound_cent_count = 0;
	// go through all centroids locations to determine current centroid
	for (short index = 0; index<m_nTotalCentroids; index++)
	{
		if (m_centroid_matrix->m_bUse_centroid[index])
		{
			//find centroid in that box
			if (find_centroid(index,imgbuf))
				m_centroid_matrix->m_sFound_cent_count++;
		}
		else
			m_centroid_matrix->m_bFound[index]=0;
	}
	return (m_bMinCent = (m_centroid_matrix->m_sFound_cent_count >= m_sMinCent));
}

void CCentroid::LocateOneCentroid(unsigned char *imgbuf, short index)
{		
	if (m_centroid_matrix->m_bUse_centroid[index])
	{
		find_centroid(index, imgbuf);
		m_centroid_matrix->m_sUse_cent_count++;
	}
	else
	{
		m_centroid_matrix->m_bFound[index] = 0; // if there is a centroid found, then turn it off
		m_centroid_matrix->m_sUse_cent_count--;
	}
}

//**********************************************************************************************************
//Set_Master_Centroid now copies single MASTERCENTROID item to m_centroid_matrix array
//Allows access to m_centroid_matrix via function instead of extern global
void CCentroid::Reset_Master_Centroid()
//Note, if reset, use internal init value, otherwise use passed x and y value
//for the first time set it to the ideal reference array
{
	short index;
	double xcenterpoint, ycenterpoint, xstart, ystart;

	double spacing = (g_AOSACAParams->LENSLET_DIAMETER_MICRONS/g_AOSACAParams->MICRONS_PER_PIXEL);

	xcenterpoint = (g_AOSACAParams->PUPIL_CENTER).x;
	ycenterpoint = (g_AOSACAParams->PUPIL_CENTER).y;

	xstart = xcenterpoint - ((g_AOSACAParams->LENSLETGRID-1)>>1)*spacing;
	ystart = ycenterpoint - ((g_AOSACAParams->LENSLETGRID-1)>>1)*spacing;
		
	double shiftval;
	for (index=0; index<g_AOSACAParams->LENSLETGRID; index++)
	{
		shiftval = (index*spacing);
		m_dBox_center[index<<1] = xstart + shiftval;
		m_dBox_center[(index<<1)+1] = ystart + shiftval;
	}
}

//**********************************************************************************************************

short CCentroid::Add_Delete_Centroid(double x, double y)
{
	short index, id=-1;
	double xleftlim,xrightlim,ytoplim,ybotlim;

	//Returning a number that signifies that the right mouse button was not clicked in a box
	for (index = 0;index<m_nTotalCentroids;index++)
	{
		xleftlim=(m_centroid_matrix->m_dRef_centersV[index] - g_AOSACAParams->SEARCHBOX_SIZE/2);
		xrightlim=(m_centroid_matrix->m_dRef_centersV[index] + g_AOSACAParams->SEARCHBOX_SIZE/2);
		ytoplim=(m_centroid_matrix->m_dRef_centersV[index+m_nTotalCentroids] - g_AOSACAParams->SEARCHBOX_SIZE/2);
		ybotlim=(m_centroid_matrix->m_dRef_centersV[index+m_nTotalCentroids] + g_AOSACAParams->SEARCHBOX_SIZE/2);
		if (x >= xleftlim && x <= xrightlim && y >= ytoplim && y <= ybotlim)
		{
			m_centroid_matrix->m_bUse_centroid[index] = !m_centroid_matrix->m_bUse_centroid[index];
			id = index;
		}
	}
	if(id > -1)
		return true;	// ok
	else
		return false;	// out of range
}
				

//**********************************************************************************************************

void CCentroid::Estimate_Center(bool reset_user_center)
{
	double dist, dist_prev;
	short index;
	CDPoint center;
	short Geo_ind = m_centroid_matrix->m_sGeo_cent_ind;

	dist_prev = sqrt(pow(((g_AOSACAParams->PUPIL_CENTER).x - m_centroid_matrix->m_dCentroidV[0]),2)+
							pow(((g_AOSACAParams->PUPIL_CENTER).y - m_centroid_matrix->m_dCentroidV[m_nTotalCentroids]),2));
	for (index=0; index<m_nTotalCentroids; index++)
	{
		if (m_centroid_matrix->m_bFound[index])
		{
			dist = sqrt(pow(((g_AOSACAParams->PUPIL_CENTER).x - m_centroid_matrix->m_dCentroidV[index]),2)+
				pow(((g_AOSACAParams->PUPIL_CENTER).y - m_centroid_matrix->m_dCentroidV[index+m_nTotalCentroids]),2));
			if (dist <= dist_prev)
			{
				dist_prev = dist;
				Geo_ind = index;				
			}
		}
	}
	m_centroid_matrix->m_sGeo_cent_ind = Geo_ind;
	if (reset_user_center)
	{
		(g_AOSACAParams->PUPIL_CENTER).x = m_centroid_matrix->m_dCentroidV[m_centroid_matrix->m_sGeo_cent_ind];
		(g_AOSACAParams->PUPIL_CENTER).y = m_centroid_matrix->m_dCentroidV[m_centroid_matrix->m_sGeo_cent_ind+m_nTotalCentroids];
		Reset_Master_Centroid();
	}
}

bool CCentroid::find_centroid(int boxindex,unsigned char *imgbuf)
{
	CDPoint center;
	short lengthx, lengthy;
	unsigned short peakint; 
	double avgint;
	double xleftlim,xrightlim,ytoplim,ybotlim;	
	
	lengthx = lengthy = g_AOSACAParams->SEARCHBOX_SIZE;
	center.x = m_centroid_matrix->m_dRef_centersV[boxindex]; // set initial best guess for x centroid
	center.y = m_centroid_matrix->m_dRef_centersV[boxindex+m_nTotalCentroids]; // set initial best guess for y centroid

	if(center.x < 0 || center.y < 0	|| center.x > g_AOSACAParams->IMAGE_HEIGHT_PIX || center.y > g_AOSACAParams->IMAGE_WIDTH_PIX)
		; 
	else
	{
		m_centroid_matrix->m_bFound[boxindex] = false;
		if (max_in_box(&center, g_AOSACAParams->SEARCHBOX_SIZE, g_AOSACAParams->SEARCHBOX_SIZE, imgbuf, &peakint))
		{
			centroid_of_max(&center,g_AOSACAParams->CENT_MAX_PASS1,g_AOSACAParams->CENT_MAX_PASS1, imgbuf, false);
			centroid_of_max(&center,g_AOSACAParams->CENT_MAX_PASS2,g_AOSACAParams->CENT_MAX_PASS2, imgbuf, false);
			avgint = centroid_of_max(&center,g_AOSACAParams->CENT_MAX_PASS3,g_AOSACAParams->CENT_MAX_PASS3, imgbuf, true);		
    
			// If centroid is found outside of the search box or less than threshold then indicate
			// that the Centroid has not been found
			xleftlim=(m_centroid_matrix->m_dRef_centersV[boxindex] - g_AOSACAParams->SEARCHBOX_SIZE/2);
			xrightlim=(m_centroid_matrix->m_dRef_centersV[boxindex] + g_AOSACAParams->SEARCHBOX_SIZE/2);
			ytoplim=(m_centroid_matrix->m_dRef_centersV[boxindex+m_nTotalCentroids] - g_AOSACAParams->SEARCHBOX_SIZE/2);
			ybotlim=(m_centroid_matrix->m_dRef_centersV[boxindex+m_nTotalCentroids] + g_AOSACAParams->SEARCHBOX_SIZE/2);
			if (center.x > xleftlim || center.x < xrightlim
   				|| center.y > ytoplim || center.y < ybotlim)//found centroid above threshold
			{// great!! then change all values for that index
				m_centroid_matrix->m_dCentroidV[boxindex] = center.x; // update x centroid location with exact centroid
				m_centroid_matrix->m_dCentroidV[boxindex+m_nTotalCentroids] = center.y; // update y centroid location with exact centroid
				m_centroid_matrix->m_bFound[boxindex] = true; // indicate that the centroid has been found
				m_centroid_matrix->m_dAverageintensity[boxindex] = avgint;
				m_centroid_matrix->m_sPeakintensity[boxindex] = peakint;
			}
		}
	}
	return (m_centroid_matrix->m_bFound[boxindex]);
}

//**********************************************************************************************************

// Inside the box centered at the input point pcenter,
// find a point with the max intensity, and record
// the resulted point in pcenter to return back.
bool CCentroid::max_in_box(CDPoint *pcenter, short lengthx, short lengthy, 
							unsigned char *imgbuf, unsigned short *peak)
{
	unsigned char *ptr;
	unsigned long xx, yy;
	unsigned long m;//array offsets are longs
	short x_start, y_start, x_end, y_end;
	short i, j;
	unsigned short max, intensity;
	double xc, yc;
	short n = 0;
	bool status = false;

	ptr = imgbuf;

	xc = pcenter->x;
	yc = pcenter->y;

	xx = (unsigned long)g_AOSACAParams->IMAGE_WIDTH_PIX;
	yy = (unsigned long)g_AOSACAParams->IMAGE_HEIGHT_PIX;
	x_start = (int) (xc - lengthx / 2);
	y_start = (int) (yc - lengthy / 2);
	x_end = (int) (xc + lengthx / 2);
	y_end = (int) (yc + lengthy / 2);

	xc = 0;
	yc = 0;
	max = 0;
	
	for (j = y_start; j <= y_end; j++) 
	{
		m = (long) ((j - 1) * xx + x_start);
		ptr = imgbuf + m;

		for (i = x_start; i <= x_end; i++) 
		{
			intensity = (short) *(ptr);
			if (intensity > g_AOSACAParams->THRESHOLD)
			{
				if (intensity > max) 
				{
					max = intensity;
					xc = i;
					yc = j;
				}
				n = n + 1;
			}

			ptr++;
		}
	}
	
	if (n > 5)
	{
		pcenter->x = xc;
		pcenter->y = yc;
		peak = &max;
		status = true;
	}

	return status;
}

//**********************************************************************************************************
// Inside the box centered at the input point pcenter,
// do some average to decide another point, and record
// the resulted point in pcenter to return back.
double CCentroid::centroid_of_max(CDPoint *pcenter, short lengthx, short lengthy, 
								  unsigned char *imgbuf, bool retVariance)
{
	unsigned char *ptr;
	unsigned long xx, yy;
	unsigned long m; //array offsets are long
	short	x_start, y_start, x_end, y_end;
	long	tempx=0, tempy=0, temp=0;
	short	j;
	short	i;
	short intensity;
	double tempsquared;
	double xc, yc;
	
	xc = pcenter->x;
	yc = pcenter->y;

	ptr = imgbuf;
	
	xx = (unsigned long)g_AOSACAParams->IMAGE_WIDTH_PIX;
	yy = (unsigned long)g_AOSACAParams->IMAGE_HEIGHT_PIX;

	x_start = (int) (xc - lengthx / 2);
	y_start = (int) (yc - lengthy / 2);
	x_end = (int) (xc + lengthx / 2);
	y_end = (int) (yc + lengthy / 2);

	tempx = 0;
	tempy = 0;
	temp = 0;
	tempsquared = 0;
	
	for (j = y_start; j <= y_end; j++) 
	{
		m = (long) ((j - 1) * xx + x_start);
		ptr = imgbuf + m;
		for (i = x_start; i <= x_end; i++)
		{
			if ((intensity = (short) (*ptr)) > 20) 
			{				
				tempx += (long) i * (long) intensity;
				tempy += (long) j * (long) intensity;
				temp += intensity;
			}
			ptr++;
		}
	}
	
	if(tempx && tempy && temp)
	{
		pcenter->x = (double) tempx / temp;
		pcenter->y = (double) tempy / temp;
	}

	return (temp/((1+(double)lengthy)*(1+(double)lengthx)));
}

//**********************************************************************************************************
CString CCentroid::SaveCentroidTextFile(CString centtextfile)
{
	CString cstrStatus;
	CString cstrFileContents;
	CFile centFile;
	CFileException centFileError;
	BOOL centFileOK;
	CString cstrTemp;

	centFileOK=centFile.Open(centtextfile,CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite,
		&centFileError);
	if (centFileOK) {
		
		cstrFileContents="indexx\tindexy\txcent\tycent\n";
		for (short index = 0;index<m_nTotalCentroids;index++)
		{			
			if (m_centroid_matrix->m_bFound[index])
			{
			/*	cstrTemp.Format("%d\t%d\t%f\t%f\n", indexx,indexy, 
					m_centroid_matrix[index].x, m_centroid_matrix[index].y);
				cstrFileContents+=cstrTemp;*/
			}
		}
		centFile.Write(cstrFileContents,cstrFileContents.GetLength());
		centFile.Close();
		cstrStatus=centtextfile;
	}
	else
	{
		cstrStatus.Format(_T("Error. Failed to save centroids information: %d"),centFileError.m_cause);
	}
	return cstrStatus;

}//**********************************************************************************************************

bool CCentroid::SavePmat(CString fileTitle)
{
	bool status = false;
	CStringA filename;
	filename = g_AOSACAParams->g_stAppHomePath+_T("utils\\");
	filename += fileTitle;
	filename += ".pmat";
	FILE *fp;
	fp = NULL;
	fopen_s(&fp, filename,"w+");
	if (fp)
	{
		MD_store(fp, &m_dPmat, m_nTotalCentroids<<1, (g_AOSACAParams->NUMACTS)); 
		fclose(fp);
		status = true;
	}

	//save Poke matrix expressed as Zernike coefficients  10/20/2015
	filename = g_AOSACAParams->g_stAppHomePath+_T("utils\\P_mat_Zernike_0.txt");
	fopen_s(&fp, filename,"w+");
	if (fp)
	{
		MD_write(fp, &m_dPmat_Zernike, MAX_TERM_NUMBER, (g_AOSACAParams->NUMACTS));
		fclose(fp);
	}
	else
	{
		AfxMessageBox(_T("Can not open Pmat Zernike file to write, check folder permissions") ,MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);		
		status = false;
	}
    ////
	return status;
}

void CCentroid::set_Pmat(double *src)
{
	memset(m_dPmat, 0, (m_nTotalCentroids<<1)*(g_AOSACAParams->NUMACTS)*sizeof(double));
	MD_transpose(&m_dPmat, &src, m_nTotalCentroids<<1, (g_AOSACAParams->NUMACTS));
	Generate_PTPmat();
}



void CCentroid::CalcSlopes(double *dest, bool process)
{
	short size = m_nTotalCentroids<<1;	
	double *tempPhi;
	tempPhi = new double[size];
	memset(tempPhi, 0, (size)*sizeof(double));
	VD_subV(tempPhi, m_centroid_matrix->m_dCentroidV, m_centroid_matrix->m_dRef_centersV, size); // difference in pixels
	VD_subV(tempPhi, tempPhi, m_dRefPhiV_sensorlessAO, size);		//induce static abberations from sensorless AO protocol
	Remove_BadSpots(tempPhi);  //set the value of the missing spots 0

	//For wavefront computation by Southwell (Zernike free)
	memcpy(m_dXYSlopeV, tempPhi, size*sizeof(double));
	VD_mulC(m_dXYSlopeV, m_dXYSlopeV, size, g_AOSACAParams->MICRONS_PER_PIXEL/g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS);

	Remove_TipTilt(m_dXYSlopeV); //remove tip/tilt	
	VD_mulC(m_dXYSlopeV, m_dXYSlopeV, (size), (g_AOSACAParams->MAGNIFICATION));
	VD_subV(m_dXYSlopeV, m_dXYSlopeV, m_dRefPhiV, size);	
	VD_mulC(m_dXYSlopeV, m_dXYSlopeV, (size), 1/(g_AOSACAParams->MAGNIFICATION));
	//END Southwell	


	//For zernike coeffs computation
	int j=0;
	for (int i=0; i<m_nTotalCentroids; i++)
		if (m_centroid_matrix->m_bUse_centroid[i])
		{
			m_dPhiVZ[1+j] = tempPhi[i];
			m_dPhiVZ[1+j+m_centroid_matrix->m_sUse_cent_count] = tempPhi[i+m_nTotalCentroids];
			j++;
		}
	VD_subvector_mulC( &m_dPhiVZ[1], m_centroid_matrix->m_sUse_cent_count, 1, -1.);						//For zernike coeffs computation
	VD_mulC(m_dPhiVZ, m_dPhiVZ, (m_centroid_matrix->m_sUse_cent_count<<1)+1, (g_AOSACAParams->MAGNIFICATION*g_AOSACAParams->MICRONS_PER_PIXEL)/g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS);	//For zernike coeffs computation	
	//END //For zernike coeffs computation//
	
	if (process)
	{		
		VD_mulC(tempPhi, tempPhi, size, (g_AOSACAParams->MAGNIFICATION*g_AOSACAParams->MICRONS_PER_PIXEL)/g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS); // slopes in arb units
		if (dest != NULL)//Poke matrix generation
			memcpy(dest, tempPhi, size*sizeof(double));									
		else//Closeloop	
		{
			Remove_TipTilt(tempPhi); //remove tip/tilt
			if(m_bRenewPmat)//generate new constructor
			{	
				memcpy(m_dPmat_new, m_dPmat, (g_AOSACAParams->NUMACTS)*(m_nTotalCentroids)*sizeof(double));
				Update_Pmat();
				Generate_Reconstructor_New();			
			}
			VD_subV(tempPhi, m_dRefPhiV, tempPhi, size); //induce static abberations		//can be used for true rms	//negative feedback:m_dRefPhiV - tempPhi				
			Remove_TipTilt(tempPhi); //remove tip/tilt
			Remove_BadSpots(tempPhi);  //set the value of the missing spots 0 
			MD_mulV( m_dErrV, &m_dReconM, tempPhi, (g_AOSACAParams->NUMACTS), size );	//Closedloop computation
		}

	} else {
		VD_mulC(tempPhi, tempPhi, size, (g_AOSACAParams->MAGNIFICATION*g_AOSACAParams->MICRONS_PER_PIXEL)/g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS); // slopes in arb units
		if (dest != NULL)													//Poke matrix generation
			memcpy(dest, tempPhi, size*sizeof(double));
	}
	delete [] tempPhi, tempPhi=NULL;
}

bool CCentroid::Initialize_Phi()//double defocus)
{
	double *temp_dZAbber;
	temp_dZAbber = new double[MAX_TERM_NUMBER+1];
	VD_mulC(temp_dZAbber, m_dZAbber, MAX_TERM_NUMBER+1, (2/(g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS)));
	MD_mulV(m_dRefPhiV, &m_dZM, temp_dZAbber, m_nTotalCentroids<<1, MAX_TERM_NUMBER+1);
	memset(m_dErrV, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));

	return true;
}

bool CCentroid::FetchSlopeVector(double *dest, bool *found)
{
	memcpy(dest, m_centroid_matrix->m_dCentroidV, (m_nTotalCentroids<<1)*sizeof(double));
	memcpy(found, m_centroid_matrix->m_bFound, m_nTotalCentroids);
	return true;
}

void CCentroid::set_MinCentroids(short count)
{
	m_sMinCent = count;
}

short CCentroid::get_UseCentCount()
{
	return (m_centroid_matrix->m_sUse_cent_count);
}

void CCentroid::get_RefPoints(double *dest)
{
	memcpy(dest, m_centroid_matrix->m_dRef_centersV, (m_nTotalCentroids<<1)*sizeof(double));
}


/////////////////////////////////////////////////
//Zonal 
double CCentroid::Measure_Rotation(double *tempPhi2)
{
	int nx, ny, N;
	N=g_AOSACAParams->LENSLETGRID;
	double temp;
	temp=0;
	int num;
	num=0;
	for (nx=0;nx<(N);nx++)
	{
		for (ny=0;ny<(N-1);ny++)
		{
			if (m_bWFSMask[ny*N+nx] && m_bWFSMask[(ny+1)*N+nx])
			{
				temp=temp+tempPhi2[(m_nWFS_index[ny+1+N*nx])]-tempPhi2[(m_nWFS_index[ny+N*nx])];
				num=num+1;
			}
		}
	}
	if (num!=0)
		temp=temp/num;
	return temp;
}


void CCentroid::Remove_Rotation(double *tempPhi2)
{
	int nx, ny, boxindex, N;
	boxindex=0;
	N=g_AOSACAParams->LENSLETGRID;
	
	double a1, a2;
	a1=0.000038*g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS/g_AOSACAParams->MICRONS_PER_PIXEL;
	a2=-0.000038*g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS/g_AOSACAParams->MICRONS_PER_PIXEL;
	for (nx=0;nx<N;nx++)
	{
		for (ny=0;ny<N;ny++)
		{
			if (m_bWFSMask[ny*N+nx]==1)
			{
				tempPhi2[boxindex]=tempPhi2[boxindex]-a1*(ny-(N>>1));
				tempPhi2[m_nTotalCentroids+boxindex]=tempPhi2[m_nTotalCentroids+boxindex]-a2*(nx-(N>>1));
				boxindex++;
			}
		}
	}

}


void CCentroid::Remove_BadSpots(double *tempPhi3)
//if the spots are not found, the slopes x and y are set 0
{
	for (short boxindex=0;boxindex<m_nTotalCentroids;boxindex++)
	{
		if (m_centroid_matrix->m_bFound[boxindex] == 0)
		{
			tempPhi3[boxindex]=0;
			tempPhi3[m_nTotalCentroids+boxindex]=0;
		}
	}	
}


void CCentroid::Remove_TipTilt(double *tempPhi4)
{
	int boxindex;
	boxindex=0;
	double SlopeX_mean, SlopeY_mean;
	SlopeX_mean=0;
	SlopeY_mean=0;
	
	for (boxindex=0;boxindex<m_nTotalCentroids;boxindex++)
	{
		
		if (m_centroid_matrix->m_bFound[boxindex] == 1 )
		{
			SlopeX_mean=SlopeX_mean+tempPhi4[boxindex];
			SlopeY_mean=SlopeY_mean+tempPhi4[m_nTotalCentroids+boxindex];
		}
	}
	SlopeX_mean=SlopeX_mean/(m_centroid_matrix->m_sFound_cent_count);
	SlopeY_mean=SlopeY_mean/(m_centroid_matrix->m_sFound_cent_count);
	
	for (boxindex=0;boxindex<m_nTotalCentroids;boxindex++)
	{
		if (m_centroid_matrix->m_bFound[boxindex] == 1)
		{
			tempPhi4[boxindex]=tempPhi4[boxindex]-SlopeX_mean;
			tempPhi4[m_nTotalCentroids+boxindex]=tempPhi4[m_nTotalCentroids+boxindex]-SlopeY_mean;
		}
	}

}

void CCentroid::Update_Pmat()
{
	int boxindex, col;
	boxindex=0;
	for (boxindex=0;boxindex<m_nTotalCentroids;boxindex++)
	{
		if (m_centroid_matrix->m_bFound[boxindex] == 0)
		{
			for (col=0; col<(g_AOSACAParams->NUMACTS); col++)
			{
				m_dPmat_new[boxindex*(g_AOSACAParams->NUMACTS)+col]=0;
				m_dPmat_new[(m_nTotalCentroids+boxindex)*(g_AOSACAParams->NUMACTS)+col]=0;
			}				
		}	
	}

}

void CCentroid::Generate_Reconstructor_New()
{
	m_bRecon = false;

		
	unsigned long len=(g_AOSACAParams->NUMACTS);
	unsigned long ht=(g_AOSACAParams->NUMACTS);
	double *temp0;	
	temp0 = new double[len*ht];
	memset(temp0, 0, (len*ht)*sizeof(double));
	
	MD_TmulM( &temp0, &m_dPmat_new, &m_dPmat_new, (m_nTotalCentroids<<1), (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS));
	
	//tip/tilt penalization
	double u;
	u=0.01;
	if (m_bPmat_Zernike_ready && m_bPenalizeTipTilt)
	{
		double *P_tiptilt;
		P_tiptilt = new double[len*ht];
		memset(P_tiptilt, 0, (len*ht)*sizeof(double));
		MD_TmulM( &P_tiptilt, &m_dPmat_Zernike, &m_dPmat_Zernike, 2, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS) );
		//
		MD_mulC( &P_tiptilt, &P_tiptilt, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS), u ) ;
		MD_addM( &temp0, &temp0, &P_tiptilt, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS) );
		delete [] P_tiptilt,P_tiptilt=NULL;
	}

	double *invtemp;	
	invtemp = new double[len*ht];
	memset(invtemp, 0, (len*ht)*sizeof(double));

	Pinv_SVD(invtemp, temp0,  len , ht, m_nModestoZero);  //SVD
	MD_mulMT(&m_dReconM, &invtemp, &m_dPmat_new, len, ht, (m_nTotalCentroids<<1));

	m_bRecon = true;
	delete [] temp0, temp0 = NULL;
	delete [] invtemp, invtemp = NULL;
	

}

void CCentroid::set_Pmat_Zernike(double *src)
{
	memset(m_dPmat_Zernike, 0, MAX_TERM_NUMBER*(g_AOSACAParams->NUMACTS)*sizeof(double));
	MD_transpose(&m_dPmat_Zernike, &src, MAX_TERM_NUMBER, (g_AOSACAParams->NUMACTS));
}

void CCentroid::Generate_Reconstructor_Zernike()
{		
	unsigned long len=(g_AOSACAParams->NUMACTS);
	unsigned long ht=(g_AOSACAParams->NUMACTS);
	double *temp0;	
	temp0 = new double[len*ht];
	memset(temp0, 0, (len*ht)*sizeof(double));
	double *invtemp;	
	invtemp = new double[len*ht];
	memset(invtemp, 0, (len*ht)*sizeof(double));
	
	MD_TmulM( &temp0, &m_dPmat_Zernike, &m_dPmat_Zernike, MAX_TERM_NUMBER, (g_AOSACAParams->NUMACTS), (g_AOSACAParams->NUMACTS) );
	
	Pinv_SVD(invtemp, temp0,  len , ht, m_nModestoZero);  //SVD
	MD_mulMT(&m_dReconM_Zer, &invtemp, &m_dPmat_Zernike, len, ht, MAX_TERM_NUMBER);
	m_bReconM_Zer_ready=true;
	delete [] temp0, temp0 = NULL;
	delete [] invtemp, invtemp = NULL;

}

void CCentroid::Pinv_SVD(double *MA_pinv, double *MA, unsigned long len , unsigned long ht, unsigned short CutTermNum )
{

		double *Umat, *WImat, *Vmat;
		Umat = new double[ht*len];
		WImat = new double[len*len];
		Vmat = new double[ len*len];

	double *temp;	
	temp = new double[len*len];
	memset(temp, 0, (len*len)*sizeof(double));

	double *W, *MOne;
	W = new double[len];
	MOne = new double[ len*len]; 
	MD_equ1( &MOne, len);

	MD_SVdecompose( &Umat, &Vmat, W, &MA, ht, len);	
	MD_SVsort(&Umat, &Vmat, W, &Umat, &Vmat, W, ht, len);


	//any singular value less than tolerance are treated as zero
	unsigned int i;
	for (i=0; i<len; i++)
	{
		if (W[i]<0.0000000001)
		{
			W[i]=0;
		}
		else
		{
			W[i]=1/W[i];
		}
	}

	for (i=1; i<CutTermNum; i++)
		W[len-i] = 0;


	MD_mulMdia( &WImat, &MOne, W, len, len);
	MD_mulM(&temp, &Vmat, &WImat, len, len, len);
	MD_mulMT(&MA_pinv, &temp, &Umat, len, len, ht);


	delete [] W, W = NULL;
	delete [] MOne, MOne = NULL;
	delete [] temp, temp = NULL;
	delete [] Umat, Umat=NULL;
	delete [] WImat, WImat=NULL;
	delete [] Vmat, Vmat=NULL;
}


unsigned short  CCentroid::Update_bUse_centroid(double pupildia) // Also updates the used actuators
{
	int boxindex;
	unsigned short use_count = 0;
	double pupilradius = pupildia/2.;
	
	for (boxindex=0;boxindex<m_nTotalCentroids;boxindex++)
	{
		if (m_dWFS_Lens_Position[boxindex]>(pupilradius*g_AOSACAParams->MAGNIFICATION))
		{
			m_centroid_matrix->m_bUse_centroid[boxindex] =0;			
		}	
		else
		{
			m_centroid_matrix->m_bUse_centroid[boxindex] = 1;
			use_count++;
		}
	}

	m_centroid_matrix->m_sUse_cent_count = use_count;
	
	VD_cmp_leC(m_dActMask, m_dAct_Positions_um_squared, g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID, pow(1.15*pupilradius,2));

	int countActuator;
	countActuator = 0;
	for (int ii = 0; ii<(g_AOSACAParams->DMGRID*g_AOSACAParams->DMGRID);ii++)
	{
		if (m_dActMaskFull[ii] == 1)
		{
			m_bUse_actuator[countActuator] = (m_dActMask[ii] != 0);
			countActuator++;
		}
	}

	pupil_usedforcorrection = pupildia;
	if (m_bRenewPmat== false)
		Generate_PTPmat();
	return m_centroid_matrix->m_sUse_cent_count;
}

