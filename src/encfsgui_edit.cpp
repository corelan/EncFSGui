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
    ID_BTN_CHOOSE_DESTINATION,
    ID_CHKBX_SAVEPASSWORD
};



// event table
wxBEGIN_EVENT_TABLE(frmEditDialog, wxDialog)
    EVT_BUTTON(ID_BTN_CHOOSE_DESTINATION,  frmEditDialog::ChooseDestinationFolder)
    EVT_BUTTON(wxID_APPLY, frmEditDialog::SaveSettings)
    EVT_CHECKBOX(ID_CHKBX_SAVEPASSWORD, frmEditDialog::ChangePWFieldState)
wxEND_EVENT_TABLE()


// constructor
frmEditDialog::frmEditDialog(wxWindow *parent, 
                           const wxString& title, 
                           const wxPoint &pos, 
                           const wxSize &size, 
                           long style,
                           wxString selectedvolume,
                           std::map<wxString, DBEntry*> volumedata) :  wxDialog(parent, wxID_ANY, title, pos, size, style)
{
    m_volumename = selectedvolume;
    DBEntry * thisvol = volumedata[selectedvolume];
    m_mounted = thisvol->getMountState();
    m_editVolumeData = volumedata;
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
    m_pwsaved = savedpassword;

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
    m_chkbx_save_password  = new wxCheckBox(this, ID_CHKBX_SAVEPASSWORD, "Save password in Keychain");
    m_chkbx_save_password->SetValue(savedpassword);
    sizerPassword->Add(m_chkbx_save_password);

    sizerPassword->Add(new wxStaticText(this, wxID_ANY, "Set/update keychain password: (this won't change the password of encfs itself)"));
    sizerPassword->AddSpacer(5);
    wxSizer * const sizerPW1 = new wxBoxSizer(wxHORIZONTAL);
    sizerPW1->Add(new wxStaticText(this, wxID_ANY, "Enter password:"));
    m_pass1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200,22), wxTE_PASSWORD);
    sizerPW1->Add(m_pass1, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerPassword->Add(sizerPW1, wxSizerFlags(1).Expand().Border());

    wxSizer * const sizerPW2 = new wxBoxSizer(wxHORIZONTAL);
    sizerPW2->Add(new wxStaticText(this, wxID_ANY, "Enter password again:"));
    m_pass2 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200,22), wxTE_PASSWORD);
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


    // disable/enable controls based on 'save password' state
    if (m_chkbx_save_password->GetValue())
    {
        m_pass1->Enable();
        m_pass2->Enable();
    }
    else
    {
        m_pass1->Disable();
        m_pass2->Disable();
    }

    // disable/enable controls based on mount state
    if (m_mounted)
    {
        m_volumename_field->Disable();
        m_destination_field->Disable();
        m_selectdst_button->Disable();
    }


    CentreOnScreen();

    SetSizer(sizerMaster);

}

void frmEditDialog::ChooseDestinationFolder(wxCommandEvent& WXUNUSED(event))
{
    wxString currentdir;
    currentdir = m_destination_field->GetValue();
    if (currentdir.IsEmpty())
    {
        currentdir = "/Volumes";
    }
    wxDirDialog openDirDialog(this, 
                              "Select destination mount point folder", 
                              currentdir, 
                              wxDD_DEFAULT_STYLE);
    if (openDirDialog.ShowModal() == wxID_OK)
    {
       wxString fn = openDirDialog.GetPath();
        m_destination_field->SetValue(fn);
    }
    openDirDialog.Destroy();
}


void frmEditDialog::ChangePWFieldState(wxCommandEvent& WXUNUSED(event))
{
    if (m_chkbx_save_password->GetValue())
    {
        m_pass1->Enable();
        m_pass2->Enable();
    }
    else
    {
        m_pass1->Disable();
        m_pass2->Disable();
    }
}


