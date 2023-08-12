
// AOSACA.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AOSACA.h"
#include "AOSACADlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HWND    ghDlg = 0;          // Handle to main dialog box.
HACCEL  ghAccelTable = 0;   // Handle to accelerator table.


// CAOSACAApp

BEGIN_MESSAGE_MAP(CAOSACAApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAOSACAApp construction

CAOSACAApp::CAOSACAApp()
{
	EnableHtmlHelp();

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CAOSACAApp object

CAOSACAApp theApp;


// CAOSACAApp initialization

BOOL CAOSACAApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	// Add accelerator table
	ghAccelTable = LoadAccelerators(AfxGetInstanceHandle(),
	MAKEINTRESOURCE(IDR_ACCELERATOR1));

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Roorda Lab, UC Berkeley"));
	
	m_WinMsg_ui=RegisterWindowMessage(_T("APPEXISTS"));
	m_Mutex_h=::CreateMutex(NULL, FALSE, _T("{AOSACA-00C269EA-B1B6-4342-BE89-698DD933B95F}"));
	if ((m_Mutex_h!=NULL)&&(GetLastError()!=ERROR_ALREADY_EXISTS))
	{
		// Initialize GDI+.
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		// App is NOT running twice
		CAOSACADlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
	}
	else
	{
		// App is running twice send message to running app, the app will now know a 2nd instance was started
		::SendMessage(HWND_BROADCAST, m_WinMsg_ui, 0, 0); 
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CAOSACAApp::ExitInstance()
{
	GdiplusShutdown(gdiplusToken);
	return CWinApp::ExitInstance();
}



BOOL CAOSACAApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (code < 0)
		CWinApp::ProcessMessageFilter(code, lpMsg);

	if (ghDlg && ghAccelTable)
	{
		if (::TranslateAccelerator(ghDlg, ghAccelTable, lpMsg))
			return(TRUE);
	}

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}
