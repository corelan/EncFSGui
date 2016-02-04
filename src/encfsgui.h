/*
    encFSGui - encfsgui.h
    this file contains 
    class & helper function declarations

    written by Peter Van Eeckhoutte

*/

#pragma once
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/config.h>

#include <wx/listctrl.h>

#include <wx/taskbar.h>

#include <map>




// ----------------------------------------------------------------------------
// Classes
// ----------------------------------------------------------------------------


// TaskBar Icon


class TaskBarIcon : public wxTaskBarIcon
{
public:
    //ctor
    TaskBarIcon(wxTaskBarIconType iconType);

    void OnMenuExit(wxCommandEvent& event);
    void OnMenuShow(wxCommandEvent& event);
    void OnMenuHide(wxCommandEvent& event);
    void OnMenuSettings(wxCommandEvent& event);
    void OnMenuUpdate(wxCommandEvent& event);
    void OnOtherMenuClick(wxCommandEvent& event);
    virtual wxMenu *CreatePopupMenu() wxOVERRIDE;
    wxDECLARE_EVENT_TABLE();

private:
     wxMenu *m_taskBarMenu;
     wxMenu *m_taskBarVolumesMenu;
};


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
            bool pwsaved,
            bool allowother,
            bool mountaslocal);

    void setMountState(bool);
    bool getMountState();
    bool getPwSavedState();
    wxString getEncPath();
    bool getAutoMount();
    wxString getMountPath();
    wxString getVolName();
    bool getPreventAutoUnmount();
    bool getAllowOther();
    bool getMountAsLocal();

private:
    bool m_mountstate;
    bool m_automount;
    bool m_preventautounmount;
    bool m_pwsaved;
    bool m_allowother;
    bool m_mountaslocal;
    wxString m_volname;
    wxString m_enc_path;
    wxString m_mount_path;
};


// mainListCtrl - Class for the list control inside the main window

class mainListCtrl: public wxListCtrl
{
public:
    // ctor
    mainListCtrl(wxWindow *parent, 
                 const wxWindowID id, 
                 const wxPoint& pos, 
                 const wxSize& size, 
                 long style, 
                 wxStatusBar * statusbar);
    // event handlers
    //void OnMouseEvent(wxMouseEvent& event);
    void OnItemSelected(wxListEvent& event);
    void OnItemDeSelected(wxListEvent& event);
    void OnItemActivated(wxListEvent& event);
    void OnRightClick(wxListEvent& event);
    void OnPopupMenuClick(wxCommandEvent& event);
    void SetSelectedIndex(int);
    void LinkToolbar(wxToolBarBase*);
    void UpdateToolBarButtons();

private:
    wxDECLARE_EVENT_TABLE();
    wxToolBarBase *m_toolBar;
    wxStatusBar *m_statusBar;
};



// encFSGuiApp - main Class

class encFSGuiApp : public wxApp
{
public:
    bool OnInit();
};

