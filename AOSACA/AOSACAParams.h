#include <iostream>
#include "windows.h"
#include <atltypes.h>
#include <atlstr.h>
#include <fstream>

// Colors for dialog controls
#define BACKGROUNDCOLOR RGB(0,0,0)
#define EDITBACKGROUNDCOLOR RGB(255,255,255)
#define FOREGROUNDCOLOR RGB(96,122,134)
#define EDITTEXTCOLOR RGB(0,0,0)
#define TEXTCOLOR RGB(255,119,34)

#define X_OFFSET_DM		8		// Location of the DM map to begin painting for actuators along x direction
#define Y_OFFSET_DM		20		// Same as above in the y direction
#define ACTR_BOX_SIZE	19		// in pixels
#define BOX_GAP			2		// in pixels
#define X_OFFSET_WFM	10		// Location of the wave front image to begin painting for actuators along x direction
#define Y_OFFSET_WFM	10		// Same as above in the y direction
#define X_OFFSET_PSF	10		// Location of the PSF to begin painting for actuators along x direction
#define Y_OFFSET_PSF	10		// Same as above in the y direction
#define X_OFFSET_SVM	10		// Location of the Slope Vector Map to begin painting for actuators along x direction
#define Y_OFFSET_SVM	10		// Same as above in the y direction
#define	SVMAP_SCALE		0.65	// Slope Vector map scaling
#define PLOT_XOFFSET	1	//  1 pixel of the plotting area from the left and right are not used
#define PLOT_YOFFSET	0	//  1 pixel of the plotting area from the top and bottom are not used
#define PLOT_GAP		21
#define PLOT_BARWIDTH	10

#define ARROW	1
#define CROSS	2

// Enumerated constants
typedef enum  {ABS_ZERO_BIT=0, MAX_BIT=1, MIN_BIT=2, BIAS_BIT=3, ALLIGN_BIT=4, COMP_BIT=5} mirror_state;
// Camera frame mode
typedef enum {WAIT, LIVESHOW, SNAPSHOT } HFRAME_MODE;
// Control panel active state
typedef enum {UNDEF, LIVE, SNAP, SUBBKGND, SELCENT, CENTROIDS, OPTPERFORM, AUTO } HLAST_CLICK;

class CDPoint
{
public:
	double x;
	double y;

	CDPoint()
	{
		x = 0.;
		y = 0.;
	}
};

class AOSACAParams
{
public:
	long int m_lnFreq;
	// Application parameters	
	CString	g_stAppHomePath;
	CString g_stAppParamsFile;	
	CString g_stAppErrBuff;
	// System Params
	short	WAVE_WIDTH_PIX;
	short	PSF_MTF_WIDTH_PIX;
	double	WAVELENGTH_MICRONS;
	double	LENSLET_DIAMETER_MICRONS;
	double	LENSLET_FOCAL_LENGTH_MICRONS;
	double	MAGNIFICATION;
	short	LENSLETGRID;
	// User parameters;	
	double	PUPIL_FIT_SIZE_MICRONS;
	short	THRESHOLD;
	short	SEARCHBOX_SIZE;
	short	CENT_MAX_PASS1;
	short	CENT_MAX_PASS2;
	short	CENT_MAX_PASS3;
	CDPoint	PUPIL_CENTER;	
	// Camera Parameters
	short	IMAGE_WIDTH_PIX;
	short	IMAGE_HEIGHT_PIX;
	bool	USECOLORMAP;
	unsigned int CAM_SERIAL_NO;
	short	EXPOSURE_MS;
	BYTE	CAMGAIN_DB;
	double	MICRONS_PER_PIXEL;
	// DM Parameters
	CString	DM_SERIAL_NO;
	double	DM_POKE_MAT_MAX_DEF;
	double	DM_MAX_DEFLECTION;
	short	DMGRID;
	short	NUMACTS;
	//AO Loop Parameters
	short	DM_POKE_MAT_ITER;
	short	CYCLES_BELOW_SETCENTROIDNO;
	CString POKE_MATRIX_FILE;

