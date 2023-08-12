#pragma once

#include "MyEditCtrl.h"
#include "afxwin.h"

#define MAX_SAVE_LENGTH_SECS	120

// CControlPanelDlg dialog

class CControlPanelDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CControlPanelDlg)
	
private:
	CAOSACADlg	*m_pParent;
	//Dialog Edit boxes
	CMyEditCtrl	m_eCamExp;
	CMyEditCtrl m_eCamGain;
	CMyEditCtrl	m_eCentThreshold;
	CMyEditCtrl	m_eMinCent;
	CMyEditCtrl	m_eIntGain;
	CMyEditCtrl	m_eModestoZero;
	CMyEditCtrl	m_ePupilSize;
	CMyEditCtrl	m_eFixedDef;
	CMyEditCtrl	m_eDefStep;
	CMyEditCtrl	m_ePreCorrection_Def;
	CMyEditCtrl	m_ePreCorrection_Cyl;
	CMyEditCtrl	m_ePreCorrection_Axis;
	CMyEditCtrl	m_ePrefix;
	CMyEditCtrl	m_eLength;
	// Dialog brushes
	CBrush m_editbkgndbrush;
	CBrush m_frgndbrush;
	
	//Program dialog variables
//	CSockClient	*m_SCClient;
	bool		m_bImaging;
	bool		m_bSubBkgnd;
	short		m_MinCent;
	float		m_fMinCentPercent;
	double		m_IntGain;	
	short		m_ModestoZero;
	double		m_PupilSize;
	double		m_DefocusStepSize;
	double		m_PreDefocusValue;
	double		m_PreCylinderValue;
	double		m_PreAxisValue;
	short		m_SvBkgndExp;
	double		m_Duration;
	char		m_cNetBuffer[1024];
	CString		m_sOldPrefix;
	int			m_nGrabCount;
	bool		*m_bFlagsPrev;
	bool		m_bSvBkgndExpChange;
	bool		m_bFlatMirror;
	CString		m_strdir_path;
	HANDLE		Thread_Dialog_Controls;
	DWORD		ThreadId_Dialog_Controls;	
	HANDLE		m_ehCamSnap;
	HANDLE		m_ehCPanelThreadClose;	
	HANDLE		m_ehCPanelThreadClosed;
	bool		m_bRunning;

	//Funcitons
	void	Get_SnapShot();
	void	send_DM_update_Defocus(double);
	void	send_update_Modestozero(short);
	CString		parse_Outputfile(CString inp_str, CString search_str);
	bool	FindCentroids();
	void	SubBkgnd();
	void	createDirectory();
	void	Update_Defocus(BOOL);
	void	SendImagingCommand(CString);
	void	send_DM_update_PreCorrection();
	void	Update_Pupilsize(double pupilsize);
	static DWORD WINAPI ControlPanelThread(LPVOID param);

public:
	CControlPanelDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CControlPanelDlg();
	double	get_AppliedDefocus() {return m_FixedDefocusValue;};
	void	UpdateCloopmodeStatus();
	void	UpdateMinCent();
	void	FlatDM();
	void	SubbkgndButton();
	void	CentroidsButton();
	void	OptPerfButton();
	void	Update_PupilSize();
	
	HANDLE		m_ehUpdateDialog;
	bool		*m_bFlags;	
	double		m_FixedDefocusValue;

// Dialog Data
	enum { IDD = IDD_CONTROLPANEL };
	BOOL	m_bCSubBkgndinLive;
	BOOL	m_bCPenalizeTilt;
	BOOL	m_bCRenewPmat;	
	BOOL	m_bCAutoMeasure;
	BOOL	m_bCAOtoggle;
	BOOL	m_bCPreCorrApply;
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:	
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CBitmapButton m_CamLive;
	CBitmapButton m_CamSnap;
	CBitmapButton m_SaveBkgnd;
	CBitmapButton m_SubBkgnd;
	CBitmapButton m_FindCentroids;
	CBitmapButton m_Refraction;
	CBitmapButton m_CLoopToggle;
	CBitmapButton m_CLoopToggleStop;
	CBitmapButton m_FlatDM;
	CBitmapButton m_ICANDILive;
	CBitmapButton m_ICANDIRec;
	CScrollBar m_SCamExp;
	CScrollBar m_SCamGain;
	CScrollBar m_SThreshold;
	CScrollBar m_SMinCent;
	CScrollBar m_SIntGain;
	CScrollBar m_SModestoZero;
	CScrollBar m_SPupilSize;
	CScrollBar m_SPreCorrection_Def;
	CScrollBar m_SPreCorrection_Cyl;
	CScrollBar m_SPreCorrection_Axis;
	CScrollBar m_SFixedDef;
	CButton m_CApplyRefCorr;
	CButton m_CAutoRefraction;
	CButton m_CAOtoggle;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedControlCamLive();
	afx_msg void OnBnClickedControlCamSnap();
	afx_msg void OnBnClickedControlCamSubBkgndinLive();
	afx_msg void OnBnClickedControlCamSavebkgnd();
	afx_msg void OnBnClickedControlAnalysisSubbkgnd();
	afx_msg void OnBnClickedControlAnalysisCentroids();
	afx_msg void OnBnClickedControlAnalysisOptperf();
	afx_msg void OnBnClickedControlDmCloop();
	afx_msg void OnBnClickedControlDmFlat();
	afx_msg void OnBnClickedControlIcandiLive();
	afx_msg void OnBnClickedControlIcandiRecord();
	afx_msg void OnBnClickedControlIcandiStabStart();
	afx_msg void OnBnClickedControlIcandiStabReset();
	afx_msg void OnBnClickedControlAnalysisPenalizetiptilt();
	afx_msg void OnBnClickedControlAnalysisRenewpmat();
	afx_msg void OnBnClickedControlAnalysisShowcentroids();
	afx_msg void OnBnClickedControlAnalysisAuto();
	afx_msg void OnBnClickedControlDmAoonoff();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedControlDmCloopstop();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedControlRefractionApply();
};