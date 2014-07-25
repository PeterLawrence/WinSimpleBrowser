//////////////////////////////////////////////////////////////////////////////
//
// TCP Client Class
//
// Author P.J.Lawrence Jan 2007 Email P.J.Lawrence@gre.ac.uk
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
//
#include <windows.h>
#include <tchar.h>

#include <string.h> /* memset(), memcpy() */
#include <stdlib.h> /* atexit() */
#include <stdio.h> /* printf() */

/* MinGW, Borland C 5.5 */
#ifdef WIN32
#include <winsock.h>

/* Linux */
/*
#elif defined(linux)
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> */ /* gethostbyname() */
/*#include <errno.h>

  #define	closesocket(S)	close(S)
  
    #else
    #error Unsupported OS or compiler
*/
#endif
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
////////////////////////////////////////////////////////////////////////////////
#include "TCPClient.h"
#include <richedit.h>
#include "dlgtxtctrl.h"
////////////////////////////////////////////////////////////////////////////////
int GetStartOfFileName(const char *TheString);
int GetStartOfAddress(const char *TheString);
bool GetHostName(const char *TheString,std::string &aHostAddress,int &TCPPort);
//////////////////////////////////////////////////////////////////////////////////
bool isPrintable(const char aC)
{
    return (aC>=32 && aC<=126);
}
////////////////////////////////////////////////////////////////////////////////
int CompairStrings(const char *OrgString,const char *SubString)
{
    if (!OrgString || !SubString)
    {
        return (-1);
    }
    int SubStringLen=strlen(SubString);
    int OrgStringLen=strlen(OrgString);
    if (SubStringLen==0 && OrgStringLen==0) return (0);
    if (SubStringLen==0 || OrgStringLen==0) return (-1);

    int MatchPos=0;
    int MatchLoc=-1;
    for (int i=0;i<OrgStringLen;i++)
    {
        if (tolower(OrgString[i])==tolower(SubString[MatchPos]))
        {
            if (MatchPos==0) 
            {
                MatchLoc=i;
            }
            MatchPos++;
            if (MatchPos>=SubStringLen)
            {
                return(MatchLoc);
            }
        }
        else
        {
            MatchLoc=-1;
            MatchPos=0;
        }
    }
    return (-1);
}
////////////////////////////////////////////////////////////////////////////////
bool TCPDataClass::WaitForData()
{
    int WaitCounter=m_TimeOutVal/1000;
    if (WaitCounter<=0)
        WaitCounter++;

	if (ReceivedData(WaitCounter)>0)
	{
		return (true);
	}
    return (false);
}
////////////////////////////////////////////////////////////////////////////////
TCPDataClass::~TCPDataClass()
{
    if (isConnected())
    {
        CloseConnection();
    }
    ClearLinkList();
#ifdef WIN32 
    WSACleanup();
#endif
}
////////////////////////////////////////////////////////////////////////////////
bool TCPDataClass::DownloadPage(const char *TheLocation)
{  
    int TCPPort=80;

    std::string HostAddress;
    std::string URLaddress;

    if(TheLocation!=NULL)
    {
        std::string InputString;
        InputString.assign(TheLocation);
        
        if (GetHostName(InputString.c_str(),HostAddress,TCPPort))
        {
            int aStartofAddress=GetStartOfAddress(InputString.c_str());
            if (aStartofAddress>0)
            {
                URLaddress.assign(InputString,aStartofAddress,strlen(InputString.c_str()));
                int Pos=URLaddress.find(" ");
                while (Pos>0)
                {
                    URLaddress.replace(Pos,1,"%20");
                    Pos=URLaddress.find(" ");
                }
            }
        }
        else
        {
            HostAddress.assign(TheLocation);
            URLaddress.assign("/");
        }
    }
    else
    {
        return (false);
    }

    SetTCPPort(TCPPort);
    OpenConnection(HostAddress.c_str());

    if (!isConnected())
        return (false);

    if (URLaddress.size()==0)
    {
        URLaddress.assign("/");
    }

    ClearLinkList();
    GetHtmlPage(URLaddress.c_str());

    OutputWebPage(false,true);

    if (!isConnected())
    {
        // something when wrong (i.e. process aborted by user)
        return (false);
    }
    
    CloseConnection();

    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool RemoveEndSpaces(std::string &aString)
{
    int aSize=aString.length()-1;
    if (aSize>0)
    {
        for (int i=aSize;i>0;i--)
        {
            if (aString.at(i)==' ')
            {
                aString.erase(i);
            }
            else
            {
                if (i<aSize)
                {
                    return (true);
                }
                return (false);
            }
        }
    }
    return (true);
}
////////////////////////////////////////////////////////////////////////////////
bool RemoveSpaces(std::string &aString)
{
    bool Updated=false;
    int Pos=aString.find(" ");
    while (Pos>0)
    {
        Updated=true;
        aString.replace(Pos,1,"%20");
        Pos=aString.find(" ");
    }
    return (Updated);
}
////////////////////////////////////////////////////////////////////////////////
bool RemoveStartingSpaces(std::string &aString)
{
    int aSize=aString.length();
    int RemoveCount=0;
    if (aSize>0)
    {
        for (int i=0;i<aSize;i++)
        {
            if (aString.at(i)==' ')
            {
                RemoveCount++;
            }
            else
            {
                break;
            }
        }
    }
    if (RemoveCount>0)
    {
        aString.erase(0,RemoveCount);
        return (true);
    }
    return (false);
}
////////////////////////////////////////////////////////////////////////////////
TCPDataClass::TCPDataClass(int a_port)
{
        HTTPVersion = 11;
        m_port=a_port;
        m_socket=-1;
        memset(&m_their_adr, 0, sizeof(m_their_adr));
        m_HostName.assign("193.60.49.86");
        ResetReceivedValues();
        m_TimeOutVal=2000;
        m_RemoveExtraSpaces=true;
        m_StoreLinkList=true;
        m_OutputReturns=false;

        m_TxtOutput=NULL;
        m_StatusOutputCtrl=0;
        m_hDlg=NULL;
    #ifdef WIN32    
        /* Winsock start up */
        WORD wVersionRequested = MAKEWORD( 2, 2 );
        int err = WSAStartup(wVersionRequested, &m_wsdata);
        if ( err != 0 ) 
        {
            /* Could not find a usable WinSock DLL.   */
            return;
        }
    #endif
}
////////////////////////////////////////////////////////////////////////////////
bool TCPDataClass::SaveAllImages()
{
    int ImageCount=0;

    if (LinkList.size()>0)
    {
        if (m_TxtOutput)
        {
            m_TxtOutput->Clear();
            m_TxtOutput->AppendText("Downloading All Images in page...");
                m_TxtOutput->NewLine();
        }
    
        std::string aLinkAddress;
        for (int aPhotoID=0;aPhotoID<LinkList.size();aPhotoID++)
        {
            LinkClass *aLink=aLink = LinkList.at(aPhotoID);
            if (aLink)
            {
                if (aLink->GetLinkType()==LinkIMG)
                {
                    aLink->GetLinkAddress(aLinkAddress);
                    if (SaveDocument(aLinkAddress.c_str()))
                    {
                        ImageCount++;
                    }
                    else                
                    {
                        // aborted
                        if (m_hDlg)
                        {
                            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, "Download Aborted" );
                            return (0);
                        }
                    }
                }
            }
        }

        if (m_TxtOutput)
        {
            m_TxtOutput->AppendText("==================");
            m_TxtOutput->NewLine();
            m_TxtOutput->AppendText("All Done");
            m_TxtOutput->NewLine();
        }
    }

    if (ImageCount>0)
    {
        return (true);
    }
    return (false);
}
/////////////////////////////////////////////////////////
bool isMember(const std::vector<char *> &aPhotoList,const char *TheString)
{
    if (TheString==NULL || strlen(TheString)==0)
    {
        return (false);
    }
    char *SrcStr=NULL;
    for (int i=0;i<aPhotoList.size();i++)
    {
        SrcStr=aPhotoList[i];
        if (SrcStr)
        {
            if (strcmp(SrcStr,TheString)==0)
            {
                return (true);
            }
        }
    }
    return (false);
}
/////////////////////////////////////////////////////////
int GetStartOfFileName(const char *TheString)
{
    if (TheString==NULL || strlen(TheString)==0)
    {
        return (0);
    }

    int StrLen=strlen(TheString);
    for (int i=(StrLen-1);i>0;i--)
    {
        if (TheString[i]=='/')
        {
            return (i+1);
        }
    }
    return (0);
}
/////////////////////////////////////////////////////////
int HasDot(const char *TheString)
{
    if (TheString==NULL || strlen(TheString)==0)
    {
        return (0);
    }

    int StrLen=strlen(TheString);
    for (int i=(StrLen-1);i>0;i--)
    {
        if (TheString[i]=='.')
        {
            return (i);
        }
    }
    return (0);
}
/////////////////////////////////////////////////////////
int GetStartOfAddress(const char *TheString)
{
    if (TheString==NULL || strlen(TheString)==0)
    {
        return (0);
    }

    int StrLen=strlen(TheString);
    bool Skip=false;
    for (int i=0;i<(StrLen-1);i++)
    {
        if (Skip)
        {
            Skip=false;
        }
        else
        {
            if (TheString[i]=='/') 
            {
                // if host name skip
                if (TheString[i+1]=='/')
                {
                    Skip=true;
                }
                else
                {
                    return (i);
                }
            }
        }
    }
    return (0);
}
/////////////////////////////////////////////////////////
//
// Host name is between // and /
//
bool GetHostName(const char *TheString,std::string &aHostAddress,int &TCPPort)
{
    if (TheString==NULL || strlen(TheString)==0)
    {
        return (false);
    }

    // search for "//"
    int HostNameStartLoc=0;
    int HostNameEndLoc=0;
    int StrLen=strlen(TheString);
    int i;
    for ( i=0;i<(StrLen-1);i++)
    {
        if (TheString[i]=='/') 
        {
            // if host name skip
            if (TheString[i+1]=='/')
            {
                HostNameStartLoc=(i+2);
                break;
            }
        }
    }

    // now search for next "/"
    for (i=HostNameStartLoc;i<StrLen;i++)
    {
        if (TheString[i]=='/') 
        {
            HostNameEndLoc=i;
            break;
        }
    }

    if (HostNameEndLoc==0)
    {
        HostNameEndLoc=StrLen;
    }

    if (HostNameStartLoc<HostNameEndLoc)
    {
        aHostAddress.assign(TheString,HostNameStartLoc,(HostNameEndLoc-HostNameStartLoc));
        if (aHostAddress.length()>0)
        {
            // now check for TCP port number
            std::string PortID;
            for (i=aHostAddress.length()-1;i>0;i--)
            {
                char aC=aHostAddress[i];
                if (aC>='0' && aC<='9')
                {
                    PortID.insert(0,aHostAddress[i]);
                }
                else if (aC==':')
                {
                    // found number
                    if (PortID.length()>0)
                    {
                        PortID.append("\n");
                        aHostAddress.erase(i,aHostAddress.length());
                        sscanf(PortID.c_str(),"%i",&TCPPort);
                    }
                    i=0;
                }
                else
                {
                    // end loop
                    i=0;
                }
            }
            return (true);
        }
    }
    return (false);
}
/////////////////////////////////////////////////////////
bool TCPDataClass::OpenConnection(const char* aHostName)
{
    if (!isConnected())
        {
       /* create socket */
        if (m_hDlg)
        {
            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Connecting socket...") );
        }
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(m_socket < 0)
        {
            _snprintf(m_buf,TCPDataClassBUF_LEN,"socket() returned %d, errno=%d", m_socket, errno);
            if (m_TxtOutput)
            {
                m_TxtOutput->AppendText(m_buf);
                m_TxtOutput->NewLine();
            }
            return false;
        }

        struct hostent *a_hostent;
        /* get IP address of other end */

        if (m_hDlg)
        {
            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Getting IP...") );
        }

        if(aHostName && strlen(aHostName)>0)
        {
            m_HostName.assign(aHostName);
        }

        if(m_HostName.length()>0)
        {
            a_hostent = gethostbyname(m_HostName.c_str());
            if(a_hostent == NULL)
            {
                _snprintf(m_buf,TCPDataClassBUF_LEN,"can't obtain IP address of host '%s'", m_HostName.c_str());
                if (m_TxtOutput)
                {
                    m_TxtOutput->AppendText(m_buf);
                    m_TxtOutput->NewLine();
                }
                CloseConnection();
                return false;
            }
        }
        else
        {
            return (false);
        }

        m_their_adr.sin_family = AF_INET;
        memcpy(&m_their_adr.sin_addr, a_hostent->h_addr, a_hostent->h_length);
        m_their_adr.sin_port = htons(m_port);

        /* connect */

        if (m_hDlg)
        {
            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Connecting to host...") );
        }
        int i = connect(m_socket, (struct sockaddr *)&m_their_adr, sizeof(m_their_adr));
        if(i != 0)
        {
            _snprintf(m_buf,TCPDataClassBUF_LEN,"connect() returned %d, errno=%d", i, errno);
            if (m_TxtOutput)
            {
                m_TxtOutput->AppendText(m_buf);
                m_TxtOutput->NewLine();
            }
            CloseConnection();
            return false;
        }
        return true;
    }
    return (false);
}
/////////////////////////////////////////////////////////
bool TCPDataClass::SaveDocument(const char* TheLocation)
{
    int TCPPort=80;

    std::string HostAddress;
    std::string URLaddress;

    if(TheLocation!=NULL)
    {
        std::string InputString;
        InputString.assign(TheLocation);
        
        if (GetHostName(InputString.c_str(),HostAddress,TCPPort))
        {
            int aStartofAddress=GetStartOfAddress(InputString.c_str());
            if (aStartofAddress>0)
            {
                URLaddress.assign(InputString,aStartofAddress,strlen(InputString.c_str()));
                int Pos=URLaddress.find(" ");
                while (Pos>0)
                {
                    URLaddress.replace(Pos,1,"%20");
                    Pos=URLaddress.find(" ");
                }
            }
        }
        else
        {
            HostAddress.assign(TheLocation);
            URLaddress.assign("/");
        }
    }
    else
    {
        return (false);
    }

    SetTCPPort(TCPPort);

    OpenConnection(HostAddress.c_str());

    if (!isConnected())
        return (false);

    if (URLaddress.size()==0)
    {
        URLaddress.assign("/");
    }

    GetHtmlPage(URLaddress.c_str());
    int aFileNameStr=GetStartOfFileName(URLaddress.c_str());
    std::string aFileName = &URLaddress.c_str()[aFileNameStr];
    if (aFileName.length()==1)
    {
        if (aFileName.at(0)=='/')
        {
            aFileName.assign("index.html");
        }
    } 
    if (aFileName.length()==0)
    {
        aFileName.assign("index.html");
    }
    SaveDocumentToFile(aFileName.c_str());

    if (!isConnected())
    {
        // something when wrong (i.e. process aborted by user)
        return (false);
    }

    CloseConnection();

    return (true);
}
/////////////////////////////////////////////////////////
LinkClass *TCPDataClass::FindLink(const int aLocation,const SimpleHTMLLinkType aType)
{
    if (LinkList.size()>0)
    {
        for (int i=0;i<LinkList.size();i++)
        {
            LinkClass *aLink=aLink = LinkList.at(i);
            if (aLink)
            {
                if (aType==LinkAny || aType == aLink->GetLinkType())
                {
                    if (aLink->GetEndLoc()>-1 && aLink->GetStartLoc()<=aLocation && aLocation<=aLink->GetEndLoc())
                    {
                        return (aLink);
                    }
                }
            }           
        }
    }
    return (NULL);
}
/////////////////////////////////////////////////////////
bool TCPDataClass::CloseAllLinks()
{
    bool UpdatedLinks=false;
    if (m_TxtOutput)
    {
        if (LinkList.size()>0)
        {
            for (int i=0;i<LinkList.size();i++)
            {
                LinkClass *aLink=aLink = LinkList.at(i);
                if (aLink && aLink->GetEndLoc()<-1)
                {
                    int aEndPos=-1;
                    aEndPos=m_TxtOutput->GetCurrentPos();
                    aLink->SetEndLoc(aEndPos);
                    UpdatedLinks=true;
                }           
            }
        }
    }
    return (UpdatedLinks);
}
/////////////////////////////////////////////////////////
LinkClass *TCPDataClass::AddLink(std::string a_LinkAddress,int a_StartLoc, int a_EndLoc,SimpleHTMLLinkType a_LinkType)
{
    std::string baseName;
    baseName=a_LinkAddress;
    RemoveStartingSpaces(baseName);

    LinkClass *aLink=NULL;
    if (CompairStrings(baseName.c_str(),"http")==0)
    {
        // already starts with http
        RemoveSpaces(baseName);
        aLink=new LinkClass(baseName.c_str(),a_StartLoc,a_EndLoc,a_LinkType);
    }
    else
    {
        // add host Name and append link Address
        std::string FullURL;
        if (m_HostName.length()>0)
        {
            FullURL.assign("http://");
            FullURL.append(m_HostName);
        }
        else
        {
            FullURL.erase();
        }

        if (a_LinkAddress.size()>0)
        {
            if (a_LinkAddress.at(0)=='/')
            {
                // reference from root of server
                FullURL.append(a_LinkAddress);
            }
            else
            {
                if (m_URLaddress.at(0)!='/')
                {
                    FullURL.assign("/");
                }
                FullURL.append(m_URLaddress);
                int LastPos= FullURL.length()-1;
                if (LastPos>-1)
                {
                    if (FullURL.at(LastPos)!='/')
                    {
                        int NamePos=GetStartOfFileName(m_URLaddress.c_str());
                        if (NamePos>0) //  && HasDot(&(m_URLaddress.c_str()[NamePos]))
                        {
                            // remove original file name
                            NamePos=m_URLaddress.length()-NamePos;
                            int StartPos=FullURL.length()-NamePos;
                            FullURL.erase(StartPos,NamePos);
                            LastPos= FullURL.length()-1;

                        }
                        if (LastPos > - 1 && FullURL.at(LastPos)!='/')
                        {
                            FullURL.assign("/");
                        }
                        // node add new link's name
                        FullURL.append(a_LinkAddress);
                    }
                    else
                    {
                        // just add link's name
                        FullURL.append(a_LinkAddress);
                    }
                }
                else
                {
                    FullURL.append(a_LinkAddress);
                }
            }

        }
        else
        {
            FullURL.append(a_LinkAddress);
        }
        RemoveSpaces(a_LinkAddress);
        aLink=new LinkClass(FullURL.c_str(),a_StartLoc,a_EndLoc,a_LinkType);
    }

    if (aLink)
    {
        LinkList.push_back(aLink);
    }
    return (aLink);
}
/////////////////////////////////////////////////////////
void TCPDataClass::ResetReceivedValues()
{
    m_DataSize = 0;
    m_HeaderSize = 0;
    m_BytesReceived = 0;
    m_PersentDownloaded = 0.0f;
    m_CurrentLine.erase();
    m_OutputReturns=false;
}
/////////////////////////////////////////////////////////
void TCPDataClass::ClearLinkList()
{
    if (LinkList.size()>0)
    {
        for (int i=0;i<LinkList.size();i++)
        {
            LinkClass *aLink=aLink = LinkList.at(i);
            if (aLink)
            {
                delete aLink;
            }           
        }
        LinkList.clear();
    }
}
/////////////////////////////////////////////////////////
bool TCPDataClass::CloseConnection()
{
    /* close socket */
    if (isConnected())
    {   
        ResetReceivedValues();
        if (m_hDlg)
        {
            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Closing Connection...") );
        }
        closesocket(m_socket);
        m_socket=-1;
        memset(&m_their_adr, 0, sizeof(m_their_adr));
        return (true);
    }
    return (false);
}
/////////////////////////////////////////////////////////
int TCPDataClass::SendData(char *buf)
{
    _snprintf(m_buf,TCPDataClassBUF_LEN,"sending data \"%1s\"\n",buf);
    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, buf );
    }
    int i = send(m_socket, buf, strlen(buf), 0);
    return(i);
}
/////////////////////////////////////////////////////////
int TCPDataClass::GetHtmlPage(const char *URLAddress)
{
    if (URLAddress && strlen(URLAddress)>0)
    {
        std::string HTTPcmd;
        HTTPcmd.append("GET ");
        HTTPcmd.append(URLAddress);
        switch (HTTPVersion)
        {
        case 11: // version 1.1
            HTTPcmd.append(" HTTP/1.1\r\nHost: ");
            HTTPcmd.append(m_HostName);
            HTTPcmd.append("\r\n\r\n");
            break;
        default:
        case 10: // version 1.0
            HTTPcmd.append(" HTTP/1.0\r\n\r\n");
            break;
        }
        _snprintf(m_buf,TCPDataClassBUF_LEN,"sending data \"%1s\"",HTTPcmd.c_str());
        if (m_hDlg)
        {
            SetDlgItemText( m_hDlg, m_StatusOutputCtrl,m_buf);
        }
        int i = send(m_socket, HTTPcmd.c_str(), strlen(HTTPcmd.c_str()), 0);
        m_URLaddress.assign(URLAddress);
        return(i);
    }
    return (0);
}
/////////////////////////////////////////////////////////
int TCPDataClass::OutputString(const char *buf,const int BufLen)
{
    int i;
    bool DoLoop=true;
    for (i=0;(i<BufLen && DoLoop);i++)
    {
        switch (buf[i])
        {
        case 13:
            break;
        case 10:
            {
                if (m_TxtOutput)
                {
                    m_TxtOutput->NewLine();
                }
            }
            break;
        case 0:
            DoLoop=false;
            break;
        default:
            if (isPrintable(buf[i]))
            {
                if (m_TxtOutput)
                {
                    char TmpTxt[2];
                    TmpTxt[0]=buf[i];
                    TmpTxt[1]=0;
                    m_TxtOutput->AppendText(TmpTxt);
                }
            }
            break;
        }
    }
    return (i);
}
/////////////////////////////////////////////////////////
int TCPDataClass::ReceivedData(int TimeOut)
{
    fd_set fds ;
    int n ;
    struct timeval tv ;

    // Set up the file descriptor set.
    FD_ZERO(&fds) ;
    FD_SET(m_socket, &fds) ;

    // Set up the struct timeval for the timeout.
    tv.tv_sec = TimeOut ;
    tv.tv_usec = 0 ;

    // Wait until timeout or data received.
    n = select ( m_socket+1, &fds, NULL, NULL, &tv ) ;
    if ( n == 0  )
        return 0;                      // Timeout
    if ( n == -1 ) 
        return -1;                     // Error
    return (1);
}
/////////////////////////////////////////////////////////
int TCPDataClass::ReceiveData()
{
    int DataIn = ReceivedData(2);

    if (DataIn>0)
    {
        DataIn=recv(m_socket, m_buf, TCPDataClassBUF_LEN, 0);
        if (DataIn>0)
        {
            m_BytesReceived+=DataIn;
        }
    }
    return (DataIn);
}
/////////////////////////////////////////////////////////
int TCPDataClass::WaitForReply()
{
    /* get reply */
    int i=0;
    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Waiting for data...") );
    }
    int zeroreturn=0;
    do
    {
        i = ReceiveData();
        if(i < 0)
        {
            _snprintf(m_buf,TCPDataClassBUF_LEN,"recv() returned %d, errno=%d", i, errno);
            
            if (m_TxtOutput)
            {
                m_TxtOutput->AppendText("Connecting socket...");
                m_TxtOutput->NewLine();
            }
            break;
        }
        if(i > TCPDataClassBUF_LEN)
            i = TCPDataClassBUF_LEN;

        OutputString(m_buf,i );

        u_long aRst=0;
        if (ioctlsocket(m_socket,SIOCATMARK,&aRst)==0)
        {
            if (aRst==0)
            {
                // more data to be processed
                i=TCPDataClassBUF_LEN;
            }

        }
        if(i < TCPDataClassBUF_LEN - 1)
        {
            zeroreturn++;
            if (zeroreturn>2)
            {
                break;
            }
            else
            {
                WaitForData();
                i=1;
            }
        }
    } while(i != 0 && isConnected());

    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Done...") );
    }
    return(i);
}
/////////////////////////////////////////////////////////
void TCPDataClass::ParseHeader(char aChar,bool DisplayText)
{
    if (aChar==0x0D)
    {
        if (m_CurrentLine.length()>0)
        {
            if (DisplayText && m_TxtOutput)
            {
                m_TxtOutput->AppendText(m_CurrentLine.c_str());
                m_TxtOutput->NewLine();
            }
            int aLoc=m_CurrentLine.find("Content-Length:");
            if (aLoc>-1 && aLoc<m_CurrentLine.length())
            {
                aLoc=aLoc+strlen("Content-Length:");
                m_CurrentLine.erase(0,aLoc);
                sscanf(m_CurrentLine.c_str(),"%i",&m_DataSize);
            }
        }
        m_CurrentLine.erase();
    }
    else
    {
        if (isPrintable(aChar) )
        {
            char aTmp[2];
            aTmp[0]=aChar;
            aTmp[1]=0;
            if (isPrintable(aChar))
            {
                m_CurrentLine.append(aTmp);
            }
        }
    }
}
/////////////////////////////////////////////////////////
int TCPDataClass::ScanForIMGCommands(int &FoundImageCount,std::vector<char *> &aPhotoList)
{
    std::vector<int> ImgList;
    /* get reply */
    int i=0;

    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Waiting for data...") );
    }

    int zeroreturn=0;

    bool OpenImg=true;
    const char* StartImg="[img]\0";
    int StartImgLen=strlen(StartImg);
    const char* EndImg="[/img]\0";
    int EndImgLen=strlen(EndImg);

    std::string TheData;
    FoundImageCount=0;

    int MatchLoc=0;
    int ZeroAZeroDCount=0;
    do
    {
        i = ReceiveData();
        if(i < 0)
        {
            errno=WSAGetLastError();
            _snprintf(m_buf,TCPDataClassBUF_LEN,"recv() returned %d, errno=%d\0", i, errno);
            if (m_TxtOutput)
            {
                m_TxtOutput->AppendText(m_buf);
                m_TxtOutput->NewLine();
            }
            break;
        }
        if(i > TCPDataClassBUF_LEN)
            i = TCPDataClassBUF_LEN;

        for (int Pos=0;Pos<i;Pos++)
        {
            if (ZeroAZeroDCount<4)
            {
                // still reading header
                ParseHeader(m_buf[Pos],false);
                if ( ( ((ZeroAZeroDCount % 2) == 0) && m_buf[Pos]==0x0D) ||
                     ( ((ZeroAZeroDCount % 2) == 1) && m_buf[Pos]==0x0A) )
                {   
                    ZeroAZeroDCount++;
                    if (ZeroAZeroDCount==4)
                    {
                        m_HeaderSize=(Pos+1);
                        if (m_hDlg)
                        {
                            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Receiving Data.") );
                        }
                    }
                }
                else
                {
                    ZeroAZeroDCount=0;
                }
            }
            else
            {
                if (OpenImg)
                {
                    if (tolower(m_buf[Pos])==StartImg[MatchLoc])
                    {
                        MatchLoc++;
                        if (MatchLoc>=StartImgLen)
                        {
                            OpenImg=false;
                            MatchLoc=0;
                            TheData.erase();
                        }
                    }
                    else
                    {
                        MatchLoc=0;
                    }
                }
                else
                {
                    TheData.append(&m_buf[Pos],1);
                    if (tolower(m_buf[Pos])==EndImg[MatchLoc])
                    {
                        MatchLoc++;
                        if (MatchLoc>=EndImgLen)
                        {
                            OpenImg=true;
                            TheData.erase(TheData.length()-EndImgLen,TheData.length());
                            MatchLoc=0;
                            if (!isMember(aPhotoList,TheData.c_str()))
                            {
                                FoundImageCount++;
                                if (m_TxtOutput)
                                {
                                    m_TxtOutput->AppendText("Found :");
                                    m_TxtOutput->AppendText(TheData.c_str());
                                    m_TxtOutput->NewLine();
                                }
                        
                                int StrLen=TheData.length()+1;
                                char *TheImageName = new char[StrLen+1];
                                if (TheImageName)
                                {
                                    strcpy(TheImageName,TheData.c_str());
                                    aPhotoList.push_back(TheImageName);
                                }
                            }
                        }
                    }
                    else
                    {
                        MatchLoc=0;
                    }
                }
            }
        }

        if(i < TCPDataClassBUF_LEN - 1)
        {
            zeroreturn++;
            if (zeroreturn>2)
            {
                break;
            }
            else
            {
                // wait to see if nay more data comes along
                WaitForData();
                i=1;
            }
        }
        else
        {
            zeroreturn=0;
        }
        if (m_DataSize>0 && m_BytesReceived==(m_HeaderSize+m_DataSize))
        {
            // all data read
            i=0;
        }
    } while(i != 0 && isConnected());


    if (m_DataSize>0 && m_BytesReceived!=(m_HeaderSize+m_DataSize))
    {
        if (m_TxtOutput)
        {
            m_TxtOutput->AppendText("Connection timed out!");
            m_TxtOutput->NewLine();
        }
    }

    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Done...") );
    }
    return(i);
}
/////////////////////////////////////////////////////////
bool TCPDataClass::SaveDocumentToFile(const char* aFilename)
{
    if (!aFilename)
    {
        return (false);
    }

    /* get reply */
    int i=0;
    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Waiting for data...") );
    }
    if (m_TxtOutput)
    {
        m_TxtOutput->NewLine();
        m_TxtOutput->AppendText("==================");
        m_TxtOutput->NewLine();
        _snprintf(m_buf,TCPDataClassBUF_LEN,"Downloading %1s",aFilename);
        m_TxtOutput->AppendText(m_buf);
        m_TxtOutput->NewLine();
    }
    int zeroreturn=0;

    FILE *fp = NULL; 

    fp = fopen(aFilename,"wb");
    if (!fp)
    {
        return (false);
    }

    int ZeroAZeroDCount=0;
    do
    {
        i = ReceiveData();
        if(i < 0)
        {
            errno=WSAGetLastError();

            _snprintf(m_buf,TCPDataClassBUF_LEN,"recv() returned %d, errno=%d", i, errno);
            if (m_TxtOutput)
            {
                m_TxtOutput->AppendText(m_buf);
                m_TxtOutput->NewLine();
            }
            break;
        }
        if(i > TCPDataClassBUF_LEN)
            i = TCPDataClassBUF_LEN;

        for (int aPos=0;aPos<i;aPos++)
        {
            if (ZeroAZeroDCount==4)
            {
                fputc(m_buf[aPos],fp);
            }
            else
            {
                // still reading header
                ParseHeader(m_buf[aPos],false);
                if ( ( ((ZeroAZeroDCount % 2) == 0) && m_buf[aPos]==0x0D) ||
                     ( ((ZeroAZeroDCount % 2) == 1) && m_buf[aPos]==0x0A) )
                {
                    ZeroAZeroDCount++;
                    if (ZeroAZeroDCount==4)
                    {
                        m_HeaderSize=(aPos+1);
                        if (m_hDlg)
                        {
                            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Receiving Data") );
                        }
                        if (m_TxtOutput)
                        {
                            if (m_DataSize>0)
                            {
                                _snprintf(m_buf,TCPDataClassBUF_LEN,"File Size %ikb",(m_DataSize/1024));
                            }
                            else
                            {
                                _snprintf(m_buf,TCPDataClassBUF_LEN,"File Size Unknown",(m_DataSize/1024));
                            }
                            m_TxtOutput->AppendText(m_buf);
                            m_TxtOutput->NewLine();
                        }
                    }
                }
                else
                {
                    ZeroAZeroDCount=0;
                }
            }
        }

        if(i < TCPDataClassBUF_LEN - 1)
        {
            zeroreturn++;
            if (zeroreturn>2)
            {
                break;
            }
            else
            {
                // wait to see if nay more data comes along
                WaitForData();
                i=1;
            }
        }
        else
        {
            zeroreturn=0;
        }
        if (m_DataSize>0)
        {
            if (m_BytesReceived==(m_HeaderSize+m_DataSize))
            {
                // all data read
                i=0;
            }
            float PersentDownloaded=( (m_BytesReceived-m_HeaderSize)/float(m_DataSize)) * 100.0f;
            PersentDownloaded=int(PersentDownloaded*10)/10.0f;
            if ( int(PersentDownloaded*10)!=int(m_PersentDownloaded*10))
            {
                _snprintf(m_buf,TCPDataClassBUF_LEN,"Downloaded %3.1f%% of %s",PersentDownloaded,aFilename);
                if (m_hDlg)
                {
                    SetDlgItemText( m_hDlg, m_StatusOutputCtrl, m_buf );
                }
            }
            m_PersentDownloaded=PersentDownloaded;
        }
        else
        {
            if (ZeroAZeroDCount==4)
            {
                _snprintf(m_buf,TCPDataClassBUF_LEN,"Downloaded %ikb",(m_BytesReceived/1024));
                if (m_hDlg)
                {
                    SetDlgItemText( m_hDlg, m_StatusOutputCtrl, m_buf );
                }
            }
        }
    } while(i != 0 && isConnected());

    if (m_DataSize>0 && m_BytesReceived!=(m_HeaderSize+m_DataSize))
    {
        if (m_hDlg)
        {
            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, "Connection timed out!" );
        }
    }
    else
    {
        if (m_hDlg)
        {
            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, "Download Completed!" );
        }
    }

    fclose(fp);
    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, "Done..." );
    }
    return(true);
}
/////////////////////////////////////////////////////////
//
// See if the tag matches a HTML command
// Note the command must start at the beginning of TheTag
//
bool TCPDataClass::MatchHTMLCommand(const char* TheTag,const char* TheCommand,bool &ClosingTag,int &StartLoc,int &EndLoc,bool CanBeFollowedByNumber)
{
    if (!TheTag || !TheCommand)
    {
        return (false);
    }
    int TheCommandStringLen=strlen(TheCommand);
    int TheTagLen=strlen(TheTag);
    if (TheCommandStringLen==0 && TheTagLen==0) return (true);
    if (TheCommandStringLen==0 || TheTagLen==0) return (false);

    int MatchPos=0;
    StartLoc=-1;
    EndLoc=StartLoc;
    ClosingTag=false;
    
    for (int i=0;i<TheTagLen;i++)
    {
        switch (TheTag[i])
        {
        case '/':
            if (MatchPos==0)
            {
                ClosingTag=true;
            }
            else
            {
                // does not match the html command
                return (false);
            }
            break;

        case ' ':
            if (MatchPos!=0)
            {
                // does not match the html command
                return (false);
            }
            // otherwise Ignore
            break;

        default:
            if (tolower(TheTag[i])==tolower(TheCommand[MatchPos]))
            {
                if (MatchPos==0) 
                {
                    StartLoc=i;
                }
                MatchPos++;
                if (MatchPos==TheCommandStringLen)
                {
                    if (i==(TheTagLen-1) || TheTag[i+1]==0 || TheTag[i+1]==32)
                    {
                        EndLoc=(i+1);
                        return(true);
                    }
                    else
                    {
                        if (CanBeFollowedByNumber)
                        {
                            if (TheTag[i+1]>='0' && TheTag[i+1]<='9')
                            {
                                EndLoc=(i+1);
                                return (true);
                            }
                        }
                        return (false);
                    }
                }
            }
            else
            {
                // does not match the html command
                return (false);
            }
            break;
        }
    }
    return (false);
}
/////////////////////////////////////////////////////////
//
// See if the tag matches a HTML command
// Note the command must start at the beginning of TheTag
//
bool TCPDataClass::GetHTMLLink(const char* TheTag,int &StartLoc,int &EndLoc)
{
    if (!TheTag)
    {
        return (false);
    }
    int TheTagLen=strlen(TheTag);
    if (TheTagLen==0) return (true);

    bool ReadingLink=false;
    StartLoc=-1;
    EndLoc=StartLoc;
    
    for (int i=0;i<TheTagLen;i++)
    {
        switch (TheTag[i])
        {
        case '"':
            if (i==0 || TheTag[i-1]!='\\')
            {
                if (ReadingLink)
                {
                    EndLoc=i;
                    if (EndLoc>StartLoc)
                    {
                        return (true);
                    }
                    else
                    {
                        return (false);
                    }
                }
                else
                {
                    StartLoc=i+1;
                    ReadingLink=true;
                }
            }
        }
    }
    return (false);
}
/////////////////////////////////////////////////////////
void TCPDataClass::OutputTextToScreen(bool NewLine)
{
    if (m_CurrentLine.length()>0)
    {
        if (m_TxtOutput)
        {
            m_TxtOutput->AppendText(m_CurrentLine.c_str());
        }
        m_CurrentLine.erase();
    }
    if (NewLine)
    {
        if (m_TxtOutput)
        {
            m_TxtOutput->NewLine();
        }
    }
}
/////////////////////////////////////////////////////////
bool TCPDataClass::CheckHTMLCode(const char* TheTag,bool ShowLink)
{
    bool ClosingTag;
    int StartLoc,EndLoc;
    if (MatchHTMLCommand(TheTag,"amp",ClosingTag,StartLoc,EndLoc))
    {
        m_CurrentLine.append(" ");
    }
    else if (MatchHTMLCommand(TheTag,"quot",ClosingTag,StartLoc,EndLoc))
    {
        m_CurrentLine.append("\"");
    }
    else if (MatchHTMLCommand(TheTag,"lt",ClosingTag,StartLoc,EndLoc))
    {
        m_CurrentLine.append("<");
    }
    else if (MatchHTMLCommand(TheTag,"gt",ClosingTag,StartLoc,EndLoc))
    {
        m_CurrentLine.append(">");
    }
    else if (MatchHTMLCommand(TheTag,"nbsp",ClosingTag,StartLoc,EndLoc))
    {
        m_CurrentLine.append(" ");
    }
    else if (MatchHTMLCommand(TheTag,"cent",ClosingTag,StartLoc,EndLoc))
    {
        m_CurrentLine.append("¢");
    }
    else if (MatchHTMLCommand(TheTag,"pound",ClosingTag,StartLoc,EndLoc))
    {
        m_CurrentLine.append("£");
    }
    else if (MatchHTMLCommand(TheTag,"&copy",ClosingTag,StartLoc,EndLoc))
    {
        m_CurrentLine.append("©");
    }
    return (true);
}
/////////////////////////////////////////////////////////
bool TCPDataClass::CompleteLinkLastOpenLink(SimpleHTMLLinkType aLinkType)
{
    if (LinkList.size()>0)
    {
        LinkClass *aLink=NULL;
        int iPos = LinkList.size()-1;
        while (!aLink && iPos>-1)
        {
            aLink = LinkList.at(iPos);
            if (aLink)
            {
                if (aLink->GetLinkType()==aLinkType && aLink->GetEndLoc()==-1)
                {
                    break;
                }
                else
                {
                    // keep looking
                    aLink = NULL;
                }
            }
            iPos--;
        }
        if (aLink)
        {
            int aEndPos=-1;
            aEndPos=m_TxtOutput->GetCurrentPos();
            aLink->SetEndLoc(aEndPos);
            return (true);
        }
    }
    return (false);
}
/////////////////////////////////////////////////////////
bool TCPDataClass::CheckHTMLCommand(const char* TheTag,bool ShowLink)
{
    bool ClosingTag;

    int StartLoc,EndLoc;
    if (MatchHTMLCommand(TheTag,"hr",ClosingTag,StartLoc,EndLoc))
    {
        OutputTextToScreen(true);
        if (m_TxtOutput)
        {
            m_TxtOutput->AppendText("=======================================================");
            m_TxtOutput->NewLine();
        }
    }
    else if (MatchHTMLCommand(TheTag,"h",ClosingTag,StartLoc,EndLoc,true))
    {
        if (m_TxtOutput)
        {
            if (ClosingTag)
            {
                OutputTextToScreen(true);
                m_TxtOutput->SizeText(false);
            }
            else
            {
                OutputTextToScreen(true);
                m_TxtOutput->SizeText(true);
            }
        }
    }
    else if (MatchHTMLCommand(TheTag,"ol",ClosingTag,StartLoc,EndLoc))
    {
        // Ordered list
        OutputTextToScreen(true);
    }
    else if (MatchHTMLCommand(TheTag,"li",ClosingTag,StartLoc,EndLoc))
    {
        // indicates the start of a new line item within a list
        OutputTextToScreen(true);
        if (!ClosingTag)
        {
            if (m_CurrentLine.length()==0 || m_CurrentLine.at(m_CurrentLine.length()-1)!=' ')
            {
                m_CurrentLine.append(" + ");
            }
            else
            {
                m_CurrentLine.append("+ ");
            }
        }
    }
    else if (MatchHTMLCommand(TheTag,"dir",ClosingTag,StartLoc,EndLoc))
    {
        // indicates the start of a new line item within a list
        OutputTextToScreen(true);
        if (!ClosingTag)
        {
            if (m_CurrentLine.length()==0 || m_CurrentLine.at(m_CurrentLine.length()-1)!=' ')
            {
                m_CurrentLine.append(" 0 ");
            }
            else
            {
                m_CurrentLine.append("0 ");
            }
        }
    }
    else if (MatchHTMLCommand(TheTag,"menu",ClosingTag,StartLoc,EndLoc))
    {
        // start a series of choices
        OutputTextToScreen(true);
    }
    else if (MatchHTMLCommand(TheTag,"title",ClosingTag,StartLoc,EndLoc))
    {
        OutputTextToScreen(true);
    }
    else if (MatchHTMLCommand(TheTag,"p",ClosingTag,StartLoc,EndLoc))
    {
        OutputTextToScreen(true);
    }
    else if (MatchHTMLCommand(TheTag,"pre",ClosingTag,StartLoc,EndLoc))
    {
        if (ClosingTag)
        {
            m_OutputReturns=false;
        }
        else
        {
            m_OutputReturns=true;
        }
        OutputTextToScreen(true);
    }
    else if (MatchHTMLCommand(TheTag,"script",ClosingTag,StartLoc,EndLoc))
    {
        if (ClosingTag)
        {
            CompleteLinkLastOpenLink(LinkHTTP);
            m_CurrentLine.append(">");
            OutputTextToScreen(false);
            m_TxtOutput->ColourText(0x000000);
            m_OutputReturns=false;
        }
        else
        {
            OutputTextToScreen(false);
            m_TxtOutput->ColourText(0x008200);
            m_CurrentLine.append("<SCRIPT:");
            m_OutputReturns=true;

            int RefStart=CompairStrings(&TheTag[EndLoc+1],"src=");
            if (RefStart>-1)
            {
                RefStart+=EndLoc+4;
                int LinkStart,LinkEnd;
                if (GetHTMLLink(&TheTag[RefStart+1],LinkStart,LinkEnd))
                {
                    if (m_StoreLinkList)
                    {
                        OutputTextToScreen(false);
                        m_CurrentLine.erase();
                        int aStartPos=-1;
                        if (m_TxtOutput)
                        {
                            aStartPos=m_TxtOutput->GetCurrentPos();
                        }
                        m_CurrentLine.append(TheTag,RefStart+LinkStart+1,(LinkEnd-LinkStart));
                        AddLink(m_CurrentLine,aStartPos,-1,LinkHTTP);
                        m_CurrentLine.erase();
                    }
                    m_CurrentLine.append(TheTag,RefStart+LinkStart,(LinkEnd-LinkStart+1));
                    m_CurrentLine.append("\"");
                }
            }
        }
    }
    else if (MatchHTMLCommand(TheTag,"br",ClosingTag,StartLoc,EndLoc))
    {
        OutputTextToScreen(true);
    }
    else if (MatchHTMLCommand(TheTag,"a",ClosingTag,StartLoc,EndLoc))
    {
        if (ClosingTag)
        {
            if (m_TxtOutput)
            {
                OutputTextToScreen(false);
                m_TxtOutput->ColourText(0x000000);
                CompleteLinkLastOpenLink(LinkHTTP);
            }
        }
        else
        {
            if (m_TxtOutput)
            {
                OutputTextToScreen(false);
                m_TxtOutput->ColourText(0xFF0000);
            }
            // read link information
            if (EndLoc<(strlen(TheTag)-1))
            {
                int RefStart=CompairStrings(&TheTag[EndLoc+1],"href");
                if (RefStart<0) RefStart=0;
                RefStart+=EndLoc+4;

                int LinkStart,LinkEnd;
                if (GetHTMLLink(&TheTag[RefStart+1],LinkStart,LinkEnd))
                {
                    if (m_StoreLinkList)
                    {
                        m_CurrentLine.erase();
                        int aStartPos=-1;
                        if (m_TxtOutput)
                        {
                            aStartPos=m_TxtOutput->GetCurrentPos();
                        }
                        m_CurrentLine.append(TheTag,RefStart+LinkStart+1,(LinkEnd-LinkStart));
                        AddLink(m_CurrentLine,aStartPos,-1,LinkHTTP);
                        m_CurrentLine.erase();
                    }
                    if (ShowLink)
                    {
                        m_CurrentLine.append("<LINK:");
                        m_CurrentLine.append(TheTag,RefStart+LinkStart,(LinkEnd-LinkStart+1));
                        m_CurrentLine.append("\">");
                    }
                }
            }
        }
    }
    else if (MatchHTMLCommand(TheTag,"img",ClosingTag,StartLoc,EndLoc))
    {
        if (ClosingTag)
        {
            
        }
        else
        {
            if (m_TxtOutput)
            {
                OutputTextToScreen(false);
                m_TxtOutput->ColourText(0xFF5500);
            }
            // read link information
            if (EndLoc<(strlen(TheTag)-1))
            {
                int RefStart=CompairStrings(&TheTag[EndLoc+1],"src");
                if (RefStart<0) RefStart=0;
                RefStart+=EndLoc+3;
                int LinkStart,LinkEnd;
                if (GetHTMLLink(&TheTag[RefStart+1],LinkStart,LinkEnd))
                {
                    LinkClass *aLink=NULL;
                    if (m_StoreLinkList)
                    {
                        m_CurrentLine.erase();
                        int aStartPos=-1;
                        if (m_TxtOutput)
                        {
                            aStartPos=m_TxtOutput->GetCurrentPos();
                        }
                        m_CurrentLine.append(TheTag,RefStart+LinkStart+1,(LinkEnd-LinkStart));
                        aLink=AddLink(m_CurrentLine,aStartPos,-1,LinkIMG);
                        m_CurrentLine.erase();
                    }
                    m_CurrentLine.append("<IMAGE:");
                    m_CurrentLine.append(TheTag,RefStart+LinkStart,(LinkEnd-LinkStart+1));
                    m_CurrentLine.append("\">");

                    OutputTextToScreen(false);
                    m_TxtOutput->ColourText(0x000000);
                    int aEndPos=-1;
                    aEndPos=m_TxtOutput->GetCurrentPos();
                    aLink->SetEndLoc(aEndPos);
                }
            }
        }
    }
else if (MatchHTMLCommand(TheTag,"s",ClosingTag,StartLoc,EndLoc)      ||
             MatchHTMLCommand(TheTag,"strike",ClosingTag,StartLoc,EndLoc) )
    {
        if (m_TxtOutput)
        {
            if (ClosingTag)
            {
                OutputTextToScreen(false);
                m_TxtOutput->StrikeText(false);
            }
            else
            {
                OutputTextToScreen(false);
                m_TxtOutput->StrikeText();
            }
        }
    }
    else if (MatchHTMLCommand(TheTag,"b",ClosingTag,StartLoc,EndLoc)      ||
             MatchHTMLCommand(TheTag,"strong",ClosingTag,StartLoc,EndLoc) ||
             MatchHTMLCommand(TheTag,"em",ClosingTag,StartLoc,EndLoc)     )
    {
        if (m_TxtOutput)
        {
            if (ClosingTag)
            {
                OutputTextToScreen(false);
                m_TxtOutput->BoldText(false);
            }
            else
            {
                OutputTextToScreen(false);
                m_TxtOutput->BoldText();
            }
        }
    }
    else if (MatchHTMLCommand(TheTag,"u",ClosingTag,StartLoc,EndLoc))
    {
        if (m_TxtOutput)
        {
            if (ClosingTag)
            {
                OutputTextToScreen(false);
                m_TxtOutput->UnderlineText(false);
            }
            else
            {
                OutputTextToScreen(false);
                m_TxtOutput->UnderlineText();
            }
        }
    }
    else if (MatchHTMLCommand(TheTag,"i",ClosingTag,StartLoc,EndLoc))
    {
        if (m_TxtOutput)
        {
            if (ClosingTag)
            {
                OutputTextToScreen(false);
                m_TxtOutput->ItalicText(false);
            }
            else
            {
                OutputTextToScreen(false);
                m_TxtOutput->ItalicText();
            }
        }
    }
    return (true);
}
/////////////////////////////////////////////////////////
int TCPDataClass::OutputWebPage(bool ShowCommands,bool ShowLinks)
{
    /* get reply */
    int i=0;

    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Waiting for data...") );
    }
    int zeroreturn=0;

    bool aHTMLCommand=false;
    bool aHTMLCode = false;
    bool LastCharWasHtmlCommand=false;
    const char* StartImg="[img]\0";
    int StartImgLen=strlen(StartImg);
    const char* EndImg="[/img]\0";
    int EndImgLen=strlen(EndImg);

    std::string TheHTMLCommand;
    char PreChar='X';

    int ZeroAZeroDCount=0;
    do
    {
        i = ReceiveData();
        if(i < 0)
        {
            errno=WSAGetLastError();
            _snprintf(m_buf,TCPDataClassBUF_LEN,"recv() returned %d, errno=%d", i, errno);
            if (m_hDlg)
            {
                SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Connecting socket...") );
            }
            break;
        }
        if(i > TCPDataClassBUF_LEN)
            i = TCPDataClassBUF_LEN;

        for (int Pos=0;Pos<i;Pos++)
        {
            if (ZeroAZeroDCount<4)
            {
                // still reading header
                ParseHeader(m_buf[Pos]);
                if ( ( ((ZeroAZeroDCount % 2) == 0) && m_buf[Pos]==0x0D) ||
                     ( ((ZeroAZeroDCount % 2) == 1) && m_buf[Pos]==0x0A) )
                {   
                    ZeroAZeroDCount++;
                    if (ZeroAZeroDCount==4)
                    {
                        m_HeaderSize=(Pos+1);
                        if (m_hDlg)
                        {
                            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Receiving Data.") );
                            if (m_TxtOutput)
                            {
                                m_TxtOutput->AppendText("=======================================================");
                                m_TxtOutput->NewLine();
                            }
                        }
                    }
                }
                else
                {
                    ZeroAZeroDCount=0;
                }
            }
            else
            {
                if (aHTMLCommand)
                {
                    if ((m_buf[Pos]=='>' || (aHTMLCode && m_buf[Pos]==';')) && PreChar!='\\')
                    {
                        aHTMLCommand=false;
                        if (aHTMLCode)
                        {
                            CheckHTMLCode(TheHTMLCommand.c_str());
                        }
                        else
                        {
                            CheckHTMLCommand(TheHTMLCommand.c_str());
                        }
                        aHTMLCode=false;
                        
                        if (ShowCommands)
                        {
                            if (TheHTMLCommand.length()>0)
                            {
                                if (m_TxtOutput)
                                {
                                    m_TxtOutput->AppendText(TheHTMLCommand.c_str());
                                    m_TxtOutput->NewLine();
                                }
                            }
                        }
                        TheHTMLCommand.erase();
                    }
                    else
                    {
                        if (isPrintable(m_buf[Pos]) && PreChar!='\\')
                        {
                            char aTmp[2];
                            aTmp[0]=m_buf[Pos];
                            aTmp[1]=0;
                            TheHTMLCommand.append(aTmp);
                        }
                        if (aHTMLCode && (TheHTMLCommand.length()>8 || m_buf[Pos]==13 || m_buf[Pos]==32))
                        {
                            // problem with html code
                            m_CurrentLine.append(TheHTMLCommand);
                            aHTMLCommand=false;
                            aHTMLCode=false;
                        }
                        else if (m_buf[Pos]==10)
                        {
                            TheHTMLCommand.append(" ");
                        }
                    }
                    LastCharWasHtmlCommand=true;
                }
                else
                {
                    if ((m_buf[Pos]=='<' || m_buf[Pos]=='&') && PreChar!='\\')
                    {
                        aHTMLCommand=true;
                        if (m_buf[Pos]=='&')
                        {
                            aHTMLCode=true;
                        }
                        TheHTMLCommand.erase();
                    }
                    else
                    {
                        if (isPrintable(m_buf[Pos]) )
                        {
                            if (!m_RemoveExtraSpaces || m_CurrentLine.length()==0 ||
                                 m_buf[Pos]!=' ' || m_CurrentLine.at(m_CurrentLine.length()-1)!=' ')
                            {
                                char aTmp[2];
                                aTmp[0]=m_buf[Pos];
                                aTmp[1]=0;
                                m_CurrentLine.append(aTmp);
                            }
                            else
                            {
                                if (m_OutputReturns && m_buf[Pos]==' ')
                                {
                                    m_CurrentLine.append(" ");
                                }
                            }
                        }
                        else if (m_buf[Pos]==10)
                        {
                            // return character
                            if (m_OutputReturns)
                            {
                                OutputTextToScreen(true);
                            }
                            else if (LastCharWasHtmlCommand)
                            {
                                // may count as a space
                                if (m_TxtOutput && m_TxtOutput->CurrentLineLength()>0 && m_CurrentLine.length()==0)
                                {
                                    m_CurrentLine.append(" ");
                                    PreChar=' ';
                                }
                            }
                        }
                    }
                    if (m_buf[Pos]!=13)
                        LastCharWasHtmlCommand=false;
                }
                PreChar=m_buf[Pos];
            }
        }

        if(i < TCPDataClassBUF_LEN - 1)
        {
            zeroreturn++;
            if (zeroreturn>2)
            {
                break;
            }
            else
            {
                // wait to see if any more data comes along
                WaitForData();
                i=1;
            }
        }
        else
        {
            zeroreturn=0;
        }
        if (m_DataSize>0 && m_BytesReceived==(m_HeaderSize+m_DataSize))
        {
            // all data read
            i=0;
        }
    } while(i != 0 && isConnected());

    if (m_CurrentLine.length()>0)
    {
        if (m_TxtOutput)
        {
            OutputTextToScreen(true);
        }
    }

    if (m_DataSize>0 && m_BytesReceived!=(m_HeaderSize+m_DataSize))
    {
        if (m_hDlg)
        {
            SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Connection timed out!") );
        }
    }
    
    CloseAllLinks();

    if (m_hDlg)
    {
        SetDlgItemText( m_hDlg, m_StatusOutputCtrl, _T("Done...") );
    }
    return(i);
}

