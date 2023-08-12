#pragma once


// CAOSACAParamsDlg dialog

class CAOSACAParamsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAOSACAParamsDlg)
private:
	//Dialog Edit boxes
	CMyEditCtrl	m_eWFmapWidth;
	CMyEditCtrl m_ePSFmapWidth;
	CMyEditCtrl	m_eWavelength;
	CMyEditCtrl	m_eLensletDia;
	CMyEditCtrl	m_eLensletFL;
	CMyEditCtrl	m_eLensletGrid;
	CMyEditCtrl	m_eMagnification;
	CMyEditCtrl	m_ePupilSizemax;
	CMyEditCtrl	m_eSearchboxSize;
	CMyEditCtrl	m_eSearchboxStep;
	CMyEditCtrl	m_eCentMaxPass1;
	CMyEditCtrl	m_eCentMaxPass2;
	CMyEditCtrl	m_eCentMaxPass3;
	CMyEditCtrl	m_eCamPitch;
	CMyEditCtrl	m_eDMSerial;
	CMyEditCtrl	m_eDMDefPMG;
	CMyEditCtrl	m_eDMmaxDef;
	CMyEditCtrl	m_ePMGIter;
	CMyEditCtrl	m_eAOCycles;

	double	m_dWavelength;
	double	m_dLensletDia;
	double	m_dLensletFL;
	double	m_dMagnification;
	double	m_dCamPitch;
	double	m_dDMdefPMG;
	double	m_dDMmaxDef;

	short	m_shWFmapWidth;
	short	m_shPSFmapWidth;
	short	m_shLensletGrid;
	short	m_shPupilSizemax;
	short	m_shSearchboxSize;
	short	m_shSearchboxStep;
	short	m_shCentMaxPass1;
	short	m_shCentMaxPass2;
	short	m_shCentMaxPass3;
	short	m_shPMGIter;
	short	m_shAOCycles;

	CString	m_stDMSerial;
	CString m_stPokeMatrixFile;

public:
	CAOSACAParamsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAOSACAParamsDlg();
	bool	m_bAppUpdate[7];

// Dialog Data
	enum { IDD = IDD_PARAMETERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel();
	int m_WFSImgColor;
	afx_msg void OnBnClickedParametersAoloopInfmatSel();
};
