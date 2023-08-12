#pragma once

//PTGrey Camera library initialization
#pragma comment(lib, "delayimp")
#pragma comment(lib, "FlyCapture2d_v100")
//Camera specific header file
#include "FlyCapture2.h"
using namespace FlyCapture2;

#include "AOSACADlg.h"

class CCamera
{
private:
	//variables
	CAOSACADlg	*m_pParent;
	HANDLE		m_ehCamThreadClose;
	HANDLE		m_ehCamThreadShutdown;
	DWORD		thdid_handle;			// Camera thread id
	HANDLE		thd_handle;				// Camera thread handle	
	long		m_nFrameSizeInBytes;
	BOOL		m_bSubBkgndinLive;
	// Camera variables
	BusManager	m_FCbusMgr;
	PGRGuid		m_FCguid;
	Camera		m_FCcam;
	BYTE		*m_pImgBuff;			// buffer to store the current active image from camera
	BYTE		*m_pBkgndBuff;			// buffer to store the background image

	//functions
	bool	Camera_Initialization(void);
	void	RotateImageCCW(unsigned char *in);
	void	RotateImageCW(unsigned char *in);
	void	FlipImageUD(unsigned char *in);
	void	FlipImageLR(unsigned char *in);
	void	antiTranspose(unsigned char *in);
	void	Transpose(unsigned char *in);
	static	DWORD WINAPI CamThread(LPVOID pParam);

public:
	CCamera(CAOSACADlg *parent);
	~CCamera(void);
	void	UpdateSubBkgndinLiveflag(BOOL flag) {m_bSubBkgndinLive = flag;};
	bool	UpdateExposureTime(void);
	bool	UpdateCameraGain(void);
	void	SubtractBackground(void);
	BOOL	SaveImage(CStringA filename, bool bkgnd=false);
};

