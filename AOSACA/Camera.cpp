#include "StdAfx.h"
#include "AOSACA.h"
#include "Camera.h"
#include <sstream>
#include <fstream>
#include "utils\FreeImage\FreeImage.h"

//PTGrey Camera library initialization
#pragma comment(lib, "delayimp")
#pragma comment(lib, "utils\\FreeImage\\FreeImage")
//Optivec library headers
#include "VecALL.h"
#include "MatALL.h"

using namespace std;
extern AOSACAParams	*g_AOSACAParams;

const Mode k_fmt7Mode = MODE_1;
const FlyCapture2::PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;

CCamera::CCamera(CAOSACADlg *parent)
{
	m_pParent = parent;
	m_pImgBuff = NULL;
	m_pBkgndBuff = NULL;
	m_bSubBkgndinLive = FALSE;
	
	if (g_AOSACAParams->g_bCamReady = Camera_Initialization())	
	{ // Set camera thread and its events
		g_AOSACAParams->g_ehCamLive = CreateEventA(NULL, TRUE, FALSE, "WFS_CAM_LIVE_EVENT");
		g_AOSACAParams->g_ehCamSnap = CreateEventA(NULL, FALSE, FALSE, "WFS_CAM_SNAP_EVENT");
		g_AOSACAParams->g_ehCamNewFrame = CreateEventA(NULL, FALSE, FALSE, "WFS_CAM_NEW_FRAME_EVENT");
		m_ehCamThreadClose = CreateEventA(NULL, FALSE, FALSE, "WFS_CAM_THREAD_CLOSE_EVENT");
		m_ehCamThreadShutdown = CreateEventA(NULL, FALSE, FALSE, "WFS_CAM_THREAD_SHUTDOWN_EVENT");

		thd_handle = CreateThread(NULL, 0, CamThread, this, 0, &thdid_handle);
		SetThreadPriority(thd_handle, THREAD_PRIORITY_HIGHEST);
	}
}

CCamera::~CCamera(void)
{
	if (g_AOSACAParams->g_bCamReady)
	{
		if (g_AOSACAParams->g_frame_mode == LIVESHOW) 
			ResetEvent(g_AOSACAParams->g_ehCamLive);			
		SetEvent(m_ehCamThreadClose);	
		::WaitForSingleObject(m_ehCamThreadShutdown, g_AOSACAParams->EXPOSURE_MS<<2);				
		if (m_pImgBuff != NULL)
			delete [] m_pImgBuff;
		if (m_pBkgndBuff != NULL)
			delete [] m_pBkgndBuff;
		// Stop capturing images
		m_FCcam.StopCapture();
		// Power OFF the camera
	//	m_FCcam.WriteRegister(0x610, 0);
		// Disconnect the camera
		m_FCcam.Disconnect();
		CloseHandle(g_AOSACAParams->g_ehCamNewFrame);
		CloseHandle(g_AOSACAParams->g_ehCamSnap);
		CloseHandle(m_ehCamThreadClose);
		CloseHandle(m_ehCamThreadShutdown);
	}
	g_AOSACAParams->g_bCamReady = false;	
}

