//////////////////////////////////////////////////////////////////////////////////
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
#ifndef TCPDataClass_H
#define TCPDataClass_H
////////////////////////////////////////////////////////////////////////////////
class DlgTxtControl;
////////////////////////////////////////////////////////////////////////////////
#define	TCPDataClassBUF_LEN		512
/////////////////////////////////////////////////////////////////////////////////
typedef enum SimpleHTMLLinkType
{
    LinkAny,
	LinkHTTP,
    LinkIMG,
    LinkUnknown,
};
////////////////////////////////////////////////////////////////////////////////
class LinkClass
{
private:
    std::string m_LinkAddress;
    int m_StartLoc;
    int m_EndLoc;
    SimpleHTMLLinkType m_LinkType;

public:
    LinkClass(std::string a_LinkAddress,int a_StartLoc=-1, int a_EndLoc=-1,SimpleHTMLLinkType a_LinkType=LinkHTTP)
    {
        m_LinkAddress=a_LinkAddress;
        m_StartLoc=a_StartLoc;
        m_EndLoc=a_EndLoc;
        m_LinkType = a_LinkType;
    }

    int  GetStartLoc() { return (m_StartLoc); }
    int  GetEndLoc()   { return (m_EndLoc); }
    void SetStartLoc(int aLoc) { m_StartLoc=aLoc; }
    void SetEndLoc(int aLoc)   { m_EndLoc=aLoc; }

    SimpleHTMLLinkType GetLinkType() { return(m_LinkType); }
    void SetLinkType(SimpleHTMLLinkType aType) { m_LinkType = aType;}
    
    void GetLinkAddress(std::string &a_LinkAddress) { a_LinkAddress = m_LinkAddress; }
};
////////////////////////////////////////////////////////////////////////////////
class TCPDataClass
{
private:
    int HTTPVersion;
    int m_socket;
    int m_port;
  #ifdef WIN32
    WSADATA m_wsdata;
  #endif
    struct sockaddr_in m_their_adr;
    std::string m_HostName;
    std::string m_URLaddress;

    int m_DataSize;
    int m_HeaderSize;
    int m_BytesReceived;
    int m_TimeOutVal;
    float m_PersentDownloaded;
    bool m_RemoveExtraSpaces;
    bool m_OutputReturns;

    std::string m_CurrentLine;
    std::vector<LinkClass*> LinkList;
    bool m_StoreLinkList;

    void OutputTextToScreen(bool NewLine);

    char m_buf[TCPDataClassBUF_LEN];

    int OutputString(const char *buf,const int BufLen);
    void ParseHeader(char aChar,bool DisplayText=true);
    bool MatchHTMLCommand(const char* TheTag,const char* TheCommand,bool &ClosingTag,int &StartLoc,int &EndLoc,bool CanBeFollowedByNumber=false);
    bool GetHTMLLink(const char* TheTag,int &StartLoc,int &EndLoc);
    bool CheckHTMLCommand(const char* TheTag,bool ShowLink=FALSE);
    bool CheckHTMLCode(const char* TheTag,bool ShowLink=FALSE);

    bool CompleteLinkLastOpenLink(SimpleHTMLLinkType aLinkType);
    void ResetReceivedValues();
    void ClearLinkList();
    int ReceivedData(int TimeOut);
    int ReceiveData();

    DlgTxtControl *m_TxtOutput;
    int m_StatusOutputCtrl;
    HWND m_hDlg;

    void  SetTCPPort(int TCPPort) { m_port=TCPPort; }
    void  Set2HTTPVersion(int aVersion) { HTTPVersion = aVersion; }
    int   Get2HTTPVersion() { return (HTTPVersion); }
    void  SetHTTPVersion(float aVersion) { HTTPVersion = int(10.0*aVersion); }
    float GetHTTPVersion() { return (float(HTTPVersion)/1.0f); }

    bool OpenConnection(const char* aHostName);

    int SendData(char *buf);
    int GetHtmlPage(const char *URLAddress);
    int WaitForReply();
    bool WaitForData();
    int ScanForIMGCommands(int &FoundImageCount,std::vector<char *> &aPhotoList);
    bool SaveDocumentToFile(const char* Filename);
    int OutputWebPage(bool ShowCommands,bool ShowLinksL);

public:
    TCPDataClass(int a_port=80);
    ~TCPDataClass();

    void SetTxtOutput(DlgTxtControl *TxtOutput) { m_TxtOutput=TxtOutput; }
    void SetStatusOutputCtrl(int aCtrl) { m_StatusOutputCtrl = aCtrl; }
    void SetDlg(HWND ahDlg) { m_hDlg = ahDlg; }
    bool CloseConnection();

    void StoreLinkList(bool aStatus) {m_StoreLinkList=aStatus;}
    bool GetStoreLinkListFlag() { return(m_StoreLinkList);}

    bool DownloadPage(const char *TheLocation);
    bool SaveDocument(const char* TheLocation);
    bool SaveAllImages();
    bool isConnected() { return (m_socket>=0); }

    LinkClass *FindLink(const int aLocation,const SimpleHTMLLinkType aType=LinkAny);
    LinkClass *AddLink(std::string a_LinkAddress,int a_StartLoc=-1, int a_EndLoc=-1,SimpleHTMLLinkType a_LinkType=LinkHTTP);
    bool CloseAllLinks();
};
#endif