	// Application runtime parameters
	HANDLE		g_ehCamLive;			// event to trigger conitnuous frame capture
	HANDLE		g_ehCamSnap;			// event to trigger new frame sanpshot
	HANDLE		g_ehCamNewFrame;		// event to indicate a new frame is ready to be processed
	HFRAME_MODE	g_frame_mode;			// flag to indicate whether liveshow, snapshot or none
	HLAST_CLICK	g_last_click;	
	HLAST_CLICK	g_cur_click;	
	HANDLE		g_ehPokeThread;			// event to trigger poke matrix generation loop
	HANDLE		g_ehCLoopThread;		// event to trigger AOSACA closed loop
	HANDLE		g_ehCLoopShutdown;		// event to indicate termination of closed loop 
	HANDLE		g_ehAOSACALoopParamsReady; // event to indicate whether initializing loop parameters is done
	bool		g_bCamReady;			// flag to indicate whether a camera is active
	bool		g_bDMReady;				// flag to indicate whether a DM is active
	bool		g_bSubstractBkGnd;		// flag to indicate whether to substract brackground from current frame
	bool		g_bAOSACALoop;			// flag to initiate AOSACA loop
	bool		g_bAOSACALoopReady;		// flag to check whether all the parameters are initialized before continuing the loop
	bool		g_bUpdateCenter;		// flag to indicate whether the user wants to update the center
	bool		g_bAutoMeasureON;		// flag to initiate/indicate conitnuous auto measure routine (measures refraction values/WFM/PSF/SLV)
	bool		g_bCPreCorrApply;		// flag to indicate whether to apply pre-correction (refractive correction) to the mirror
	bool		g_bControlON;			// flag to toggle closed loop algorithm
	bool		g_bResetMirror;
	bool		g_bAutoMeasure;			// flag to toggle auto measuring optical perofmance (centroids, refraction values, wave front map, PSF, Slope vectors)
	bool		g_bPokeMatGeneration;	// flag to trigger poke matrix generation
	bool		g_bDrawCentroids;		// flag to indicate whether to draw centroids on WFS image
	bool		g_bDrawSlopeVectors;	// flag to indicate whether to draw slope vectors on WFS Image
	bool		g_bCloseLoopThread;		// flag to indicate whether the closed loop thread is active	
	bool		g_bUpdateDMVMap;		// flag to indicate whether to update DM Map while in closed loop
	BYTE		*g_pImgBuffPrc;			// buffer to store the current image ready for analysis in camera class
	BYTE		*g_pLocalImgBuff;		// buffer to store image for analysis across the applicaiton
	BYTE		*g_bActuatorMask;		// logical mask to identify actuators position in a square grid
	double		*g_DMDeflections;	
	double		*g_DMBiasDeflections;

	RGBQUAD		summercmap[256];
	RGBQUAD		greyscale[256];
	COLORREF	dmcolormap[512];
	CBrush		m_bkgndbrush;

