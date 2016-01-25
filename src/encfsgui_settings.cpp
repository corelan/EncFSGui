/*
    encFSGui - encfsgui_settings.cpp
    source file contains code to change app settings

    written by Peter Van Eeckhoutte

*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/filename.h>
#include <wx/config.h>

#include "encfsgui.h"

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

enum
{
    ID_BTN_CHOOSE_ENCFS,
    ID_BTN_CHOOSE_MOUNT,
    ID_BTN_CHOOSE_UMOUNT,
    ID_CHECK_STARTATLOGIN,
    ID_CHECK_UNMOUNT_ON_QUIT
};

// ----------------------------------------------------------------------------
// Classes
// ----------------------------------------------------------------------------

class frmSettingsDialog : public wxDialog
{
public:
    //ctor
    frmSettingsDialog(wxWindow *parent, 
                      wxString& title, 
                      const wxPoint& pos, 
                      const wxSize& size, 
                      long style);
    void Create();
    void SelectEncFSBinPath(wxCommandEvent &event);
    void SetCurrentEncFSBinPath(wxString &encfsbinpath);
    void SelectMountBinPath(wxCommandEvent &event);
    void SetCurrentMountBinPath(wxString &mountbinpath);
    void SelectUMountBinPath(wxCommandEvent &event);
    void SetCurrentUMountBinPath(wxString &umountbinpath);
    void SaveSettings(wxCommandEvent &event);

private:
    wxString currentEncFSBinPath;
    wxString currentMountBinPath;
    wxString currentUMountBinPath;
    wxDECLARE_EVENT_TABLE();
    wxTextCtrl * m_encfsbin_field;
    wxTextCtrl * m_mountbin_field;
    wxTextCtrl * m_umountbin_field;
    wxCheckBox * m_chkbx_startatlogin;
    wxCheckBox * m_chkbx_unmount_on_quit;
};


// Events

wxBEGIN_EVENT_TABLE(frmSettingsDialog, wxDialog)
    EVT_BUTTON(ID_BTN_CHOOSE_ENCFS,  frmSettingsDialog::SelectEncFSBinPath)
    EVT_BUTTON(ID_BTN_CHOOSE_MOUNT,  frmSettingsDialog::SelectMountBinPath)
    EVT_BUTTON(ID_BTN_CHOOSE_UMOUNT,  frmSettingsDialog::SelectUMountBinPath)
    EVT_BUTTON(wxID_APPLY, frmSettingsDialog::SaveSettings)
wxEND_EVENT_TABLE()

// dialog constructionr
frmSettingsDialog::frmSettingsDialog(wxWindow *parent, 
                                     wxString& title, 
                                     const wxPoint& pos, 
                                     const wxSize& size, 
                                     long style) : wxDialog(parent, wxID_ANY, title, pos, size, style)
{
    currentEncFSBinPath = getEncFSBinPath();
}

void frmSettingsDialog::SetCurrentEncFSBinPath(wxString &encfsbinpath)
{
    currentEncFSBinPath = encfsbinpath;
    m_encfsbin_field->ChangeValue(currentEncFSBinPath);
}

void frmSettingsDialog::SetCurrentMountBinPath(wxString &mountbinpath)
{
    currentMountBinPath = mountbinpath;
    m_mountbin_field->ChangeValue(currentMountBinPath);
}

void frmSettingsDialog::SetCurrentUMountBinPath(wxString &umountbinpath)
{
    currentUMountBinPath = umountbinpath;
    m_umountbin_field->ChangeValue(currentUMountBinPath);
}


void frmSettingsDialog::SelectEncFSBinPath(wxCommandEvent& WXUNUSED(event))
{

    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    wxString currentbin;
    currentbin = getEncFSBinPath();

    wxFileDialog openFileDialog(this, _("Select full path to 'encfs' executable"), currentbin, "",
                       "All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_OK)
    {
        wxString fn = openFileDialog.GetPath();
        
        if ( wxFileName::FileExists(fn) )
        {
            SetCurrentEncFSBinPath(fn);
        }
    }
    openFileDialog.Destroy();
}

void frmSettingsDialog::SelectMountBinPath(wxCommandEvent& WXUNUSED(event))
{
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    wxString currentbin;
    currentbin = pConfig->Read(wxT("mountbinpath"), "/sbin/mount");
    wxFileDialog openFileDialog(this, _("Select full path to 'mount' executable"), currentbin, "",
                       "All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_OK)
    {
        wxString fn = openFileDialog.GetPath();
        
        if ( wxFileName::FileExists(fn) )
        {
            SetCurrentMountBinPath(fn);
        }
    }
    openFileDialog.Destroy();
}


void frmSettingsDialog::SelectUMountBinPath(wxCommandEvent& WXUNUSED(event))
{
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    wxString currentbin;
    currentbin = pConfig->Read(wxT("umountbinpath"), "/sbin/umount");
    wxFileDialog openFileDialog(this, _("Select full path to 'umount' executable"), currentbin, "",
                       "All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_OK)
    {
        wxString fn = openFileDialog.GetPath();
        
        if ( wxFileName::FileExists(fn) )
        {
            SetCurrentUMountBinPath(fn);
        }
    }
    openFileDialog.Destroy();
}


void frmSettingsDialog::SaveSettings(wxCommandEvent& WXUNUSED(event))
{
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    pConfig->Write(wxT("encfsbinpath"), m_encfsbin_field->GetValue());
    pConfig->Write(wxT("mountbinpath"), m_mountbin_field->GetValue());
    pConfig->Write(wxT("umountbinpath"), m_umountbin_field->GetValue());
    pConfig->Write(wxT("startatlogin"), m_chkbx_startatlogin->GetValue());
    pConfig->Write(wxT("autounmount"), m_chkbx_unmount_on_quit->GetValue());
    pConfig->Flush();
    Close(true);
}

void frmSettingsDialog::Create()
{

    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));

    // add controls to the settings page
    // put everything into a master Sizer
    wxSizer * const sizerTop = new wxBoxSizer(wxVERTICAL);
    
    // section Global options
    wxSizer * const sizerGlobal = new wxStaticBoxSizer(wxVERTICAL, this, "Application dependencies / binaries");

    // encfspath
    sizerGlobal->Add(new wxStaticText(this, wxID_ANY, "&Full path to 'encfs' executable:"));
    m_encfsbin_field = new wxTextCtrl(this, wxID_ANY, getEncFSBinPath());
    sizerGlobal->Add(m_encfsbin_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 10).Expand());
    sizerGlobal->Add(new wxButton( this , ID_BTN_CHOOSE_ENCFS, wxT("Choose 'encfs' executable")));

    sizerGlobal->AddSpacer(15);

    // mount binary
    sizerGlobal->Add(new wxStaticText(this, wxID_ANY, "&Full path to 'mount' executable:"));    
    m_mountbin_field = new wxTextCtrl(this, wxID_ANY, pConfig->Read(wxT("mountbinpath"), "/sbin/mount"));
    sizerGlobal->Add(m_mountbin_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 10).Expand());
    sizerGlobal->Add(new wxButton( this , ID_BTN_CHOOSE_MOUNT, wxT("Choose 'mount' executable")));

    sizerGlobal->AddSpacer(15);

    // umount binary
    sizerGlobal->Add(new wxStaticText(this, wxID_ANY, "&Full path to 'umount' executable:"));   
    m_umountbin_field = new wxTextCtrl(this, wxID_ANY, pConfig->Read(wxT("umountbinpath"), "/sbin/umount"));
    sizerGlobal->Add(m_umountbin_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 10).Expand());
    sizerGlobal->Add(new wxButton( this , ID_BTN_CHOOSE_UMOUNT, wxT("Choose 'umount' executable")));


    // start at login
    wxSizer * const sizerStartup = new wxStaticBoxSizer(wxVERTICAL, this, "Startup & exit options");
    m_chkbx_startatlogin  = new wxCheckBox(this, ID_CHECK_STARTATLOGIN, "Start encfsgui at login");
    // 0l = disabled by default
    // 1l = enabled by default
    m_chkbx_startatlogin->SetValue(pConfig->Read(wxT("startatlogin"), 0l) != 0);
    sizerStartup->Add(m_chkbx_startatlogin);

    // unmount when exit
    m_chkbx_unmount_on_quit  = new wxCheckBox(this, ID_CHECK_UNMOUNT_ON_QUIT, "Auto unmount volumes when closing app");
    // 0l = disabled by default
    // 1l = enabled by default
    m_chkbx_unmount_on_quit->SetValue(pConfig->Read(wxT("autounmount"), 0l) != 0);
    sizerStartup->Add(m_chkbx_unmount_on_quit);

    // glue together
    sizerTop->Add(sizerGlobal, wxSizerFlags(1).Expand().Border());
    sizerTop->Add(sizerStartup, wxSizerFlags(1).Expand().Border());

    // Add "Apply" and "Cancel"
    sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCANCEL),
                  wxSizerFlags().Right().Border());

    CentreOnScreen();

    //SetSizerAndFit(sizerTop);
    SetSizer(sizerTop);

}

///
//
//

void openSettings(wxWindow *parent)
{   
    wxSize dlgSettingsSize;
    // make height larger when adding more options
    dlgSettingsSize.Set(400,450);

    long style = wxDEFAULT_DIALOG_STYLE;// | wxRESIZE_BORDER;

    wxString strTitle;
    strTitle.Printf( "EncFSGui Settings");  

    frmSettingsDialog* dlg = new frmSettingsDialog(parent, 
                                                   strTitle, 
                                                   wxDefaultPosition, 
                                                   dlgSettingsSize, 
                                                   style);
    dlg->Create();
    dlg->ShowModal();
}
