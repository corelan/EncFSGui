/*
    encFSGui - encfsgui_edit.cpp
    This file contains code to edit the 
    configuration of encFS folders

    written by Peter Van Eeckhoutte

*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/dirdlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/ffile.h>
#include <wx/file.h>
#include <wx/time.h>
#include <wx/stdpaths.h>
#include <vector>
#include <map>

#include "encfsgui.h"


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

enum
{
    ID_BTN_CHOOSE_DESTINATION
};


// ----------------------------------------------------------------------------
// Classes
// ----------------------------------------------------------------------------

//   ------------------------------------------
// frmEditDialog - edit an encfs folder
//   ------------------------------------------


class frmEditDialog : public wxDialog
{
public:
    frmEditDialog(wxWindow *parent, 
                 const wxString& title, 
                 const wxPoint& pos, 
                 const wxSize& size, 
                 long style,
                 wxString selectedvolume,
                 bool ismounted);
    void Create();
    void ChooseDestinationFolder(wxCommandEvent &event);
    void SaveSettings(wxCommandEvent &event);
private:
    wxString m_volumename;
    wxTextCtrl * m_destination_field;
    wxTextCtrl * m_volumename_field;
    wxTextCtrl * m_pass_current;
    wxTextCtrl * m_pass1;
    wxTextCtrl * m_pass2;
    wxCheckBox * m_chkbx_automount;
    wxCheckBox * m_chkbx_prevent_autounmount;
    wxCheckBox * m_chkbx_save_password;
    wxButton * m_selectdst_button;
    bool m_mounted;
    wxDECLARE_EVENT_TABLE();
};

// event table
wxBEGIN_EVENT_TABLE(frmEditDialog, wxDialog)
    EVT_BUTTON(ID_BTN_CHOOSE_DESTINATION,  frmEditDialog::ChooseDestinationFolder)
    EVT_BUTTON(wxID_APPLY, frmEditDialog::SaveSettings)
wxEND_EVENT_TABLE()


// constructor
frmEditDialog::frmEditDialog(wxWindow *parent, 
                           const wxString& title, 
                           const wxPoint &pos, 
                           const wxSize &size, 
                           long style,
                           wxString selectedvolume,
                           bool ismounted) :  wxDialog(parent, wxID_ANY, title, pos, size, style)
{
    m_volumename = selectedvolume;
    m_mounted = ismounted;
}


void frmEditDialog::Create()
{

    wxString config_volname;
    wxString srcfolder;
    wxString dstfolder;
    bool automount;
    bool prevent_autounmount;
    bool savedpassword;

    wxConfigBase *pConfig = wxConfigBase::Get();
    config_volname.Printf(wxT("/Volumes/%s"), m_volumename);
    pConfig->SetPath(config_volname);
    srcfolder = pConfig->Read(wxT("enc_path"));
    dstfolder = pConfig->Read(wxT("mount_path"));
    automount = pConfig->ReadBool(wxT("automount"), 0l);
    prevent_autounmount = pConfig->ReadBool(wxT("preventautounmount"),0l);
    savedpassword = pConfig->ReadBool(wxT("passwordsaved"),0l);

    wxSizer * const sizerMaster = new wxBoxSizer(wxVERTICAL);
    wxSizer * const sizerVolume = new wxStaticBoxSizer(wxVERTICAL, this, "Volume");
    wxSizer * const sizerVolName = new wxBoxSizer(wxHORIZONTAL);

    // desired volume name
    sizerVolName->Add(new wxStaticText(this, wxID_ANY, "&Volume name:"));
    m_volumename_field = new wxTextCtrl(this, wxID_ANY, m_volumename, wxDefaultPosition, wxSize(300,22));
    sizerVolName->Add(m_volumename_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerVolume->Add(sizerVolName, wxSizerFlags(1).Expand().Border());

    // folder locations    
    sizerVolume->Add(new wxStaticText(this, wxID_ANY, "&Location of encrypted Encfs folder:"));

    wxSizer * const sizerSRC = new wxBoxSizer(wxHORIZONTAL);
    sizerSRC->Add(new wxStaticText(this, wxID_ANY, srcfolder));
    sizerVolume->Add(sizerSRC,wxSizerFlags(1).Expand().Border());

    // mount point
    sizerVolume->Add(new wxStaticText(this, wxID_ANY, "&Destination (mount) folder:"));
    wxSizer * const sizerDST = new wxBoxSizer(wxHORIZONTAL);
    m_destination_field = new wxTextCtrl(this, wxID_ANY, dstfolder, wxDefaultPosition, wxSize(470,22));
    sizerDST->Add(m_destination_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    m_selectdst_button = new wxButton( this , ID_BTN_CHOOSE_DESTINATION, wxT("Select"));
    sizerDST->Add(m_selectdst_button);
    sizerVolume->Add(sizerDST,wxSizerFlags(1).Expand().Border());

    // password settings
    wxSizer * const sizerPassword = new wxStaticBoxSizer(wxVERTICAL, this, "Password settings");
    m_chkbx_save_password  = new wxCheckBox(this, wxID_ANY, "Save password in Keychain");
    m_chkbx_save_password->SetValue(savedpassword);
    sizerPassword->Add(m_chkbx_save_password);

    sizerPassword->Add(new wxStaticText(this, wxID_ANY, "Change password:"));
    wxSizer * const sizerPW0 = new wxBoxSizer(wxHORIZONTAL);    
    sizerPW0->Add(new wxStaticText(this, wxID_ANY, "Enter current password:"));
    m_pass_current = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizerPW0->Add(m_pass_current, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerPassword->Add(sizerPW0, wxSizerFlags(1).Expand().Border());

    wxSizer * const sizerPW1 = new wxBoxSizer(wxHORIZONTAL);
    sizerPW1->Add(new wxStaticText(this, wxID_ANY, "Enter new password:"));
    m_pass1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizerPW1->Add(m_pass1, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerPassword->Add(sizerPW1, wxSizerFlags(1).Expand().Border());

    wxSizer * const sizerPW2 = new wxBoxSizer(wxHORIZONTAL);
    sizerPW2->Add(new wxStaticText(this, wxID_ANY, "Enter new password again:"));
    m_pass2 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizerPW2->Add(m_pass2, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerPassword->Add(sizerPW2, wxSizerFlags(1).Expand().Border());


    // mount options
    wxSizer * const sizerMount = new wxStaticBoxSizer(wxVERTICAL, this, "Mount options");
    m_chkbx_automount  = new wxCheckBox(this, wxID_ANY, "Automatically mount this volume when application starts");
    m_chkbx_automount->SetValue(automount);
    sizerMount->Add(m_chkbx_automount);

    // prevent auto unmount
    m_chkbx_prevent_autounmount  = new wxCheckBox(this, wxID_ANY, "Prevent auto-unmounting this volume on application exit");
    m_chkbx_prevent_autounmount->SetValue(prevent_autounmount);
    sizerMount->Add(m_chkbx_prevent_autounmount);

    sizerMaster->Add(sizerVolume, wxSizerFlags(1).Expand().Border());
    sizerMaster->Add(sizerPassword, wxSizerFlags(1).Expand().Border());
    sizerMaster->Add(sizerMount, wxSizerFlags(1).Expand().Border());

    // Add "Apply" and "Cancel"
    sizerMaster->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCANCEL), wxSizerFlags().Right().Border());


    // disable/enable controls based on mount state
    if (m_mounted)
    {
        m_volumename_field->Disable();
        m_destination_field->Disable();
        m_chkbx_save_password->Disable();
        m_pass_current->Disable();
        m_pass1->Disable();
        m_pass2->Disable();
        m_selectdst_button->Disable();
    }

    CentreOnScreen();

    SetSizer(sizerMaster);

}

void frmEditDialog::ChooseDestinationFolder(wxCommandEvent& WXUNUSED(event))
{

}

void frmEditDialog::SaveSettings(wxCommandEvent& WXUNUSED(event))
{

}




// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

void editExistingEncFSFolder(wxWindow *parent, wxString& selectedvolume, bool ismounted)
{
    wxSize frmEditSize;
    frmEditSize.Set(600,540);
    long framestyle;
    framestyle = wxDEFAULT_FRAME_STYLE | wxFRAME_EX_METAL;

    wxString strTitle;
    strTitle.Printf( "Edit EncFS folder"); 
    if (ismounted)
    {
        strTitle << " (MOUNTED)";
    }
    
    frmEditDialog* dlg = new frmEditDialog(parent, 
                                           strTitle, 
                                           wxDefaultPosition, 
                                           frmEditSize, 
                                           framestyle,
                                           selectedvolume,
                                           ismounted);
    dlg->Create();
    dlg->ShowModal();
}