	// Log file
	std::wofstream g_Logfile;

public:
	AOSACAParams()
	{
		//Get application root path
		CString g_stAppParamsFile, strCurrentDir;
		TCHAR	szAppPath[80];	
		::GetModuleFileName(0, szAppPath, sizeof(szAppPath) - 1);	
		// Extract directory
		strCurrentDir = szAppPath;
		strCurrentDir = strCurrentDir.Left(strCurrentDir.ReverseFind('\\'));
		g_stAppParamsFile = strCurrentDir.Right(strCurrentDir.Find('\\'));	
		if (g_stAppParamsFile.Find(_T("ug")) >= 0 || g_stAppParamsFile.Find(_T("se")) >= 0)
			strCurrentDir = strCurrentDir.Left(strCurrentDir.ReverseFind('\\'));
		g_stAppParamsFile = strCurrentDir.Right(strCurrentDir.Find('\\'));
		if (g_stAppParamsFile.Find(_T("64")) >= 0)
			strCurrentDir = strCurrentDir.Left(strCurrentDir.ReverseFind('\\'));
		g_stAppHomePath = strCurrentDir;
		g_stAppHomePath += "\\";
		DM_SERIAL_NO.Empty();
		// Load the system & user parameters from the ini file
		LoadConfigFile();
		// Initialize application runtime buffers
		LARGE_INTEGER ticks;
		QueryPerformanceFrequency(&ticks);
		m_lnFreq = (long)ticks.QuadPart;
		// Wave front sensor image related buffers 
		g_pImgBuffPrc = NULL;
		g_pImgBuffPrc = new BYTE[IMAGE_WIDTH_PIX*IMAGE_HEIGHT_PIX];
		ZeroMemory(g_pImgBuffPrc, IMAGE_WIDTH_PIX*IMAGE_HEIGHT_PIX*sizeof(BYTE));
		g_pLocalImgBuff = NULL;
		g_pLocalImgBuff = new BYTE[IMAGE_WIDTH_PIX*IMAGE_HEIGHT_PIX];
		ZeroMemory(g_pLocalImgBuff, IMAGE_WIDTH_PIX*IMAGE_HEIGHT_PIX*sizeof(BYTE));
		g_bActuatorMask = NULL;
		g_bActuatorMask = new BYTE[DMGRID*DMGRID];
		ZeroMemory(g_bActuatorMask, DMGRID*DMGRID*sizeof(BYTE));
		g_DMDeflections = NULL;
		g_DMDeflections = new double[NUMACTS];
		ZeroMemory(g_DMDeflections, NUMACTS*sizeof(double));
		g_DMBiasDeflections = NULL;
		g_DMBiasDeflections = new double[NUMACTS];
		ZeroMemory(g_DMBiasDeflections, NUMACTS*sizeof(double));

		if (g_pImgBuffPrc == NULL || g_pLocalImgBuff == NULL)
		{
			g_stAppErrBuff.Empty();
			g_stAppErrBuff = _T("Unable to initialize image buffers");
			ShowError(MB_ICONWARNING);
		}

		std::ifstream infile;
		unsigned int i = 0;
		//Load DM flatness offsets file
		if (!DM_SERIAL_NO.IsEmpty())
		{
			CString filename;
			filename.Format(L"utils\\%s.txt",DM_SERIAL_NO);
			infile.open(filename, std::ifstream::in);
			if (infile != NULL)
			{
				double deflection = 0.;
				while (infile >> deflection)
				{
					g_DMBiasDeflections[i++] = deflection;
				}
				infile.close();
				memcpy(g_DMDeflections, g_DMBiasDeflections, NUMACTS*sizeof(double));
			} else {
				g_stAppErrBuff.Empty();
				g_stAppErrBuff.Format(L"Unable to locate %s file, DM bias voltages initialized to ZERO",filename);
				ShowError(MB_ICONWARNING);
			}
		}

		g_bCamReady = false;
		g_bDMReady = false;
		g_bSubstractBkGnd = false;
		g_frame_mode = WAIT;
		g_last_click = UNDEF;
		g_cur_click = UNDEF;
		g_bAOSACALoop = false;
		g_bAOSACALoopReady = false;
		g_bUpdateCenter = false;
		g_bAutoMeasureON = false;
		g_bCPreCorrApply = false;
		g_bControlON = false;
		g_bAutoMeasure = false;
		g_bResetMirror = false;
		g_bPokeMatGeneration = false;
		g_bDrawCentroids = false;
		g_bDrawSlopeVectors = false;
		g_bCloseLoopThread = false;
		g_bUpdateDMVMap	= false;

		g_ehCLoopShutdown	= CreateEvent(NULL, FALSE, FALSE, _T("CLOSED_LOOP_SHUTDOWN_EVENT"));
		g_ehPokeThread		= CreateEvent(NULL, FALSE, FALSE, _T("POKE_MAT_RUN_EVENT"));
		g_ehCLoopThread		= CreateEvent(NULL, TRUE, FALSE, _T("CLOSED_LOOP_RUN_EVENT"));
		g_ehAOSACALoopParamsReady = CreateEvent(NULL, FALSE, FALSE, _T("AOSACA_LOOP_PARAMS_READY"));

		// define color palettes
		summercmap[0].rgbBlue = 102; summercmap[0].rgbGreen = 128; summercmap[0].rgbRed = 0; summercmap[0].rgbReserved = 0;
		greyscale[0].rgbBlue = 0; greyscale[0].rgbGreen = 0; greyscale[0].rgbRed = 0; greyscale[0].rgbReserved = 0;		
		for (int color_index = 1; color_index < 256; color_index++)
		{
			greyscale[color_index].rgbBlue = color_index;
			greyscale[color_index].rgbGreen = color_index;
			greyscale[color_index].rgbRed = color_index;
			greyscale[color_index].rgbReserved = 0;
		}
		// Summer colormap
		infile.open("utils\\WFSColorMap.map", std::ifstream::in);
		int r, g, b;
		i=0;
		while (infile >> r >> g >> b)
		{
			summercmap[i].rgbBlue = b;
			summercmap[i].rgbGreen = g;
			summercmap[i].rgbRed = r;
			summercmap[i].rgbReserved = 0;
			i++;
		}
		infile.close();
		// DM colormap
		infile.open("utils\\DMColorMap.map", std::ifstream::in);
		i=0;
		while (infile >> r >> g >> b)
		{
			dmcolormap[i] = RGB(r, g, b);
			i++;
		}
		infile.close();

		m_bkgndbrush.CreateSolidBrush(BACKGROUNDCOLOR);

		FillMemory(g_bActuatorMask, DMGRID*DMGRID*sizeof(BYTE), 1);
		b = 3;
		for (g=0; g<3; g++)
		{
			for (r=0; r<b; r++)
			{
				g_bActuatorMask[g*DMGRID+r] = 0;
				g_bActuatorMask[g*DMGRID+(DMGRID-1-r)] = 0;
				g_bActuatorMask[(DMGRID-1-g)*DMGRID+r] = 0;
				g_bActuatorMask[(DMGRID-1-g)*DMGRID+(DMGRID-1-r)] = 0;
			}
			b--;
		}

	//	g_Logfile.open("logfile.txt", std::wofstream::out);
	}