bool CCamera::Camera_Initialization()
{
	CStringA filepath;
	filepath = g_AOSACAParams->g_stAppHomePath+_T("CameraInfo.txt");
	ofstream caminfo(filepath);	

	// Get library information
	FC2Version fc2Version;
    Utilities::GetLibraryVersion( &fc2Version );
	caminfo<<"FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build<<endl;
	caminfo<<"Application build date: " << __DATE__ << " " << __TIME__<<endl<<endl;

	Error error;
	// Get camera information
    unsigned int numCameras;
    error = m_FCbusMgr.GetNumOfCameras(&numCameras);

	if (!numCameras)
	{// no camera found
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "..Camera NOT FOUND.. \n Closing AOSACA";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}
	
    error = m_FCbusMgr.GetCameraFromSerialNumber(g_AOSACAParams->CAM_SERIAL_NO, &m_FCguid);
	if (error != PGRERROR_OK)
    {
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff.Format(L"..Camera with Serial#%d NOT FOUND.. \n Closing AOSACA", g_AOSACAParams->CAM_SERIAL_NO);		
		g_AOSACAParams->ShowError(MB_ICONERROR);
        return false;
    }

    // Connect to camera
    error = m_FCcam.Connect(&m_FCguid);
    if (error != PGRERROR_OK)
    {
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "..Camera Bus error.. \n Closing AOSACA";
		g_AOSACAParams->ShowError(MB_ICONERROR);
        return false;
    }

	// Power ON the camera
//	m_FCcam.WriteRegister(0x610, 0x80000000);

    // Get the camera information
    CameraInfo camInfo;
    error = m_FCcam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "..Unable to read Camera Information.. \n Closing AOSACA";
		g_AOSACAParams->ShowError(MB_ICONERROR);
        return false;
    }

    // Print Camera Info
	caminfo << "*** CAMERA INFORMATION ***" << endl;
	caminfo << "Serial number -" << camInfo.serialNumber << endl;
    caminfo << "Camera model - " << camInfo.modelName << endl;
    caminfo << "Camera vendor - " << camInfo.vendorName << endl;
    caminfo << "Sensor - ICX825" << endl;
    caminfo << "Max Resolution - " << camInfo.sensorResolution << endl;
    caminfo << "Firmware version - " << camInfo.firmwareVersion << endl;
    caminfo << "Firmware build time - " <<camInfo.firmwareBuildTime << endl << endl;

	// Query for available Format7 modes
    Format7Info fmt7Info;
    bool supported;
    fmt7Info.mode = k_fmt7Mode;
    error = m_FCcam.GetFormat7Info( &fmt7Info, &supported );
    if (error != PGRERROR_OK)
    {
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "..Required Image format not supported by the Camera.. \n Closing AOSACA";
		g_AOSACAParams->ShowError(MB_ICONERROR);
        return false;
    }

	// Print image format details
	caminfo << "*** IMAGE FORMAT INFORMATION ***" << endl;
	caminfo << "Max image pixels: (" << fmt7Info.maxWidth << ", " << fmt7Info.maxHeight << ")" << endl;
	caminfo << "Image Unit size: (" << fmt7Info.imageHStepSize << ", " << fmt7Info.imageVStepSize << ")" << endl;
    caminfo << "Offset Unit size: (" << fmt7Info.offsetHStepSize << ", " << fmt7Info.offsetVStepSize << ")" << endl;
	caminfo << "Pixel format bitfield: MONO8" << endl << endl;

	if ( (k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0 )
    {// Pixel format not supported!
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "..Required Pixel format not supported by the Camera.. \n Closing AOSACA";
		g_AOSACAParams->ShowError(MB_ICONERROR);
        return false;
    }
    
    Format7ImageSettings fmt7ImageSettings;
    fmt7ImageSettings.mode = k_fmt7Mode;
    fmt7ImageSettings.offsetX = 90;
    fmt7ImageSettings.offsetY = 2;
    fmt7ImageSettings.width = g_AOSACAParams->IMAGE_WIDTH_PIX;
    fmt7ImageSettings.height = g_AOSACAParams->IMAGE_HEIGHT_PIX;
    fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;
	
	bool valid;
    Format7PacketInfo fmt7PacketInfo;

    // Validate the settings to make sure that they are valid
    error = m_FCcam.ValidateFormat7Settings(
        &fmt7ImageSettings,
        &valid,
        &fmt7PacketInfo );   

    if ( !valid )
    {// Settings are not valid
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "Format settings are not valid for Camera \n Closing AOSACA";
		g_AOSACAParams->ShowError(MB_ICONERROR);
        return false;
    }
	// Set the settings to the camera
    error = m_FCcam.SetFormat7Configuration(
        &fmt7ImageSettings,
        fmt7PacketInfo.recommendedBytesPerPacket );

	// Print image format details
	caminfo << "*** SET IMAGE FORMAT INFORMATION ***" << endl;
	caminfo << "Image resolution: (" <<  fmt7ImageSettings.width << ", " << fmt7ImageSettings.height << ")" << endl;
    caminfo << "Offset Unit size: (" << fmt7ImageSettings.offsetX << ", " << fmt7ImageSettings.offsetY << ")" << endl;

	//Read & Set triggermode settings from camera
	TriggerMode  pTriggerMode;
	error = m_FCcam.GetTriggerMode(&pTriggerMode); 
	pTriggerMode.onOff = true;
	pTriggerMode.mode = 0;
	error = m_FCcam.SetTriggerMode(&pTriggerMode);

	//Read & Set exposure property from camera
	Property exposure;
	exposure.type = AUTO_EXPOSURE;
	error = m_FCcam.GetProperty( &exposure );    
	exposure.absControl = true;
	exposure.autoManualMode = false;
	exposure.onOff = false;
	error = m_FCcam.SetProperty( &exposure );

	//Read & Set sharpness property from camera
	Property sharp;
	sharp.type = SHARPNESS;
	error = m_FCcam.GetProperty( &sharp );
	sharp.autoManualMode = false;
	sharp.onOff = false;
	error = m_FCcam.SetProperty( &sharp );

	//Read & Set gammaCorrection property from camera
	Property gammaCorrection;
	gammaCorrection.type = GAMMA;
	error = m_FCcam.GetProperty( &gammaCorrection );
	gammaCorrection.absControl = true;
	gammaCorrection.onOff = false;
	error = m_FCcam.SetProperty( &gammaCorrection );

	//Read & Set shutterTime property from camera
	Property shutterTime;
	shutterTime.type = SHUTTER;
	error = m_FCcam.GetProperty( &shutterTime );
	shutterTime.absControl = true;
	shutterTime.autoManualMode = false;
	error = m_FCcam.SetProperty( &shutterTime );
	shutterTime.absValue = g_AOSACAParams->EXPOSURE_MS;
	error = m_FCcam.SetProperty( &shutterTime );

	//Read & Set digitalGain property from camera
	Property digitalGain;
	digitalGain.type = GAIN;
	error = m_FCcam.GetProperty( &digitalGain );
	digitalGain.absControl = true;
	digitalGain.autoManualMode = false;
	error = m_FCcam.SetProperty( &digitalGain );
	digitalGain.absValue = (float)g_AOSACAParams->CAMGAIN_DB;
	error = m_FCcam.SetProperty( &digitalGain );

	//Read & Set framespersec property from camera
	Property framespersec;
	framespersec.type = FRAME_RATE;
	error = m_FCcam.GetProperty( &framespersec );
	framespersec.absControl = true;
	framespersec.autoManualMode = false;
	framespersec.onOff = false;
	error = m_FCcam.SetProperty( &framespersec );

	// Start capturing images
    error = m_FCcam.StartCapture();
    if (error != PGRERROR_OK)
    {
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "Unable to start capture on Camera \n Closing AOSACA";
		g_AOSACAParams->ShowError(MB_ICONERROR);
        return false;
    }

	m_nFrameSizeInBytes = g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX*sizeof(unsigned char);	
	m_pImgBuff = new BYTE[m_nFrameSizeInBytes];
	ZeroMemory(m_pImgBuff, m_nFrameSizeInBytes);
	m_pBkgndBuff = new BYTE[m_nFrameSizeInBytes];
	ZeroMemory(m_pBkgndBuff, m_nFrameSizeInBytes);
	
	//Load background image into buffer if available
	CStringA filename = "utils\\background.bimg";	
	ifstream bkgndFile;
	bkgndFile.open(filename, ios::in | ios::binary);
	if (!bkgndFile.read ((char*)m_pBkgndBuff, m_nFrameSizeInBytes))
		ZeroMemory(m_pBkgndBuff, m_nFrameSizeInBytes);
	bkgndFile.close();

	return true;
}

