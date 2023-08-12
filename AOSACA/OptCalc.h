// OptCalc.h: interface for the COptCalc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTCALC_H__4B43F241_6B78_11D6_8008_006008A334DD__INCLUDED_)
#define AFX_OPTCALC_H__4B43F241_6B78_11D6_8008_006008A334DD__INCLUDED_

#pragma once

#include <math.h>
#include "VecALL.h"
#include "MatALL.h"
#include "DMirror.h"

////////////////////////////////////////////////////////
#define PI		3.1415926
#define NAP		999999999
#define MEMINIT	30000
#define TINY	1.0e-20

#include "Recipes.h"

class CPlotInfo
{
public:
	int dim;//dim is the true size of matrix WAVE, PSF or MTF
	double step;
	double orig;
	CString title;
	CString xlabel;
	CString ylabel;
	CString zlabel;
};


class COptCalcData
{
public:
	unsigned long row;
	unsigned long col;
	double *data;

	//Member functions
	COptCalcData(int size);
	virtual ~COptCalcData();
};


class COptCalc  
{
public:	
	enum opt_data_enum {MTFDATA,PSFDATA,WAVEDATA };
	enum correction_plane_enum {ENTRANCEPLANE,CORNEALPLANE,SPECTACLEPLANE};
	double	retn_RMS();
	double	retn_Strehl();
	void	get_ZCoeffs(double*, short num_of_coeffs);
	
	bool	get_CloopFlag(void);
	void	set_CloopFlag(bool val);	
	double	get_MinBit(){return m_dMinBit;}
	double	get_MaxBit(){return m_dMaxBit;}
	double	Diopters_to_Microns(double, double);
	void	get_DefCylAxis(double *def, double *cyl, double *axis);
	CStringA retn_Voltages(void);
	void	Compute_Openloop_Voltages(double);
	void	Compute_Voltages();
	void	SendVoltsTest(double* volts =NULL);
	void	Send_Voltages(mirror_state state);
	BOOL	SaveVoltages(CStringA outfile);
	BOOL	CreateDMDevice();
	void	reset_ReferenceDefocus() {m_ReferenceDefocus = 0.0;}
	void	get_DMVoltages(double*);
	void	reset_Voltages();
	void	Induce_Abberations(double values[MAX_TERM_NUMBER+1], double defocus=0., double fit_size=0);
	double*	get_ZernikeCoeffs(){return c;}
	void	UpdateDMFlatData();	
	double	get_currentPupilSize(){return m_pupil_field_size_microns;}	
	//Construction/destruction functions
	COptCalc();
	virtual ~COptCalc();

	// Initialization Functions
	void	UpdateUserParameters(double pupildia);
	BOOL	InitializeSystemParameters(BYTE *mask);
	void	UpdateDMMaxDeflection(double);

	// External Data Retrieval Functions
	COptCalcData * FetchPlotInfo(CPlotInfo * pPlot_Info, unsigned short data_type);
	COptCalcData * FetchOptCalcData(opt_data_enum idex);
	int		GetSize(opt_data_enum idex);
	
	CStringA FormatOpticalResult(double pupil_fit_size, double induced_defocus);
	
	// Identify which correction plane for calcuations
	CStringA Compute_Best_Spectacle_Correction(
				BOOL c3term=TRUE,
				BOOL c4term=TRUE,
				BOOL c5term=TRUE,
				correction_plane_enum plane=ENTRANCEPLANE,
				BOOL generate_string=FALSE);

	// Computing Zernikes/WAVE/MTF/PSF
	void	Init_Mahajan();
	int		Compute_WAVE (double*);
	double	Compute_PSF_MTF(double*);
	double	Compute_RMS(double);
	CStringA Compute_RMSZernikesVoltages(double, double);
	double	get_Defocus();
	void	SetActuator(int, mirror_state);
	short	InitWaveMatrix(bool*);
	void	AO_Off(BOOL);
	void	set_ZReconReady(bool state){m_bZReconReady = state;}
	void	set_ZOpenLoopReady(bool state){m_bZOpenLoopReady = state;}
	double	get_Gain() {return m_dIntGain;}
	void	set_Gain(double int_gain) {m_dIntGain = int_gain;}
	void	Initialize_Zernike_Coeffs(double pupildia);

