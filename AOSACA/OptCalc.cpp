// OptCalc.cpp: implementation of the COptCalc class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AOSACA.h"
#include "OptCalc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SPECTACLE_PLANE_FACTOR 0.014
#define CORNEAL_PLANE_FACTOR 0.003

// Global object for storing/accessing applicaiton parameters
extern AOSACAParams	*g_AOSACAParams;
extern CDMirror		*g_dmirror;
extern CCentroid	*g_centroids;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//************************************************************************
COptCalcData::COptCalcData(int size)
{
	row=size;
	col=size;
	data = new double[size*size];
}
//************************************************************************
COptCalcData::~COptCalcData()
{
	delete [] data; 
}
//************************************************************************

COptCalc::COptCalc()
{
	m_rms = m_strehl_ratio = 0.;
	m_SystemInitDone=false;
	d_WAVE=NULL;
	d_PSF=NULL;
	d_MTF=NULL;
	m_dZReconM = NULL;	
	m_pupilfuncR = NULL;
	m_pupilfuncI = NULL;
	m_HampR = NULL;
	m_HampI = NULL;
	m_Hint = NULL;
	Zcoeff = NULL;	
	m_CurVoltages = NULL;
	m_PrevVoltages1 = NULL;
	m_dBiasBit = NULL;
	m_DMVoltages_buf = NULL;			//Used for AO ON/OFF mode
	m_CloopFlag = false;
	m_CloopMode = false;
	m_AOOnFlag = true;
	leak = 1.0;
	m_bZReconReady = false;
	m_bZOpenLoopReady = false;
	m_dFFactor = 0.975;
	m_dIntGain = 0.8;
	m_ReferenceDefocus = 0.0;
	
	m_Ref_PreZ345=new double[3];
	m_Ref_PreZ345[0] =0.0;
	m_Ref_PreZ345[1] =0.0;
	m_Ref_PreZ345[2] =0.0;	

	m_bPhiReconReady=false;
	
	m_dMatrixD = NULL;
	m_dMatrixG = NULL;
	m_dPhiReconM = NULL;
	m_bPhiReconReady = NULL;
	m_dWaveV = NULL;
	m_PreCorrectionVoltages = NULL;
	UpdateUserParameters(g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS);
	InitializeSystemParameters(g_AOSACAParams->g_bActuatorMask);
}

//************************************************************************
COptCalc::~COptCalc()
{
	if (m_SystemInitDone)
	{		
		unsigned int i=0;
		Send_Voltages(ABS_ZERO_BIT);
		for (i=0;i<d_WAVE->row;i++)
			delete [] m_nWData[i];
		delete [] m_nWData;
		if (Zcoeff != NULL)
			delete [] Zcoeff;
		delete [] d_WAVE;
		delete [] d_PSF;
		delete [] d_MTF;
		if (m_dZReconM != NULL) 
			delete [] m_dZReconM;
		for (i=0; i<(MAX_TERM_NUMBER+1); i++)
			delete [] m_zernike_terms[i];
		delete [] m_CurVoltages;
		delete [] m_PrevVoltages1;
		delete [] m_dBiasBit;
		delete [] m_DMVoltages_buf;
		delete [] m_pupilfuncR;
		delete [] m_pupilfuncI;	
		delete [] m_HampR;
		delete [] m_HampI;
		delete [] m_Hint;

		if (m_dPhiReconM != NULL) 
			delete [] m_dPhiReconM;
		if (m_dMatrixG != NULL) 
			delete [] m_dMatrixG;
		if (m_dMatrixD != NULL) 
			delete [] m_dMatrixD;
		if (m_dWaveV != NULL) 
			delete [] m_dWaveV;
		if (m_PreCorrectionVoltages != NULL) 
			delete m_PreCorrectionVoltages;
	}
}
//*************************************************************************
// INITIALIZATION/RETREIVAL FUNCTIONS
//*************************************************************************
BOOL COptCalc::InitializeSystemParameters(BYTE *mask)
{
	BOOL status=true; // assume ok until you get a null pointer
	unsigned int i=0;
	
	Zcoeff = new double[MAX_TERM_NUMBER+1];
	memset(Zcoeff, 0, (MAX_TERM_NUMBER+1)*sizeof(double));
	
	memset(c, 0, (MAX_TERM_NUMBER+1)*sizeof(double));

	d_WAVE = new COptCalcData(g_AOSACAParams->WAVE_WIDTH_PIX);//construct with size
	if (d_WAVE==NULL) return(false);

	d_PSF  = new COptCalcData(g_AOSACAParams->PSF_MTF_WIDTH_PIX);
	if (d_PSF==NULL) return(false);
	
	d_MTF  = new COptCalcData(g_AOSACAParams->PSF_MTF_WIDTH_PIX);
	if (d_MTF==NULL) return(false);	
	// Pertinent to computing voltages for the DM
	for (i=0; i<MAX_TERM_NUMBER+1; i++)
	{
		m_zernike_terms[i] = new double[g_AOSACAParams->NUMACTS];
		memset(m_zernike_terms[i], 0, (g_AOSACAParams->NUMACTS)*sizeof(double));
	}
	m_CurVoltages = new double[(g_AOSACAParams->NUMACTS)];
	memset(m_CurVoltages, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));
	memcpy(m_CurVoltages, g_AOSACAParams->g_DMBiasDeflections, g_AOSACAParams->NUMACTS*sizeof(double));
	m_PrevVoltages1 = new double[(g_AOSACAParams->NUMACTS)];
	memset(m_PrevVoltages1, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));
	m_dBiasBit = new double[(g_AOSACAParams->NUMACTS)];
	memset(m_dBiasBit, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));
	memcpy(m_dBiasBit, g_AOSACAParams->g_DMBiasDeflections, g_AOSACAParams->NUMACTS*sizeof(double));
	g_AOSACAParams->g_DMDeflections = new double[(g_AOSACAParams->NUMACTS)];
	memset(g_AOSACAParams->g_DMDeflections, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));
	m_DMVoltages_buf = new double[(g_AOSACAParams->NUMACTS)];		
	memset(m_DMVoltages_buf, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));
	m_PreCorrectionVoltages = new double[(g_AOSACAParams->NUMACTS)]; 
	memset(m_PreCorrectionVoltages, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));

	compute_ZernikeTerms(mask);
	//////////////////////////////////////////////////////////////////////////////
	// This if for computing the psf & wavefront
	m_pupilfuncR = new double [(g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX)];	
	memset(m_pupilfuncR, 0, ((g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX))*sizeof(double));
	m_pupilfuncI = new double [(g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX)];
	memset(m_pupilfuncI, 0, ((g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX))*sizeof(double));
	m_HampR      = new double [(g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX)];
	memset(m_HampR, 0, ((g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX))*sizeof(double));
	m_HampI      = new double [(g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX)];
	memset(m_HampI, 0, ((g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX))*sizeof(double));
	m_Hint		 = new double [(g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX)];	
	memset(m_Hint, 0, ((g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (g_AOSACAParams->PSF_MTF_WIDTH_PIX))*sizeof(double));

	m_nWData = new double*[d_WAVE->row];
	for (i=0;i<d_WAVE->row;i++)
	{
		m_nWData[i] = new double[(MAX_TERM_NUMBER+1)*d_WAVE->row];
		memset(m_nWData[i], 0, (MAX_TERM_NUMBER+1)*d_WAVE->row*sizeof(double));
	}
	Zwave_Matrix();
	m_SystemInitDone=true;
	return true;
}

//*************************************************************************
void COptCalc::UpdateUserParameters(double pupildia)
{
	m_pupil_field_size_microns = m_pupil_size_zernike = pupildia;
	m_psf_size_pix = 0;
	m_pupil_field_size_psf = 4.0 * m_pupil_field_size_microns;
	m_bZReconReady = false;
}

void COptCalc::UpdateDMMaxDeflection(double max_def)
{
	m_dMaxBit = -(max_def);
	m_dMinBit = max_def;
}

//*************************************************************************
COptCalcData * COptCalc::FetchOptCalcData(opt_data_enum idex)
{
	switch (idex)
	{
	case MTFDATA: return d_MTF;
	case PSFDATA: return d_PSF;
	case WAVEDATA: return d_WAVE;
	}
	return NULL;
}

int COptCalc::GetSize(opt_data_enum idex)
{
	switch (idex)
	{
	case MTFDATA: return d_MTF->row;
	case PSFDATA: return d_PSF->row;
	case WAVEDATA: return d_WAVE->row;
	}
	return NULL;
}

//*************************************************************************
// PUBLIC CENTROID CALCULATION FUNCTIONS
//*************************************************************************

void COptCalc::Init_Mahajan()
{
	//==================================================================
	// Initialize some info variable to zero
	//==================================================================
	m_defocus = m_cylinder = m_axis = m_strehl_ratio = 0.0;
	
	Initialize_Zernike_Coeffs (m_pupil_size_zernike);

	m_c3_old = c[3];// make a copy of astigmatism term
	m_c4_old = c[4];// make a copy of defocus term
	m_c5_old = c[5];// make a copy of astigmatism term
	Compute_Best_Spectacle_Correction();
//	for (int i=0; i<MAX_TERM_NUMBER+1; i++)
	//	Zcoeff[i] = c[i];
}

//*************************************************************************************************
//==================================================================
// Best spectacle correction
//     Calculate defocus, cylinder, axis, etc.
// These numbers are only used here to print out some info.
//
// Note: computation here depends on stored zernike components
// and pupil fit for calculations
//==================================================================
CStringA COptCalc::Compute_Best_Spectacle_Correction(
		BOOL c3term,
		BOOL c4term,
		BOOL c5term,
		correction_plane_enum plane,
		BOOL generate_string)
{
	double cm3, cm4, cm5, phi, Dcoeff, Acoeff;//, axis, cylinder, defocus;

	// Handle options for different included terms
	if (c3term) c[3]=m_c3_old; //astigmatism
	else c[3]=0.0;
	if (c4term) c[4]=m_c4_old; //defocus
	else c[4]=0.0;
	if (c5term) c[5]=m_c5_old; //astigmatism
	else c[5]=0.0;


	if (plane == COptCalc::ENTRANCEPLANE) {
	// Calculate the best defocus and astigmatic correction
	cm3 = m_c3_old / 1E6;	cm4 = m_c4_old / 1E6;	cm5 = m_c5_old / 1E6;
	cm3 = c[3] / 1E6;	cm4 = c[4] / 1E6;	cm5 = c[5] / 1E6;
	
	if (cm5 == 0)
		phi = (-1.0) * (cm3 >= 0 ? 1.0 : -1.0) * PI / 4.0;
	else
		phi = (-1.0) * 0.5 * atan(cm3 / cm5);
	
	if ( fabs(cm5) < fabs(cm3) )
		Acoeff =  cm3 * 2 * sqrt(6.) / sin(2 * phi);
	else
		Acoeff = -cm5 * 2 * sqrt(6.) / cos(2 * phi);
	
	m_axis = (1)*(180 * phi / PI);

//	m_axis = 90 + 180 * phi / PI; //original line 
	
	if (Acoeff <= 0.0) {
		Acoeff = (-1.) * Acoeff;
		m_axis = m_axis - 90.;
	}
	
	// This fix ensures that the cylinder is always between 0 and 180 deg.
	if (m_axis <= 0.)
		m_axis = m_axis + 180.;
	
	Dcoeff = cm4 * 2. * sqrt(3.) - Acoeff / 2.;
	
	m_cylinder = 2. * Acoeff / ( pow((m_pupil_size_zernike / (2000.*1000.)),2.));
	m_defocus =  2. * Dcoeff / ( pow((m_pupil_size_zernike / (2000.*1000.)),2.));
	
	}	// end if ENTRANCEPLANE; no need to recompute for other planes;


	CStringA cstrCorrection;
	if (generate_string) {
		switch(plane) {

		case COptCalc::ENTRANCEPLANE:	
			cstrCorrection.Format("#Refraction(Entrance Pupil)  %4.4f %4.4f %4.4f \n",
				m_defocus,m_cylinder,m_axis);
			break;

		case COptCalc::CORNEALPLANE:
			cstrCorrection.Format("#Refraction(Corneal Plane)  %4.4f %4.4f %4.4f \n",
				AlternatePlane(plane,m_defocus),
				AlternatePlane(plane,m_cylinder),
				m_axis);
			break;
		
		case COptCalc::SPECTACLEPLANE:
			cstrCorrection.Format("#Refraction(Spectacle Plane)  %4.4f %4.4f %4.4f \n",
				AlternatePlane(plane,m_defocus),
				AlternatePlane(plane,m_cylinder),
				m_axis);
			break;
		default:
			cstrCorrection.Format("BUG: OptCalc:Compute_Best_Spectacle_Correction");
		
		}//end switch
	} //end if
	else 
	{
		cstrCorrection.Format(
			"The best spectacle correction is %3.5g DS X %3.5g DC : AXIS %4g deg\n", 
			(-1) * m_defocus, (-1) * m_cylinder, m_axis);
	}
return cstrCorrection;
}

//*************************************************************************************************
double COptCalc::AlternatePlane(correction_plane_enum plane, double refparam)
{
	switch(plane)
	{
	case COptCalc::CORNEALPLANE:
			return (-1)*refparam/(1 - CORNEAL_PLANE_FACTOR*refparam);
	case COptCalc::SPECTACLEPLANE:
			return (-1)*refparam/(1 - SPECTACLE_PLANE_FACTOR*refparam);
	default:
		AfxMessageBox(_T("BUG!!! Unrecognized plane while computing Alternate Plane"), MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
	}
	return 0.0;
}
//*************************************************************************************************

int COptCalc::Compute_WAVE (double *src)
{
	unsigned int i, j, size;
	
	// Use pointer as one-dimentional array to store matrix.
	
	//==================================================================
	// The Pupil Function
	//==================================================================
	size = d_WAVE->col; 
	
	for (i = 0; i < d_WAVE->row; i++) 
	{
		for (j = 0; j < d_WAVE->col; j++)
			d_WAVE->data[i * size + j] = 0.0;
	}
///////////////////////////////////////////////	
	//Zwave_Mahajan_new(src);  //Zernike
	Wave_Translation(src);    //southwell
	return size; 
}

//*************************************************************************************************

double COptCalc::Compute_PSF_MTF(double *src)
{
	int i, j, size, idx;
	double tmp;
	

	//==================================================================
	// The Point Spread Function
	//==================================================================
	size = (unsigned long) g_AOSACAParams->PSF_MTF_WIDTH_PIX;
	if (d_PSF->row != (unsigned long)size) return 0;//error condition


	for (i = 0; i < size; i++) {
		for (j = 0;j < size; j++)
		m_HampR[i * size + j] = m_HampI[i * size + j] = 
		m_pupilfuncR[i * size + j] = m_pupilfuncI[i * size + j] = 0.0;
	}

//////////////////////////////////////////////////////
 	//Zphase_Mahajan(m_pupilfuncR, m_pupilfuncI, size);
	Phase_Translation(m_pupilfuncR, m_pupilfuncI, size);   //southell
	
	CRecipes::CalcFFT2(m_HampR, m_HampI, m_pupilfuncR, m_pupilfuncI, size);
	

	for (i = 0; i < size; i++) 
	{
		for (j = 0; j < size; j++) 
		{
			d_PSF->data[i * size + j] = 0.0;
			m_Hint[i * size + j] = m_HampR[i * size + j] * m_HampR[i * size + j] + m_HampI[i * size + j] * m_HampI[i * size + j];
		}
	}
	
	// NOTE: The dimension of a single pixel in the PSF in radians is the wavelength
	// divided by the size of the pupil field.
	CRecipes::ShiftFFT(d_PSF->data, m_Hint, size);
	tmp = pow(m_psf_size_pix, 2);// To scale the PSF so that peak represents the Strehl ratio
	for (i = 0; i < size; i++) 
	{
		for (j = 0; j < size; j++)
		{
			idx = i * size + j;
	 		src[idx] = d_PSF->data[idx] = d_PSF->data[idx] / tmp;
		}
	}
	
	// Compute Strehl Ratio. This is the maximum intensity value of PSF
	m_strehl_ratio = 0.0;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			if (d_PSF->data[i * size + j] > m_strehl_ratio)
				m_strehl_ratio = d_PSF->data[i * size + j];
		}
	}

	return m_strehl_ratio;
}

//*************************************************************************************************
void COptCalc::Initialize_Zernike_Coeffs(double pupildia)
{
	unsigned long i;
	
	memset(c, 0, (MAX_TERM_NUMBER+1)*sizeof(double));
	
	Calculate_Zernike_Coeffs_New(pupildia);

	// set piston and tilt to zero
	c[0] = 0;
}