bool CCamera::UpdateExposureTime(void)
{
	Property shutterTime;
	Error error;
	shutterTime.type = SHUTTER;
	error = m_FCcam.GetProperty( &shutterTime );
	shutterTime.absValue = g_AOSACAParams->EXPOSURE_MS;
	error = m_FCcam.SetProperty( &shutterTime );
	if (error != PGRERROR_OK)
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "Unable to update exposure time on Camera";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}
	return true;
}

bool CCamera::UpdateCameraGain(void)
{
	Property digitalGain;
	Error error;
	digitalGain.type = GAIN;
	error = m_FCcam.GetProperty( &digitalGain );
	digitalGain.absValue = (float)g_AOSACAParams->CAMGAIN_DB;
	error = m_FCcam.SetProperty( &digitalGain );
	if (error != PGRERROR_OK)
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "Unable to update Camera Gain";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}
	return true;
}

void CCamera::SubtractBackground(void)
{	
	long  i;
	BYTE *buf_f_ptr;
	BYTE *buf_b_ptr;

	// Read the current spots on the screen into buf_f
	buf_f_ptr = g_AOSACAParams->g_pImgBuffPrc;
	buf_b_ptr = m_pBkgndBuff;

	for ( i=0; i<m_nFrameSizeInBytes; i++) 
	{
		if(*(buf_f_ptr) >= *(buf_b_ptr))
			*(buf_f_ptr) = *(buf_f_ptr)- *(buf_b_ptr);
		else 
			*(buf_f_ptr) = 0;

		buf_f_ptr++;
		buf_b_ptr++;
	}
}