void frmEditDialog::SaveSettings(wxCommandEvent& WXUNUSED(event))
{

    // has Volumename been changed -> cause rename
    wxString errormsg;
    bool volname_ok = true;
    bool dst_folder_ok = true;
    bool renameneeded = false;
    if (m_volumename_field->GetValue().IsEmpty())
    {
        m_volumename_field->SetValue(m_volumename);
    }
    wxString newvolname = m_volumename_field->GetValue();

    //sanitize the volume name
    newvolname.Replace("/","");
    newvolname.Replace(" ","");
    newvolname.Replace("'","");
    newvolname.Replace('"',"");
    m_volumename_field->SetValue(newvolname);

    if (!(m_volumename == newvolname))
    {
        // check if the new name is still unique
        if (doesVolumeExist(newvolname))
        {
            errormsg << "- The new volume name is not unique\n";
            volname_ok = false;            
        }
        else
        {
            renameneeded = true;
        }
    }

    //Does destination folder exist, and is it empty ?
    // only check when not mounted, or check will fail
    if (!m_mounted)
    {
        wxString dstfolder = m_destination_field->GetValue();
        if (!dstfolder.IsEmpty())
        {
            wxDir * dir = new wxDir(dstfolder);
            if (!dir->Exists(dstfolder))
            {
                errormsg << "- Please specify a valid/existing destination mount point location\n";
                dst_folder_ok = false;
            }
            else
            {
                if (dir->HasFiles() && dir->HasSubDirs())
                {
                    dst_folder_ok = false;
                    errormsg << "- Destination mount point is not empty\n";
                }
                else
                {
                    dst_folder_ok = true;   
                }
                
            }
        }    
    }
    
    if (!volname_ok || !dst_folder_ok)
    {
        wxString title;
        title.Printf(wxT("Errors found:"));
        wxMessageDialog * dlg = new wxMessageDialog(this, errormsg, title, wxOK|wxCENTRE|wxICON_ERROR);
        dlg->ShowModal();
        dlg->Destroy();
    }
    else
    {
        // execute actions
        if (renameneeded)
        {
            // rename volumne name in config
            // no need to update map, will be repopulated anyway
            wxString oldvolname = m_volumename;
            renameVolume(m_volumename, newvolname);
            // rename password entry in Keychain if pw was saved
            if (m_pwsaved)
            {
                wxString cmd;
                wxString pwaddoutput;
                wxString previouspw;
                // get previous pass first
                previouspw = getKeychainPassword(oldvolname);
                // remove old entry
                cmd.Printf(wxT("sh -c \"security delete-generic-password -U -a 'EncFSGUI_%s' -s 'EncFSGUI_%s' login.keychain\""), oldvolname, oldvolname);
                pwaddoutput = StrRunCMDSync(cmd);
                // add entry with new name
                cmd.Printf(wxT("sh -c \"security add-generic-password -U -a 'EncFSGUI_%s' -s 'EncFSGUI_%s' -w '%s' login.keychain\""), newvolname, newvolname, previouspw);
                pwaddoutput = StrRunCMDSync(cmd);
                previouspw = "";

            }
            m_volumename = newvolname;
        }

        // update destination mount point and mount options
        wxConfigBase *pConfig = wxConfigBase::Get();
        wxString config_volname;
        config_volname.Printf(wxT("/Volumes/%s"), m_volumename);
        pConfig->SetPath(config_volname);
        wxString mount_path = m_destination_field->GetValue();
        pConfig->Write(wxT("mount_path"), mount_path );
        pConfig->Write(wxT("automount"), m_chkbx_automount->GetValue());
        pConfig->Write(wxT("preventautounmount"),m_chkbx_prevent_autounmount->GetValue());
        pConfig->Flush();

        bool okToClose = true;
        // password updates needed ?
        // if pw was saved, and setting changed, ask for confirmation to be sure
        bool savepw = m_chkbx_save_password->GetValue();
        wxString msgtitle;
        wxString msgbody;
        if (m_pwsaved && !savepw)
        {
            // remove entry from keychain?
            msgtitle.Printf(wxT("Remove password from Keychain?"));
            msgbody.Printf(wxT("Are you sure you want to remove the password for volume '%s' from Keychain?"), newvolname);
            wxMessageDialog * dlg = new wxMessageDialog(this, 
                                                            msgbody, 
                                                            msgtitle, 
                                                            wxYES_NO|wxCENTRE|wxNO_DEFAULT|wxICON_QUESTION);
            if (dlg->ShowModal() == wxID_YES)
            {
                wxString cmd;
                wxString pwaddoutput;
                cmd.Printf(wxT("sh -c \"security delete-generic-password -U -a 'EncFSGUI_%s' -s 'EncFSGUI_%s' login.keychain\""), newvolname, newvolname);
                pwaddoutput = StrRunCMDSync(cmd);
                pConfig->SetPath(config_volname);
                pConfig->Write(wxT("passwordsaved"), false);
                pConfig->Flush();
                okToClose = true;
            }
        }
        else if (!m_pwsaved && savepw)
        {
            // password was not saved before, save it now ?
            if ( not (m_pass1->GetValue().IsEmpty()) && (m_pass1->GetValue() == m_pass2->GetValue()))
            {
                msgtitle.Printf(wxT("Save password into Keychain?"));
                msgbody.Printf(wxT("Are you sure you want to save the password for volume '%s' into Keychain?"), newvolname);
                wxMessageDialog * dlg = new wxMessageDialog(this, 
                                                                msgbody, 
                                                                msgtitle, 
                                                                wxYES_NO|wxCENTRE|wxNO_DEFAULT|wxICON_QUESTION);
                if (dlg->ShowModal() == wxID_YES)
                {
                    wxString cmd;
                    wxString pwaddoutput;
                    wxString pw;
                    pw = m_pass1->GetValue();
                    cmd.Printf(wxT("sh -c \"security add-generic-password -U -a 'EncFSGUI_%s' -s 'EncFSGUI_%s' -w '%s' login.keychain\""), newvolname, newvolname, pw);
                    pw = "";
                    pwaddoutput = StrRunCMDSync(cmd);
                    pConfig->SetPath(config_volname);
                    pConfig->Write(wxT("passwordsaved"), true);
                    pConfig->Flush();
                    okToClose = true;
                }
            }
        }
        else if (m_pwsaved && savepw)
        {
            // if new passwords were entered, update Keychain with them ?
            if ( not (m_pass1->GetValue().IsEmpty()) && (m_pass1->GetValue() == m_pass2->GetValue()))
            {
                msgtitle.Printf(wxT("Update Keychain password?"));
                msgbody.Printf(wxT("Are you sure you want to update the Keychain password for volume '%s'?"), newvolname);
                wxMessageDialog * dlg = new wxMessageDialog(this, 
                                                                msgbody, 
                                                                msgtitle, 
                                                                wxYES_NO|wxCENTRE|wxNO_DEFAULT|wxICON_QUESTION);
                if (dlg->ShowModal() == wxID_YES)
                {
                    wxString cmd;
                    wxString pwaddoutput;
                    wxString pw;
                    pw = m_pass1->GetValue();
                    cmd.Printf(wxT("sh -c \"security add-generic-password -U -a 'EncFSGUI_%s' -s 'EncFSGUI_%s' -w '%s' login.keychain\""), newvolname, newvolname, pw);
                    pw = "";
                    pwaddoutput = StrRunCMDSync(cmd);
                    pConfig->SetPath(config_volname);
                    pConfig->Write(wxT("passwordsaved"), true);
                    pConfig->Flush();
                    okToClose = true;
                }
            }
        }
        if (okToClose)
        {
            Close(true);
        }
    }
}




// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

void editExistingEncFSFolder(wxWindow *parent, wxString& selectedvolume, std::map<wxString, DBEntry*> volumedata)
{
    wxSize frmEditSize;
    frmEditSize.Set(600,540);
    long framestyle;
    framestyle = wxDEFAULT_FRAME_STYLE | wxFRAME_EX_METAL;

    wxString strTitle;
    strTitle.Printf(wxT("Edit EncFS folder '%s'"), selectedvolume); 
    bool ismounted = false;
    DBEntry * thisvol = volumedata[selectedvolume];
    ismounted = thisvol->getMountState();
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
                                           volumedata);
    dlg->Create();
    dlg->ShowModal();
}