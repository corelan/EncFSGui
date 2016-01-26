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

#include <wx/listctrl.h>

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

private:
    wxString m_datadir;
    // toolbar stuff
    size_t              m_rows;             // 1
    wxPanel            *m_panel;

        // statusbar
    wxStatusBar* m_statusBar;

    int mountSelectedFolder(wxString& pw);

    wxString getPassWord(wxString&, wxString&);

    // list stuff
    void RecreateList();
    // fill the control with items
    void FillListWithVolumes();
    
    // ListView stuff
    mainListCtrl *m_listCtrl;

    // any class wishing to process wxWidgets events must use this macro
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
wxString getExpectScriptContents();
wxString getChangePasswordScriptContents(wxString&);
wxString getLaunchAgentContents();

//encfsgui_settings.cpp
void openSettings(wxWindow *);