/*************************************************************************
 *
 * Compute the coeff C[].
 *
 * C = M * P, where
 *		M = (D * D^T) ^ (-1) * D is a given 65 x 2 x NumberOfCentroids matrix,
 *		D is a 65 * 2 * NumberOfCentroids matrix,
 *		P is a 2 * NumberOfCentroids by 1 matrix we need to compute.
 *
 * C[] should be C[66]. The return result is in C[1] to C[65].
 *************************************************************************/
void COptCalc::Calculate_Zernike_Coeffs_New(double pupildia)
{ 
	int j,k; // term/mode index
	int sdx; // spot index;
	float sx,sy,devx,devy;
	float sx2,sx3,sx4,sx5,sx6,sx7,sx8,sx9;
	float sy2,sy3,sy4,sy5,sy6,sy7,sy8,sy9;
	float a[MAX_TERM_NUMBER+1][MAX_TERM_NUMBER+1];
	float ym[MAX_TERM_NUMBER+1][MAX_TERM_NUMBER+1],d,*col;
	int *indx,v,w,u;
	double magnification_factor=g_AOSACAParams->MAGNIFICATION;
	double microns_per_pixel=g_AOSACAParams->MICRONS_PER_PIXEL;
	double lenslet_focal_length=g_AOSACAParams->LENSLET_FOCAL_LENGTH_MICRONS;
	int num_of_spots = g_centroids->get_UseCentCount();

	if(!m_bZReconReady)
	{
		if (m_dZReconM != NULL)
			delete [] m_dZReconM;
		m_dZReconM = new double[(MAX_TERM_NUMBER+1)*((num_of_spots<<1)+1)];
		memset(m_dZReconM, 0, (MAX_TERM_NUMBER+1)*((num_of_spots<<1)+1)*sizeof(double));
		double *normalizedgridV = new double[num_of_spots<<1];
		memset(normalizedgridV, 0, (num_of_spots<<1)*sizeof(double));
		num_of_spots = g_centroids->get_NormalizedCenters(normalizedgridV, pupildia);
		double* m_recon = new double[((num_of_spots+1)<<1)*(MAX_TERM_NUMBER+1)];		
		memset(m_recon, 0, ((num_of_spots+1)<<1)*(MAX_TERM_NUMBER+1)*sizeof(double));
		float (*dt)[MAX_TERM_NUMBER+1] = new float [(num_of_spots+1)<<1][MAX_TERM_NUMBER+1];
		float (*dm)[MAX_TERM_NUMBER+1] = new float [(num_of_spots+1)<<1][MAX_TERM_NUMBER+1];

		memset(c, 0, (MAX_TERM_NUMBER+1)*sizeof(double));
	     for(j=0;j<=MAX_TERM_NUMBER;j++)
		 {
		    for(sdx=0;sdx<=num_of_spots<<1;sdx++)
			{
				dm[sdx][j]=0.0;
				dt[sdx][j] = 0.0;
			}
		 }

		 for(sdx=1;sdx<=num_of_spots;sdx++)
		 {
			 sx=(float)normalizedgridV[sdx-1];
			 sy=(float)normalizedgridV[sdx-1+num_of_spots];
			 sx2=sx*sx; sx3=sx2*sx; sx4=sx3*sx; sx5=sx4*sx;
			 sx6=sx5*sx; sx7=sx6*sx; sx8=sx7*sx; sx9=sx8*sx;
	
	         sy2=sy*sy; sy3=sy2*sy; sy4=sy3*sy; sy5=sy4*sy;
			 sy6=sy5*sy; sy7=sy6*sy; sy8=sy7*sy; sy9=sy8*sy;			 
	
			 for(j=1;j<=MAX_TERM_NUMBER;j++)
			 {		
				 memset(c, 0, (MAX_TERM_NUMBER+1)*sizeof(double));
	
				 c[j]=1.0;	

				 devx =(float)(
		         c[1]*0 +
		         c[2]*sqrt(4.) +
		         c[3]*2*sqrt(6.)*sy +
		         c[4]*4*sqrt(3.)*sx +
		         c[5]*2*sqrt(6.)*sx +
		         c[6]*6*sqrt(8.)*sx*sy +
		         c[7]*6*sqrt(8.)*sx*sy +
		         c[8]*sqrt(8.)*(9*sx2+3*sy2-2) +
		         c[9]*sqrt(8.)*(3*sx2-3*sy2) +
		         c[10]*sqrt(10.)*(12*sx2*sy-4*sy3) +
		         c[11]*sqrt(10.)*(24*sx2*sy+8*sy3-6*sy) +
		         c[12]*sqrt(5.)*(24*sx3+24*sx*sy2-12*sx) +
		         c[13]*sqrt(10.)*(16*sx3-6*sx) +
		         c[14]*sqrt(10.)*(4*sx3-12*sx*sy2) +
		         c[15]*sqrt(12.)*(20*sx3*sy-20*sx*sy3) +
		         c[16]*sqrt(12.)*(60*sx3*sy+20*sx*sy3-24*sx*sy) +
		         c[17]*sqrt(12.)*(40*sx3*sy+40*sx*sy3-24*sx*sy) +
		         c[18]*sqrt(12.)*(50*sx4+60*sx2*sy2+10*sy4-36*sx2-12*sy2+3) +
		         c[19]*sqrt(12.)*(25*sx4-30*sx2*sy2-12*sx2-15*sy4+12*sy2) +
		         c[20]*sqrt(12.)*(5*sx4-30*sx2*sy2+5*sy4) +	
		         c[21]*sqrt(14.)*(30*sx4*sy-60*sx2*sy3+6*sy5) +
		         c[22]*sqrt(14.)*(120*sx4*sy-60*sx2*sy-24*sy5+20*sy3) +
		         c[23]*sqrt(14.)*(150*sx4*sy+180*sx2*sy3+30*sy5-120*sx2*sy-40*sy3+12*sy) +
			     c[24]*sqrt(7.)*(120*sx5+240*sx3*sy2+120*sx*sy4-120*sx3-120*sx*sy2+24*sx) +
	  	         c[25]*sqrt(14.)*(90*sx5+60*sx3*sy2-30*sx*sy4-80*sx3+12*sx) +
		         c[26]*sqrt(14.)*(36*sx5-120*sx3*sy2-20*sx3-60*sx*sy4+60*sx*sy2) +	
		         c[27]*sqrt(14.)*(6*sx5-60*sx3*sy2+30*sx*sy4) +
		         c[28]*sqrt(16.)*(42*sx5*sy-140*sx3*sy3+42*sx*sy5) +
		         c[29]*sqrt(16.)*(210*sx5*sy-140*sx3*sy3-120*sx3*sy-126*sx*sy5+120*sx*sy3) +
		         c[30]*sqrt(16.)*(378*sx5*sy+420*sx3*sy3+42*sx*sy5-360*sx3*sy-120*sx*sy3+60*sx*sy) +
		         c[31]*sqrt(16.)*(210*sx5*sy+420*sx3*sy3+210*sx*sy5-240*sx3*sy-240*sx*sy3+60*sx*sy) +
		         c[32]*sqrt(16.)*(245*sx6+525*sx4*sy2+315*sx2*sy4+35*sy6-300*sx4-360*sx2*sy2-60*sy4+90*sx2+30*sy2-4) +
		         c[33]*sqrt(16.)*(147*sx6-105*sx4*sy2-315*sx2*sy4-150*sx4+180*sx2*sy2+30*sx2-63*sy6+90*sy4-30*sy2) +
		         c[34]*sqrt(16.)*(49*sx6-315*sx4*sy2-30*sx4-105*sx2*sy4+180*sx2*sy2+35*sy6-30*sy4) +
		         c[35]*sqrt(16.)*(7*sx6-105*sx4*sy2+105*sx2*sy4-7*sy6) +
		         c[36]*sqrt(18.)*(56*sx6*sy-280*sx4*sy3+168*sx2*sy5-8*sy7) +
		         c[37]*sqrt(18.)*(336*sx6*sy-560*sx4*sy3-210*sx4*sy-336*sx2*sy5+420*sx2*sy3+48*sy7-42*sy5) +
		         c[38]*sqrt(18.)*(784*sx6*sy+560*sx4*sy3-336*sx2*sy5-840*sx4*sy+180*sx2*sy-112*sy7+168*sy5-60*sy3) +
		         c[39]*sqrt(18.)*(784*sx6*sy+1680*sx4*sy3+1008*sx2*sy5+112*sy7-1050*sx4*sy-1260*sx2*sy3-210*sy5+360*sx2*sy+120*sy3-20*sy) +
		         c[40]*sqrt(9.)*(560*sx7+1680*sx5*sy2+1680*sx3*sy4+560*sx*sy6-840*sx5-1680*sx3*sy2-840*sx*sy4+360*sx3+360*sx*sy2-40*sx) +	
		         c[41]*sqrt(18.)*(448*sx7+672*sx5*sy2-224*sx*sy6-630*sx5-420*sx3*sy2+210*sx*sy4+240*sx3-20*sx) +
		         c[42]*sqrt(18.)*(224*sx7-672*sx5*sy2-1120*sx3*sy4-252*sx5+840*sx3*sy2+60*sx3-224*sx*sy6+420*sx*sy4-180*sx*sy2) +
		         c[43]*sqrt(18.)*(64*sx7-672*sx5*sy2-42*sx5+420*sx3*sy2+224*sx*sy6-210*sx*sy4) +
		         c[44]*sqrt(18.)*(8*sx7-168*sx5*sy2+280*sx3*sy4-56*sx*sy6) +
		         c[45]*sqrt(20.)*(72*sx7*sy-504*sx5*sy3+504*sx3*sy5-72*sx*sy7) +
		         c[46]*sqrt(20.)*(504*sx7*sy-1512*sx5*sy3-336*sx5*sy-504*sx3*sy5+1120*sx3*sy3+360*sx*sy7-336*sx*sy5) +
		         c[47]*sqrt(20.)*(1440*sx7*sy-2016*sx3*sy5-1680*sx5*sy+1120*sx3*sy3+420*sx3*sy-576*sx*sy7+1008*sx*sy5-420*sx*sy3) +
		         c[48]*sqrt(20.)*(2016*sx7*sy+4032*sx5*sy3+2016*sx3*sy5-3024*sx5*sy-3360*sx3*sy3-336*sx*sy5+1260*sx3*sy+420*sx*sy3-120*sx*sy) +
		         c[49]*sqrt(20.)*(1008*sx7*sy+3024*sx5*sy3+3024*sx3*sy5+1008*sx*sy7-1680*sx5*sy-3360*sx3*sy3-1680*sx*sy5+840*sx3*sy+840*sx*sy3-120*sx*sy) +
		         c[50]*sqrt(20.)*(1134*sx8+3528*sx6*sy2+3780*sx4*sy4+1512*sx2*sy6+126*sy8-1960*sx6-4200*sx4*sy2-2520*sx2*sy4-280*sy6+1050*sx4+1260*sx2*sy2+210*sy4-180*sx2-60*sy2+5) +
		         c[51]*sqrt(20.)*(756*sx8-2520*sx4*sy4-2016*sx2*sy6-1176*sx6+840*sx4*sy2+2520*sx2*sy4+525*sx4-630*sx2*sy2-60*sx2-252*sy8+504*sy6-315*sy4+60*sy2) +
		         c[52]*sqrt(20.)*(324*sx8-2016*sx6*sy2-2520*sx4*sy4-392*sx6+2520*sx4*sy2+105*sx4+840*sx2*sy4-630*sx2*sy2+180*sy8-280*sy6+105*sy4) +
		         c[53]*sqrt(20.)*(81*sx8-1260*sx6*sy2-56*sx6+630*sx4*sy4+840*sx4*sy2+756*sx2*sy6-840*sx2*sy4-63*sy8+56*sy6) +
		         c[54]*sqrt(20.)*(9*sx8-252*sx6*sy2+630*sx4*sy4-252*sx2*sy6+9*sy8) +
		         c[55]*sqrt(22.)*(90*sx8*sy-840*sx6*sy3+1260*sx4*sy5-360*sx2*sy7+10*sy9) +
		         c[56]*sqrt(22.)*(720*sx8*sy-3360*sx6*sy3-504*sx6*sy+2520*sx4*sy3+1440*sx2*sy7-1512*sx2*sy5-80*sy9+72*sy7) +
		         c[57]*sqrt(22.)*(2430*sx8*sy-2520*sx6*sy3-6300*sx4*sy5-3024*sx6*sy+5040*sx4*sy3+840*sx4*sy-1080*sx2*sy7+3024*sx2*sy5-1680*sx2*sy3+270*sy9-432*sy7+168*sy5) +
		         c[58]*sqrt(22.)*(4320*sx8*sy+6720*sx6*sy3-2880*sx2*sy7-7056*sx6*sy-5040*sx4*sy3+3024*sx2*sy5+3360*sx4*sy-420*sx2*sy-480*sy9+1008*sy7-672*sy5+140*sy3) +
		         c[59]*sqrt(22.)*(3780*sx8*sy+11760*sx6*sy3+12600*sx4*sy5+5040*sx2*sy7+420*sy9-7056*sx6*sy-15120*sx4*sy3-9072*sx2*sy5-1008*sy7+4200*sx4*sy+5040*sx2*sy3+840*sy5-840*sx2*sy-280*sy3+30*sy) +
		         c[60]*sqrt(11.)*(2520*sx9+10080*sx7*sy2+15120*sx5*sy4+10080*sx3*sy6+2520*sx*sy8-5040*sx7-15120*sx5*sy2-15120*sx3*sy4-5040*sx*sy6+3360*sx5+6720*sx3*sy2+3360*sx*sy4-840*sx3-840*sx*sy2+60*sx) +
		         c[61]*sqrt(22.)*(-560*sx3+30*sx+2520*sx5-840*sx*sy4+1680*sx3*sy2+2016*sx*sy6-6048*sx5*sy2-4032*sx7+2100*sx9+5040*sx7*sy2+2520*sx5*sy4-1680*sx3*sy6-1260*sx*sy8) +
		         c[62]*sqrt(22.)*(420*sx*sy2-140*sx3+1008*sx5-1680*sx*sy4-3360*sx3*sy2+2016*sx*sy6+6048*sx5*sy2+10080*sx3*sy4-2016*sx7+1200*sx9-2880*sx7*sy2-10080*sx5*sy4-6720*sx3*sy6-720*sx*sy8) +
		         c[63]*sqrt(22.)*(450*sx9-4680*sx7*sy2-3780*sx5*sy4-576*sx7+6048*sx5*sy2+168*sx5+2520*sx3*sy6-1680*sx3*sy2+1170*sx*sy8-2016*sx*sy6+840*sx*sy4) +
		         c[64]*sqrt(22.)*(100*sx9-2160*sx7*sy2-72*sx7+2520*sx5*sy4+1512*sx5*sy2+1680*sx3*sy6-2520*sx3*sy4-540*sx*sy8+504*sx*sy6) +
		         c[65]*sqrt(22.)*(10*sx9-360*sx7*sy2+1260*sx5*sy4-840*sx3*sy6+90*sx*sy8));
		

		         devy = (float)(
		         c[1]*sqrt(4.) +
		         c[2]*0 +
		         c[3]*2*sqrt(6.)*sx +
		         c[4]*4*sqrt(3.)*sy +
		         c[5]*(-2)*sqrt(6.)*sy +
		         c[6]*sqrt(8.)*(3*sx2-3*sy2) +
		         c[7]*sqrt(8.)*(3*sx2+9*sy2-2) +
		         c[8]*6*sqrt(8.)*sx*sy +
		         c[9]*(-6)*sqrt(8.)*sx*sy +
		         c[10]*sqrt(10.)*(4*sx3-12*sx*sy2) +
		         c[11]*sqrt(10.)*(8*sx3+24*sx*sy2-6*sx) +
		         c[12]*sqrt(5.)*(24*sx2*sy+24*sy3-12*sy) +
		         c[13]*sqrt(10.)*(-16*sy3+6*sy) +
		         c[14]*sqrt(10.)*(-12*sx2*sy+4*sy3) +
		         c[15]*sqrt(12.)*(5*sx4-30*sx2*sy2+5*sy4) +
		         c[16]*sqrt(12.)*(15*sx4+30*sx2*sy2-12*sx2-25*sy4+12*sy2) +
		         c[17]*sqrt(12.)*(10*sx4+60*sx2*sy2+50*sy4-12*sx2-36*sy2+3) +
		         c[18]*sqrt(12.)*(40*sx3*sy+40*sx*sy3-24*sx*sy) +
		         c[19]*sqrt(12.)*(-20*sx3*sy-60*sx*sy3+24*sx*sy) +
		         c[20]*sqrt(12.)*(-20*sx3*sy+20*sx*sy3) +
		         c[21]*sqrt(14.)*(6*sx5-60*sx3*sy2+30*sx*sy4) +
		         c[22]*sqrt(14.)*(24*sx5-20*sx3-120*sx*sy4+60*sx*sy2) +
		         c[23]*sqrt(14.)*(30*sx5+180*sx3*sy2+150*sx*sy4-40*sx3-120*sx*sy2+12*sx) +
		         c[24]*sqrt(7.)*(120*sx4*sy+240*sx2*sy3+120*sy5-120*sx2*sy-120*sy3+24*sy) +
		         c[25]*sqrt(14.)*(30*sx4*sy-60*sx2*sy3-90*sy5+80*sy3-12*sy) +
		         c[26]*sqrt(14.)*(-60*sx4*sy-120*sx2*sy3+60*sx2*sy+36*sy5-20*sy3) +
		         c[27]*sqrt(14.)*(-30*sx4*sy+60*sx2*sy3-6*sy5) +
		         c[28]*sqrt(16.)*(7*sx6-105*sx4*sy2+105*sx2*sy4-7*sy6) +
		         c[29]*sqrt(16.)*(35*sx6-105*sx4*sy2-30*sx4-315*sx2*sy4+180*sx2*sy2+49*sy6-30*sy4) +
		         c[30]*sqrt(16.)*(63*sx6+315*sx4*sy2+105*sx2*sy4-90*sx4-180*sx2*sy2+30*sx2-147*sy6+150*sy4-30*sy2) +
		         c[31]*sqrt(16.)*(35*sx6+315*sx4*sy2+525*sx2*sy4+245*sy6-60*sx4-360*sx2*sy2-300*sy4+30*sx2+90*sy2-4) +
		         c[32]*sqrt(16.)*(210*sx5*sy+420*sx3*sy3+210*sx*sy5-240*sx3*sy-240*sx*sy3+60*sx*sy) +
		         c[33]*sqrt(16.)*(-42*sx5*sy-420*sx3*sy3+120*sx3*sy-378*sx*sy5+360*sx*sy3-60*sx*sy) +
		         c[34]*sqrt(16.)*(-126*sx5*sy-140*sx3*sy3+120*sx3*sy+210*sx*sy5-120*sx*sy3) +
		         c[35]*sqrt(16.)*(-42*sx5*sy+140*sx3*sy3-42*sx*sy5) +
		         c[36]*sqrt(18.)*(8*sx7-168*sx5*sy2+280*sx3*sy4-56*sx*sy6) +
		         c[37]*sqrt(18.)*(48*sx7-336*sx5*sy2-42*sx5-560*sx3*sy4+420*sx3*sy2+336*sx*sy6-210*sx*sy4) +
		         c[38]*sqrt(18.)*(112*sx7+336*sx5*sy2-560*sx3*sy4-168*sx5+60*sx3-784*sx*sy6+840*sx*sy4-180*sx*sy2) +
		         c[39]*sqrt(18.)*(112*sx7+1008*sx5*sy2+1680*sx3*sy4+784*sx*sy6-210*sx5-1260*sx3*sy2-1050*sx*sy4+120*sx3+360*sx*sy2-20*sx) +
		         c[40]*sqrt(9.)*(560*sx6*sy+1680*sx4*sy3+1680*sx2*sy5+560*sy7-840*sx4*sy-1680*sx2*sy3-840*sy5+360*sx2*sy+360*sy3-40*sy) +
		         c[41]*sqrt(18.)*(224*sx6*sy-672*sx2*sy5-210*sx4*sy+420*sx2*sy3-448*sy7+630*sy5-240*sy3+20*sy) +
		         c[42]*sqrt(18.)*(-224*sx6*sy-1120*sx4*sy3+420*sx4*sy-672*sx2*sy5+840*sx2*sy3-180*sx2*sy+224*sy7-252*sy5+60*sy3) +
		         c[43]*sqrt(18.)*(-224*sx6*sy+210*sx4*sy+672*sx2*sy5-420*sx2*sy3-64*sy7+42*sy5) +
		         c[44]*sqrt(18.)*(-56*sx6*sy+280*sx4*sy3-168*sx2*sy5+8*sy7) +
		         c[45]*sqrt(20.)*(9*sx8-252*sx6*sy2+630*sx4*sy4-252*sx2*sy6+9*sy8) +
		         c[46]*sqrt(20.)*(63*sx8-756*sx6*sy2-56*sx6-630*sx4*sy4+840*sx4*sy2+1260*sx2*sy6-840*sx2*sy4-81*sy8+56*sy6) +
		         c[47]*sqrt(20.)*(180*sx8-2520*sx4*sy4-280*sx6+840*sx4*sy2+105*sx4-2016*sx2*sy6+2520*sx2*sy4-630*sx2*sy2+324*sy8-392*sy6+105*sy4) +
		         c[48]*sqrt(20.)*(252*sx8+2016*sx6*sy2+2520*sx4*sy4-504*sx6-2520*sx4*sy2-840*sx2*sy4+315*sx4+630*sx2*sy2-60*sx2-756*sy8+1176*sy6-525*sy4+60*sy2) +
		         c[49]*sqrt(20.)*(126*sx8+1512*sx6*sy2+3780*sx4*sy4+3528*sx2*sy6+1134*sy8-280*sx6-2520*sx4*sy2-4200*sx2*sy4-1960*sy6+210*sx4+1260*sx2*sy2+1050*sy4-60*sx2-180*sy2+5) +
		         c[50]*sqrt(20.)*(1008*sx7*sy+3024*sx5*sy3+3024*sx3*sy5+1008*sx*sy7-1680*sx5*sy-3360*sx3*sy3-1680*sx*sy5+840*sx3*sy+840*sx*sy3-120*sx*sy) +
		         c[51]*sqrt(20.)*(-2016*sx5*sy3-4032*sx3*sy5+336*sx5*sy+3360*sx3*sy3-420*sx3*sy-2016*sx*sy7+3024*sx*sy5-1260*sx*sy3+120*sx*sy) +
		         c[52]*sqrt(20.)*(-576*sx7*sy-2016*sx5*sy3+1008*sx5*sy+1120*sx3*sy3-420*sx3*sy+1440*sx*sy7-1680*sx*sy5+420*sx*sy3) +
		         c[53]*sqrt(20.)*(-360*sx7*sy+504*sx5*sy3+336*sx5*sy+1512*sx3*sy5-1120*sx3*sy3-504*sx*sy7+336*sx*sy5) +
		         c[54]*sqrt(20.)*(-72*sx7*sy+504*sx5*sy3-504*sx3*sy5+72*sx*sy7) +
		         c[55]*sqrt(22.)*(10*sx9-360*sx7*sy2+1260*sx5*sy4-840*sx3*sy6+90*sx*sy8) +
		         c[56]*sqrt(22.)*(80*sx9-1440*sx7*sy2-72*sx7+1512*sx5*sy2+3360*sx3*sy6-2520*sx3*sy4-720*sx*sy8+504*sx*sy6) +
		         c[57]*sqrt(22.)*(270*sx9-1080*sx7*sy2-6300*sx5*sy4-432*sx7+3024*sx5*sy2+168*sx5-2520*sx3*sy6+5040*sx3*sy4-1680*sx3*sy2+2430*sx*sy8-3024*sx*sy6+840*sx*sy4) +
		         c[58]*sqrt(22.)*(480*sx9+2880*sx7*sy2-6720*sx3*sy6-1008*sx7-3024*sx5*sy2+5040*sx3*sy4+672*sx5-140*sx3-4320*sx*sy8+7056*sx*sy6-3360*sx*sy4+420*sx*sy2) +
		         c[59]*sqrt(22.)*(420*sx9+5040*sx7*sy2+12600*sx5*sy4+11760*sx3*sy6+3780*sx*sy8-1008*sx7-9072*sx5*sy2-15120*sx3*sy4-7056*sx*sy6+840*sx5+5040*sx3*sy2+4200*sx*sy4-280*sx3-840*sx*sy2+30*sx) +
		         c[60]*sqrt(11.)*(2520*sx8*sy+10080*sx6*sy3+15120*sx4*sy5+10080*sx2*sy7+2520*sy9-5040*sx6*sy-15120*sx4*sy3-15120*sx2*sy5-5040*sy7+3360*sx4*sy+6720*sx2*sy3+3360*sy5-840*sx2*sy-840*sy3+60*sy) +
		         c[61]*sqrt(22.)*(560*sy3-30*sy-2520*sy5-1680*sx2*sy3+840*sx4*sy+6048*sx2*sy5-2016*sx6*sy+4032*sy7-2100*sy9+1260*sx8*sy+1680*sx6*sy3-2520*sx4*sy5-5040*sx2*sy7) +
		         c[62]*sqrt(22.)*(-140*sy3+420*sx2*sy+1008*sy5-3360*sx2*sy3-1680*sx4*sy+6048*sx2*sy5+2016*sx6*sy+10080*sx4*sy3-2016*sy7+1200*sy9-720*sx8*sy-6720*sx6*sy3-10080*sx4*sy5-2880*sx2*sy7) +
		         c[63]*sqrt(22.)*(-1170*sx8*sy-2520*sx6*sy3+2016*sx6*sy+3780*sx4*sy5-840*sx4*sy+4680*sx2*sy7-6048*sx2*sy5+1680*sx2*sy3-450*sy9+576*sy7-168*sy5) +
		         c[64]*sqrt(22.)*(-540*sx8*sy+1680*sx6*sy3+504*sx6*sy+2520*sx4*sy5-2520*sx4*sy3-2160*sx2*sy7+1512*sx2*sy5+100*sy9-72*sy7) +
		         c[65]*sqrt(22.)*(-90*sx8*sy+840*sx6*sy3-1260*sx4*sy5+360*sx2*sy7-10*sy9));		

			     dm[sdx][j]=devx;
				 dm[sdx+num_of_spots][j]=devy;
				 if (!m_bZOpenLoopReady) 
				 {
					 g_centroids->m_dZM[(sdx-1)*(MAX_TERM_NUMBER+1)+j] = devx;
					 g_centroids->m_dZM[(sdx+num_of_spots-1)*(MAX_TERM_NUMBER+1)+j] = -devy;
				 }
			 }  ///end j
		}///end sdx

		if (!m_bZOpenLoopReady)
		{// write a copy of m_dZM to a text file on G: for sensorlessAO protocol access
			FILE* dZM;
			fopen_s(&dZM, "G:\\dZM.txt", "w+");
			MD_write(dZM, &g_centroids->m_dZM, g_centroids->get_TotalCentCount()<<1, MAX_TERM_NUMBER+1 );
			fclose(dZM);
		}

		m_bZOpenLoopReady = true;

		for(j=1;j<=MAX_TERM_NUMBER;j++)
		{
			for(sdx=1;sdx<=(num_of_spots<<1);sdx++)
			{
				dt[sdx][j]=dm[sdx][j];
			}
		}

		for(j=1;j<=MAX_TERM_NUMBER;j++)
		{
			for(k=1;k<=MAX_TERM_NUMBER;k++)
			{   a[j][k]=0.0;
			   for(sdx=1;sdx<=(num_of_spots<<1);sdx++)
			   {
			       a[j][k]=a[j][k]+dm[sdx][j]*dt[sdx][k];
			   }
			}
		}

	    indx = new int [MEMINIT];
	    col = new float [MEMINIT];
		CRecipes::ludcmp(a,MAX_TERM_NUMBER,indx, &d);
	
	    for(v=1;v<=MAX_TERM_NUMBER;v++)
		{
			for(w=1;w<=MAX_TERM_NUMBER;w++) col[w]=0.0;
			col[v]=1.0;
			CRecipes::lubksb(a,MAX_TERM_NUMBER,indx,col);
			for(u=1;u<=MAX_TERM_NUMBER;u++)  ym[u][v]=col[u];
		}
	
	    for(j=1;j<=MAX_TERM_NUMBER;j++)
			for(sdx=1;sdx<=(num_of_spots<<1);sdx++)
			{ 
				m_recon[sdx*(MAX_TERM_NUMBER+1)+j]=0.0;
	            for(k=1;k<=MAX_TERM_NUMBER;k++)
					m_recon[sdx*(MAX_TERM_NUMBER+1)+j] =m_recon[sdx*(MAX_TERM_NUMBER+1)+j]+ym[j][k]*dm[sdx][k];
			}
		MD_transpose(&m_dZReconM, &m_recon, MAX_TERM_NUMBER+1, (num_of_spots<<1)+1);
		
		c[1]=0.0; c[2]=0.0;
		delete [] dt;
		delete [] indx;
		delete [] col;
		delete [] m_recon;
		delete [] normalizedgridV;
		m_bZReconReady = true;
    } 
	
	memset(c, 0, (MAX_TERM_NUMBER+1)*sizeof(double));
	VD_mulC(&g_centroids->m_dPhiVZ[1], &g_centroids->m_dPhiVZ[1], (num_of_spots<<1), (pupildia)/2.);		
	MD_mulV(c, &m_dZReconM, g_centroids->m_dPhiVZ, MAX_TERM_NUMBER+1, (num_of_spots<<1)+1);
}

