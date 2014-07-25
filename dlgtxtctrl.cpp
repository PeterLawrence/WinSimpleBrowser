//////////////////////////////////////////////////////////////////////////////////
//
// Dialog Text Control Class
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
//////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <tchar.h>
#include <string>
#include <richedit.h>
#include "dlgtxtctrl.h"
//////////////////////////////////////////////////////////////////////////////////
bool DlgTxtControl::NewLine()
{
    return (AppendText("\r\n"));
}
//////////////////////////////////////////////////////////////////////////////////
bool DlgTxtControl::AppendText(const char* TheText)
{
    if (TheText)
    {
        size_t NewTxtLen=strlen(TheText);
        if (NewTxtLen>0)
        {
            if (m_TxtLimit==0)
            {
                LONG TxtLimit=SendDlgItemMessage( m_hDlg, m_ControlID,EM_GETLIMITTEXT,(WPARAM) 0,(LPARAM) 0);
                if (TxtLimit>0)
                {
                    m_TxtLimit = (UINT) TxtLimit;
                }
            }

            if (m_TxtLimit>0)
            {
                LONG TxtLength=SendDlgItemMessage( m_hDlg, m_ControlID,WM_GETTEXTLENGTH,(WPARAM) 0,(LPARAM) 0);
                m_TxtLength=(TxtLength+NewTxtLen);
                if (TxtLength>0)
                {
                    if (m_TxtLength>m_TxtLimit)
                    {
                        // increase limit
                        m_TxtLimit = ( (20*(NewTxtLen+2))+m_TxtLimit);
                        SendDlgItemMessage( m_hDlg, m_ControlID,EM_SETLIMITTEXT,(WPARAM) m_TxtLimit,(LPARAM) 0);
                    }
                }
                // add text to edit control
                SendDlgItemMessage( m_hDlg, m_ControlID,EM_SETSEL,m_TxtLimit,(LPARAM) m_TxtLimit);
                SendDlgItemMessage( m_hDlg, m_ControlID,EM_REPLACESEL,TRUE,(LPARAM) TheText);
                return (true);
            }
        }
    }
    return (false);
}
/////////////////////////////////////////////////////////////////
LONG DlgTxtControl::GetCurrentPos()
{
    DWORD StartPos,EndPos;
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_GETSEL,(WPARAM) &StartPos,(LPARAM) &EndPos);
    return((LONG) StartPos);
}
/////////////////////////////////////////////////////////////////
bool DlgTxtControl::GetTextFormat(CHARFORMAT &aFormat)
{
    aFormat.cbSize=sizeof(aFormat);
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_GETCHARFORMAT,SCF_SELECTION,(LPARAM) &aFormat);
    return(true);
}
//////////////////////////////////////////////////////////////////////////////////
bool DlgTxtControl::BoldText(bool TurnOn)
{
    CHARFORMAT aFormat;
    GetTextFormat(aFormat);
    aFormat.cbSize=sizeof(aFormat);
    if (TurnOn)
    {
        aFormat.dwEffects=CFE_BOLD;
    }
    else
    {
        aFormat.dwEffects=0;
    }
    aFormat.dwMask=CFM_BOLD;
    aFormat.yHeight=240;
    aFormat.yOffset=0;
    aFormat.crTextColor=0x000000;
    aFormat.bCharSet=NULL;
    aFormat.bPitchAndFamily=NULL;
    aFormat.szFaceName[0]=NULL;
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM) &aFormat);
    return(true);
}
//////////////////////////////////////////////////////////////////////////////////
bool DlgTxtControl::ItalicText(bool TurnOn)
{
    CHARFORMAT aFormat;
    GetTextFormat(aFormat);
    aFormat.cbSize=sizeof(aFormat);
    if (TurnOn)
    {
        aFormat.dwEffects=CFE_ITALIC;
    }
    else
    {
        aFormat.dwEffects=0;
    }
    aFormat.dwMask=CFM_ITALIC;
    aFormat.yHeight=240;
    aFormat.yOffset=0;
    aFormat.crTextColor=0x000000;
    aFormat.bCharSet=NULL;
    aFormat.bPitchAndFamily=NULL;
    aFormat.szFaceName[0]=NULL;
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM) &aFormat);
    return(true);
}
//////////////////////////////////////////////////////////////////////////////////
bool DlgTxtControl::ColourText(COLORREF aTextColour)
{
    CHARFORMAT aFormat;
    GetTextFormat(aFormat);
    aFormat.cbSize=sizeof(aFormat);
    if (aTextColour==0x000000)
    {
        aFormat.dwEffects=CFE_AUTOCOLOR;
    }
    else
    {
        aFormat.dwEffects=0;
    }
    aFormat.dwMask=CFM_COLOR;
    aFormat.yHeight=240;
    aFormat.yOffset=0;
    aFormat.crTextColor=aTextColour;
    aFormat.bCharSet=NULL;
    aFormat.bPitchAndFamily=NULL;
    aFormat.szFaceName[0]=NULL;
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM) &aFormat);
    return(true);
}
/////////////////////////////////////////////////////////////////
bool DlgTxtControl::UnderlineText(bool TurnOn)
{
    CHARFORMAT aFormat;
    GetTextFormat(aFormat);
    aFormat.cbSize=sizeof(aFormat);
    if (TurnOn)
    {
        aFormat.dwEffects=CFE_UNDERLINE;
    }
    else
    {
        aFormat.dwEffects=0;
    }
    aFormat.dwMask=CFM_UNDERLINE;
    aFormat.yHeight=240;
    aFormat.yOffset=0;
    aFormat.crTextColor=0x000000;
    aFormat.bCharSet=NULL;
    aFormat.bPitchAndFamily=NULL;
    aFormat.szFaceName[0]=NULL;
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM) &aFormat);
    return(true);
}
/////////////////////////////////////////////////////////////////
bool DlgTxtControl::StrikeText(bool TurnOn)
{
    CHARFORMAT aFormat;
    GetTextFormat(aFormat);
    aFormat.cbSize=sizeof(aFormat);
    if (TurnOn)
    {
        aFormat.dwEffects=CFE_STRIKEOUT;
    }
    else
    {
        aFormat.dwEffects=0;
    }
    aFormat.dwMask=CFM_STRIKEOUT;
    aFormat.yHeight=240;
    aFormat.yOffset=0;
    aFormat.crTextColor=0x000000;
    aFormat.bCharSet=NULL;
    aFormat.bPitchAndFamily=NULL;
    aFormat.szFaceName[0]=NULL;
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM) &aFormat);
    return(true);
}
/////////////////////////////////////////////////////////////////
bool DlgTxtControl::SizeText(bool LargerText)
{
    CHARFORMAT aFormat;
    GetTextFormat(aFormat);
    aFormat.cbSize=sizeof(aFormat);
    aFormat.dwEffects=0;
    aFormat.dwMask=CFM_SIZE;
    if (LargerText)
    {
        aFormat.yHeight+=80;
    }
    else
    {
        aFormat.yHeight-=80;
    }
    aFormat.yOffset=0;
    aFormat.crTextColor=0x000000;
    aFormat.bCharSet=NULL;
    aFormat.bPitchAndFamily=NULL;
    aFormat.szFaceName[0]=NULL;
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM) &aFormat);
    return(true);
}
//////////////////////////////////////////////////////////////////////////////////
bool DlgTxtControl::AppendText(const std::string &TheText)
{
    return (AppendText(TheText.c_str()));
}
//////////////////////////////////////////////////////////////////////////////////
bool DlgTxtControl::Clear()
{
    SetDlgItemText( m_hDlg, m_ControlID, _T("") );
    m_TxtLength=0;
    return (true);
}
//////////////////////////////////////////////////////////////////////////////////
bool DlgTxtControl::GetSelectedText(int &aStartPos,int &aEndPos)
{
    SendDlgItemMessage( m_hDlg, m_ControlID,EM_GETSEL,(WPARAM) &aStartPos,(LPARAM) &aEndPos);
    return (true);
}
//////////////////////////////////////////////////////////////////////////////////
int DlgTxtControl::CurrentLineLength()
{
    int LineLenght=SendDlgItemMessage( m_hDlg, m_ControlID,EM_LINEINDEX,(WPARAM) -1,(LPARAM) 0);
    LineLenght=SendDlgItemMessage( m_hDlg, m_ControlID,EM_LINELENGTH,(WPARAM) LineLenght,(LPARAM) 0);
    return(LineLenght);
}