// Define a new frame type: this is going to be our main frame
class frmMain : public wxFrame
{
public:
    // ctor(s)
    frmMain(const wxString& title, 
            const wxPoint& pos, 
            const wxSize& size, 
            long style);
    // dtor
    virtual ~frmMain();

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewFolder(wxCommandEvent& event);
    void OnAddExistingFolder(wxCommandEvent& event);
    void OnBrowseFolder(wxCommandEvent& event);
    void OnEditFolder(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnMount(wxCommandEvent& event);
    void OnUnMount(wxCommandEvent& event);
    void OnForceUnMountAll(wxCommandEvent& event);
    void OnInfo(wxCommandEvent& event);
    void OnRemoveFolder(wxCommandEvent& event);

    // generic routine
    bool unmountVolumeAsk(wxString& volumename);   // ask for confirmation
    // function that does actual unmount is not a member function

    int mountFolder(wxString& volumename, wxString& pw);

    // override default OnExit handler (so we can run code when user clicks close button on frame)
    virtual int OnExit(wxCommandEvent& event);

    // handle clicks on toolbar
    void OnToolLeftClick(wxCommandEvent& event);

    // auto mount routine
    void AutoMountVolumes();
    // FYI -  auto unmount routine is not a member function

    void PopulateVolumes();
    void PopulateToolbar(wxToolBarBase* toolBar);
    void CreateToolbar();  
    void RecreateStatusbar(); 
    void RefreshAll();
    //void UpdateToolBarButtons();  // 
    void SetToolBarButtonState(int, bool);
    void DoSize();
    void CheckUpdates();
    void CheckUpdates(bool);

    int GetListCtrlIndex(wxString&);

    bool GetVisibleState();
    void SetVisibleState(bool);

private:
    bool m_visible;
    wxString m_datadir;
    // toolbar stuff
    size_t              m_rows;             // 1
    wxPanel            *m_panel;

    // statusbar
    wxStatusBar* m_statusBar;

    // private member functions
    int mountSelectedFolder(wxString& pw);
    wxString getPassWord(wxString&, wxString&);

    // list stuff
    void RecreateList();
    // fill the control with items
    void FillListWithVolumes();
    
    // ListView stuff
    mainListCtrl *m_listCtrl;

    wxDECLARE_EVENT_TABLE();

protected:
    TaskBarIcon   *m_taskBarIcon;
#if defined(__WXOSX__) && wxOSX_USE_COCOA
    TaskBarIcon   *m_dockIcon;
#endif
};



// frmAddDialog - create a new encfs folder


class frmAddDialog : public wxDialog
{
public:
    frmAddDialog(wxWindow *parent, 
                 const wxString& title, 
                 const wxPoint& pos, 
                 const wxSize& size, 
                 long style);
    void Create();
    void ChooseSourceFolder(wxCommandEvent &event);
    void ChooseDestinationFolder(wxCommandEvent &event);
    void SaveSettings(wxCommandEvent &event);
    void SetEncFSProfileSelection(wxCommandEvent &event);
    void ApplyEncFSProfileSelection(int);
private:
    wxTextCtrl * m_source_field;
    wxTextCtrl * m_destination_field;
    wxTextCtrl * m_volumename_field;
    wxTextCtrl * m_pass1;
    wxTextCtrl * m_pass2;
    wxCheckBox * m_chkbx_automount;
    wxCheckBox * m_chkbx_prevent_autounmount;
    wxCheckBox * m_chkbx_save_password;
    wxCheckBox * m_chkbx_allow_other;
    wxCheckBox * m_chkbx_mount_as_local;    
    wxCheckBox * m_chkbx_perfile_iv;
    wxCheckBox * m_chkbx_iv_chaining;
    wxCheckBox * m_chkbx_filename_to_iv_header_chaining;
    wxCheckBox * m_chkbx_block_mac_headers;
    wxComboBox * m_combo_cipher_algo;
    wxComboBox * m_combo_cipher_keysize;
    wxComboBox * m_combo_cipher_blocksize;
    wxComboBox * m_combo_filename_enc;
    std::map<wxString, wxString> m_encodingcaps;
    //wxComboBox * m_combo_keyderivation;
    wxDECLARE_EVENT_TABLE();
    void SetEncfsOptionsState(bool);
    bool createEncFSFolder();
};



// frmOpenDialog - add existing encfs folder


class frmOpenDialog : public wxDialog
{
public:
    frmOpenDialog(wxWindow *parent, 
                  const wxString& title, 
                  const wxPoint& pos, 
                  const wxSize& size, 
                  long style);
    void Create();
    void ChooseSourceFolder(wxCommandEvent &event);
    void ChooseDestinationFolder(wxCommandEvent &event);
    void SaveSettings(wxCommandEvent &event);
private:
    wxTextCtrl * m_source_field;
    wxTextCtrl * m_destination_field;
    wxTextCtrl * m_volumename_field;
    wxTextCtrl * m_pass1;
    wxTextCtrl * m_pass2;
    wxCheckBox * m_chkbx_automount;
    wxCheckBox * m_chkbx_prevent_autounmount;
    wxCheckBox * m_chkbx_save_password;
    wxCheckBox * m_chkbx_allow_other;
    wxCheckBox * m_chkbx_mount_as_local;    
    wxDECLARE_EVENT_TABLE();
};



// frmEditDialog - edit an encfs folder


class frmEditDialog : public wxDialog
{
public:
    frmEditDialog(wxWindow *parent, 
                 const wxString& title, 
                 const wxPoint& pos, 
                 const wxSize& size, 
                 long style,
                 wxString selectedvolume,
                 std::map<wxString, DBEntry*> volumedata);
    void Create();
    void ChooseDestinationFolder(wxCommandEvent &event);
    void SaveSettings(wxCommandEvent &event);
    void ChangePWFieldState(wxCommandEvent &event);
private:
    wxString m_volumename;
    wxTextCtrl * m_destination_field;
    wxTextCtrl * m_volumename_field;
    wxTextCtrl * m_pass1;
    wxTextCtrl * m_pass2;
    wxCheckBox * m_chkbx_automount;
    wxCheckBox * m_chkbx_prevent_autounmount;
    wxCheckBox * m_chkbx_save_password;
    wxCheckBox * m_chkbx_allow_other;
    wxCheckBox * m_chkbx_mount_as_local;
    wxButton * m_selectdst_button;
    std::map<wxString, DBEntry*> m_editVolumeData;
    bool m_mounted;
    bool m_pwsaved;
    wxDECLARE_EVENT_TABLE();
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
wxString getExpectScriptContents(bool);
wxString getChangePasswordScriptContents(wxString&);
wxString getLaunchAgentContents();
wxString getLatestVersion();
bool IsLatestVersionNewer(const wxString&, wxString&);

//encfsgui_settings.cpp
void openSettings(wxWindow *);