//*************************************************************************************************
DWORD WINAPI CCamera::CamThread( LPVOID pParam )
{
	CCamera *parent = (CCamera *)pParam;
	HANDLE hCamEvents[3];
	bool bRunCamThread = true;
	CString text;
	double telapse;
	FlyCapture2::Image* rawImage;
	Error error;
	unsigned char* pImagePixels = NULL;
	rawImage = new FlyCapture2::Image(g_AOSACAParams->IMAGE_HEIGHT_PIX, 
										g_AOSACAParams->IMAGE_WIDTH_PIX, 
										g_AOSACAParams->IMAGE_WIDTH_PIX, 
										parent->m_pImgBuff, 
										g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX, 
										PIXEL_FORMAT_MONO8, FlyCapture2::NONE);
	
	hCamEvents[0] = parent->m_ehCamThreadClose; 
	hCamEvents[1] = g_AOSACAParams->g_ehCamLive;
	hCamEvents[2] = g_AOSACAParams->g_ehCamSnap;

	LARGE_INTEGER time1;
	LARGE_INTEGER time2;
	
	do
	{
		switch (::WaitForMultipleObjects(3, hCamEvents, FALSE, INFINITE)) 
		{
		case WAIT_OBJECT_0:
			bRunCamThread = false;			
			delete rawImage;
			break;
		case WAIT_OBJECT_0+1:			
			g_AOSACAParams->g_frame_mode = LIVESHOW;
			//Get current CPU clock time
			QueryPerformanceCounter(&time1);
			//Retrieve an image
			error = parent->m_FCcam.FireSoftwareTrigger();
			error = parent->m_FCcam.RetrieveBuffer( rawImage );
			pImagePixels = rawImage->GetData();
			memcpy(g_AOSACAParams->g_pImgBuffPrc, pImagePixels, parent->m_nFrameSizeInBytes);
			parent->m_bSubBkgndinLive?parent->SubtractBackground():0;
			//Send window update signal to WFS Image dialog	
			parent->m_pParent->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WFSIMG_WINDOW);
			//Get current CPU clock time
			QueryPerformanceCounter(&time2);			
			telapse = (double)(g_AOSACAParams->m_lnFreq/(time2.QuadPart-time1.QuadPart));//ticks passed
			text.Format(_T("%2.2f"),telapse);
			(parent->m_pParent)->SetDlgItemText(IDE_FREQUENCY, text);
			break;
		case WAIT_OBJECT_0+2:
			g_AOSACAParams->g_frame_mode = SNAPSHOT;
			error = parent->m_FCcam.FireSoftwareTrigger();
			error = parent->m_FCcam.RetrieveBuffer( rawImage );
			pImagePixels = rawImage->GetData();
			memcpy(g_AOSACAParams->g_pImgBuffPrc, pImagePixels, parent->m_nFrameSizeInBytes);
			g_AOSACAParams->g_bSubstractBkGnd?parent->SubtractBackground():0;	
			SetEvent(g_AOSACAParams->g_ehCamNewFrame);
			break;
		default:
			break;
		}
	}while (bRunCamThread);

	SetEvent(parent->m_ehCamThreadShutdown);

	return 0;
}

BOOL CCamera::SaveImage(CStringA filename, bool bkgnd)
{
	BOOL result = FALSE;
	g_AOSACAParams->g_frame_mode = WAIT;	
	if (bkgnd)
		memcpy(m_pBkgndBuff, g_AOSACAParams->g_pImgBuffPrc, g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX*sizeof(BYTE));

	FlyCapture2::Image saveImage(g_AOSACAParams->IMAGE_HEIGHT_PIX, 
										g_AOSACAParams->IMAGE_WIDTH_PIX, 
										g_AOSACAParams->IMAGE_WIDTH_PIX, 
										g_AOSACAParams->g_pImgBuffPrc, 
										g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX, 
										PIXEL_FORMAT_MONO8, FlyCapture2::NONE);
	// now, we can create a FIBITMAP
	FIBITMAP *dib;
	dib = FreeImage_ConvertFromRawBits(g_AOSACAParams->g_pImgBuffPrc, g_AOSACAParams->IMAGE_WIDTH_PIX, g_AOSACAParams->IMAGE_HEIGHT_PIX, g_AOSACAParams->IMAGE_HEIGHT_PIX, 8, 0, 0, 0, true);

	result = FreeImage_Save(FIF_TIFF, dib, filename, TIFF_NONE);
	// unload the FIBITMAP 
	FreeImage_Unload(dib);

	if (bkgnd)
	{
		memcpy(m_pBkgndBuff, g_AOSACAParams->g_pImgBuffPrc, g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX*sizeof(BYTE));
		CStringA filename = "utils\\background.bimg";
		ofstream bkgndFile;
		bkgndFile.open(filename, ios::out | ios::binary);
		bkgndFile.write((const char*)m_pBkgndBuff, g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX);
		bkgndFile.close();
		result = true;
	}
	return result;
}