	~AOSACAParams()
	{
		CloseHandle(g_ehCLoopThread);
		CloseHandle(g_ehPokeThread);
		CloseHandle(g_ehCLoopShutdown);
		
		if (g_DMBiasDeflections != NULL)
			delete [] g_DMBiasDeflections;
		if (g_DMDeflections != NULL)
			delete [] g_DMDeflections;
		if (g_bActuatorMask != NULL)
			delete [] g_bActuatorMask;	
		if (g_pImgBuffPrc != NULL)
			delete [] g_pImgBuffPrc;	
		if (g_pLocalImgBuff != NULL)
			delete [] g_pLocalImgBuff;	
		
		SaveConfigFile();
	//	g_Logfile.close();
	}

	void LoadConfigFile()
	{
		g_stAppParamsFile = g_stAppHomePath + _T("AOSACA.ini");

		//Check for params file existence
		CFile sysFile;
		CFileException sysFileError;
		BOOL sysFileOK;
		sysFileOK=sysFile.Open(g_stAppParamsFile, CFile::modeRead | CFile::shareDenyWrite, &sysFileError);

		if (sysFileOK) 
		{
			sysFile.Close();			
			TCHAR     temp[80];
			// System parameters
			::GetPrivateProfileString(_T("SystemParams"), _T("WAVE_WIDTH_PIX"), _T(""), temp, 40, g_stAppParamsFile);
			WAVE_WIDTH_PIX = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("SystemParams"), _T("PSF_MTF_WIDTH_PIX"), _T(""), temp, 40, g_stAppParamsFile);
			PSF_MTF_WIDTH_PIX = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("SystemParams"), _T("WAVELENGTH_MICRONS"), _T(""), temp, 40, g_stAppParamsFile);
			WAVELENGTH_MICRONS = _tstof(temp);
			::GetPrivateProfileString(_T("SystemParams"), _T("LENSLET_DIAMETER_MICRONS"), _T(""), temp, 40, g_stAppParamsFile);
			LENSLET_DIAMETER_MICRONS = _tstof(temp);
			::GetPrivateProfileString(_T("SystemParams"), _T("LENSLET_FOCAL_LENGTH_MICRONS"), _T(""), temp, 40, g_stAppParamsFile);
			LENSLET_FOCAL_LENGTH_MICRONS = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("SystemParams"), _T("MAGNIFICATION"), _T(""), temp, 40, g_stAppParamsFile);
			MAGNIFICATION = _tstof(temp);
			::GetPrivateProfileString(_T("SystemParams"), _T("LENSLETGRID"), _T(""), temp, 40, g_stAppParamsFile);
			LENSLETGRID = (short)_tstoi(temp);
			// User parameters
			::GetPrivateProfileString(_T("UserParams"), _T("PUPIL_FIT_SIZE_MICRONS"), _T(""), temp, 40, g_stAppParamsFile);
			PUPIL_FIT_SIZE_MICRONS = _tstof(temp);
			::GetPrivateProfileString(_T("UserParams"), _T("THRESHOLD"), _T(""), temp, 40, g_stAppParamsFile);
			THRESHOLD = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("UserParams"), _T("SEARCHBOX_SIZE"), _T(""), temp, 40, g_stAppParamsFile);
			SEARCHBOX_SIZE = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("UserParams"), _T("CENT_MAX_PASS1"), _T(""), temp, 40, g_stAppParamsFile);
			CENT_MAX_PASS1 = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("UserParams"), _T("CENT_MAX_PASS2"), _T(""), temp, 40, g_stAppParamsFile);
			CENT_MAX_PASS2 = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("UserParams"), _T("CENT_MAX_PASS3"), _T(""), temp, 40, g_stAppParamsFile);
			CENT_MAX_PASS3 = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("UserParams"), _T("PUPIL_CENTER_X"), _T(""), temp, 40, g_stAppParamsFile);
			(PUPIL_CENTER).x = _tstof(temp);
			::GetPrivateProfileString(_T("UserParams"), _T("PUPIL_CENTER_Y"), _T(""), temp, 40, g_stAppParamsFile);
			(PUPIL_CENTER).y = _tstof(temp);
			// Camera parameters
			::GetPrivateProfileString(_T("CameraParams"), _T("IMAGE_WIDTH_PIX"), _T(""), temp, 40, g_stAppParamsFile);		
			IMAGE_WIDTH_PIX = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("CameraParams"), _T("IMAGE_HEIGHT_PIX"), _T(""), temp, 40, g_stAppParamsFile);
			IMAGE_HEIGHT_PIX = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("CameraParams"), _T("USECOLORMAP"), _T(""), temp, 40, g_stAppParamsFile);
			USECOLORMAP = ((short)_tstoi(temp))?true:false;	
			::GetPrivateProfileString(_T("CameraParams"), _T("CAM_SERIAL_NO"), _T(""), temp, 40, g_stAppParamsFile);
			CAM_SERIAL_NO = (unsigned int)_tstoi(temp);
			::GetPrivateProfileString(_T("CameraParams"), _T("EXPOSURE_MS"), _T(""), temp, 40, g_stAppParamsFile);
			EXPOSURE_MS	= (short)_tstoi(temp);
			::GetPrivateProfileString(_T("CameraParams"), _T("CAMGAIN_DB"), _T(""), temp, 40, g_stAppParamsFile);
			CAMGAIN_DB = (BYTE)_tstoi(temp);	
			::GetPrivateProfileString(_T("CameraParams"), _T("MICRONS_PER_PIXEL"), _T(""), temp, 40, g_stAppParamsFile);
			MICRONS_PER_PIXEL = _tstof(temp);		
			// DM parameters
			::GetPrivateProfileString(_T("DMParams"), _T("DM_SERIAL_NO"), _T(""), temp, 80, g_stAppParamsFile);
			DM_SERIAL_NO = temp;
			::GetPrivateProfileString(_T("DMParams"), _T("DM_MAX_DEFLECTION"), _T(""), temp, 40, g_stAppParamsFile);
			DM_MAX_DEFLECTION = _tstof(temp);
			::GetPrivateProfileString(_T("DMParams"), _T("DM_POKE_MAT_MAX_DEF"), _T(""), temp, 40, g_stAppParamsFile);
			DM_POKE_MAT_MAX_DEF = _tstof(temp);
			::GetPrivateProfileString(_T("DMParams"), _T("DMGRID"), _T(""), temp, 40, g_stAppParamsFile);
			DMGRID = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("DMParams"), _T("NUMACTS"), _T(""), temp, 40, g_stAppParamsFile);
			NUMACTS = (short)_tstoi(temp);
			::GetPrivateProfileString(_T("LoopParams"), _T("DM_POKE_MAT_ITER"), _T(""), temp, 40, g_stAppParamsFile);
			DM_POKE_MAT_ITER = _tstoi(temp);
			::GetPrivateProfileString(_T("LoopParams"), _T("CYCLES_BELOW_SETCENTROIDNO"), _T(""), temp, 40, g_stAppParamsFile);
			CYCLES_BELOW_SETCENTROIDNO = _tstoi(temp);
			::GetPrivateProfileString(_T("LoopParams"), _T("POKE_MATRIX_FILE"), _T(""), temp, 40, g_stAppParamsFile);
			POKE_MATRIX_FILE = temp;
		}
		else 
		{
			g_stAppErrBuff.Empty();
			g_stAppErrBuff = _T("Application Parameter file not found. Using defaults.");
			ShowError(MB_ICONWARNING);
			// System Params
			WAVE_WIDTH_PIX = 100;
			PSF_MTF_WIDTH_PIX = 64;
			WAVELENGTH_MICRONS = 0.840;
			LENSLET_DIAMETER_MICRONS = 300.;
			LENSLET_FOCAL_LENGTH_MICRONS = 7600.;
			MAGNIFICATION = 1.;
			LENSLETGRID = 21;
			// User parameters;	
			PUPIL_FIT_SIZE_MICRONS = 7200.;
			THRESHOLD = 30;
			SEARCHBOX_SIZE = 20;
			CENT_MAX_PASS1 = 21;
			CENT_MAX_PASS2 = 17;
			CENT_MAX_PASS3 = 13;
			PUPIL_CENTER.x = 128.;
			PUPIL_CENTER.y = 128.;
			// Camera Parameters
			IMAGE_WIDTH_PIX = 512;
			IMAGE_HEIGHT_PIX = 512;
			USECOLORMAP = true;
			EXPOSURE_MS = 30;
			CAMGAIN_DB = 8;
			MICRONS_PER_PIXEL = 12.;
			// DM Parameters
			DM_SERIAL_NO = _T("BAL119");			
			DM_MAX_DEFLECTION = 0.25;
			DM_POKE_MAT_MAX_DEF = 0.1;
			DMGRID = 11;
			NUMACTS = 97;
			// AO Loop Parameters
			DM_POKE_MAT_ITER = 3;
			CYCLES_BELOW_SETCENTROIDNO = 30;
		}		
	};

	void SaveConfigFile()
	{
		g_stAppParamsFile = g_stAppHomePath + _T("AOSACA.ini");

		//Check for params file existence
		CFile sysFile;
		CFileException sysFileError;
		BOOL sysFileOK;
		sysFileOK=sysFile.Open(g_stAppParamsFile, CFile::modeRead | CFile::shareDenyWrite, &sysFileError);

		if (sysFileOK) 
		{
			sysFile.Close();
			CString temp;
			// System parameters
			temp.Format(L"%3.4f", WAVELENGTH_MICRONS);			
			::WritePrivateProfileString(_T("SystemParams"), _T("WAVELENGTH_MICRONS"), temp, g_stAppParamsFile);
			temp.Format(L"%3.4f", LENSLET_DIAMETER_MICRONS);
			::WritePrivateProfileString(_T("SystemParams"), _T("LENSLET_DIAMETER_MICRONS"), temp, g_stAppParamsFile);
			temp.Format(L"%3.4f", LENSLET_FOCAL_LENGTH_MICRONS);
			::WritePrivateProfileString(_T("SystemParams"), _T("LENSLET_FOCAL_LENGTH_MICRONS"), temp, g_stAppParamsFile);
			temp.Format(L"%3.4f", MAGNIFICATION);
			::WritePrivateProfileString(_T("SystemParams"), _T("MAGNIFICATION"), temp, g_stAppParamsFile);
			temp.Format(L"%d", LENSLETGRID);
			::WritePrivateProfileString(_T("SystemParams"), _T("LENSLETGRID"), temp, g_stAppParamsFile);
			// User parameters
			temp.Format(L"%4.1f", PUPIL_FIT_SIZE_MICRONS);
			::WritePrivateProfileString(_T("UserParams"), _T("PUPIL_FIT_SIZE_MICRONS"), temp, g_stAppParamsFile);
			temp.Format(L"%d", THRESHOLD);
			::WritePrivateProfileString(_T("UserParams"), _T("THRESHOLD"), temp, g_stAppParamsFile);
			temp.Format(L"%d", SEARCHBOX_SIZE);
			::WritePrivateProfileString(_T("UserParams"), _T("SEARCHBOX_SIZE"), temp, g_stAppParamsFile);
			temp.Format(L"%d", CENT_MAX_PASS1);
			::WritePrivateProfileString(_T("UserParams"), _T("CENT_MAX_PASS1"), temp, g_stAppParamsFile);
			temp.Format(L"%d", CENT_MAX_PASS2);
			::WritePrivateProfileString(_T("UserParams"), _T("CENT_MAX_PASS2"), temp, g_stAppParamsFile);
			temp.Format(L"%d", CENT_MAX_PASS3);
			::WritePrivateProfileString(_T("UserParams"), _T("CENT_MAX_PASS3"), temp, g_stAppParamsFile);
			temp.Format(L"%3.4f", (PUPIL_CENTER).x);
			::WritePrivateProfileString(_T("UserParams"), _T("PUPIL_CENTER_X"), temp, g_stAppParamsFile);
			temp.Format(L"%3.4f", (PUPIL_CENTER).y);
			::WritePrivateProfileString(_T("UserParams"), _T("PUPIL_CENTER_Y"), temp, g_stAppParamsFile);
			// Camera parameters	
			temp.Format(L"%d", USECOLORMAP);
			::WritePrivateProfileString(_T("CameraParams"), _T("USECOLORMAP"), temp, g_stAppParamsFile);		
			temp.Format(L"%d", EXPOSURE_MS);
			::WritePrivateProfileString(_T("CameraParams"), _T("EXPOSURE_MS"), temp, g_stAppParamsFile);
			temp.Format(L"%d", CAMGAIN_DB);
			::WritePrivateProfileString(_T("CameraParams"), _T("CAMGAIN_DB"), temp, g_stAppParamsFile);
			// DM parameters
			::WritePrivateProfileString(_T("DMParams"), _T("DM_SERIAL_NO"), DM_SERIAL_NO, g_stAppParamsFile);
			temp.Format(L"%f", DM_MAX_DEFLECTION);
			::WritePrivateProfileString(_T("DMParams"), _T("DM_MAX_DEFLECTION"), temp, g_stAppParamsFile);
			temp.Format(L"%f", DM_POKE_MAT_MAX_DEF);
			::WritePrivateProfileString(_T("DMParams"), _T("DM_POKE_MAT_MAX_DEF"), temp, g_stAppParamsFile);
			// AO Loop Paramters
			temp.Format(L"%d", DM_POKE_MAT_ITER);
			::WritePrivateProfileString(_T("LoopParams"), _T("DM_POKE_MAT_ITER"), temp, g_stAppParamsFile);
			temp.Format(L"%d", CYCLES_BELOW_SETCENTROIDNO);
			::WritePrivateProfileString(_T("LoopParams"), _T("CYCLES_BELOW_SETCENTROIDNO"), temp, g_stAppParamsFile);
			::WritePrivateProfileString(_T("LoopParams"), _T("POKE_MATRIX_FILE"), POKE_MATRIX_FILE, g_stAppParamsFile);
		}
		else
		{ 
			g_stAppErrBuff.Empty();
			g_stAppErrBuff = _T("Error writing application ini file!");
			ShowError(MB_ICONERROR);
		}
	};

	int ShowError(int msgtype)
	{
		int ret;
		switch (msgtype)
		{
		case MB_ICONQUESTION:
			ret = MessageBox(0, g_stAppErrBuff, _T("AOSACA Confirmation!"), msgtype | MB_YESNO| MB_TOPMOST | MB_SETFOREGROUND);
			break;
		case MB_ICONERROR:
			ret = MessageBox(0, g_stAppErrBuff, _T("AOSACA Error!"), msgtype | MB_OK| MB_TOPMOST | MB_SETFOREGROUND);
			break;		
		case MB_ICONINFORMATION:
			ret = MessageBox(0, g_stAppErrBuff, _T("AOSACA Information!"), msgtype | MB_OK| MB_TOPMOST | MB_SETFOREGROUND);
			break;
		case MB_ICONWARNING:
			ret = MessageBox(0, g_stAppErrBuff, _T("AOSACA Warning!"), msgtype | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
			break;
		}
		return ret;
	}

	CString GetTimeStamp()
	{
		CString timestamp;
		SYSTEMTIME systime;
		GetLocalTime(&systime);
		timestamp.Format(_T("_%i_%i_%i_"), systime.wMinute, systime.wSecond, systime.wMilliseconds);
		return timestamp;
	}
};