	//Computing Wave based on Southwell method
	void set_bPhiReconReady(bool flag){m_bPhiReconReady = flag;}
	bool Generate_PhiReconstrutor(unsigned long GandDy);
	bool Generate_Matrices_DG();
	void Init_Southwell();
	void Phase_Translation(double *phasemapR, double *phasemapI,unsigned long N);
	double Compute_RMS_Southwell();
	bool Load_PhiReconM();
	void Wave_Translation (double *src);
	void get_ZCoeffs_all(double*, short num_of_coeffs);
	void PreCorrection(double defocus, double cylinder, double axis, double fit_size);
	void Compute_PreCorrection_Voltages(double *Z345);
	void Diopters_to_Zernike_Microns(double defocus, double cylinder, double axis, double fit_size, double *Z345);
	double *m_Ref_PreZ345;
/////////
private:
	double	m_dMaxBit;
	double	m_dMinBit;
	BOOL	m_AOOnFlag;
	bool	m_CloopFlag;
	bool	m_CloopMode;
	double	m_ReferenceDefocus;
	double	m_dIntGain;	
	double	m_dFFactor;
	void	compute_ZernikeTerms(BYTE*);
	bool	m_bZReconReady;
	bool	m_bZOpenLoopReady;

	// Zernike coeffs
	// Note: the first element c[0] is not used at all.
	double	c[MAX_TERM_NUMBER+1];
	double	m_c3_old;
	double	m_c4_old;
	double	m_c5_old;

	// Use pointer as one-dimentional array to store matrix. while computing the Psf 	
	double	*m_pupilfuncR;
	double	*m_pupilfuncI;
	double	*m_HampR;
	double	*m_HampI;
	double	*m_Hint;
	// Internal data storage for WAVE, PSF, MTF
	COptCalcData *d_WAVE;
	COptCalcData *d_PSF;
	COptCalcData *d_MTF;
	double	**m_nWData;
	double	*Zcoeff;
	double	leak;
	
	// For Zernike Calculations
	double	*m_zernike_terms[MAX_TERM_NUMBER+1];
	double	*m_CurVoltages;
	double	*m_PrevVoltages1;
	double	*m_dBiasBit;
	double	*m_DMVoltages_buf;			//Used for AO ON/OFF mode
	double	*m_dZReconM;
	double	m_pupil_field_size_psf;
	int		m_mode;						// variable for the number of modes
	
	// Measurement results
	double	m_defocus;
	double	m_cylinder;
	double	m_axis;
	double	m_strehl_ratio;
	double	m_rms;

	// System Parameters
	double	m_pupil_size_zernike;		//Size of pupil that Zernike coefficients define. NOTE: You can define the aberrations
	double	m_pupil_field_size_microns; // Size of pupil field in mm (use a large field to magnify the PSF)
	double	m_psf_size_pix;				//  Number of pixels over which PSF is calculated (set in Zernikephase subroutine)
	BOOL	m_SystemInitDone;

	// Internal computations for Zernikes, RMS, WAVE, PSF, MTF
	void	Calculate_Zernike_Coeffs_New(double pupildia);
	void	Zwave_Mahajan_new(double*);
	void	Zwave_Matrix(void);
	void	Zphase_Mahajan(double *phasemapR, double *phasemapI, unsigned long N);
	double	AlternatePlane(correction_plane_enum plane,double refparam);
	CStringA FormatNthOrder(int start, int order);

	//Computing Wave based on Southwell method
	double *m_dMatrixD;  
	double *m_dMatrixG;  
	double *m_dPhiReconM;
	bool m_bPhiReconReady;
	double *m_dWaveV;
	double	*m_PreCorrectionVoltages;

};

#endif // !defined(AFX_OPTCALC_H__4B43F241_6B78_11D6_8008_006008A334DD__INCLUDED_)
