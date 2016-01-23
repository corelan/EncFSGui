#pragma once
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/config.h>
#ifndef RLOG_COMPONENT
//#  warning RLOG_COMPONENT not defined - setting to UNKNOWN
#define RLOG_COMPONENT "[unknown]"
#endif // RLOG_COMPONENT not defined
#define FUSE_USE_VERSION 26

#include "fuse/fuse.h"
#include "openssl.h"
#include "encfs.h"


// ----------------------------------------------------------------------------
// function declarations
// ----------------------------------------------------------------------------

// encfsgui_add.cpp
void createNewEncFSFolder(wxWindow *);
void openExistingEncFSFolder(wxWindow *);

// encfsgui_helpers.cpp
bool isEncFSBinInstalled();
wxString getEncFSBinPath();
wxString getMountBinPath();
wxString getUMountBinPath();
void ShowMsg(wxString);
wxString getEncFSBinVersion();
wxString StrRunCMDSync(wxString);
wxArrayString ArrRunCMDSync(wxString);
wxString arrStrTowxStr(wxArrayString&);
bool IsVolumeSystemMounted(wxString, wxArrayString);
void BrowseFolder(wxString&);
wxString getKeychainPassword(wxString&);
bool doesVolumeExist(wxString&);

//encfsgui_settings.cpp
void openSettings(wxWindow *);



