///////////////////////////////////////////////////////////////////////////////////
//
// SimpleBrowser.cpp 
//
// Author Peter J Lawrence Dec. 2006 Email P.J.Lawrence@gre.ac.uk
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose
// and to alter it and redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Modification History
//
// 
///////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <tchar.h>
#include <richedit.h>
#include "resource.h"
#include "stdio.h"
#include "math.h"
#include "commctrl.h"
#include "commdlg.h"
#include <sys/stat.h>
#include <time.h>
#include "cderr.h"
#include "direct.h"
#include <winsock.h>
#include "shlobj.h"
#include <richedit.h>
///////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "dlgtxtctrl.h"
#include "TCPClient.h"
////////////////////////////////////////////////////////////////////////////////
#define UseThreads
#ifdef UseThreads
    #if defined(_MT) || defined(_DLL)
    // Due to the nature of the multithreaded C runtime lib we
    // need to use _beginthreadex() and _endthreadex() instead
    // of CreateThread() and ExitThread().
    #include <process.h>
    #endif
#endif
///////////////////////////////////////////////////////////////////////////////////
#define MinXValDlg 366
#define MinYValDlg 200
///////////////////////////////////////////////////////////////////////////////////
HINSTANCE g_hInstance;
///////////////////////////////////////////////////////////////////////////////////
#define GET_Y_LPARAM(lp)   ((int)(short)HIWORD(lp))
#define GET_X_LPARAM(lp)   ((int)(short)LOWORD(lp))
///////////////////////////////////////////////////////////////////////////////////
// Foward declarations of functions included in this code module:
LRESULT CALLBACK	BrowserControlDlg(HWND, UINT, WPARAM, LPARAM);
///////////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    g_hInstance=hInstance;
    // Load the library containing the rich edit control
	HMODULE hMod = LoadLibrary( TEXT( "riched20.dll" ) );
	if( !hMod )
	{
		MessageBox( NULL, _T("Couldn't find riched20.dll. Shutting down!"), 
					_T("Error - Missing dll"), MB_OK );
	}
    INITCOMMONCONTROLSEX icex;

    // Ensure that the common control DLL is loaded.
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    BOOL CommonControlsDLL=InitCommonControlsEx(&icex);
    if (!CommonControlsDLL)
    {
        MessageBox( NULL, _T("Couldn't find riched20.dll. Shutting down!"), 
					_T("Error - Missing dll"), MB_OK );
    }

    if ( hMod && CommonControlsDLL)
	{
        DialogBox(hInstance, (LPCTSTR)IDD_BROWSER_DIALOG, NULL, (DLGPROC)BrowserControlDlg);
        FreeLibrary( hMod );
    }
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////
BOOL SaveCurrentStatus(HWND hWnd,const std::string aWorkingFolder,std::string LastWebAddress)
{
    RECT TheRect;
    GetWindowRect(hWnd,&TheRect);  

    LONG            lRetVal;
    HKEY            hkResult;
    lRetVal = RegCreateKeyEx( HKEY_CURRENT_USER, 
                        _T("SOFTWARE\\WinSimpleBrowser"), 0, NULL, 0,
                        KEY_ALL_ACCESS, NULL, &hkResult, NULL );

    if( lRetVal == ERROR_SUCCESS )
    {
        if (!IsIconic(hWnd) && !IsZoomed(hWnd))
        {
            TheRect.right-=TheRect.left;
            TheRect.bottom-=TheRect.top;
            RegSetValueEx( hkResult, _T("WinLeftPos"),NULL, REG_DWORD, (PBYTE) &TheRect.left, sizeof(TheRect.left) );
            RegSetValueEx( hkResult, _T("WinTopPos"), NULL, REG_DWORD, (PBYTE) &TheRect.top, sizeof(TheRect.top) );
            RegSetValueEx( hkResult, _T("WinWidth"),  NULL, REG_DWORD, (PBYTE) &TheRect.right, sizeof(TheRect.right) );
            RegSetValueEx( hkResult, _T("WinHeight"), NULL, REG_DWORD, (PBYTE) &TheRect.bottom, sizeof(TheRect.bottom) );
        }

        if (aWorkingFolder.length()>0)
        {
            RegSetValueEx( hkResult, _T("WorkingFolder"), NULL, REG_EXPAND_SZ, (PBYTE)aWorkingFolder.c_str(), (DWORD) aWorkingFolder.length()+1 );
        }

        if (LastWebAddress.length()>0)
        {
            RegSetValueEx( hkResult, _T("WebPage"), NULL, REG_EXPAND_SZ, (PBYTE)LastWebAddress.c_str(), (DWORD) LastWebAddress.length()+1 );
        }

        RegCloseKey( hkResult );
        return(TRUE);
    }
    return(FALSE);
}
//////////////////////////////////////////////////////////////////////////////////
BOOL LoadCurrentStatus(HWND hWnd,bool RetoreWindowStatus,std::string &aWorkingFolder,std::string &LastWebAddress)
{
    LONG            lRetVal;
    HKEY            hkResult;
    DWORD size;

    lRetVal = RegCreateKeyEx( HKEY_CURRENT_USER, 
                        _T("SOFTWARE\\WinSimpleBrowser"), 0, NULL, 0,
                        KEY_ALL_ACCESS, NULL, &hkResult, NULL );
    if( lRetVal == ERROR_SUCCESS )
    {
        RECT TheRect;
        GetWindowRect(hWnd,&TheRect);
        TheRect.right-=TheRect.left;
        TheRect.bottom-=TheRect.top;

        size = sizeof(TheRect.left);
        lRetVal = RegQueryValueEx( hkResult, _T("WinLeftPos"),NULL, NULL, (PBYTE) &TheRect.left, &size );
        if (lRetVal == ERROR_SUCCESS)
            lRetVal = RegQueryValueEx( hkResult, _T("WinTopPos"), NULL, NULL, (PBYTE) &TheRect.top,  &size);
        if (lRetVal == ERROR_SUCCESS)
            lRetVal = RegQueryValueEx( hkResult, _T("WinWidth"),  NULL, NULL, (PBYTE) &TheRect.right, &size );
        if (lRetVal == ERROR_SUCCESS)
            lRetVal = RegQueryValueEx( hkResult, _T("WinHeight"), NULL, NULL, (PBYTE) &TheRect.bottom, &size );

        if (RetoreWindowStatus && lRetVal == ERROR_SUCCESS)
        {
            if (TheRect.left<0)
            {
                TheRect.left=0;
            }
            if (TheRect.top<0)
            {
                TheRect.top=0;
            }
            if (TheRect.right<MinXValDlg)
            {
                TheRect.right=MinXValDlg;
            }
            if (TheRect.bottom<MinYValDlg)
            {
                TheRect.bottom=MinYValDlg;
            }
            SetWindowPos(hWnd,HWND_TOP,TheRect.left,TheRect.top,TheRect.right,TheRect.bottom,SWP_SHOWWINDOW);
        }

        DWORD MaxSubKeyLen,ValuesCount,MaxValueNameLen,MaxValueLen;
        lRetVal= RegQueryInfoKey(hkResult, NULL, NULL, NULL, NULL, &MaxSubKeyLen, NULL, 
                           &ValuesCount, &MaxValueNameLen, &MaxValueLen, NULL, NULL);

        DWORD ReadMaxValueLen= MaxValueLen;

        if( lRetVal == ERROR_SUCCESS )
        {
            if (MaxValueLen>0)
            {
                MaxValueLen++;
                char *DataStr=new char[MaxValueLen];
                if (DataStr)
                {
                    lRetVal = RegQueryValueEx( hkResult, _T("WorkingFolder"), NULL, NULL, (PBYTE) DataStr, &MaxValueLen );
                    if (lRetVal == ERROR_SUCCESS)
                    {
                        aWorkingFolder.assign(DataStr);
                    }
                    delete [] DataStr;
                }
            }
        }

        MaxValueLen=ReadMaxValueLen;
        if( lRetVal == ERROR_SUCCESS )
        {
            if (MaxValueLen>0)
            {
                MaxValueLen++;
                char *DataStr=new char[MaxValueLen];
                if (DataStr)
                {
                    lRetVal = RegQueryValueEx( hkResult, _T("WebPage"), NULL, NULL, (PBYTE) DataStr, &MaxValueLen );
                    if (lRetVal == ERROR_SUCCESS)
                    {
                        LastWebAddress.assign(DataStr);
                    }
                    delete [] DataStr;
                }
            }
        }


        RegCloseKey( hkResult );
        return(TRUE);
    }
    return (FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////
//
//  Given a filename with a directory and extension this routine removes
//  the directory from the filename.
//  The directory is returned in the string directory.
//
bool ExtractFilenameAndDirectory(char *filename, char *directory)
{
    if (!filename) return false;
    const char DirectorySty='\\';
    char* TheLoc=strrchr(filename,DirectorySty);
    if (TheLoc>0)
    {
        size_t result = (size_t) (TheLoc - filename + 1);
        size_t i;
        for (i=0;i<result;i++)
        {
            directory[i]=filename[i];
        }
        directory[result]='\0';
        size_t j=0;
        for (i=result;i<(int) strlen(filename);i++)
        {
            filename[j]=filename[i];
            j++;
        }
        filename[j]='\0';
    }
    return(true);
}
//////////////////////////////////////////////////////////////////////////////////
#define LEFTSTICKY         0xFF000000UL
#define TOPSTICKY          0x00FF0000UL
#define RIGHTSTICKY        0x0000FF00UL
#define BOTTOMSTICKY       0x000000FFUL
#define SAMESTICKY         0xF0F0F0F0UL
#define ALLSTICKY          0xFFFFFFFFUL
#define TOPLEFTSTICKY      0xFFFF0000UL
#define TOPRIGHTSTICKY     0x00FFFF00UL
#define BOTTOMLEFTSTICKY   0xFF0000FFUL
#define BOTTOMRIGHTSTICKY  0x0000FFFFUL
#define NULLSTICKY         0x00000000UL
//////////////////////////////////////////////////////////////////////////////////
void MoveDlgItem(HWND DlgWin,unsigned long TheID,int XOffset,int YOffset, unsigned int GlueType)
{
    RECT TheRect;
    HWND aWin=GetDlgItem(DlgWin,TheID);
    GetWindowRect(aWin,&TheRect);
    POINT TheLocation;

    TheLocation.x=TheRect.left;
    TheLocation.y=TheRect.top;
    ScreenToClient(DlgWin,&TheLocation);
    TheRect.left=TheLocation.x;
    TheRect.top=TheLocation.y;
    TheLocation.x=TheRect.right;
    TheLocation.y=TheRect.bottom;
    ScreenToClient(DlgWin,&TheLocation);
    TheRect.right=TheLocation.x;
    TheRect.bottom=TheLocation.y;

    if (GlueType==TOPLEFTSTICKY)
    {
        TheRect.bottom=TheRect.bottom+YOffset;
    }
    else if (GlueType==BOTTOMRIGHTSTICKY)
    {
        TheRect.right=TheRect.right+XOffset;
        TheRect.top=TheRect.top+YOffset;
        TheRect.bottom=TheRect.bottom+YOffset;
    }
    else if (GlueType==ALLSTICKY)
    {
        TheRect.right=TheRect.right+XOffset;
        TheRect.bottom=TheRect.bottom+YOffset;
    }
    else
    {
        if ((GlueType & RIGHTSTICKY)==RIGHTSTICKY)
        {
            TheRect.right=TheRect.right+XOffset;
            if ((GlueType & LEFTSTICKY)!=LEFTSTICKY)
            {
                TheRect.left=TheRect.left+XOffset;
            }
        }
        if ((GlueType & BOTTOMSTICKY)==BOTTOMSTICKY)
        {
            TheRect.top=TheRect.top+YOffset;
            TheRect.bottom=TheRect.bottom+YOffset;
        }
        if ((GlueType & LEFTSTICKY)==LEFTSTICKY)
        {
        }
        if ((GlueType & TOPSTICKY)==TOPSTICKY)
        {
        }
    }
    MoveWindow(aWin,TheRect.left,TheRect.top,TheRect.right-TheRect.left,TheRect.bottom-TheRect.top,TRUE);
}
//////////////////////////////////////////////////////////////////////////////////
bool HandleSize(HWND DlgWin,int theWidth, int theHeight, int &oldWidth, int &oldHeight)
{ 
    if (IsIconic(DlgWin)) return(true);

    RECT ClientRect;
    GetClientRect(DlgWin,&ClientRect);
  
    // difference in size
    int XOffset=theWidth-oldWidth;
    int YOffset=theHeight-oldHeight;

    // ratios
    float ExpandX=theWidth/float(oldWidth);
    float ExpandY=theHeight/float(oldHeight);
    MoveDlgItem(DlgWin,IDC_AddressTxt,XOffset,YOffset,TOPSTICKY);

    MoveDlgItem(DlgWin,IDC_URLADDRESS,XOffset,YOffset,RIGHTSTICKY|LEFTSTICKY);
    MoveDlgItem(DlgWin,IDC_StatusText,XOffset,YOffset,RIGHTSTICKY|LEFTSTICKY);
    MoveDlgItem(DlgWin,IDOPENABORT,XOffset,YOffset,TOPSTICKY);

    MoveDlgItem(DlgWin,IDC_HTMLWIN,XOffset,YOffset,ALLSTICKY);

    MoveDlgItem(DlgWin,IDFOLDER,XOffset,YOffset,BOTTOMSTICKY);
    MoveDlgItem(DlgWin,IDSAVEIMGS,XOffset,YOffset,BOTTOMSTICKY);
    MoveDlgItem(DlgWin,IDBACK,XOffset,YOffset,BOTTOMSTICKY);
    MoveDlgItem(DlgWin,IDFORWARD,XOffset,YOffset,BOTTOMSTICKY);
    MoveDlgItem(DlgWin,IDEXIT,XOffset,YOffset,BOTTOMSTICKY);

    oldWidth=ClientRect.right;
    oldHeight=ClientRect.bottom;
 
    InvalidateRect( DlgWin, NULL, FALSE );
    return(true);
}
//////////////////////////////////////////////////////////////////////////////////
void ProcessPendingEvents()
{
    MSG msg;         // address of structure with message
    if (PeekMessage (&msg, NULL, 0, 0,PM_REMOVE))
    {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
}

//////////////////////////////////////////////////////////////////////////
void EnableHistoryCtls(HWND hDlg,const int &HistoryLoc,const std::vector<std::string> &HistoryList)
{
    if (HistoryLoc>0 && HistoryLoc<HistoryList.size())
    {
        EnableWindow(GetDlgItem(hDlg,IDFORWARD), true);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg,IDFORWARD), false);
    }
    if (HistoryLoc>1)
    {
        EnableWindow(GetDlgItem(hDlg,IDBACK), true);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg,IDBACK), false);
    }
}
//////////////////////////////////////////////////////////////////////////
void EnableButtonsCtls(HWND hDlg,bool Status,const int &HistoryLoc,const std::vector<std::string> &HistoryList)
{
    if (Status)
    {
        KillTimer(hDlg, 1); 
        EnableWindow(GetDlgItem(hDlg,IDSAVEIMGS), true);
        EnableWindow(GetDlgItem(hDlg,IDFOLDER), true);
        SetWindowText(GetDlgItem(hDlg,IDOPENABORT),"Open");
        EnableHistoryCtls(hDlg,HistoryLoc,HistoryList);
    }
    else
    {
        SetTimer(hDlg,1, 500,(TIMERPROC) NULL);
        SetWindowText(GetDlgItem(hDlg,IDOPENABORT),"Abort");
        EnableWindow(GetDlgItem(hDlg,IDSAVEIMGS), false);
        EnableWindow(GetDlgItem(hDlg,IDFOLDER), false);
        EnableWindow(GetDlgItem(hDlg,IDFORWARD), false);
        EnableWindow(GetDlgItem(hDlg,IDBACK), false);
    }
}
//////////////////////////////////////////////////////////////////////////
LPITEMIDLIST ConvertPathToLpItemIdList(const char *pszPath)
{
    LPITEMIDLIST  pidl;
    LPSHELLFOLDER pDesktopFolder;
    OLECHAR       olePath[MAX_PATH];
    ULONG         chEaten;
    ULONG         dwAttributes;
    HRESULT       hr;

    if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
    {
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszPath, -1, olePath, MAX_PATH);
        hr = pDesktopFolder->ParseDisplayName(NULL,NULL,olePath,&chEaten,                                              &pidl,&dwAttributes);
        pDesktopFolder->Release();
    }
    return pidl;
}
//////////////////////////////////////////////////////////////////////////
int CALLBACK BrowseCallbackProc(HWND hWnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
    switch(uMsg) 
    {
    case BFFM_INITIALIZED:
        if (pData)
        {
            SendMessage(hWnd, BFFM_SETSELECTION, 1, (LPARAM) pData);
        }
        break;
    case BFFM_SELCHANGED: 
    {
        TCHAR szDir[MAX_PATH];
        // Set the status window to the currently selected path.
        if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
        {
            SendMessage(hWnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
        }
       break;
    }
    default:
       break;
    }
    return 0;
}
/////////////////////////////////////////////////////////
struct ThreadDataStruct
{
    TCPDataClass *TCPConnection;
    char* WebAddress;
};
/////////////////////////////////////////////////////////
unsigned long __stdcall ThreadDownLoadPageProc(void* anAddress)
{
    TCPDataClass* aTCPConnection = ((ThreadDataStruct*) anAddress)->TCPConnection;
    char* WebAddress = ((ThreadDataStruct*) anAddress)->WebAddress;

	if (aTCPConnection && WebAddress)
	{
		aTCPConnection->DownloadPage(WebAddress);
	}
    return (1);
}
/////////////////////////////////////////////////////////
unsigned long __stdcall ThreadSaveAllImagesProc(void* anAddress)
{
    TCPDataClass* aTCPConnection = ((ThreadDataStruct*) anAddress)->TCPConnection;

	if (aTCPConnection)
	{
		aTCPConnection->SaveAllImages();
	}
    return (1);
}
//////////////////////////////////////////////////////////////////////////////////
void LoadWebPage(HWND hDlg,TCPDataClass &aTCPConnection,char *LocalBuffer,int LocalBufferSize,
                 bool &ReceivingData,DlgTxtControl &TxtWindow,std::string &LastWebAddress,
                 const int &HistoryLoc,const std::vector<std::string> &HistoryList)
{
    LocalBuffer[0]=0;
    GetDlgItemText( hDlg, IDC_URLADDRESS, LocalBuffer,LocalBufferSize );
    if (strlen(LocalBuffer)>0)
    {
        ReceivingData=true;
        EnableButtonsCtls(hDlg,false,HistoryLoc,HistoryList);
        TxtWindow.Clear();
        LastWebAddress.assign(LocalBuffer);

		DWORD dwThreadID;
		ThreadDataStruct aVal;

		aVal.TCPConnection=&aTCPConnection;
		aVal.WebAddress=LocalBuffer;

		LPVOID pParam = (LPVOID) &aVal;

		HANDLE hThread;
		#if defined(_MT) || defined(_DLL)
		  hThread = (HANDLE) _beginthreadex(NULL, 0, (UINT (WINAPI*)(void*)) ThreadDownLoadPageProc, pParam, CREATE_SUSPENDED, (UINT*) &dwThreadID);
		#else
		  hThread = ::CreateThread(NULL, 0, ThreadDownLoadPageProc, pParam, CREATE_SUSPENDED, &dwThreadID);
		#endif
    
		if( hThread )
		{
			if(::ResumeThread(hThread) == (DWORD) -1 )
			{
				::CloseHandle(hThread);
			}
			else
			{
				while (hThread)
				{
					DWORD dwCode = 0;
					::GetExitCodeThread(hThread, &dwCode);
					if (dwCode != STILL_ACTIVE)
					{
						CloseHandle(hThread);
						hThread=NULL;
					}
					else
					{
						ProcessPendingEvents();
					}
				}
			}
		}

        ReceivingData=false;
        EnableButtonsCtls(hDlg,true,HistoryLoc,HistoryList);
    }
}
//////////////////////////////////////////////////////////////////////////////////
void DownloadAllImages(HWND hDlg,TCPDataClass &aTCPConnection,bool &ReceivingData,
                       const int &HistoryLoc,const std::vector<std::string> &HistoryList)
{
    ReceivingData=true;
    EnableButtonsCtls(hDlg,false,HistoryLoc,HistoryList);

	DWORD dwThreadID;
	ThreadDataStruct aVal;

	aVal.TCPConnection=&aTCPConnection;
	aVal.WebAddress=NULL;

	LPVOID pParam = (LPVOID) &aVal;

	HANDLE hThread;
	#if defined(_MT) || defined(_DLL)
	  hThread = (HANDLE) _beginthreadex(NULL, 0, (UINT (WINAPI*)(void*)) ThreadSaveAllImagesProc, pParam, CREATE_SUSPENDED, (UINT*) &dwThreadID);
	#else
	  hThread = ::CreateThread(NULL, 0, ThreadSaveAllImagesProc, pParam, CREATE_SUSPENDED, &dwThreadID);
	#endif

	if( hThread )
	{
		if(::ResumeThread(hThread) == (DWORD) -1 )
		{
			::CloseHandle(hThread);
		}
		else
		{
			while (hThread)
			{
				DWORD dwCode = 0;
				::GetExitCodeThread(hThread, &dwCode);
				if (dwCode != STILL_ACTIVE)
				{
					CloseHandle(hThread);
					hThread=NULL;
				}
				else
				{
					ProcessPendingEvents();
				}
			}
		}
	}

    ReceivingData=false;
    EnableButtonsCtls(hDlg,true,HistoryLoc,HistoryList);
}
//////////////////////////////////////////////////////////////////////////////////
//
// Mesage handler for about box.
//
LRESULT CALLBACK BrowserControlDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int oldWidth=0,oldHeight=0;
    static std::string WorkingFolder;
    static std::string LastWebAddress;
    static DlgTxtControl TxtWindow(hDlg,IDC_HTMLWIN);
    static TCPDataClass aTCPConnection;
    static std::vector<std::string> HistoryList;
    static int HistoryLoc=0;
    #define  LocalBufferSize 512
    static char LocalBuffer[LocalBufferSize];
    static HWND hwndTT=NULL;
    static BOOL TTisVisible=FALSE;
    static int AbortAnim=0;
    static BOOL OpenLinkNow=FALSE;

    static bool ReceivingData=false;

	switch (message)
	{
		case WM_INITDIALOG:
            // set date for controls

            SendDlgItemMessage( hDlg, IDC_HTMLWIN,EM_SETREADONLY,TRUE,0);
            SetWindowText(GetDlgItem(hDlg,IDOPENABORT),"Open");
            EnableWindow(GetDlgItem(hDlg,IDBACK), FALSE);
            EnableWindow(GetDlgItem(hDlg,IDFORWARD), FALSE);
            HistoryLoc=0;
            HistoryList.clear();

            TxtWindow.Clear();
            TxtWindow.AppendText(_T("Status Window....\r\n"));
            TxtWindow.NewLine();
            TxtWindow.BoldText();
            TxtWindow.AppendText(_T("WinSimpleBrowser\r\n"));
            TxtWindow.BoldText(false);
#ifdef _WIN64
            TxtWindow.AppendText(_T("Version: 0.0.1 (Alpha) May 2007 (Freeware) Win64\r\n"));
#else
            TxtWindow.AppendText(_T("Version: 0.0.1 (Alpha) May 2007 (Freeware) Win32\r\n"));
#endif
            {
                #ifdef _WIN64
                  HINSTANCE hInstance=(HINSTANCE) GetWindowLongPtr(hDlg, GWLP_HINSTANCE );
                #else
                  HINSTANCE hInstance=(HINSTANCE) GetWindowLong(hDlg, GWL_HINSTANCE );
                #endif

                HICON hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_SIMPLEBROWSERWIN));
                HICON hIconSmall = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_SMALL));
                PostMessage (hDlg, WM_SETICON, ICON_BIG, (LPARAM) hIcon);
                PostMessage (hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIconSmall);
                DestroyIcon(hIcon);
                DestroyIcon(hIconSmall);

                RECT ClientRect;
                GetClientRect(hDlg,&ClientRect);
                oldWidth=ClientRect.right;
                oldHeight=ClientRect.bottom;

                HWND TheCtrl=GetDlgItem(hDlg,IDC_HTMLWIN);
                if (TheCtrl)
                {
                    hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        TheCtrl, NULL, hInstance, NULL);

                    if (hwndTT)
                    {
                        SetWindowPos(hwndTT,
                            HWND_TOPMOST,0,0,0,0,
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

                        GetClientRect(TheCtrl,&ClientRect);
                        TOOLINFO ti;
                        /* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
                        ti.cbSize = sizeof(TOOLINFO);
                        ti.uFlags =  TTF_SUBCLASS  ;// TTF_SUBCLASS;
                        ti.hwnd = TheCtrl;
                        ti.hinst = hInstance;
                        ti.uId = (UINT) hDlg;
                        ti.lpszText = "Default Text";
                        
                        // ToolTip control will cover the whole control area
                        ti.rect.left = ClientRect.left;    
                        ti.rect.top = ClientRect.top;
                        ti.rect.right = ClientRect.right;
                        ti.rect.bottom = ClientRect.bottom;

                        /* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
                        if (SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti))
                        {
                            SendMessage((HWND) hwndTT,(UINT) TTM_ACTIVATE,(WPARAM) FALSE, (LPARAM) 0);
                            TTisVisible=FALSE;
                        }
                    }
                }
            }

            if (LoadCurrentStatus(hDlg,true,WorkingFolder,LastWebAddress))
            {
                if (!WorkingFolder.empty())
                {
                    if (_chdir(WorkingFolder.c_str())!=0)
                    {
                        WorkingFolder.erase();
                    }
                }
                if (WorkingFolder.empty())
                {
                    // assign working folder
                    char *Buffer;
                    if ( (Buffer=_getcwd(NULL,0)) )
                    {
                        WorkingFolder.assign(Buffer);
                        free(Buffer);
                    } 
                }
            }
            TxtWindow.NewLine();
            TxtWindow.BoldText();
            if (!WorkingFolder.empty())
            {
                TxtWindow.AppendText(_T("Output Folder :"));
                TxtWindow.AppendText(WorkingFolder);
                TxtWindow.NewLine();
                TxtWindow.AppendText(_T("Select the Folder button (bottom left of window) to change."));
                TxtWindow.NewLine();
            }
            else
            {
                TxtWindow.AppendText(_T("Select the Folder button (bottom left of window) to set output folder."));
                TxtWindow.NewLine();
            }
            TxtWindow.BoldText(false);

            if (LastWebAddress.length()>0)
            {
                SetDlgItemText( hDlg, IDC_URLADDRESS, LastWebAddress.c_str() );
            }
            else
            {
                SetDlgItemText( hDlg, IDC_URLADDRESS, _T("http://staffweb.cms.gre.ac.uk/~lp03/WinSimpleBrowser/index.html") );
            }

            aTCPConnection.SetDlg(hDlg);
            aTCPConnection.SetTxtOutput(&TxtWindow);
            aTCPConnection.SetStatusOutputCtrl(IDC_StatusText);
            SendDlgItemMessage( hDlg, IDC_HTMLWIN,EM_SETEVENTMASK,0, ENM_MOUSEEVENTS);

            TxtWindow.NewLine();
            TxtWindow.AppendText(_T("Author: Peter J.Lawrence\r\n"));
            TxtWindow.AppendText(_T("email P.J.Lawrence@gre.ac.uk\r\n\r\n"));
            TxtWindow.AppendText(_T("This software is provided \"as is\", without any guarantee made as to its suitability or fitness for any particular use.\r\nThe Author takes no responsibility for any damage that may unintentionally be caused through its use.\r\n"));

            return TRUE;

        case WM_SIZING:
            {
                RECT* ClientRect = (LPRECT) lParam;
                if ( (ClientRect->right-ClientRect->left)<MinXValDlg ||
                     (ClientRect->bottom-ClientRect->top)<MinYValDlg )
                {
                    if ((ClientRect->right-ClientRect->left)<MinXValDlg)
                    {
                        ClientRect->right=ClientRect->left+MinXValDlg;
                    }
                    if ((ClientRect->bottom-ClientRect->top)<MinYValDlg)
                    {
                        ClientRect->bottom=ClientRect->top+MinYValDlg;
                    }
                    return (TRUE);
                }
            }
            break;

        case WM_SIZE:
        {
            int nWidth = LOWORD(lParam);  // width of client area 
            int nHeight = HIWORD(lParam); // height of client area
            if (HandleSize(hDlg,nWidth,nHeight,oldWidth,oldHeight))
            {
                if (hwndTT)
                {
                    HWND TheCtrl=GetDlgItem(hDlg,IDC_HTMLWIN);
                    if (TheCtrl)
                    {
                        RECT aRect;
                        GetClientRect(TheCtrl,&aRect);
                        TOOLINFO ti;
                        memset(&ti, 0, sizeof(ti));
                        ti.cbSize = sizeof(TOOLINFO);
                        ti.hwnd = TheCtrl;
                        ti.uId = (UINT) hDlg;

                        SendMessage(hwndTT, TTM_GETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &ti);
                        ti.lpszText = "Default Text";
                        ti.rect.left = aRect.left;    
                        ti.rect.top = aRect.top;
                        ti.rect.right = aRect.right;
                        ti.rect.bottom = aRect.bottom;
                        SendMessage(hwndTT, TTM_SETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &ti);
                    }
                }
            }
            return (FALSE);
        }

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = ((LPNMHDR) lParam);
            switch (pnmh->code)
            {
            case EN_MSGFILTER:
                {
                    MSGFILTER* lpMsgFilter = ((MSGFILTER*) lParam);
                    if (lpMsgFilter)
                    {
                        switch (lpMsgFilter->msg)
                        {
                        case WM_MOUSEMOVE:
                            {
                                POINT pt,ptMenu;
                                GetCursorPos(&pt);
                                GetCursorPos(&ptMenu);
                                ScreenToClient(GetDlgItem(hDlg,IDC_HTMLWIN),&pt);
                            
                                int results = SendDlgItemMessage( hDlg, IDC_HTMLWIN,EM_CHARFROMPOS,0, (WPARAM) &pt   );
                                WORD CharIndex = LOWORD(results);
                                LinkClass *aLink = aTCPConnection.FindLink(CharIndex,LinkAny);
                                if (aLink)
                                {
                                    std::string a_LinkAddress;
                                    aLink->GetLinkAddress(a_LinkAddress);
                                    HWND TheCtrl=GetDlgItem(hDlg,IDC_HTMLWIN);
                                    if (TheCtrl)
                                    {
                                        TOOLINFO ti;
                                        memset(&ti, 0, sizeof(ti));
                                        ti.cbSize = sizeof(TOOLINFO);
                                        ti.hwnd = TheCtrl;
                                        ti.uId = (UINT) hDlg;
                                        ti.lpszText = "link";
                                        char szTextPopUp[64];
                                        _snprintf(szTextPopUp,63,"%s",a_LinkAddress.c_str());
                                        ti.lpszText = szTextPopUp;
                                        SendMessage((HWND) hwndTT,(UINT) TTM_UPDATETIPTEXT,(WPARAM) 0, (LPARAM) (LPTOOLINFO) &ti);
                                    }
                                    SendMessage((HWND) hwndTT,(UINT) TTM_ACTIVATE,(WPARAM) TRUE, (LPARAM) 0);
                                    TTisVisible=TRUE;
                                }
                                else
                                {
                                    SendMessage((HWND) hwndTT,(UINT) TTM_ACTIVATE,(WPARAM) FALSE, (LPARAM) 0);
                                    TTisVisible=FALSE;
                                }
                            }
                            break;
                        case WM_RBUTTONUP:
                            {
                                POINT pt,ptMenu;
                                GetCursorPos(&pt);
                                GetCursorPos(&ptMenu);
                                ScreenToClient(GetDlgItem(hDlg,IDC_HTMLWIN),&pt);
                                
                                int results = SendDlgItemMessage( hDlg, IDC_HTMLWIN,EM_CHARFROMPOS,0, (WPARAM) &pt   );
                                WORD CharIndex = LOWORD(results);
                                LinkClass *aLink = aTCPConnection.FindLink(CharIndex,LinkAny);
                                if (aLink)
                                {
                                    std::string a_LinkAddress;
                                    aLink->GetLinkAddress(a_LinkAddress);
                                    if (a_LinkAddress.length()>0)
                                    {
                                        HMENU hmenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_CONTEXTMAIN));
                                        HMENU hpopup = GetSubMenu(hmenu, 0);
                                    
                                        SetForegroundWindow(hDlg);
                                        int aMenuCommand= TrackPopupMenu(hpopup,TPM_RETURNCMD | TPM_RIGHTBUTTON,ptMenu.x, ptMenu.y,0,hDlg,NULL);
                                        switch (aMenuCommand)
                                        {
                                        case ID_OPTIONS_SAVE:
                                            {
                                                LinkClass *aLink2 = aTCPConnection.FindLink(CharIndex,LinkIMG);
                                                if (aLink2 && aLink2!=aLink)
                                                {
                                                    aLink2->GetLinkAddress(a_LinkAddress);
                                                    if (a_LinkAddress.length()==0)
                                                    {
                                                        aLink->GetLinkAddress(a_LinkAddress);
                                                    }
                                                }
                                                if (aTCPConnection.isConnected())
                                                {
                                                    SendMessage(hDlg,WM_COMMAND,IDOPENABORT,0);
                                                }
                                                aTCPConnection.SaveDocument(a_LinkAddress.c_str());
                                            }
                                            break;
                                        case ID_OPTIONS_OPEN:
                                            {
                                                LinkClass *aLink2 = aTCPConnection.FindLink(CharIndex,LinkHTTP);
                                                if (aLink2 && aLink2!=aLink)
                                                {
                                                    aLink2->GetLinkAddress(a_LinkAddress);
                                                    if (a_LinkAddress.length()==0)
                                                    {
                                                        aLink->GetLinkAddress(a_LinkAddress);
                                                    }
                                                }
                                                SetDlgItemText( hDlg, IDC_URLADDRESS, a_LinkAddress.c_str() );
                                                if (aTCPConnection.isConnected())
                                                {
                                                    OpenLinkNow=TRUE;
                                                }
                                                SendMessage( hDlg, WM_COMMAND, IDOPENABORT ,0);
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        case WM_LBUTTONUP:
                            {
                                POINT pt,ptMenu;
                                GetCursorPos(&pt);
                                GetCursorPos(&ptMenu);
                                ScreenToClient(GetDlgItem(hDlg,IDC_HTMLWIN),&pt);
                                
                                int results = SendDlgItemMessage( hDlg, IDC_HTMLWIN,EM_CHARFROMPOS,0, (WPARAM) &pt   );
                                WORD CharIndex = LOWORD(results);
                                LinkClass *aLink = aTCPConnection.FindLink(CharIndex,LinkAny);
                                if (aLink)
                                {
                                     std::string a_LinkAddress;
                                     aLink->GetLinkAddress(a_LinkAddress);
                                     if (a_LinkAddress.length()>0)
                                     {
                                         SetDlgItemText( hDlg, IDC_URLADDRESS, a_LinkAddress.c_str() );
                                         if (aTCPConnection.isConnected())
                                         {
                                             OpenLinkNow=TRUE;
                                         }
                                         SendMessage( hDlg, WM_COMMAND, IDOPENABORT ,0);
                                     }
                                 }
                            }
                            break;
                        }
                    }
                }
                break;
            }
            break;
        }

        case WM_TIMER:
            {
                AbortAnim=AbortAnim+1;
                AbortAnim = AbortAnim % 5;
                switch (AbortAnim)
                {
                case 0:
                    SetWindowText(GetDlgItem(hDlg,IDOPENABORT),"-Abort-");
                    break;
                case 1:
                    SetWindowText(GetDlgItem(hDlg,IDOPENABORT),"\\Abort/");
                    break;
                case 2:
                    SetWindowText(GetDlgItem(hDlg,IDOPENABORT),"|Abort|");
                    break;
                case 3:
                    SetWindowText(GetDlgItem(hDlg,IDOPENABORT),"/Abort\\");
                    break;
                case 4:
                    SetWindowText(GetDlgItem(hDlg,IDOPENABORT),"+Abort+");
                    break;
                }
            }
            break;

        case WM_CLOSE:
            SaveCurrentStatus(hDlg,WorkingFolder,LastWebAddress);
            if (hwndTT)
            {
                DestroyWindow(hwndTT);
                hwndTT=NULL;
            }
            DestroyWindow(hDlg);
            return (0);

		case WM_COMMAND:
            switch (LOWORD(wParam))
			{
            case IDSAVEIMGS:
                if (!ReceivingData)
                {
					DownloadAllImages(hDlg,aTCPConnection,ReceivingData,HistoryLoc,HistoryList);
                }
                break;

            case IDEXIT:
                SaveCurrentStatus(hDlg,WorkingFolder,LastWebAddress);
                if (hwndTT)
                {
                    DestroyWindow(hwndTT);
                    hwndTT=NULL;
                }
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
                break;

            case IDOPENABORT:
                if (ReceivingData)
                {
                    aTCPConnection.CloseConnection();
                }
                else
                {
                    LocalBuffer[0]=0;
                    GetDlgItemText( hDlg, IDC_URLADDRESS, LocalBuffer,LocalBufferSize );
                    
                    if (strlen(LocalBuffer)>0)
                    {
                        if (HistoryLoc>=HistoryList.size())
                        {
                            HistoryList.push_back(LastWebAddress);
                        }
                        else
                        {
                            int NumberToRemove=HistoryList.size()-HistoryLoc;

                            std::vector<std::string>::iterator theIterator = HistoryList.end()-NumberToRemove;
                            HistoryList.erase(theIterator,HistoryList.end());
                            HistoryList.push_back(LastWebAddress);
                        }

                        HistoryLoc=HistoryList.size();

						LoadWebPage(hDlg,aTCPConnection,LocalBuffer,LocalBufferSize,ReceivingData,TxtWindow,
                                    LastWebAddress,HistoryLoc,HistoryList);

                        if (OpenLinkNow)
                        {
                            OpenLinkNow=FALSE;
                            SendMessage( hDlg, WM_COMMAND, IDOPENABORT ,0);
                        }   
                    }
                }
                return (TRUE);
                break;

            case IDFORWARD:
                if (HistoryLoc<HistoryList.size())
                {
                    LastWebAddress.assign(HistoryList.at(HistoryLoc));
                    SetDlgItemText( hDlg, IDC_URLADDRESS, LastWebAddress.c_str() );
					SetFocus(GetDlgItem(hDlg,IDC_URLADDRESS));

                    HistoryLoc++;
                    LoadWebPage(hDlg,aTCPConnection,LocalBuffer,LocalBufferSize,ReceivingData,TxtWindow,
                                LastWebAddress,HistoryLoc,HistoryList);

                    if (OpenLinkNow)
                    {
                        OpenLinkNow=FALSE;
                        SendMessage( hDlg, WM_COMMAND, IDOPENABORT ,0);
                    }   
                }
                break;

            case IDBACK:
                if (HistoryLoc>1)
                {
                    HistoryLoc--;
                    LastWebAddress.assign(HistoryList.at(HistoryLoc-1));
                    SetDlgItemText( hDlg, IDC_URLADDRESS, LastWebAddress.c_str() );
                    LoadWebPage(hDlg,aTCPConnection,LocalBuffer,LocalBufferSize,ReceivingData,TxtWindow,
                                LastWebAddress,HistoryLoc,HistoryList);
                    if (OpenLinkNow)
                    {
                        OpenLinkNow=FALSE;
                        SendMessage( hDlg, WM_COMMAND, IDOPENABORT ,0);
                    } 
                }
                break;

            case IDFOLDER:
                {    
                    BROWSEINFO bi;       // common dialog box structure
                    
                    // Initialize
                    char adir[MAX_PATH];
                    strncpy(adir,WorkingFolder.c_str(),MAX_PATH);
                    if (adir[0]==0)
                    {
                        _getcwd(adir,MAX_PATH);
                    }
                    ZeroMemory(&bi, sizeof(BROWSEINFO));
                    bi.hwndOwner = hDlg;
                    bi.pidlRoot = NULL;
                    bi.pszDisplayName = adir;
                    bi.lpszTitle = "Working Folder";
                    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;// | BIF_EDITBOX;
                    bi.lpfn = BrowseCallbackProc;
                    bi.lParam = (long) adir;
                    bi.iImage = 0;
                    
                    // Display the Open dialog box. 
                    LPITEMIDLIST pidl=SHBrowseForFolder(&bi);
                    if (pidl) 
                    {
                        SHGetPathFromIDList(pidl,LocalBuffer);
                        
                        if (_chdir(LocalBuffer)==-1)
                        {
                            // failed to set the specified folder
                            MessageBox(hDlg,"Error","Failed to set the specified folder\nFolder not set",MB_OK|MB_ICONWARNING);
                        }
                        else
                        {
                            WorkingFolder.assign(LocalBuffer);
                        }
                        if (!WorkingFolder.empty())
                        {
                            TxtWindow.NewLine();
                            TxtWindow.AppendText(_T("Output Folder :"));
                            TxtWindow.AppendText(WorkingFolder);
                            TxtWindow.NewLine();
                            TxtWindow.AppendText(_T("Select the Folder button (bottom left of window) to change."));
                            TxtWindow.NewLine();
                        }
                    }
                    break;
                }
            }
			break;
	}
    return FALSE;
}