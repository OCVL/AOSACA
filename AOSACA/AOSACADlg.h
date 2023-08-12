
// AOSACADlg.h : header file
//

#pragma once

#include "VecALL.h"
#include "MatALL.h"

#define MAIN_WINDOW_WIDTH	1280
#define MAIN_WINDOW_HEIGHT	995

enum mouse_mode_enum {MOUSE_NORMAL=0,	// 0 for doing nothing
	MOUSE_SELECT_CENTER=1,				// 1 for user selecting center,
	MOUSE_ADJUST_CENTROIDS=2,			// 2 for moving, adding and deleting centroids
	POKE_MATRIX_GENERATION=3			// 4 while generating poke matrix
	};

// CAOSACADlg dialog
class CAOSACADlg : public CDialogEx
{
// Construction
public:
	CAOSACADlg(CWnd* pParent = NULL);	// standard constructor

	//Public variables	
	bool	*m_bMenuFlags; // to update menu items status
	bool	m_bUpdatePupilCenter;
	void	OnUpdateMenu();

	// Mouse update functions/handlers	
	void	lBtnUp_ImageDlg(CDPoint point);
	void	mouseMove_ImageDlg(CPoint point);
	void	UpdateMouseMode(mouse_mode_enum modeval){m_eMouse_mode = modeval;}
	mouse_mode_enum ObtainMouseMode(void){return m_eMouse_mode;}

	// Data retrieval and update functions
	void	UpdateMenu(int menuid=-1, bool status=false);
	bool	init_Centroid_Detection(void);
	bool	Steps_in_finding_Centroids(void);
	void	Opt_Perform();
	void	Update_Displays();
	void	StartCLoopThread();
	void	StopCLoopThread();
	void	Initialize_CLoop();	
	void	OnUpdateGeneratePokeMatrixMenu();
	void	ResetZeroDMMenu();

private:
	mouse_mode_enum m_eMouse_mode;	

	bool	LocateCentroids(void);	
	bool	LocateOneCentroid (short);
	CWnd	*get_DisplayHandle(void);
// Dialog Data
	enum { IDD = IDD_AOSACA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CStatusBarCtrl m_Statusbar;

	// Generated message map functroordalab	ions
	virtual BOOL OnInitDialog();
	afx_msg LRESULT	OnUpdateWindow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFileSaveimage();
	afx_msg void OnFileExit();
	afx_msg void OnEditUpdatepupilcenter();
	afx_msg void OnEditAosacaparameters();
	afx_msg void OnToolsGeneratepokematrix();
	afx_msg void OnToolsZerodm();
	afx_msg void OnToolsTestdmactuators();
	afx_msg void OnViewDrawcentroids();
	afx_msg void OnViewSlopevectormap();
	afx_msg void OnViewWavefrontmap();
	afx_msg void OnViewPsfmap();
	afx_msg void OnViewDmmap();
	afx_msg void OnViewRealtimeplots();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnToolsAligndm();
	afx_msg void OnEditPupilcamerasettings();
	afx_msg void OnHelpAboutaosaca();
	afx_msg void OnToolsSavemirrorshape();
};