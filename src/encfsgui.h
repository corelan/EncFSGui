/*
    encFSGui - encfsgui.h
    source file contains function declarations

    written by Peter Van Eeckhoutte

*/

#pragma once
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/config.h>
#include <map>




// ----------------------------------------------------------------------------
// Classes
// ----------------------------------------------------------------------------

// DBEntry - Class for volume entry from DB

class DBEntry
{
public:
    // ctor
    DBEntry(wxString volname, 
            wxString enc_path, 
            wxString mount_path, 
            bool automount, 
            bool preventautounmount, 
            bool pwsaved);

    void setMountState(bool);
    bool getMountState();
    bool getPwSavedState();
    wxString getEncPath();
    bool getAutoMount();
    wxString getMountPath();
    wxString getVolName();
    bool getPreventAutoUnmount();

private:
    bool m_mountstate;
    bool m_automount;
    bool m_preventautounmount;
    bool m_pwsaved;
    wxString m_volname;
    wxString m_enc_path;
    wxString m_mount_path;
};



// ----------------------------------------------------------------------------
// function declarations
// ----------------------------------------------------------------------------

// encfsgui_add.cpp
void createNewEncFSFolder(wxWindow *);
void openExistingEncFSFolder(wxWindow *);

// encfsgui_edit.cpp
void editExistingEncFSFolder(wxWindow *, wxString&, std::map<wxString, DBEntry*>);

// encfsgui_helpers.cpp
bool isEncFSBinInstalled();
wxString getEncFSBinPath();
wxString getEncFSCTLBinPath();
wxString getMountBinPath();
wxString getUMountBinPath();
void ShowMsg(wxString);
wxString getEncFSBinVersion();
void renameVolume(wxString&, wxString&);

wxString StrRunCMDSync(wxString&);
wxArrayString ArrRunCMDSync(wxString&);
wxArrayString ArrRunCMDASync(wxString&);
wxString arrStrTowxStr(wxArrayString&);

bool IsVolumeSystemMounted(wxString, wxArrayString);
void BrowseFolder(wxString&);
wxString getKeychainPassword(wxString&);
bool doesVolumeExist(wxString&);
wxArrayString getEncFSVolumeInfo(wxString&);
std::map<wxString, wxString> getEncodingCapabilities();
wxString getExpectScriptContents();
wxString getChangePasswordScriptContents(wxString&);

//encfsgui_settings.cpp
void openSettings(wxWindow *);