void COptCalc::Zwave_Matrix(void)
{
	unsigned long nx, ny;
	double xpos, ypos, angle, norm_radius, sizeoffield;
	
	double norm_radius_2, norm_radius_3, norm_radius_4, norm_radius_5, norm_radius_6, norm_radius_7, norm_radius_8, norm_radius_9, norm_radius_10;
	double sin_angle, sin_2angle, sin_3angle, sin_4angle, sin_5angle, sin_6angle, sin_7angle, sin_8angle, sin_9angle, sin_10angle;
	double cos_angle, cos_2angle, cos_3angle, cos_4angle, cos_5angle, cos_6angle, cos_7angle, cos_8angle, cos_9angle, cos_10angle;
	double sqrt_3, sqrt_6, sqrt_5, sqrt_7, sqrt_8, sqrt_10, sqrt_11, sqrt_12, sqrt_14, sqrt_18, sqrt_20, sqrt_22;	
	unsigned long N=d_WAVE->row;

	sizeoffield = g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS;
	
	sqrt_3  = sqrt(3.);		sqrt_6  = sqrt(6.);		sqrt_5  = sqrt(5.);
	sqrt_7  = sqrt(7.);		sqrt_8  = sqrt(8.);		sqrt_10 = sqrt(10.);
	sqrt_11 = sqrt(11.);		sqrt_12 = sqrt(12.);		sqrt_14 = sqrt(14.);
	sqrt_18 = sqrt(18.);		sqrt_20 = sqrt(20.);		sqrt_22 = sqrt(22.);
	
	for (ny = 0; ny < N; ny++) 
	{
		for (nx = 0; nx < N; nx++) 
		{
			xpos = nx * (sizeoffield / N) - (sizeoffield / 2);
			ypos = ny * (sizeoffield / N) - (sizeoffield / 2);
			
			norm_radius = sqrt(xpos * xpos + ypos * ypos) / (m_pupil_size_zernike/2);
			
			// clock-type coordinate system to a standard coordinate system (OSA vision standard) 
			if (xpos == 0 && ypos > 0)
				angle = 3.1416/2;
			else if (xpos == 0 && ypos < 0)
				angle = -3.1416/2;
			else if (xpos == 0 && ypos == 0)
				angle = 0;
			else if (xpos > 0)
				angle = atan(ypos/xpos);
			else
				angle= 3.1416 + atan(ypos/xpos);		
			
			norm_radius_2  = pow(norm_radius, 2.0);
			norm_radius_3  = pow(norm_radius, 3.0);
			norm_radius_4  = pow(norm_radius, 4.0);
			norm_radius_5  = pow(norm_radius, 5.0);
			norm_radius_6  = pow(norm_radius, 6.0);
			norm_radius_7  = pow(norm_radius, 7.0);
			norm_radius_8  = pow(norm_radius, 8.0);
			norm_radius_9  = pow(norm_radius, 9.0);
			norm_radius_10 = pow(norm_radius, 10.0);
			
			sin_angle   = sin(angle);      cos_angle   = cos(angle);
			sin_2angle  = sin(2 * angle);  cos_2angle  = cos(2 * angle);
			sin_3angle  = sin(3 * angle);  cos_3angle  = cos(3 * angle);
			sin_4angle  = sin(4 * angle);  cos_4angle  = cos(4 * angle);
			sin_5angle  = sin(5 * angle);  cos_5angle  = cos(5 * angle);
			sin_6angle  = sin(6 * angle);  cos_6angle  = cos(6 * angle);
			sin_7angle  = sin(7 * angle);  cos_7angle  = cos(7 * angle);
			sin_8angle  = sin(8 * angle);  cos_8angle  = cos(8 * angle);
			sin_9angle  = sin(9 * angle);  cos_9angle  = cos(9 * angle);
			sin_10angle = sin(10* angle);  cos_10angle = cos(10* angle);
			
			m_nWData[ny][0*(MAX_TERM_NUMBER+1)+nx] = 0.0;
			m_nWData[ny][1*(MAX_TERM_NUMBER+1)+nx] = 2       * norm_radius * sin_angle;
			m_nWData[ny][2*(MAX_TERM_NUMBER+1)+nx]= 2 * norm_radius * cos_angle;			

			m_nWData[ny][3*(MAX_TERM_NUMBER+1)+nx]= sqrt_6  * norm_radius_2 * sin_2angle;
			m_nWData[ny][4*(MAX_TERM_NUMBER+1)+nx]= sqrt_3  * (2   * norm_radius_2 - 1);			
			m_nWData[ny][5*(MAX_TERM_NUMBER+1)+nx]= sqrt_6  * norm_radius_2 * cos_2angle;			

			m_nWData[ny][6*(MAX_TERM_NUMBER+1)+nx]= sqrt_8  * norm_radius_3 * sin_3angle;			
			m_nWData[ny][7*(MAX_TERM_NUMBER+1)+nx]= sqrt_8  * (3   * norm_radius_3 - 2 * norm_radius) * sin_angle;
			m_nWData[ny][8*(MAX_TERM_NUMBER+1)+nx]= sqrt_8  * (3   * norm_radius_3 - 2 * norm_radius) * cos_angle;
			m_nWData[ny][9*(MAX_TERM_NUMBER+1)+nx]= sqrt_8  * norm_radius_3 * cos_3angle;
			
			m_nWData[ny][10*(MAX_TERM_NUMBER+1)+nx]= sqrt_10 * norm_radius_4 * sin_4angle;
			m_nWData[ny][11*(MAX_TERM_NUMBER+1)+nx]= sqrt_10 * (4   * norm_radius_4 - 3 * norm_radius_2) * sin_2angle;
			m_nWData[ny][12*(MAX_TERM_NUMBER+1)+nx]= sqrt_5  * (6   * norm_radius_4 - 6 * norm_radius_2 + 1);
			m_nWData[ny][13*(MAX_TERM_NUMBER+1)+nx]= sqrt_10 * (4   * norm_radius_4 - 3 * norm_radius_2) * cos_2angle;
			m_nWData[ny][14*(MAX_TERM_NUMBER+1)+nx]= sqrt_10 * norm_radius_4 * cos_4angle;
			
			m_nWData[ny][15*(MAX_TERM_NUMBER+1)+nx]= sqrt_12 * norm_radius_5 * sin_5angle;
			m_nWData[ny][16*(MAX_TERM_NUMBER+1)+nx]= sqrt_12 * (5   * norm_radius_5 - 4 * norm_radius_3) * sin_3angle;
			m_nWData[ny][17*(MAX_TERM_NUMBER+1)+nx]= sqrt_12 * (10  * norm_radius_5 - 12 * norm_radius_3 + 3 * norm_radius) * sin_angle;
			m_nWData[ny][18*(MAX_TERM_NUMBER+1)+nx]= sqrt_12 * (10  * norm_radius_5 - 12 * norm_radius_3 + 3 * norm_radius) * cos_angle;
			m_nWData[ny][19*(MAX_TERM_NUMBER+1)+nx]= sqrt_12 * (5   * norm_radius_5 - 4 * norm_radius_3) * cos_3angle;
			m_nWData[ny][20*(MAX_TERM_NUMBER+1)+nx]= sqrt_12 * norm_radius_5 * cos_5angle;
			
			m_nWData[ny][21*(MAX_TERM_NUMBER+1)+nx]= sqrt_14 * norm_radius_6 * sin_6angle;
			m_nWData[ny][22*(MAX_TERM_NUMBER+1)+nx]= sqrt_14 * (6   * norm_radius_6 - 5 * norm_radius_4) * sin_4angle;
			m_nWData[ny][23*(MAX_TERM_NUMBER+1)+nx]= sqrt_14 * (15  * norm_radius_6 - 20 * norm_radius_4 + 6 * norm_radius_2) * sin_2angle;
			m_nWData[ny][24*(MAX_TERM_NUMBER+1)+nx]= sqrt_7  * (20  * norm_radius_6 - 30 * norm_radius_4 + 12 * norm_radius_2 - 1);
			m_nWData[ny][25*(MAX_TERM_NUMBER+1)+nx]= sqrt_14 * (15  * norm_radius_6 - 20 * norm_radius_4 + 6 * norm_radius_2) * cos_2angle;
			m_nWData[ny][26*(MAX_TERM_NUMBER+1)+nx]= sqrt_14 * (6   * norm_radius_6 - 5 * norm_radius_4) * cos_4angle;
			m_nWData[ny][27*(MAX_TERM_NUMBER+1)+nx]= sqrt_14 * norm_radius_6 * cos_6angle;
			
			m_nWData[ny][28*(MAX_TERM_NUMBER+1)+nx]= 4       * norm_radius_7 * sin_7angle;
			m_nWData[ny][29*(MAX_TERM_NUMBER+1)+nx]= 4       * (7   * norm_radius_7 - 6 * norm_radius_5) * sin_5angle;
			m_nWData[ny][30*(MAX_TERM_NUMBER+1)+nx]= 4       * (21  * norm_radius_7 - 30 * norm_radius_5 + 10 * norm_radius_3) * sin_3angle;
			m_nWData[ny][31*(MAX_TERM_NUMBER+1)+nx]= 4       * (35  * norm_radius_7 - 60 * norm_radius_5 + 30 * norm_radius_3 - 4 * norm_radius) * sin_angle;
			m_nWData[ny][32*(MAX_TERM_NUMBER+1)+nx]= 4       * (35  * norm_radius_7 - 60 * norm_radius_5 + 30 * norm_radius_3 - 4 * norm_radius) * cos_angle;
			m_nWData[ny][33*(MAX_TERM_NUMBER+1)+nx]= 4       * (21  * norm_radius_7 - 30 * norm_radius_5 + 10 * norm_radius_3) * cos_3angle;
			m_nWData[ny][34*(MAX_TERM_NUMBER+1)+nx]= 4       * (7   * norm_radius_7 - 6 * norm_radius_5) * cos_5angle;
			m_nWData[ny][35*(MAX_TERM_NUMBER+1)+nx]= 4       * norm_radius_7 * cos_7angle;
			
			m_nWData[ny][36*(MAX_TERM_NUMBER+1)+nx]= sqrt_18 * norm_radius_8 * sin_8angle;
			m_nWData[ny][37*(MAX_TERM_NUMBER+1)+nx]= sqrt_18 * (8   * norm_radius_8 - 7 * norm_radius_6) * sin_6angle;
			m_nWData[ny][38*(MAX_TERM_NUMBER+1)+nx]= sqrt_18 * (28  * norm_radius_8 - 42 * norm_radius_6 + 15 * norm_radius_4) * sin_4angle;
			m_nWData[ny][39*(MAX_TERM_NUMBER+1)+nx]= sqrt_18 * (56  * norm_radius_8 - 105 * norm_radius_6 + 60 * norm_radius_4 - 10 * norm_radius_2) * sin_2angle;
			m_nWData[ny][40*(MAX_TERM_NUMBER+1)+nx]= 3       * (70  * norm_radius_8 - 140 * norm_radius_6 + 90 * norm_radius_4 - 20 * norm_radius_2 + 1);
			m_nWData[ny][41*(MAX_TERM_NUMBER+1)+nx]= sqrt_18 * (56  * norm_radius_8 - 105 * norm_radius_6 + 60 * norm_radius_4 - 10 * norm_radius_2) * cos_2angle;
			m_nWData[ny][42*(MAX_TERM_NUMBER+1)+nx]= sqrt_18 * (28  * norm_radius_8 - 42 * norm_radius_6 + 15 * norm_radius_4) * cos_4angle;
			m_nWData[ny][43*(MAX_TERM_NUMBER+1)+nx]= sqrt_18 * (8   * norm_radius_8 - 7 * norm_radius_6) * cos_6angle;
			m_nWData[ny][44*(MAX_TERM_NUMBER+1)+nx]= sqrt_18 * norm_radius_8 * cos_8angle;
			
			m_nWData[ny][45*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * norm_radius_9 * sin_9angle;
			m_nWData[ny][46*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * (9   * norm_radius_9 - 8 * norm_radius_7) * sin_7angle;
			m_nWData[ny][47*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * (36  * norm_radius_9 - 56 * norm_radius_7 + 21 * norm_radius_5) * sin_5angle;
			m_nWData[ny][48*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * (84  * norm_radius_9 - 168 * norm_radius_7 + 105 * norm_radius_5 - 20 * norm_radius_3) * sin_3angle;
			m_nWData[ny][49*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * (126 * norm_radius_9 - 280 * norm_radius_7+ 210 * norm_radius_5 - 60 * norm_radius_3 + 5 * norm_radius) * sin_angle;
			m_nWData[ny][50*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * (126 * norm_radius_9 - 280 * norm_radius_7+ 210 * norm_radius_5 - 60 * norm_radius_3 + 5 * norm_radius) * cos_angle;
			m_nWData[ny][51*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * (84  * norm_radius_9 - 168 * norm_radius_7 + 105 * norm_radius_5 - 20 * norm_radius_3) * cos_3angle;
			m_nWData[ny][52*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * (36  * norm_radius_9 - 56 * norm_radius_7 + 21 * norm_radius_5) * cos_5angle;
			m_nWData[ny][53*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * (9   * norm_radius_9 - 8 * norm_radius_7) * cos_7angle;
			m_nWData[ny][54*(MAX_TERM_NUMBER+1)+nx]= sqrt_20 * norm_radius_9 * cos_9angle;
			
			m_nWData[ny][55*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * norm_radius_10 * sin_10angle;
			m_nWData[ny][56*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * (10  * norm_radius_10 - 9 * norm_radius_8) * sin_8angle;
			m_nWData[ny][57*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * (45  * norm_radius_10 - 72 * norm_radius_8 + 28 * norm_radius_6) * sin_6angle;
			m_nWData[ny][58*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * (120 * norm_radius_10 - 252 * norm_radius_8 + 168 * norm_radius_6 - 35 * norm_radius_4) * sin_4angle;
			m_nWData[ny][59*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * (210 * norm_radius_10 - 504 * norm_radius_8 + 420 * norm_radius_6 - 140 * norm_radius_4 + 15 * norm_radius_2) * sin_2angle;
			m_nWData[ny][60*(MAX_TERM_NUMBER+1)+nx]= sqrt_11 * (252 * norm_radius_10 - 630 * norm_radius_8 + 560 * norm_radius_6 - 210 * norm_radius_4 + 30 * norm_radius_2 - 1);
			m_nWData[ny][61*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * (210 * norm_radius_10 - 504 * norm_radius_8 + 420 * norm_radius_6 - 140 * norm_radius_4 + 15 * norm_radius_2) * cos_2angle;
			m_nWData[ny][62*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * (120 * norm_radius_10 - 252 * norm_radius_8 + 168 * norm_radius_6 - 35 * norm_radius_4) * cos_4angle;
			m_nWData[ny][63*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * (45  * norm_radius_10 - 72 * norm_radius_8 + 28 * norm_radius_6) * cos_6angle;
			m_nWData[ny][64*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * (10  * norm_radius_10 - 9 * norm_radius_8) * cos_8angle;
			m_nWData[ny][65*(MAX_TERM_NUMBER+1)+nx]= sqrt_22 * norm_radius_10 * cos_10angle;		
		}
	}
}

short COptCalc::InitWaveMatrix(bool *src)
{
	int nx,ny,N, count=0;
	//N = (int)d_WAVE->row;  //original code
	N=g_AOSACAParams->LENSLETGRID;  2015
	double xpos, ypos, norm_radius;

//////////////////////////////////////////////////////////////////
	/*
//Zernike wavefront  original code
	for (ny=0; ny<N;ny++)
	{		
		ypos = ny * ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / (N)) - ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / 2);
		for (nx = 0; nx < N; nx++) 
			{
				xpos = nx * ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / (N)) - ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / 2);
				
				norm_radius = sqrt(xpos * xpos + ypos * ypos) / (m_pupil_size_zernike/2);
				
				if (norm_radius > ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / m_pupil_size_zernike))
					src[nx * N + ny] = false;
				else 
				{
					src[nx * N + ny] = true;
					++count;
				}
			}
	}

*/

	//for Southwell, 4x4 pixels represent one point wavefront (one lenslet)
	for (ny=0; ny<N;ny++)
	{		
		ypos = ny - int(N-1)/2;
		for (nx = 0; nx < N; nx++) 
			{
				xpos = nx - int(N-1)/2;
				
				norm_radius = sqrt(xpos * xpos + ypos * ypos)*g_AOSACAParams->LENSLET_DIAMETER_MICRONS;
				
				if (norm_radius > ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS)*g_AOSACAParams->MAGNIFICATION / 2))
				{
					src[nx*4 * N*4 + ny*4] = false;
					src[nx*4 * N*4 + ny*4+1] = false;
					src[nx*4 * N*4 + ny*4+2] = false;
					src[nx*4 * N*4 + ny*4+3] = false;
					src[(nx*4+1) * N*4 + ny*4] = false;
					src[(nx*4+1) * N*4 + ny*4+1] = false;
					src[(nx*4+1) * N*4 + ny*4+2] = false;
					src[(nx*4+1) * N*4 + ny*4+3] = false;
					src[(nx*4+2) * N*4 + ny*4] = false;
					src[(nx*4+2) * N*4 + ny*4+1] = false;
					src[(nx*4+2) * N*4 + ny*4+2] = false;
					src[(nx*4+2) * N*4 + ny*4+3] = false;
					src[(nx*4+3) * N*4 + ny*4] = false;
					src[(nx*4+3) * N*4 + ny*4+1] = false;
					src[(nx*4+3) * N*4 + ny*4+2] = false;
					src[(nx*4+3) * N*4 + ny*4+3] = false;
				}	
				else 
				{
					src[nx*4 * N*4 + ny*4] = true;
					src[nx*4 * N*4 + ny*4+1] = true;
					src[nx*4 * N*4 + ny*4+2] = true;
					src[nx*4 * N*4 + ny*4+3] = true;
					src[(nx*4+1) * N*4 + ny*4] = true;
					src[(nx*4+1) * N*4 + ny*4+1] = true;
					src[(nx*4+1) * N*4 + ny*4+2] = true;
					src[(nx*4+1) * N*4 + ny*4+3] = true;
					src[(nx*4+2) * N*4 + ny*4] = true;
					src[(nx*4+2) * N*4 + ny*4+1] = true;
					src[(nx*4+2) * N*4 + ny*4+2] = true;
					src[(nx*4+2) * N*4 + ny*4+3] = true;
					src[(nx*4+3) * N*4 + ny*4] = true;
					src[(nx*4+3) * N*4 + ny*4+1] = true;
					src[(nx*4+3) * N*4 + ny*4+2] = true;
					src[(nx*4+3) * N*4 + ny*4+3] = true;
					count=count+16;
				}
			}
	}

	return count;
}

void COptCalc::Zwave_Mahajan_new(double *src)
{
	double *temp_col = new double[d_WAVE->row];
	memset(temp_col, 0, d_WAVE->row*sizeof(double));	
	int nx,ny,N, i=-1;
	N = (int)d_WAVE->row; 
	double xpos, ypos, norm_radius;
	double *waveabermap=d_WAVE->data;
	for (ny=0; ny<N;ny++)
	{
		MD_mulM(&temp_col, &Zcoeff, &m_nWData[ny], 1, MAX_TERM_NUMBER+1, N);		
		ypos = ny * ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / N) - ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / 2);
		for (nx = 0; nx < N; nx++) 
			{
				xpos = nx * ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / N) - ((g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / 2);
				
				norm_radius = sqrt(xpos * xpos + ypos * ypos) / (m_pupil_size_zernike/2);
				
				if (norm_radius > (g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / m_pupil_size_zernike)
					waveabermap[nx * N + ny] = NAP;
				else 
					src[++i] = waveabermap[nx * N + ny] = temp_col[nx];
			}
	}

	delete [] temp_col, temp_col=NULL;
}

//*************************************************************************************************
/*
 * Zphase_Mahajan:
 *
 * WARNING: The Zernike polynomials are normalized and ordered after Mahajan.
 *          Applied Optics 33: 8121-8124 (1994)
 *          Be careful that your coefficients match the list of polynomials that you use.
 *          The terms are ordered so that the sine expressions are always odd.
 *
 *
 * This function actually involves complex computation.
 * The return info is in phasemapR and phasemapI. Each one corresponds to a N x N matrix.
 */
void COptCalc::Zphase_Mahajan(double *phasemapR, double *phasemapI, unsigned long N)
{
	unsigned long nx, ny;
	double xpos, ypos, angle, norm_radius, phase;
	double sizeoffield = m_pupil_field_size_psf;//mm

	double norm_radius_2, norm_radius_3, norm_radius_4, norm_radius_5, norm_radius_6, norm_radius_7, norm_radius_8, norm_radius_9, norm_radius_10;
	double sin_angle, sin_2angle, sin_3angle, sin_4angle, sin_5angle, sin_6angle, sin_7angle, sin_8angle, sin_9angle, sin_10angle;
	double cos_angle, cos_2angle, cos_3angle, cos_4angle, cos_5angle, cos_6angle, cos_7angle, cos_8angle, cos_9angle, cos_10angle;
	double sqrt_3, sqrt_6, sqrt_5, sqrt_7, sqrt_8, sqrt_10, sqrt_11, sqrt_12, sqrt_14, sqrt_18, sqrt_20, sqrt_22;
	m_psf_size_pix = 0;

	for (ny = 0; ny < N; ny++) {
		for (nx = 0; nx < N; nx++) {
			xpos = (nx-1) * (sizeoffield / N) - (sizeoffield / 2);
			ypos = (ny-1) * (sizeoffield / N) - (sizeoffield / 2);
			
			norm_radius = sqrt(xpos * xpos + ypos * ypos) / (m_pupil_size_zernike/2);
			
			// clock-type coordinate system to a standard coordinate system (OSA vision standard) 
			if (xpos == 0 && ypos > 0)
				angle = 3.1416/2;
			else if (xpos == 0 && ypos < 0)
				angle = -3.1416/2;
			else if (xpos == 0 && ypos == 0)
				angle = 0;
			else if (xpos > 0)
				angle = atan(ypos/xpos);
			else
				angle= 3.1416 + atan(ypos/xpos);	
			
			if (norm_radius > (g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS) / m_pupil_size_zernike) {
				phasemapR[nx * N + ny] = 0;
				phasemapI[nx * N + ny] = 0;
			}
			else 
			{
				norm_radius_2  = pow(norm_radius, 2.0);
				norm_radius_3  = pow(norm_radius, 3.0);
				norm_radius_4  = pow(norm_radius, 4.0);
				norm_radius_5  = pow(norm_radius, 5.0);
				norm_radius_6  = pow(norm_radius, 6.0);
				norm_radius_7  = pow(norm_radius, 7.0);
				norm_radius_8  = pow(norm_radius, 8.0);
				norm_radius_9  = pow(norm_radius, 9.0);
				norm_radius_10 = pow(norm_radius, 10.0);

				sin_angle   = sin(angle);      cos_angle   = cos(angle);
				sin_2angle  = sin(2 * angle);  cos_2angle  = cos(2 * angle);
				sin_3angle  = sin(3 * angle);  cos_3angle  = cos(3 * angle);
				sin_4angle  = sin(4 * angle);  cos_4angle  = cos(4 * angle);
				sin_5angle  = sin(5 * angle);  cos_5angle  = cos(5 * angle);
				sin_6angle  = sin(6 * angle);  cos_6angle  = cos(6 * angle);
				sin_7angle  = sin(7 * angle);  cos_7angle  = cos(7 * angle);
				sin_8angle  = sin(8 * angle);  cos_8angle  = cos(8 * angle);
				sin_9angle  = sin(9 * angle);  cos_9angle  = cos(9 * angle);
				sin_10angle = sin(10* angle);  cos_10angle = cos(10* angle);

				sqrt_3  = sqrt(3.);		sqrt_6  = sqrt(6.);		sqrt_5  = sqrt(5.);
				sqrt_7  = sqrt(7.);		sqrt_8  = sqrt(8.);		sqrt_10 = sqrt(10.);
				sqrt_11 = sqrt(11.);		sqrt_12 = sqrt(12.);		sqrt_14 = sqrt(14.);
				sqrt_18 = sqrt(18.);		sqrt_20 = sqrt(20.);		sqrt_22 = sqrt(22.);


				phase  = 0.0;
				phase += c[1]  * 2       * norm_radius * sin_angle;
				phase += c[2]  * 2       * norm_radius * cos_angle;

				phase += c[3]  * sqrt_6  * norm_radius_2 * sin_2angle;
				phase += c[4]  * sqrt_3  * (2   * norm_radius_2 - 1);
				phase += c[5]  * sqrt_6  * norm_radius_2 * cos_2angle;

				phase += c[6]  * sqrt_8  * norm_radius_3 * sin_3angle;
				phase += c[7]  * sqrt_8  * (3   * norm_radius_3 - 2 * norm_radius) * sin_angle;
				phase += c[8]  * sqrt_8  * (3   * norm_radius_3 - 2 * norm_radius) * cos_angle;
				phase += c[9]  * sqrt_8  * norm_radius_3 * cos_3angle;

				phase += c[10] * sqrt_10 * norm_radius_4 * sin_4angle;
				phase += c[11] * sqrt_10 * (4   * norm_radius_4 - 3 * norm_radius_2) * sin_2angle;
				phase += c[12] * sqrt_5  * (6   * norm_radius_4 - 6 * norm_radius_2 + 1);
				phase += c[13] * sqrt_10 * (4   * norm_radius_4 - 3 * norm_radius_2) * cos_2angle;
				phase += c[14] * sqrt_10 * norm_radius_4 * cos_4angle;

				phase += c[15] * sqrt_12 * norm_radius_5 * sin_5angle;
				phase += c[16] * sqrt_12 * (5   * norm_radius_5 - 4 * norm_radius_3) * sin_3angle;
				phase += c[17] * sqrt_12 * (10  * norm_radius_5 - 12 * norm_radius_3 + 3 * norm_radius) * sin_angle;
				phase += c[18] * sqrt_12 * (10  * norm_radius_5 - 12 * norm_radius_3 + 3 * norm_radius) * cos_angle;
				phase += c[19] * sqrt_12 * (5   * norm_radius_5 - 4 * norm_radius_3) * cos_3angle;
				phase += c[20] * sqrt_12 * norm_radius_5 * cos_5angle;

				phase += c[21] * sqrt_14 * norm_radius_6 * sin_6angle;
				phase += c[22] * sqrt_14 * (6   * norm_radius_6 - 5 * norm_radius_4) * sin_4angle;
				phase += c[23] * sqrt_14 * (15  * norm_radius_6 - 20 * norm_radius_4 + 6 * norm_radius_2) * sin_2angle;
				phase += c[24] * sqrt_7  * (20  * norm_radius_6 - 30 * norm_radius_4 + 12 * norm_radius_2 - 1);
				phase += c[25] * sqrt_14 * (15  * norm_radius_6 - 20 * norm_radius_4 + 6 * norm_radius_2) * cos_2angle;
				phase += c[26] * sqrt_14 * (6   * norm_radius_6 - 5 * norm_radius_4) * cos_4angle;
				phase += c[27] * sqrt_14 * norm_radius_6 * cos_6angle;

				phase += c[28] * 4       * norm_radius_7 * sin_7angle;
				phase += c[29] * 4       * (7   * norm_radius_7 - 6 * norm_radius_5) * sin_5angle;
				phase += c[30] * 4       * (21  * norm_radius_7 - 30 * norm_radius_5 + 10 * norm_radius_3) * sin_3angle;
				phase += c[31] * 4       * (35  * norm_radius_7 - 60 * norm_radius_5 + 30 * norm_radius_3 - 4 * norm_radius) * sin_angle;
				phase += c[32] * 4       * (35  * norm_radius_7 - 60 * norm_radius_5 + 30 * norm_radius_3 - 4 * norm_radius) * cos_angle;
				phase += c[33] * 4       * (21  * norm_radius_7 - 30 * norm_radius_5 + 10 * norm_radius_3) * cos_3angle;
				phase += c[34] * 4       * (7   * norm_radius_7 - 6 * norm_radius_5) * cos_5angle;
				phase += c[35] * 4       * norm_radius_7 * cos_7angle;

				phase += c[36] * sqrt_18 * norm_radius_8 * sin_8angle;
				phase += c[37] * sqrt_18 * (8   * norm_radius_8 - 7 * norm_radius_6) * sin_6angle;
				phase += c[38] * sqrt_18 * (28  * norm_radius_8 - 42 * norm_radius_6 + 15 * norm_radius_4) * sin_4angle;
				phase += c[39] * sqrt_18 * (56  * norm_radius_8 - 105 * norm_radius_6 + 60 * norm_radius_4 - 10 * norm_radius_2) * sin_2angle;
				phase += c[40] * 3       * (70  * norm_radius_8 - 140 * norm_radius_6 + 90 * norm_radius_4 - 20 * norm_radius_2 + 1);
				phase += c[41] * sqrt_18 * (56  * norm_radius_8 - 105 * norm_radius_6 + 60 * norm_radius_4 - 10 * norm_radius_2) * cos_2angle;
				phase += c[42] * sqrt_18 * (28  * norm_radius_8 - 42 * norm_radius_6 + 15 * norm_radius_4) * cos_4angle;
				phase += c[43] * sqrt_18 * (8   * norm_radius_8 - 7 * norm_radius_6) * cos_6angle;
				phase += c[44] * sqrt_18 * norm_radius_8 * cos_8angle;

				phase += c[45] * sqrt_20 * norm_radius_9 * sin_9angle;
				phase += c[46] * sqrt_20 * (9   * norm_radius_9 - 8 * norm_radius_7) * sin_7angle;
				phase += c[47] * sqrt_20 * (36  * norm_radius_9 - 56 * norm_radius_7 + 21 * norm_radius_5) * sin_5angle;
				phase += c[48] * sqrt_20 * (84  * norm_radius_9 - 168 * norm_radius_7 + 105 * norm_radius_5 - 20 * norm_radius_3) * sin_3angle;
				phase += c[49] * sqrt_20 * (126 * norm_radius_9 - 280 * norm_radius_7+ 210 * norm_radius_5 - 60 * norm_radius_3 + 5 * norm_radius) * sin_angle;
				phase += c[50] * sqrt_20 * (126 * norm_radius_9 - 280 * norm_radius_7+ 210 * norm_radius_5 - 60 * norm_radius_3 + 5 * norm_radius) * cos_angle;
				phase += c[51] * sqrt_20 * (84  * norm_radius_9 - 168 * norm_radius_7 + 105 * norm_radius_5 - 20 * norm_radius_3) * cos_3angle;
				phase += c[52] * sqrt_20 * (36  * norm_radius_9 - 56 * norm_radius_7 + 21 * norm_radius_5) * cos_5angle;
				phase += c[53] * sqrt_20 * (9   * norm_radius_9 - 8 * norm_radius_7) * cos_7angle;
				phase += c[54] * sqrt_20 * norm_radius_9 * cos_9angle;

				phase += c[55] * sqrt_22 * norm_radius_10 * sin_10angle;
				phase += c[56] * sqrt_22 * (10  * norm_radius_10 - 9 * norm_radius_8) * sin_8angle;
				phase += c[57] * sqrt_22 * (45  * norm_radius_10 - 72 * norm_radius_8 + 28 * norm_radius_6) * sin_6angle;
				phase += c[58] * sqrt_22 * (120 * norm_radius_10 - 252 * norm_radius_8 + 168 * norm_radius_6 - 35 * norm_radius_4) * sin_4angle;
				phase += c[59] * sqrt_22 * (210 * norm_radius_10 - 504 * norm_radius_8 + 420 * norm_radius_6 - 140 * norm_radius_4 + 15 * norm_radius_2) * sin_2angle;
				phase += c[60] * sqrt_11 * (252 * norm_radius_10 - 630 * norm_radius_8 + 560 * norm_radius_6 - 210 * norm_radius_4 + 30 * norm_radius_2 - 1);
				phase += c[61] * sqrt_22 * (210 * norm_radius_10 - 504 * norm_radius_8 + 420 * norm_radius_6 - 140 * norm_radius_4 + 15 * norm_radius_2) * cos_2angle;
				phase += c[62] * sqrt_22 * (120 * norm_radius_10 - 252 * norm_radius_8 + 168 * norm_radius_6 - 35 * norm_radius_4) * cos_4angle;
				phase += c[63] * sqrt_22 * (45  * norm_radius_10 - 72 * norm_radius_8 + 28 * norm_radius_6) * cos_6angle;
				phase += c[64] * sqrt_22 * (10  * norm_radius_10 - 9 * norm_radius_8) * cos_8angle;
				phase += c[65] * sqrt_22 * norm_radius_10 * cos_10angle;

				
				phasemapR[nx * N + ny] = cos(- 2 * PI * phase / (g_AOSACAParams->WAVELENGTH_MICRONS));
				phasemapI[nx * N + ny] = sin(- 2 * PI * phase / (g_AOSACAParams->WAVELENGTH_MICRONS));
				m_psf_size_pix += 1;
			}
		}
	}
}
//*************************************************************************************************

double COptCalc::Compute_RMS(double induced_defocus)
{
	double rms;
	int loopvar;
		// Calculate RMS...
    rms = 0.0;
	for (loopvar = 0; loopvar <= MAX_TERM_NUMBER ; loopvar++) 
	{
		if(loopvar == 4)	// for defocus
			rms += (c[loopvar] - induced_defocus) * (c[loopvar] - induced_defocus);
		else
			rms += c[loopvar] * c[loopvar];
	}
    m_rms = sqrt(rms);

	return m_rms;
}

//*************************************************************************************************

CStringA COptCalc::Compute_RMSZernikesVoltages(double induced_defocus,double pup_fit_size)
{
	int loopvar;
	double pupil;
	CStringA cstrTemp;
	CStringA cstrLog;

	induced_defocus = Diopters_to_Microns(induced_defocus,pup_fit_size);
	Compute_RMS(induced_defocus);

	pupil= g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS;
    cstrLog.Format("%4.2f\n%4.4f\n%1.3f\n",pupil,m_rms,m_defocus);
	// Writing zernikes to log buffer
	for(loopvar=0;loopvar <= MAX_TERM_NUMBER ;loopvar++)
	{  
		cstrTemp.Format("%4.7f\n",c[loopvar]);
		cstrLog=cstrLog+cstrTemp;
	}
	// Writing voltages to log buffer
		cstrLog += retn_Voltages();
	return cstrLog;
}

//*************************************************************************************************

double COptCalc::retn_RMS()
{
	return m_rms;
}

//*************************************************************************************************

double COptCalc::retn_Strehl()
{
	return m_strehl_ratio;
}

void COptCalc::get_ZCoeffs(double* outdata, short num_of_coeffs)
{
	memcpy(outdata, &c[3], num_of_coeffs*sizeof(double));
}

void COptCalc::get_ZCoeffs_all(double* outdata, short num_of_coeffs)
{
	memcpy(outdata, &c[1], num_of_coeffs*sizeof(double));
}

//*************************************************************************************************

CStringA COptCalc::FormatOpticalResult(double pupil_fit_size, double induced_defocus)
{
	CStringA cstrResult;
	CStringA cstrTemp;
	CStringA newline;

	newline.Format("%c%c\0",13,10);
	
	// Output Pupil Size
	cstrTemp.Format("Pupil Size:%4.4f%s",pupil_fit_size,newline);
	cstrResult=cstrResult+cstrTemp;
	
	// Output Best Spectacle Correction
	cstrTemp.Format("%sRefractive Error (Entrance Pupil):%s",newline,newline);
	cstrResult=cstrResult+cstrTemp;

	cstrTemp.Format("Defocus:%4.4f%s", (-1) * m_defocus,newline);
	cstrResult=cstrResult+cstrTemp;
	
	cstrTemp.Format("Cylinder:%4.4f%s", (-1) * m_cylinder,newline);
	cstrResult=cstrResult+cstrTemp;

	cstrTemp.Format("Axis:%4.4f%s", m_axis,newline);
	cstrResult=cstrResult+cstrTemp;
	
	// Calculate best spectacle correction in the cornea plane (assuming measurement is at entrance pupil)
	cstrTemp.Format("%sRefractive Error (Corneal Plane):%s",newline,newline);
	cstrResult=cstrResult+cstrTemp;
	
	cstrTemp.Format("Defocus:%4.4f%s", 
		AlternatePlane(COptCalc::CORNEALPLANE,m_defocus),newline);
	cstrResult=cstrResult+cstrTemp;

	cstrTemp.Format("Cylinder:%4.4f%s",
		AlternatePlane(COptCalc::SPECTACLEPLANE,m_cylinder),newline);
	cstrResult=cstrResult+cstrTemp;

	cstrTemp.Format("Axis:%4.4f%s", m_axis,newline);
	cstrResult=cstrResult+cstrTemp;

	// Calculate best spectacle correction in the spectacle plane (assuming measurement is at entrance pupil)
	cstrTemp.Format("%sRefractive Error (Spectacle Plane 14 mm):%s",newline,newline);
	cstrResult=cstrResult+cstrTemp;

	cstrTemp.Format("Defocus:%4.4f%s", 
		(-1) * m_defocus/(1-0.014*m_defocus),newline);
	cstrResult=cstrResult+cstrTemp;

	cstrTemp.Format("Cylinder:%4.4f%s", 
		(-1) * m_cylinder/(1-0.014*m_cylinder),newline);
	cstrResult=cstrResult+cstrTemp;

	cstrTemp.Format("Axis:%4.4f%s%s", m_axis,newline,newline);
	cstrResult=cstrResult+cstrTemp;

	// Output Wavefront RMS (microns)
	cstrTemp.Format("%sWavefront RMS (for terms listed below):%4.4f%s", 
		newline,m_rms,newline);
	cstrResult=cstrResult+cstrTemp;

	// Output Strehl Ratio
	cstrTemp.Format("%sStrehl Ratio (for terms listed below):%4.4f%s",newline, 
		m_strehl_ratio,newline);
	cstrResult=cstrResult+cstrTemp;
	
	// Output Zernike Coefficients
	cstrResult=cstrResult+newline+"Zernike Coefficients:"+newline+
		"(in VSIA vision standard format)"+newline+newline;
	
	cstrResult=cstrResult + newline + "Tilt terms" + newline;;
	cstrResult=cstrResult + FormatNthOrder(1,1);

	cstrTemp = newline+"Defocus and Astigmatism"+newline;
	cstrResult=cstrResult + FormatNthOrder(3,2);

	cstrTemp=newline + "Coma and Trefoil"+newline;
	cstrResult=cstrResult + FormatNthOrder(6,3);

	cstrTemp = newline + "Spherical Aberration and 4th order" + newline;
	cstrResult=cstrResult + FormatNthOrder(10,4);

	cstrResult=cstrResult + newline + "5th order" + newline;
	cstrResult=cstrResult + FormatNthOrder(15,5);

	cstrResult=cstrResult + newline + "6th order" + newline;
	cstrResult=cstrResult + FormatNthOrder(21,6);
	
	cstrResult=cstrResult + newline + "7th order" + newline;
	cstrResult=cstrResult + FormatNthOrder(28,7);

	cstrResult=cstrResult + newline + "8th order" + newline;	
	cstrResult=cstrResult + FormatNthOrder(36,8);

	cstrResult=cstrResult + newline + "9th order" + newline;
	cstrResult=cstrResult + FormatNthOrder(45,9);

	cstrResult=cstrResult + newline + "10th order" + newline;
	cstrResult=cstrResult + FormatNthOrder(55,10);

	return cstrResult;
}

CStringA COptCalc::FormatNthOrder(int start, int order)
{
	CStringA cstrTemp;
	CStringA cstrResult="";
	CStringA newline;
	newline.Format("%c%c\0",13,10);
	int end=start+order;
	int counter=0;
	for (int i=start;i<=end;i++)
	{
		cstrTemp.Format("Z[%d,%d]\tc(%d) = %4.10f;%s",
			order,(-1)*order+2*(counter), i, c[i],newline);
		cstrResult=cstrResult+cstrTemp;
		counter++;
	}
	return cstrResult;

}

//*************************************************************************************************
// Data and labels for charts
//*************************************************************************************************

COptCalcData *COptCalc::FetchPlotInfo(CPlotInfo * pPlot_Info, unsigned short data_type)
{
	double pupfitsize= g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS;
	switch(data_type) 
	{
	case 0:
		pPlot_Info->dim = d_WAVE->col;
		pPlot_Info->step = pupfitsize/ d_WAVE->col;
		pPlot_Info->orig = - pupfitsize / 2;
		pPlot_Info->title = "Wavefront Aberration";
		pPlot_Info->xlabel = "mm";
		pPlot_Info->ylabel = "mm";
		pPlot_Info->zlabel = "microns";
		return d_WAVE;
	
	case 1:
		double tmp;
		tmp = 60 * (g_AOSACAParams->PSF_MTF_WIDTH_PIX) * (180 * 60 / 3.1416) * (g_AOSACAParams->WAVELENGTH_MICRONS) * 0.001 / m_pupil_field_size_microns;
		pPlot_Info->dim = d_PSF->col;
		pPlot_Info->step = tmp / (g_AOSACAParams->PSF_MTF_WIDTH_PIX);
		pPlot_Info->orig = - tmp / 2;
		pPlot_Info->title = "Point Spread Function";
		pPlot_Info->xlabel = "arcsec";
		pPlot_Info->ylabel = "arcsec";
		pPlot_Info->zlabel = "Intensity";
		return d_PSF;
	
	case 2: 
		double cutofffull;
		cutofffull = m_pupil_field_size_microns / ((g_AOSACAParams->WAVELENGTH_MICRONS) / 1000) / 57.3;
		pPlot_Info->dim = d_MTF->col;
		pPlot_Info->step = 2 * cutofffull / d_MTF->col;
		pPlot_Info->orig = - cutofffull;
		pPlot_Info->title = "Modulation Transfer Function";
		pPlot_Info->xlabel = "c/deg";
		pPlot_Info->ylabel = "c/deg";
		pPlot_Info->zlabel = "modulation";
		return d_MTF;

	default:
		pPlot_Info->title ="Error! Wrong data type";
		return NULL;
	
	} // end switch
}

void COptCalc::Compute_Openloop_Voltages(double induced_defocus)
{
	int i=0;
	short size;
	size=g_centroids->get_TotalCentCount();
	double *tempPhi;
	tempPhi = new double[size<<1];
	memset(tempPhi, 0, (size<<1)*sizeof(double)); 

	for (i=0; i<(size<<1);i++)
		tempPhi[i]=1.0 * (induced_defocus - m_ReferenceDefocus) * g_centroids->m_dZM[i*(MAX_TERM_NUMBER+1)+4];

	VD_mulC(tempPhi, tempPhi, (size<<1), (2./(g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS)));

	MD_mulV(m_CurVoltages , &(g_centroids->m_dReconM_fullpupil), tempPhi, (g_AOSACAParams->NUMACTS), size<<1 );

	VD_addV(m_CurVoltages, m_PrevVoltages1, m_CurVoltages, (g_AOSACAParams->NUMACTS));	
	VD_limit(m_CurVoltages, m_CurVoltages, (g_AOSACAParams->NUMACTS), -0.8, 0.8);
	memcpy(g_AOSACAParams->g_DMDeflections, m_CurVoltages, (g_AOSACAParams->NUMACTS)*sizeof(double));
}

void COptCalc::UpdateDMFlatData()
{
	if (g_AOSACAParams->g_bCPreCorrApply)
		VD_addV(m_dBiasBit, g_AOSACAParams->g_DMBiasDeflections, m_PreCorrectionVoltages, g_AOSACAParams->NUMACTS);
	else
		memcpy(m_dBiasBit, g_AOSACAParams->g_DMBiasDeflections, g_AOSACAParams->NUMACTS*sizeof(double));

	memcpy(m_CurVoltages, m_dBiasBit, g_AOSACAParams->NUMACTS*sizeof(double));
	memcpy(g_AOSACAParams->g_DMDeflections, m_CurVoltages, (g_AOSACAParams->NUMACTS)*sizeof(double));
	Send_Voltages(COMP_BIT);
}

void COptCalc::Compute_Voltages()
{
	VD_mulC(m_CurVoltages, g_centroids->m_dErrV, (g_AOSACAParams->NUMACTS), m_dIntGain);
	VD_subC(m_CurVoltages, m_CurVoltages, (g_AOSACAParams->NUMACTS), VD_mean(m_CurVoltages, (g_AOSACAParams->NUMACTS))); // Piston removal
	// volt limitation, the wavefront measured is usually wrong when the calutated voltage of single actuator is larger than 0.2.

	double min1, max1, min2, max2;	
	max1 = VD_max(m_CurVoltages, (g_AOSACAParams->NUMACTS));
	min1 = VD_min(m_CurVoltages, (g_AOSACAParams->NUMACTS));
	if (min1>-0.3 && max1<0.3) 
	{
		VD_limit(m_CurVoltages, m_CurVoltages, (g_AOSACAParams->NUMACTS), -0.3, 0.3);
		VD_addV(m_CurVoltages, m_PrevVoltages1, m_CurVoltages, (g_AOSACAParams->NUMACTS));
		VD_limit(m_CurVoltages, m_CurVoltages, (g_AOSACAParams->NUMACTS), -0.8, 0.8);
		max2 = VD_max(m_CurVoltages, (g_AOSACAParams->NUMACTS));
		min2 = VD_min(m_CurVoltages, (g_AOSACAParams->NUMACTS));
		memcpy(g_AOSACAParams->g_DMDeflections, m_CurVoltages, (g_AOSACAParams->NUMACTS)*sizeof(double));
	}
}

CStringA COptCalc::retn_Voltages()
{
	CStringA cstrBuff,cstrTemp;

	for(int i=0;i<(g_AOSACAParams->NUMACTS);i++)
	{
		if(m_AOOnFlag)	// If the  AO is on
			cstrTemp.Format("%lf\n",g_AOSACAParams->g_DMDeflections[i]);
		else			// If you have the AO option is off
			cstrTemp.Format("%d\n",0);
		if( !i )
			cstrBuff = cstrTemp;
		else				
			cstrBuff += cstrTemp;
	}
	return cstrBuff;
}

void COptCalc::compute_ZernikeTerms(BYTE *mask)
{
	int i,j;
	double x,y,in_Circle;
	double norm_radius,norm_radius_2, norm_radius_3, norm_radius_4, norm_radius_5, norm_radius_6, norm_radius_7, norm_radius_8, norm_radius_9, norm_radius_10;
	double angle,sin_angle, sin_2angle, sin_3angle, sin_4angle, sin_5angle, sin_6angle, sin_7angle, sin_8angle, sin_9angle, sin_10angle;
	double cos_angle, cos_2angle, cos_3angle, cos_4angle, cos_5angle, cos_6angle, cos_7angle, cos_8angle, cos_9angle, cos_10angle;
	double sqrt_3, sqrt_4, sqrt_6, sqrt_5, sqrt_7, sqrt_8, sqrt_10, sqrt_11, sqrt_12, sqrt_14, sqrt_18, sqrt_20, sqrt_22;

	sqrt_3  = sqrt(3.);		sqrt_4 = sqrt(4.);		sqrt_6  = sqrt(6.);		sqrt_5  = sqrt(5.);
	sqrt_7  = sqrt(7.);		sqrt_8  = sqrt(8.);		sqrt_10 = sqrt(10.);
	sqrt_11 = sqrt(11.);		sqrt_12 = sqrt(12.);		sqrt_14 = sqrt(14.);
	sqrt_18 = sqrt(18.);		sqrt_20 = sqrt(20.);		sqrt_22 = sqrt(22.);

	short l = 0;
	for(i=0;i<(g_AOSACAParams->DMGRID);i++)
	{
		for(j=0;j<(g_AOSACAParams->DMGRID);j++)
		{
			if(!mask[i*g_AOSACAParams->DMGRID + j])//  (j == 0 || j == ((g_AOSACAParams->DMGRID)-1))   &&   (i == 0 || i == ((g_AOSACAParams->DMGRID)-1))  )
				;
			else
			{
				x = 1 - (2.0*i/((g_AOSACAParams->DMGRID)-1));
				y = 1 - (2.0*j/((g_AOSACAParams->DMGRID)-1));
				angle = (x < 0)?(3.0*3.14159/2.0 + atan(y/x)):(3.14159/2.0 + atan(y/x));
				if(!x)
				{
					if(j>=5)
						angle = 0.0;		// For the circle formed, angle along the x axis is 0 in +ve direction
					else
						angle = 3.14159;	// For the circle formed, angle along the x axis is pi in the -ve direction
				}
				
				// the ratio of the sum of actuator spacings (mm) 
				y = 1.0 * cos(9.0*3.14159/180.0)*y; //the 1.1 is the magnification factor between the extreme actuator position and the edge of the normalized pupil AR 081905
				x = 1.0 *x; //changed on 08/6/2007 for new 6micron mirror to 1.24
				norm_radius    = sqrt(x*x + y*y);
				if(norm_radius <= 1.0)//the 1.15 is used to extrapolate the wavefront AR 081905
					in_Circle = 1.0;
				else
					in_Circle = 0.0;
				norm_radius_2  = pow(norm_radius, 2.0);
				norm_radius_3  = pow(norm_radius, 3.0);
				norm_radius_4  = pow(norm_radius, 4.0);
				norm_radius_5  = pow(norm_radius, 5.0);
				norm_radius_6  = pow(norm_radius, 6.0);
				norm_radius_7  = pow(norm_radius, 7.0);
				norm_radius_8  = pow(norm_radius, 8.0);
				norm_radius_9  = pow(norm_radius, 9.0);
				norm_radius_10 = pow(norm_radius, 10.0);
				
				sin_angle   = sin(angle);      cos_angle   = cos(angle);
				sin_2angle  = sin(2 * angle);  cos_2angle  = cos(2 * angle);
				sin_3angle  = sin(3 * angle);  cos_3angle  = cos(3 * angle);
				sin_4angle  = sin(4 * angle);  cos_4angle  = cos(4 * angle);
				sin_5angle  = sin(5 * angle);  cos_5angle  = cos(5 * angle);
				sin_6angle  = sin(6 * angle);  cos_6angle  = cos(6 * angle);
				sin_7angle  = sin(7 * angle);  cos_7angle  = cos(7 * angle);
				sin_8angle  = sin(8 * angle);  cos_8angle  = cos(8 * angle);
				sin_9angle  = sin(9 * angle);  cos_9angle  = cos(9 * angle);
				sin_10angle = sin(10* angle);  cos_10angle = cos(10* angle);
				
				m_zernike_terms[0][l] = 0.0;
				m_zernike_terms[1][l] =  in_Circle * sqrt_4       * norm_radius * sin_angle;
				m_zernike_terms[2][l] =  in_Circle * sqrt_4       * norm_radius * cos_angle;
				
				m_zernike_terms[3][l] = in_Circle * sqrt_6  * norm_radius_2 * sin_2angle;
				m_zernike_terms[4][l] = in_Circle * sqrt_3  * (2   * norm_radius_2 - 1);
				m_zernike_terms[5][l] = in_Circle * sqrt_6  * norm_radius_2 * cos_2angle;	
				
				m_zernike_terms[6][l] = in_Circle * sqrt_8  * norm_radius_3 * sin_3angle;
				m_zernike_terms[7][l] = in_Circle * sqrt_8  * (3   * norm_radius_3 - 2 * norm_radius) * sin_angle;
				m_zernike_terms[8][l] = in_Circle * sqrt_8  * (3   * norm_radius_3 - 2 * norm_radius) * cos_angle;
				m_zernike_terms[9][l] = in_Circle * sqrt_8  * norm_radius_3 * cos_3angle;
				
				m_zernike_terms[10][l] = in_Circle * sqrt_10 * norm_radius_4 * sin_4angle;
				m_zernike_terms[11][l] = in_Circle * sqrt_10 * (4   * norm_radius_4 - 3 * norm_radius_2) * sin_2angle;
				m_zernike_terms[12][l] = in_Circle * sqrt_5  * (6   * norm_radius_4 - 6 * norm_radius_2 + 1);
				m_zernike_terms[13][l] = in_Circle * sqrt_10 * (4   * norm_radius_4 - 3 * norm_radius_2) * cos_2angle;
				m_zernike_terms[14][l] = in_Circle * sqrt_10 * norm_radius_4 * cos_4angle;
				
				m_zernike_terms[15][l] = in_Circle * sqrt_12 * norm_radius_5 * sin_5angle;
				m_zernike_terms[16][l] = in_Circle * sqrt_12 * (5   * norm_radius_5 - 4 * norm_radius_3) * sin_3angle;
				m_zernike_terms[17][l] = in_Circle * sqrt_12 * (10  * norm_radius_5 - 12 * norm_radius_3 + 3 * norm_radius) * sin_angle;
				m_zernike_terms[18][l] = in_Circle * sqrt_12 * (10  * norm_radius_5 - 12 * norm_radius_3 + 3 * norm_radius) * cos_angle;
				m_zernike_terms[19][l] = in_Circle * sqrt_12 * (5   * norm_radius_5 - 4 * norm_radius_3) * cos_3angle;
				m_zernike_terms[20][l] = in_Circle * sqrt_12 * norm_radius_5 * cos_5angle;
				
				m_zernike_terms[21][l] = in_Circle * sqrt_14 * norm_radius_6 * sin_6angle;
				m_zernike_terms[22][l] = in_Circle * sqrt_14 * (6   * norm_radius_6 - 5 * norm_radius_4) * sin_4angle;
				m_zernike_terms[23][l] = in_Circle * sqrt_14 * (15  * norm_radius_6 - 20 * norm_radius_4 + 6 * norm_radius_2) * sin_2angle;
				m_zernike_terms[24][l] = in_Circle * sqrt_7  * (20  * norm_radius_6 - 30 * norm_radius_4 + 12 * norm_radius_2 - 1);
				m_zernike_terms[25][l] = in_Circle * sqrt_14 * (15  * norm_radius_6 - 20 * norm_radius_4 + 6 * norm_radius_2) * cos_2angle;
				m_zernike_terms[26][l] = in_Circle * sqrt_14 * (6   * norm_radius_6 - 5 * norm_radius_4) * cos_4angle;
				m_zernike_terms[27][l] = in_Circle * sqrt_14 * norm_radius_6 * cos_6angle;
				
				m_zernike_terms[28][l] = in_Circle * 4       * norm_radius_7 * sin_7angle;
				m_zernike_terms[29][l] = in_Circle * 4       * (7   * norm_radius_7 - 6 * norm_radius_5) * sin_5angle;
				m_zernike_terms[30][l] = in_Circle * 4       * (21  * norm_radius_7 - 30 * norm_radius_5 + 10 * norm_radius_3) * sin_3angle;
				m_zernike_terms[31][l] = in_Circle * 4       * (35  * norm_radius_7 - 60 * norm_radius_5 + 30 * norm_radius_3 - 4 * norm_radius) * sin_angle;
				m_zernike_terms[32][l] = in_Circle * 4       * (35  * norm_radius_7 - 60 * norm_radius_5 + 30 * norm_radius_3 - 4 * norm_radius) * cos_angle;
				m_zernike_terms[33][l] = in_Circle * 4       * (21  * norm_radius_7 - 30 * norm_radius_5 + 10 * norm_radius_3) * cos_3angle;
				m_zernike_terms[34][l] = in_Circle * 4       * (7   * norm_radius_7 - 6 * norm_radius_5) * cos_5angle;
				m_zernike_terms[35][l] = in_Circle * 4       * norm_radius_7 * cos_7angle;
				
				m_zernike_terms[36][l] = in_Circle * sqrt_18 * norm_radius_8 * sin_8angle;
				m_zernike_terms[37][l] = in_Circle * sqrt_18 * (8   * norm_radius_8 - 7 * norm_radius_6) * sin_6angle;
				m_zernike_terms[38][l] = in_Circle * sqrt_18 * (28  * norm_radius_8 - 42 * norm_radius_6 + 15 * norm_radius_4) * sin_4angle;
				m_zernike_terms[39][l] = in_Circle * sqrt_18 * (56  * norm_radius_8 - 105 * norm_radius_6 + 60 * norm_radius_4 - 10 * norm_radius_2) * sin_2angle;
				m_zernike_terms[40][l] = in_Circle * 3       * (70  * norm_radius_8 - 140 * norm_radius_6 + 90 * norm_radius_4 - 20 * norm_radius_2 + 1);
				m_zernike_terms[41][l] = in_Circle * sqrt_18 * (56  * norm_radius_8 - 105 * norm_radius_6 + 60 * norm_radius_4 - 10 * norm_radius_2) * cos_2angle;
				m_zernike_terms[42][l] = in_Circle * sqrt_18 * (28  * norm_radius_8 - 42 * norm_radius_6 + 15 * norm_radius_4) * cos_4angle;
				m_zernike_terms[43][l] = in_Circle * sqrt_18 * (8   * norm_radius_8 - 7 * norm_radius_6) * cos_6angle;
				m_zernike_terms[44][l] = in_Circle * sqrt_18 * norm_radius_8 * cos_8angle;
				
				m_zernike_terms[45][l] = in_Circle * sqrt_20 * norm_radius_9 * sin_9angle;
				m_zernike_terms[46][l] = in_Circle * sqrt_20 * (9   * norm_radius_9 - 8 * norm_radius_7) * sin_7angle;
				m_zernike_terms[47][l] = in_Circle * sqrt_20 * (36  * norm_radius_9 - 56 * norm_radius_7 + 21 * norm_radius_5) * sin_5angle;
				m_zernike_terms[48][l] = in_Circle * sqrt_20 * (84  * norm_radius_9 - 168 * norm_radius_7 + 105 * norm_radius_5 - 20 * norm_radius_3) * sin_3angle;
				m_zernike_terms[49][l] = in_Circle * sqrt_20 * (126 * norm_radius_9 - 280 * norm_radius_7+ 210 * norm_radius_5 - 60 * norm_radius_3 + 5 * norm_radius) * sin_angle;
				m_zernike_terms[50][l] = in_Circle * sqrt_20 * (126 * norm_radius_9 - 280 * norm_radius_7+ 210 * norm_radius_5 - 60 * norm_radius_3 + 5 * norm_radius) * cos_angle;
				m_zernike_terms[51][l] = in_Circle * sqrt_20 * (84  * norm_radius_9 - 168 * norm_radius_7 + 105 * norm_radius_5 - 20 * norm_radius_3) * cos_3angle;
				m_zernike_terms[52][l] = in_Circle * sqrt_20 * (36  * norm_radius_9 - 56 * norm_radius_7 + 21 * norm_radius_5) * cos_5angle;
				m_zernike_terms[53][l] = in_Circle * sqrt_20 * (9   * norm_radius_9 - 8 * norm_radius_7) * cos_7angle;
				m_zernike_terms[54][l] = in_Circle * sqrt_20 * norm_radius_9 * cos_9angle;
				
				m_zernike_terms[55][l] = in_Circle * sqrt_22 * norm_radius_10 * sin_10angle;
				m_zernike_terms[56][l] = in_Circle * sqrt_22 * (10  * norm_radius_10 - 9 * norm_radius_8) * sin_8angle;
				m_zernike_terms[57][l] = in_Circle * sqrt_22 * (45  * norm_radius_10 - 72 * norm_radius_8 + 28 * norm_radius_6) * sin_6angle;
				m_zernike_terms[58][l] = in_Circle * sqrt_22 * (120 * norm_radius_10 - 252 * norm_radius_8 + 168 * norm_radius_6 - 35 * norm_radius_4) * sin_4angle;
				m_zernike_terms[59][l] = in_Circle * sqrt_22 * (210 * norm_radius_10 - 504 * norm_radius_8 + 420 * norm_radius_6 - 140 * norm_radius_4 + 15 * norm_radius_2) * sin_2angle;
				m_zernike_terms[60][l] = in_Circle * sqrt_11 * (252 * norm_radius_10 - 630 * norm_radius_8 + 560 * norm_radius_6 - 210 * norm_radius_4 + 30 * norm_radius_2 - 1);
				m_zernike_terms[61][l] = in_Circle * sqrt_22 * (210 * norm_radius_10 - 504 * norm_radius_8 + 420 * norm_radius_6 - 140 * norm_radius_4 + 15 * norm_radius_2) * cos_2angle;
				m_zernike_terms[62][l] = in_Circle * sqrt_22 * (120 * norm_radius_10 - 252 * norm_radius_8 + 168 * norm_radius_6 - 35 * norm_radius_4) * cos_4angle;
				m_zernike_terms[63][l] = in_Circle * sqrt_22 * (45  * norm_radius_10 - 72 * norm_radius_8 + 28 * norm_radius_6) * cos_6angle;
				m_zernike_terms[64][l] = in_Circle * sqrt_22 * (10  * norm_radius_10 - 9 * norm_radius_8) * cos_8angle;
				m_zernike_terms[65][l] = in_Circle * sqrt_22 * norm_radius_10 * cos_10angle;
				l++;
			}
		}
	}
}

void COptCalc::get_DefCylAxis(double *def, double *cyl, double *axis)
{
	*(def) = (-1) * m_defocus;
	*(cyl) = (-1) * m_cylinder;
	*(axis) = m_axis;
}

void COptCalc::Induce_Abberations(double values[MAX_TERM_NUMBER+1], double defocus, double fit_size)
{
	memset(g_centroids->m_dZAbber, 0, (MAX_TERM_NUMBER+1)*sizeof(double));
	if (values != NULL) //ignore defocus argument and remember the Z terms from CLAbberations dialog are in microns
		memcpy(g_centroids->m_dZAbber, values, (MAX_TERM_NUMBER+1)*sizeof(double));
	else //ignore values argument and process defocus only
		g_centroids->m_dZAbber[4] = Diopters_to_Microns(-defocus, fit_size);

	m_CloopFlag?g_centroids->Initialize_Phi()
				:Compute_Openloop_Voltages(g_centroids->m_dZAbber[4]),Send_Voltages(COMP_BIT);
	
	m_ReferenceDefocus = g_centroids->m_dZAbber[4];
}

double COptCalc::Diopters_to_Microns(double def_diopters, double fit_size)
{
	// Converting to microns (standard equation: D = (-4*srqt(3)*Z4)/pow(radius_in_mm,2)
	return ((def_diopters*pow((fit_size/(2.*1000.)),2))/(4.*sqrt(3.))); //06/29/2010
}

void COptCalc::set_CloopFlag(bool val)
{
	m_CloopFlag = val;
}

bool COptCalc::get_CloopFlag()
{
	return m_CloopFlag;
}

void COptCalc::SendVoltsTest(double* volts)
{
	g_dmirror->SendVoltages(volts);
}

void COptCalc::Send_Voltages(mirror_state state)
{
	short i,j;
	
	switch (state)
	{
	case ABS_ZERO_BIT:
		// Set the zernikes to zero
		for(i=0;i<=MAX_TERM_NUMBER;i++)
			c[i] = 0.0;	
		m_ReferenceDefocus = 0.0;
		memset(g_AOSACAParams->g_DMDeflections, 0, g_AOSACAParams->NUMACTS*sizeof(double));
		break;
	case BIAS_BIT:		
		memcpy(g_AOSACAParams->g_DMDeflections, m_dBiasBit, g_AOSACAParams->NUMACTS*sizeof(double));
		memcpy(m_CurVoltages, m_dBiasBit, g_AOSACAParams->NUMACTS*sizeof(double));
		break;
	case ALLIGN_BIT:
		for(i=3;i<=7;i+=4)
			for(j=3;j<=7;j++)
				g_AOSACAParams->g_DMDeflections[i*(g_AOSACAParams->DMGRID)+j-12] = m_dMaxBit/1.6;
		for(i=4;i<=6;i++)
			for(j=3;j<=7;j+=4)
				g_AOSACAParams->g_DMDeflections[i*(g_AOSACAParams->DMGRID)+j-12] = m_dMaxBit/1.6;
	case COMP_BIT:		
		break;
	default:
		;
	}
	VD_limit(g_AOSACAParams->g_DMDeflections, g_AOSACAParams->g_DMDeflections, (g_AOSACAParams->NUMACTS), -1., 1.);
	memcpy(m_PrevVoltages1, g_AOSACAParams->g_DMDeflections, (g_AOSACAParams->NUMACTS)*sizeof(double));
	g_AOSACAParams->g_bDMReady?g_dmirror->SendVoltages(g_AOSACAParams->g_DMDeflections):0;
}

BOOL COptCalc::SaveVoltages(CStringA filename)
{	
	FILE* outfile;
	BOOL result = true;
	errno_t err;
	err = fopen_s(&outfile, filename, "w+");

	if(err == 0) 
	{
		for (int i=0; i<g_AOSACAParams->NUMACTS; i++)
			fprintf(outfile, "%1.6f\n", g_AOSACAParams->g_DMDeflections[i]);
		fclose(outfile);
	}
	else
		result = false;
	
	return result;
}

void COptCalc::SetActuator(int idx, mirror_state state)
{
	switch (state)
	{
	case MIN_BIT:
		g_AOSACAParams->g_DMDeflections[idx] = m_dMinBit + g_AOSACAParams->g_DMBiasDeflections[idx];
		break;
	case BIAS_BIT:
		g_AOSACAParams->g_DMDeflections[idx] = g_AOSACAParams->g_DMBiasDeflections[idx];
		break;
	case MAX_BIT:
		g_AOSACAParams->g_DMDeflections[idx] = m_dMaxBit + g_AOSACAParams->g_DMBiasDeflections[idx];
		break;
	case COMP_BIT:
		g_AOSACAParams->g_DMDeflections[idx] = m_CurVoltages[idx];
		break;
	}
	Send_Voltages(COMP_BIT);
	return;
}

double COptCalc::get_Defocus()
{
	return m_ReferenceDefocus;
}

void COptCalc::get_DMVoltages(double *dest)
{
	memcpy(dest, g_AOSACAParams->g_DMDeflections, (g_AOSACAParams->NUMACTS)*sizeof(double));
}

void COptCalc::AO_Off(BOOL state)
{
	if (state)
	{
		memcpy(m_DMVoltages_buf, g_AOSACAParams->g_DMDeflections, (g_AOSACAParams->NUMACTS)*sizeof(double));		
		Send_Voltages(BIAS_BIT);
	}
	else
	{		
		memcpy(g_AOSACAParams->g_DMDeflections, m_DMVoltages_buf, (g_AOSACAParams->NUMACTS)*sizeof(double));
		Send_Voltages(COMP_BIT);
	}
}
 
void COptCalc::reset_Voltages()
{
	memset(m_PrevVoltages1, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));
	memset(m_CurVoltages, 0, (g_AOSACAParams->NUMACTS)*sizeof(double));
	Send_Voltages(BIAS_BIT);
}


///////////////////
//codes for wavefront reconstruction based on Southwell method 
///////////////////

void COptCalc::Init_Southwell()
//  corresponding to Function "Init_Mahajan()"
{

	if(!m_bPhiReconReady)
	{
		if (m_dPhiReconM != NULL) 
			delete [] m_dPhiReconM;
		m_dPhiReconM = new double[g_centroids->get_TotalCentCount()*g_centroids->get_TotalCentCount()*2];
		memset(m_dPhiReconM, 0, (g_centroids->get_TotalCentCount()*g_centroids->get_TotalCentCount()*2)*sizeof(double));
		Generate_Matrices_DG();        //generate the wavefront reconstruction matrix 
		//Load_PhiReconM();        //load the wavefront reconstruction matrix 
		
		if (m_dWaveV != NULL)
			delete [] m_dWaveV;
		m_dWaveV= new double[g_centroids->get_TotalCentCount()];
		memset(m_dWaveV, 0, (g_centroids->get_TotalCentCount())*sizeof(double));	
		
	}
	if (m_bPhiReconReady)
	{
		MD_mulV(m_dWaveV, &m_dPhiReconM, g_centroids->m_dXYSlopeV, g_centroids->get_TotalCentCount(), (g_centroids->get_TotalCentCount())<<1);
		
  }
		
}


bool COptCalc::Load_PhiReconM()
{
	m_bPhiReconReady=false;
	short CentCount=g_centroids->get_TotalCentCount();
	FILE *fp;
	fopen_s(&fp, "utils\\S2PhiReconM.txt","r");
	if(fp)
	{	
		MD_read(&m_dPhiReconM, CentCount, (CentCount<<1), fp);
		fclose(fp);
		m_bPhiReconReady=true;
	}
	
	return (m_bPhiReconReady);
}


bool COptCalc::Generate_Matrices_DG()
{
	bool status=false;
	unsigned long cols;
	cols=g_AOSACAParams->LENSLETGRID;
	double *tempA, *tempB;
	unsigned long A_xlim, A_ylim, B_xlim, B_ylim;
	A_xlim=cols*cols;
	A_ylim=cols*(cols-1);
	B_xlim=A_xlim;
	B_ylim=A_ylim;
	tempA = new double[A_xlim*A_ylim];
	memset(tempA, 0, (A_xlim*A_ylim)*sizeof(double));
	tempB = new double[B_xlim*B_ylim];
	memset(tempB, 0, (B_xlim*B_ylim)*sizeof(double));
	unsigned long Ax, Ay, Bx, By;
	//x derivative operator (between columns: c1: 1 vs 2, c2: 2 vs 3)
	for (Ay=0; Ay<A_ylim; Ay++)
	{
		tempA[Ay*A_xlim+Ay]=-1;
		tempA[Ay*A_xlim+Ay+cols]=1;
	}
	unsigned long nx, ny;
	//y derivative operator (between row: r1: 1 vs 2, r2: 2 vs 3)
	for (nx=0; nx<cols; nx++)
	{
		for (ny=0; ny<(cols-1); ny++)
		{
			tempB[(nx*(cols-1)+ny)*B_xlim+(nx*cols+ny)]=-1;
			tempB[(nx*(cols-1)+ny)*B_xlim+(nx*cols+ny)+1]=1;
		}
	}

	//mark the rows and columns caused by unused lenslet array
	double *Mark_ax, *Mark_ay, *Mark_bx, *Mark_by;
	Mark_ax=new double[A_xlim];
	memset(Mark_ax, 0, (A_xlim)*sizeof(double));
	VD_equC(Mark_ax, A_xlim, 1.0);
	Mark_ay=new double[A_ylim];
	memset(Mark_ay, 0, (A_ylim)*sizeof(double));
	VD_equC(Mark_ay, A_ylim, 1.0);
	Mark_bx=new double[B_xlim];
	memset(Mark_bx, 0, (B_xlim)*sizeof(double));
	VD_equC(Mark_bx, B_xlim, 1.0);
	Mark_by=new double[B_ylim];
	memset(Mark_by, 0, (B_ylim)*sizeof(double));
	VD_equC(Mark_by, B_ylim, 1.0);

	for (nx=0; nx<cols; nx++)
	{
		for (ny=0; ny<cols; ny++)
		{	
			if (g_centroids->m_bWFSMask[ny * cols + nx] == 0)
			{ 
				Ax=nx * cols + ny;
				Bx=Ax;
				Mark_ax[Ax]=0;
				Mark_bx[Bx]=0;
				for (Ay=0; Ay<A_ylim; Ay++)
				{
					if(tempA[Ay*A_xlim+Ax]!=0)
					{
						Mark_ay[Ay]=0;
					}
				}
				for (By=0; By<B_ylim; By++)
				{
					if(tempB[By*B_xlim+Bx]!=0)
					{
						Mark_by[By]=0;
					}
				}
			}
		}
	}

	// remove the rows and columns caused by unused lenslet array
	double *tempA2, *tempB2;
	unsigned long A2_xlim, A2_ylim, B2_xlim, B2_ylim;
	A2_xlim=0;
	A2_ylim=0;
	B2_xlim=0;
	B2_ylim=0;

	for (Ax=0; Ax<A_xlim; Ax++)
	{
		if (Mark_ax[Ax]==1)
			A2_xlim++;
	}
	for (Ay=0; Ay<A_ylim; Ay++)
	{
		if (Mark_ay[Ay]==1)
			A2_ylim++;
	}
	B2_xlim=A2_xlim;
	for (By=0; By<B_ylim; By++)
	{
		if (Mark_by[By]==1)
			B2_ylim++;
	}

	tempA2 = new double[A2_xlim*A2_ylim];
	memset(tempA2, 0, (A2_xlim*A2_ylim)*sizeof(double));
	tempB2 = new double[B2_xlim*B2_ylim];
	memset(tempB2, 0, (B2_xlim*B2_ylim)*sizeof(double));

	unsigned long index;
	index=0;
	for (Ay=0; Ay<A_ylim; Ay++)
	{
		for (Ax=0; Ax<A_xlim; Ax++)
		{
			if (Mark_ax[Ax]==1 && Mark_ay[Ay]==1)
			{	
				tempA2[index]=tempA[Ay*A_xlim+Ax];
				index++;
			}	
		}
	}
	index=0;
	for (By=0; By<B_ylim; By++)
	{
		for (Bx=0; Bx<B_xlim; Bx++)
		{
			if (Mark_bx[Bx]==1 && Mark_by[By]==1)
			{	
				tempB2[index]=tempB[By*B_xlim+Bx];
				index++;
			}	
		}
	}
	

	// matrix G is Gradient operator, Hudgin geometry
	// matrix D is Southwell to Hudgin interpolator : D*Slope=G*Phi
	unsigned long G_xlim, G_ylim, D_xlim, D_ylim;
	G_xlim=A2_xlim;
	G_ylim=A2_ylim+B2_ylim;
	D_xlim=A2_xlim+B2_xlim;
	D_ylim=G_ylim;
	m_dMatrixG = new double[G_xlim*G_ylim];
	memset(m_dMatrixG, 0, (G_xlim*G_ylim)*sizeof(double));
	m_dMatrixD = new double[D_xlim*D_ylim];
	memset(m_dMatrixD, 0, (D_xlim*D_ylim)*sizeof(double));
	
	memcpy( m_dMatrixG, tempA2, A2_xlim*A2_ylim*sizeof(double));
	memcpy( &m_dMatrixG[A2_xlim* A2_ylim], tempB2, B2_xlim*B2_ylim*sizeof(double));
	VD_mulC(m_dMatrixG, m_dMatrixG, G_xlim*G_ylim, 1/g_AOSACAParams->LENSLET_DIAMETER_MICRONS);
	for (Ay=0; Ay<A2_ylim; Ay++)
	{
		for (Ax=0; Ax<A2_xlim; Ax++)
		{
			if (tempA2[A2_xlim*Ay+Ax]!=0)
				m_dMatrixD[D_xlim*Ay+Ax]=0.5;
		}
	}
	for (By=0; By<B2_ylim; By++)
	{
		for (Bx=0; Bx<B2_xlim; Bx++)
		{
			
			if (tempB2[B2_xlim*By+Bx]!=0) m_dMatrixD[D_xlim*(By+A2_ylim)+(A2_xlim+Bx)]=0.5;
		}
	}

	Generate_PhiReconstrutor(G_ylim);

	delete [] tempA, tempA=NULL;
	delete [] tempB, tempB=NULL;
	delete [] tempA2, tempA2=NULL;
	delete [] tempB2, tempB2=NULL;
	delete [] Mark_ax, Mark_ax=NULL;
	delete [] Mark_ay, Mark_ay=NULL;
	delete [] Mark_bx, Mark_bx=NULL;
	delete [] Mark_by, Mark_by=NULL;

	status=true;
	return status;
}


bool COptCalc::Generate_PhiReconstrutor(unsigned long GandDy)
{
	m_bPhiReconReady=false;	
	unsigned long CentNum=g_centroids->get_TotalCentCount();

	unsigned long Gx=CentNum;
	unsigned long Gy=GandDy;
	double *temp0;	
	temp0 = new double[Gx*Gx];
	memset(temp0, 0, (Gx*Gx)*sizeof(double));
	MD_TmulM(&temp0, &m_dMatrixG, &m_dMatrixG,GandDy, Gx, Gx);

	////////// SVD  //SVD(X), X=U*W*V'; Vmat*WImat*Umat'
	unsigned long len=Gx;
	unsigned long ht=Gx;

	double *Umat, *WImat, *Vmat;
	Umat = new double[ht*len];
	WImat = new double[len*len];
	Vmat = new double[ len*len];

	double *temp, *invtemp;	
	temp = new double[len*len];
	memset(temp, 0, (len*len)*sizeof(double));
	invtemp = new double[len*ht];
	memset(invtemp, 0, (len*ht)*sizeof(double));

	double *W, *MOne;
	W = new double[len];
	MOne = new double[ len*len]; 
	MD_equ1( &MOne, len);

	MD_SVdecompose( &Umat, &Vmat, W, &temp0, ht, len);	
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

	MD_mulMdia( &WImat, &MOne, W, len, len);
	MD_mulM(&temp, &Vmat, &WImat, len, len, len);
	MD_mulMT(&invtemp, &temp, &Umat, len, len, ht);
//end SVD
	double *temp2;	
	temp2 = new double[Gx*GandDy];
	memset(temp2, 0, (Gx*GandDy)*sizeof(double));
	MD_mulMT(&temp2, &invtemp, &m_dMatrixG, len, len, Gy);
	MD_mulM(&m_dPhiReconM, &temp2, &m_dMatrixD, Gx, Gy, Gx<<1);

	m_bPhiReconReady=true;
	delete [] W, W = NULL;
	delete [] MOne, MOne = NULL;
	delete [] temp, temp = NULL;
	delete [] temp0, temp0 = NULL;
	delete [] temp2, temp2 = NULL;
	delete [] invtemp, invtemp = NULL;
	delete [] Umat, Umat=NULL;
	delete [] WImat, WImat=NULL;
	delete [] Vmat, Vmat=NULL;
	
	return (m_bPhiReconReady);
}

double COptCalc::Compute_RMS_Southwell()  // Calculate RMS...
{
	double rms;
	int boxindex;
	bool *CentFound;
	CentFound=g_centroids->get_CentFound();
    rms = 0.0;
	for (boxindex = 0; boxindex < g_centroids->get_TotalCentCount() ; boxindex++) 
	{
		if(CentFound[boxindex]==1)
		{
			rms += m_dWaveV[boxindex] * m_dWaveV[boxindex];
		}
	}
    m_rms = sqrt(rms/g_centroids->GetCentroidCount());

	return m_rms;
}

void COptCalc::Phase_Translation(double *phasemapR, double *phasemapI, unsigned long N)
//  corresponding to Function "Zphase_Mahajan()"
{
	unsigned long nx, ny; 
	int			xpos, ypos;
	
	m_psf_size_pix = 0;

	int boxindex;
	boxindex=0;
	bool *CentFound;
	CentFound=g_centroids->get_CentFound();

	for (nx =0; nx<N; nx++)
	{
		for (ny=0; ny<N; ny++)
		{
			phasemapR[ny * N + nx] = 0;
			phasemapI[ny * N + nx] = 0;
			
			xpos=nx-(N>>1)+(g_AOSACAParams->LENSLETGRID>>1);
			ypos=ny-(N>>1)+(g_AOSACAParams->LENSLETGRID>>1);

			if (xpos>=0 && xpos<g_AOSACAParams->LENSLETGRID && ypos>=0 && ypos<g_AOSACAParams->LENSLETGRID)
			{
				if (g_centroids->m_bWFSMask[ypos * g_AOSACAParams->LENSLETGRID + xpos] == 1)
				{ 
					if(CentFound[boxindex]==1)
					{
						phasemapR[ny * N + nx] = cos(- 2 * PI * m_dWaveV[boxindex] / (g_AOSACAParams->WAVELENGTH_MICRONS));
						phasemapI[ny * N + nx] = sin(- 2 * PI * m_dWaveV[boxindex] / (g_AOSACAParams->WAVELENGTH_MICRONS));
						m_psf_size_pix += 1;
					}			
				boxindex++;
				}
			}
		}
	}

}

void COptCalc::Wave_Translation(double *src)
//corresponding to Function "Zwave_Mahajan_New()"
//Note that the masks of WFS image for Zonel and Zernike are different
//should change m_bData in Function "InitWaveMatrix(bool *src)"
{	
	int nx, ny, N, i=-1;
	int	xpos, ypos;
	double *waveabermap=d_WAVE->data;
	int boxindex;
	boxindex=0;
	N=g_AOSACAParams->LENSLETGRID;	

	//// 4x4 pixels per lenslet
	for (nx=0; nx<N; nx++)
	{
		for (ny=0; ny<N; ny++)
		{	
			if (g_centroids->m_bWFSMask[ny * N + nx] == 1)
			{ 	
				for (ypos=ny*4;ypos<(ny*4+4);ypos++)
				{
					for (xpos=nx*4;xpos<(nx*4+4);xpos++)
						waveabermap[xpos * (N*4) + ypos] = m_dWaveV[boxindex];
				}
				boxindex++;
			}
			else
			{
				for (ypos=ny*4;ypos<(ny*4+4);ypos++)
					for (xpos=nx*4;xpos<(nx*4+4);xpos++)
						waveabermap[xpos * (N*4) + ypos] = NAP;
			}			
		}
	}
	for (xpos=0;xpos<(N*4);xpos++)
	{
		for (ypos=0;ypos<(N*4);ypos++)
			if (waveabermap[ypos * (N*4) + xpos] != NAP)
				src[++i]=waveabermap[ypos * (N*4) + xpos];
	}

}


void COptCalc:: PreCorrection(double defocus, double cylinder, double axis, double fit_size)
{
	double *PreCorrectionZ345;
	PreCorrectionZ345= new double[3];
	PreCorrectionZ345[0]=PreCorrectionZ345[0]=PreCorrectionZ345[2]=0.0;
	Diopters_to_Zernike_Microns(-1*defocus,-1*cylinder, axis, fit_size, PreCorrectionZ345);	
	Compute_PreCorrection_Voltages(PreCorrectionZ345);
	if (g_AOSACAParams->g_bCPreCorrApply)
	{
		UpdateDMFlatData();
		memcpy(m_CurVoltages, m_dBiasBit, (g_AOSACAParams->NUMACTS)*sizeof(double));
		memcpy(g_AOSACAParams->g_DMDeflections, m_CurVoltages, (g_AOSACAParams->NUMACTS)*sizeof(double));
		Send_Voltages(COMP_BIT);
	}
	m_Ref_PreZ345[0] = PreCorrectionZ345[0];
	m_Ref_PreZ345[1] = PreCorrectionZ345[1];
	m_Ref_PreZ345[2] = PreCorrectionZ345[2];
}


void COptCalc:: Diopters_to_Zernike_Microns(double defocus, double cylinder, double axis, double fit_size, double *Z345)	
{
	double cm3, cm4, cm5, phi, Acoeff;
	
	if (axis == 0.)
		axis = 180.;
	phi = PI*(180.-axis)/180.;
	cm4 = ((defocus*pow((fit_size/2000.),2.))/(4.*sqrt(3.))) + ((cylinder*pow((fit_size/2000.),2.))/(8.*sqrt(3.)));
	
	Acoeff = (cylinder*pow((fit_size/2000.),2.))/2.;
	cm3 = -(1./(2.*sqrt(6.)))*tan(2.*phi)*Acoeff*cos(2.*phi);

	if (tan(2.*phi)==0)
		cm5 = 0.;
	else
		cm5 = -(Acoeff*sin(2.*phi)/(2.*sqrt(6.)))/(tan(2.*phi));
	Z345[0]=cm3;
	Z345[1]=cm4;
	Z345[2]=cm5;
}

void COptCalc::Compute_PreCorrection_Voltages(double *Z345)
{
	// original codes seem the value is not right 
	int i=0;
	short size;
	size=g_centroids->get_TotalCentCount();
	double *tempPhi;
	tempPhi = new double[size<<1];
	memset(tempPhi, 0, (size<<1)*sizeof(double)); 

	for (i=0; i<(size<<1);i++)
		tempPhi[i]= (Z345[0]) * g_centroids->m_dZM[i*(MAX_TERM_NUMBER+1)+3]+(Z345[1]) * g_centroids->m_dZM[i*(MAX_TERM_NUMBER+1)+4]+(Z345[2]) * g_centroids->m_dZM[i*(MAX_TERM_NUMBER+1)+5];	

	VD_mulC(tempPhi, tempPhi, (size<<1), (2/(g_AOSACAParams->PUPIL_FIT_SIZE_MICRONS)));
	
	MD_mulV(m_PreCorrectionVoltages, &(g_centroids->m_dReconM_fullpupil), tempPhi, (g_AOSACAParams->NUMACTS), size<<1 ); //m_dReconM_fullpupil

	VD_limit(m_PreCorrectionVoltages, m_PreCorrectionVoltages, (g_AOSACAParams->NUMACTS), -0.8, 0.8);	
}