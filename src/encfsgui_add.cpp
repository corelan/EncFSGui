/*
    encFSGui - source file contains
    code to handle opening existing encFS folders
    or create new folders

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
#include <vector>
#include "encfsgui.h"


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

enum
{
    ID_BTN_CHOOSE_SOURCE,
    ID_BTN_CHOOSE_DESTINATION,
    ID_RADIO_PROFILE,
    ID_ENCFSPROFILE_BALANCED,
    ID_ENCFSPROFILE_PERFORMANCE,
    ID_ENCFSPROFILE_SECURE,
    ID_ENCFSPROFILE_CUSTOM
};


// ----------------------------------------------------------------------------
// Classes
// ----------------------------------------------------------------------------

//   ------------------------------------------
// frmAddDialog - create a new encfs folder
//   ------------------------------------------


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
    wxCheckBox * m_chkbx_perfile_iv;
    wxCheckBox * m_chkbx_external_iv_chaining;
    wxCheckBox * m_chkbx_filename_to_iv_header_chaining;
    wxCheckBox * m_chkbx_block_mac_headers;
    wxComboBox * m_combo_cipher_algo;
    wxComboBox * m_combo_cipher_keysize;
    wxComboBox * m_combo_cipher_blocksize;
    wxComboBox * m_combo_filename_enc;
    wxComboBox * m_combo_keyderivation;
    wxDECLARE_EVENT_TABLE();
    void SetEncfsOptionsState(bool);
    bool createEncFSFolder();
};

// event table
wxBEGIN_EVENT_TABLE(frmAddDialog, wxDialog)
    EVT_BUTTON(ID_BTN_CHOOSE_SOURCE,  frmAddDialog::ChooseSourceFolder)
    EVT_BUTTON(ID_BTN_CHOOSE_DESTINATION,  frmAddDialog::ChooseDestinationFolder)
    EVT_BUTTON(wxID_APPLY, frmAddDialog::SaveSettings)
    EVT_RADIOBOX(ID_RADIO_PROFILE, frmAddDialog::SetEncFSProfileSelection)
wxEND_EVENT_TABLE()


// constructor
frmAddDialog::frmAddDialog(wxWindow *parent, 
                           const wxString& title, 
                           const wxPoint &pos, 
                           const wxSize &size, 
                           long style) :  wxDialog(parent, wxID_ANY, title, pos, size, style)
{

}

// event functions
void frmAddDialog::SetEncFSProfileSelection(wxCommandEvent& event)
{
    int selectedProfile = ID_ENCFSPROFILE_BALANCED;
    wxString selectedOption;
    selectedOption = event.GetString();

    if (selectedOption == "Balanced")
    {
        selectedProfile = ID_ENCFSPROFILE_BALANCED;
    }
    else if (selectedOption == "Secure")
    {
        selectedProfile = ID_ENCFSPROFILE_SECURE;
    }
    else if (selectedOption == "Performance")
    {
        selectedProfile = ID_ENCFSPROFILE_PERFORMANCE;
    }
    else if (selectedOption == "Custom")
    {
        selectedProfile = ID_ENCFSPROFILE_CUSTOM;
    }

    ApplyEncFSProfileSelection(selectedProfile);
}


// member functions

void frmAddDialog::SetEncfsOptionsState(bool enabledstate)
{
    if (!enabledstate)
    {
        m_combo_cipher_algo->Disable();
        m_combo_cipher_keysize->Disable();
        m_combo_cipher_blocksize->Disable();
        m_combo_filename_enc->Disable();
        m_combo_keyderivation->Disable();
        m_chkbx_perfile_iv->Disable();
        m_chkbx_block_mac_headers->Disable();
        m_chkbx_external_iv_chaining->Disable();
        m_chkbx_filename_to_iv_header_chaining->Disable();
    }
    else
    {
        m_combo_cipher_algo->Enable();
        m_combo_cipher_keysize->Enable();
        m_combo_cipher_blocksize->Enable();
        m_combo_filename_enc->Enable();
        m_combo_keyderivation->Enable();
        m_chkbx_perfile_iv->Enable();
        m_chkbx_block_mac_headers->Enable();
        m_chkbx_external_iv_chaining->Enable();
        m_chkbx_filename_to_iv_header_chaining->Enable();        
    }

}


// member functions

void frmAddDialog::ApplyEncFSProfileSelection(int SelectedProfile)
{
    if (SelectedProfile == ID_ENCFSPROFILE_BALANCED)
    {
        m_combo_cipher_algo->SetValue("AES");
        m_combo_cipher_blocksize->SetValue("2048");
        m_combo_cipher_keysize->SetValue("192");
        m_combo_filename_enc->SetValue("Stream");
        m_combo_keyderivation->SetValue("500");
        m_chkbx_block_mac_headers->SetValue(false);
        m_chkbx_perfile_iv->SetValue(true);
        m_chkbx_external_iv_chaining->SetValue(false);
        m_chkbx_filename_to_iv_header_chaining->SetValue(true);
        SetEncfsOptionsState(false);
    }
    else if (SelectedProfile == ID_ENCFSPROFILE_SECURE)
    {
        m_combo_cipher_algo->SetValue("AES");
        m_combo_cipher_blocksize->SetValue("4096");
        m_combo_cipher_keysize->SetValue("256");
        m_combo_filename_enc->SetValue("Block");
        m_combo_keyderivation->SetValue("1000");
        m_chkbx_block_mac_headers->SetValue(true);
        m_chkbx_perfile_iv->SetValue(true);
        m_chkbx_external_iv_chaining->SetValue(false);
        m_chkbx_filename_to_iv_header_chaining->SetValue(true);
        SetEncfsOptionsState(false);        
    }
    else if (SelectedProfile == ID_ENCFSPROFILE_PERFORMANCE)
    {
        m_combo_cipher_algo->SetValue("AES");
        m_combo_cipher_blocksize->SetValue("1024");
        m_combo_cipher_keysize->SetValue("192");
        m_combo_filename_enc->SetValue("Stream");
        m_combo_keyderivation->SetValue("500");
        m_chkbx_block_mac_headers->SetValue(false);
        m_chkbx_perfile_iv->SetValue(false);
        m_chkbx_external_iv_chaining->SetValue(false);
        m_chkbx_filename_to_iv_header_chaining->SetValue(false);        
        SetEncfsOptionsState(false);
    }
    else if (SelectedProfile == ID_ENCFSPROFILE_CUSTOM)
    {
        SetEncfsOptionsState(true);
    }
}


void frmAddDialog::Create()
{
    wxSizer * const sizerMaster = new wxBoxSizer(wxVERTICAL);
    wxSizer * const sizerVolume = new wxStaticBoxSizer(wxVERTICAL, this, "Volume settings");
    
    // desired volume name
    sizerVolume->Add(new wxStaticText(this, wxID_ANY, "&Set unique volume name:"));
    m_volumename_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    sizerVolume->Add(m_volumename_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());

    // encrypted folder

    sizerVolume->Add(new wxStaticText(this, wxID_ANY, "&Location of new empty encfs folder:"));

    wxSizer * const sizerSRC = new wxBoxSizer(wxHORIZONTAL);
    m_source_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(470,22));
    sizerSRC->Add(m_source_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerSRC->Add(new wxButton( this , ID_BTN_CHOOSE_SOURCE, wxT("Select")));
    sizerVolume->Add(sizerSRC,wxSizerFlags(1).Expand().Border());

    // mount point
    sizerVolume->Add(new wxStaticText(this, wxID_ANY, "&Destination (mount) folder:"));
    wxSizer * const sizerDST = new wxBoxSizer(wxHORIZONTAL);
    m_destination_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(470,22));
    sizerDST->Add(m_destination_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerDST->Add(new wxButton( this , ID_BTN_CHOOSE_DESTINATION, wxT("Select")));
    sizerVolume->Add(sizerDST,wxSizerFlags(1).Expand().Border());


    // Security settings
    wxArrayString arrProfilechoices;
    arrProfilechoices.Add(wxT("Balanced"));
    arrProfilechoices.Add(wxT("Performance"));
    arrProfilechoices.Add(wxT("Secure"));
    arrProfilechoices.Add(wxT("Custom"));

    wxSizer * const sizerEncFS = new wxStaticBoxSizer(wxVERTICAL, this, "EncFS options");
    // 4 profiles
    // 1. Balanced    2. Performance    3. Security   4. Custom
    wxRadioBox * encProfiles = new wxRadioBox(this, ID_RADIO_PROFILE, "EncFS profile", wxDefaultPosition, wxDefaultSize, arrProfilechoices, 0, wxRA_SPECIFY_COLS);
    sizerEncFS->Add(encProfiles);
    sizerEncFS->AddSpacer(10);

    wxSizer * const sizerEncFS_row1 = new wxBoxSizer(wxHORIZONTAL);

    // Cipher
    wxArrayString arrAlgos;
    arrAlgos.Add("AES");
    //arrAlgos.Add("Blowfish");
    wxArrayString arrKeySizes;
    arrKeySizes.Add("128");
    arrKeySizes.Add("192");
    arrKeySizes.Add("256");
    wxArrayString arrBlockSizes;
    for (int blocksize = 64; blocksize <= 4096; blocksize += 16)
    {
        wxString thissize;
        thissize.Printf(wxT("%d"), blocksize);
        arrBlockSizes.Add(thissize);
    }
    wxArrayString arrFilenameEnc;
    arrFilenameEnc.Add("Block");
    arrFilenameEnc.Add("Stream");
    arrFilenameEnc.Add("Null");
    wxArrayString arrKeyDerivationDuration;
    arrKeyDerivationDuration.Add("500");
    arrKeyDerivationDuration.Add("1000");
    arrKeyDerivationDuration.Add("2000");
    arrKeyDerivationDuration.Add("3000");
    arrKeyDerivationDuration.Add("4000");
    arrKeyDerivationDuration.Add("5000");

    // row 1 : cipher settings
    sizerEncFS_row1->Add(new wxStaticText(this, wxID_ANY, "Cipher algorithm:"));
    m_combo_cipher_algo = new wxComboBox(this, wxID_ANY, arrAlgos[0], wxDefaultPosition, wxDefaultSize, arrAlgos, wxCB_READONLY);
    sizerEncFS_row1->Add(m_combo_cipher_algo,wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerEncFS_row1->Add(new wxStaticText(this, wxID_ANY, "Keysize:"));
    m_combo_cipher_keysize = new wxComboBox(this, wxID_ANY, arrKeySizes[0], wxDefaultPosition, wxDefaultSize, arrKeySizes, wxCB_READONLY);
    sizerEncFS_row1->Add(m_combo_cipher_keysize,wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerEncFS_row1->Add(new wxStaticText(this, wxID_ANY, "Blocksize"));
    m_combo_cipher_blocksize = new wxComboBox(this, wxID_ANY, arrBlockSizes[0], wxDefaultPosition, wxDefaultSize, arrBlockSizes, wxCB_READONLY);
    sizerEncFS_row1->Add(m_combo_cipher_blocksize,wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerEncFS->Add(sizerEncFS_row1);

    // row 2 : filename encoding & key derivation
    wxSizer * const sizerEncFS_row2 = new wxBoxSizer(wxHORIZONTAL);
    sizerEncFS_row2->Add(new wxStaticText(this, wxID_ANY, "Filename encoding:"));
    m_combo_filename_enc = new wxComboBox(this, wxID_ANY, arrFilenameEnc[0], wxDefaultPosition, wxDefaultSize, arrFilenameEnc, wxCB_READONLY);
    sizerEncFS_row2->Add(m_combo_filename_enc,wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerEncFS_row2->Add(new wxStaticText(this, wxID_ANY, "PBKDF2 key derivation duration (ms):"));
    m_combo_keyderivation = new wxComboBox(this, wxID_ANY, arrKeyDerivationDuration[0], wxDefaultPosition, wxDefaultSize, arrKeyDerivationDuration, wxCB_READONLY);
    sizerEncFS_row2->Add(m_combo_keyderivation,wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerEncFS->Add(sizerEncFS_row2);

    // row 3 : HMAC & IV settings
    wxSizer * const sizerEncFS_row3 = new wxBoxSizer(wxHORIZONTAL);
    m_chkbx_block_mac_headers  = new wxCheckBox(this, wxID_ANY, "Per-block HMAC");
    sizerEncFS_row3->Add(m_chkbx_block_mac_headers);
    m_chkbx_perfile_iv = new wxCheckBox(this, wxID_ANY, "Per-file unique IV");
    sizerEncFS_row3->Add(m_chkbx_perfile_iv);
    m_chkbx_filename_to_iv_header_chaining = new wxCheckBox(this, wxID_ANY, "Chained IV");
    sizerEncFS_row3->Add(m_chkbx_filename_to_iv_header_chaining);
    m_chkbx_external_iv_chaining = new wxCheckBox(this, wxID_ANY, "External IV");
    sizerEncFS_row3->Add(m_chkbx_external_iv_chaining);
    sizerEncFS->Add(sizerEncFS_row3);
    // row 4 : idle timings

    
    wxSizer * const sizerPassword = new wxStaticBoxSizer(wxVERTICAL, this, "Password options");

    wxSizer * const sizerPW1 = new wxBoxSizer(wxHORIZONTAL);
    sizerPW1->Add(new wxStaticText(this, wxID_ANY, "Enter password:"));
    m_pass1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizerPW1->Add(m_pass1, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerPassword->Add(sizerPW1, wxSizerFlags(1).Expand().Border());

    wxSizer * const sizerPW2 = new wxBoxSizer(wxHORIZONTAL);
    sizerPW2->Add(new wxStaticText(this, wxID_ANY, "Enter password again:"));
    m_pass2 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizerPW2->Add(m_pass2, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerPassword->Add(sizerPW2, wxSizerFlags(1).Expand().Border());

    // save password ?
    m_chkbx_save_password  = new wxCheckBox(this, wxID_ANY, "Save password in Keychain");
    m_chkbx_save_password->SetValue(false);
    sizerPassword->Add(m_chkbx_save_password);

    
    wxSizer * const sizerOptions = new wxStaticBoxSizer(wxVERTICAL, this, "Mount options");
    // auto mount ?
    m_chkbx_automount  = new wxCheckBox(this, wxID_ANY, "Automatically mount this volume when application starts");
    m_chkbx_automount->SetValue(false);
    sizerOptions->Add(m_chkbx_automount);

    // prevent auto unmount
    m_chkbx_prevent_autounmount  = new wxCheckBox(this, wxID_ANY, "Prevent auto-unmounting this volume on application exit");
    m_chkbx_prevent_autounmount->SetValue(false);
    sizerOptions->Add(m_chkbx_prevent_autounmount);

    // glue together
    sizerMaster->Add(sizerVolume, wxSizerFlags(1).Expand().Border());
    sizerMaster->Add(sizerEncFS, wxSizerFlags(1).Expand().Border());
    sizerMaster->Add(sizerPassword, wxSizerFlags(1).Expand().Border());    
    sizerMaster->Add(sizerOptions, wxSizerFlags(1).Expand().Border());

    // Add "Apply" and "Cancel"
    sizerMaster->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCANCEL), wxSizerFlags().Right().Border());

    CentreOnScreen();

    SetSizer(sizerMaster);

    // set encfs options based on profile selection
    ApplyEncFSProfileSelection(ID_ENCFSPROFILE_BALANCED);

}


void frmAddDialog::ChooseSourceFolder(wxCommandEvent& WXUNUSED(event))
{
    wxString currentdir;
    currentdir = m_source_field->GetValue();
    wxDirDialog openDirDialog(this, 
                              "Select desired location of new encfs encrypted folder", 
                              currentdir, 
                              wxDD_DEFAULT_STYLE);
    if (openDirDialog.ShowModal() == wxID_OK)
    {
        wxString fn = openDirDialog.GetPath();
        m_source_field->SetValue(fn);
    }
    openDirDialog.Destroy();
}

void frmAddDialog::ChooseDestinationFolder(wxCommandEvent& WXUNUSED(event))
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


bool frmAddDialog::createEncFSFolder()
{
    bool createdok = false;
    wxString encfsbin = getEncFSBinPath();
    wxString cmd;
    wxString args;
    wxString enc_path = m_source_field->GetValue();
    wxString pw = m_pass1->GetValue();
    wxString mount_path = m_destination_field->GetValue();
    wxString volumename = m_volumename_field->GetValue();

    cmd.Printf(wxT("sh -c \"echo '%s' | %s -v -S '%s' '%s' --volname='%s'\""), pw, encfsbin, enc_path, mount_path, volumename);
    pw = "";
    return createdok;
}


void frmAddDialog::SaveSettings(wxCommandEvent& WXUNUSED(event))
{
    // check if all important fiels were populated
    bool volname_ok = true;
    wxString errormsg = ""; 
    bool src_folder_ok = false;
    bool dst_folder_ok = false;
    bool pw_ok = false;
    wxString newvolumename = m_volumename_field->GetValue();

    // sanitize the name
    newvolumename.Replace("/","");
    newvolumename.Replace(" ","");
    newvolumename.Replace("'","");
    newvolumename.Replace('"',"");
    m_volumename_field->SetValue(newvolumename);

    //1. is volume name unique?
    if (doesVolumeExist(newvolumename))
    {
        volname_ok = false;
    }

    if (newvolumename.IsEmpty())
    {
        volname_ok = false;
    }

    if (!volname_ok)
    {
        errormsg << "- Please specify a unique volume name\n";
    }

    //2. Does source folder exist
    wxString srcfolder = m_source_field->GetValue();
    if (!srcfolder.IsEmpty())
    {
        wxDir dir(srcfolder);
        if (!dir.IsOpened())
        {
            errormsg << "- Please specify a valid encfs source folder location\n";
            src_folder_ok = false;
        }
        else
        {
            src_folder_ok = true;
        }
    }

    //3. Does destination folder exist, and is it empty ?
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

    //4. password ok ?
    if (!m_pass1->IsEmpty())
    {
        if (m_pass1->GetValue() == m_pass2->GetValue())
        {
            pw_ok = true;
        }
        else
        {
            errormsg << "- Passwords do not match\n";
        }
    }
    else
    {
        errormsg << "- Empty passwords are not allowed\n";
        }

    if (!volname_ok || !src_folder_ok || !dst_folder_ok || !pw_ok)
    {
        wxString title;
        title.Printf(wxT("Errors found:"));
        wxMessageDialog * dlg = new wxMessageDialog(this, errormsg, title, wxOK|wxCENTRE|wxICON_ERROR);
        dlg->ShowModal();
        dlg->Destroy();
    }
    else
    {
        // first, create the new volume
        bool createdok = createEncFSFolder();
        if (createdok)
        {
            // next, save new volume
            wxString config_volname;
            wxConfigBase *pConfig = wxConfigBase::Get();
            config_volname.Printf(wxT("/Volumes/%s"), newvolumename);
            pConfig->SetPath(config_volname);
            pConfig->Write(wxT("enc_path"), srcfolder);
            pConfig->Write(wxT("mount_path"), dstfolder);
            pConfig->Write(wxT("automount"), m_chkbx_automount->GetValue());
            pConfig->Write(wxT("preventautounmount"), m_chkbx_prevent_autounmount->GetValue());
            pConfig->Write(wxT("passwordsaved"), m_chkbx_save_password->GetValue());
            // save password in KeyChain, if needed
            if (m_chkbx_save_password->GetValue())
            {
                wxString cmd;
                wxString pwaddoutput;
                cmd.Printf(wxT("sh -c \"security add-generic-password -U -a 'EncFSGUI_%s' -s 'EncFSGUI_%s' -w '%s' login.keychain\""), newvolumename, newvolumename, m_pass1->GetValue());
                pwaddoutput = StrRunCMDSync(cmd);
            }   
            Close(true);
        }
        else
        {
            wxString emsg;
            emsg.Printf(wxT("Unable to create encfs folder"));
            wxMessageDialog * dlg = new wxMessageDialog(this, emsg, emsg, wxOK|wxCENTRE|wxICON_ERROR);
            dlg->ShowModal();
            dlg->Destroy();
        }
    }
}



//   ------------------------------------------
// frmOpenDialog - add existing encfs folder
//   ------------------------------------------


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
    wxDECLARE_EVENT_TABLE();
};


// event table
wxBEGIN_EVENT_TABLE(frmOpenDialog, wxDialog)
    EVT_BUTTON(ID_BTN_CHOOSE_SOURCE,  frmOpenDialog::ChooseSourceFolder)
    EVT_BUTTON(ID_BTN_CHOOSE_DESTINATION,  frmOpenDialog::ChooseDestinationFolder)
    EVT_BUTTON(wxID_APPLY, frmOpenDialog::SaveSettings)
wxEND_EVENT_TABLE()

// constructor
frmOpenDialog::frmOpenDialog(wxWindow *parent, 
                             const wxString& title, 
                             const wxPoint &pos, 
                             const wxSize &size, 
                             long style) :  wxDialog(parent, wxID_ANY, title, pos, size, style)
{
    
}

// event functions

void frmOpenDialog::ChooseSourceFolder(wxCommandEvent& WXUNUSED(event))
{
    wxString currentdir;
    currentdir = m_source_field->GetValue();
    wxDirDialog openDirDialog(this, 
                              "Select existing encfs encrypted folder", 
                              currentdir, 
                              wxDD_DEFAULT_STYLE);
    if (openDirDialog.ShowModal() == wxID_OK)
    {
        wxString fn = openDirDialog.GetPath();
        m_source_field->SetValue(fn);
    }
    openDirDialog.Destroy();
}

void frmOpenDialog::ChooseDestinationFolder(wxCommandEvent& WXUNUSED(event))
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

void frmOpenDialog::SaveSettings(wxCommandEvent& WXUNUSED(event))
{
    // check if all important fiels were populated
    bool volname_ok = true;
    wxString errormsg = ""; 
    bool src_folder_ok = false;
    bool dst_folder_ok = false;
    bool pw_ok = false;
    wxString newvolumename = m_volumename_field->GetValue();

    // sanitize the name
    newvolumename.Replace("/","");
    newvolumename.Replace(" ","");
    newvolumename.Replace("'","");
    newvolumename.Replace('"',"");
    m_volumename_field->SetValue(newvolumename);

    //1. is volume name unique?
    if (doesVolumeExist(newvolumename))
    {
        volname_ok = false;
    }

    if (newvolumename.IsEmpty())
    {
        volname_ok = false;
    }

    if (!volname_ok)
    {
        errormsg << "- Please specify a unique volume name\n";
    }

    //2. Does source folder exist
    wxString srcfolder = m_source_field->GetValue();
    if (!srcfolder.IsEmpty())
    {
        wxDir dir(srcfolder);
        if (!dir.IsOpened())
        {
            errormsg << "- Please specify a valid encfs source folder location\n";
            src_folder_ok = false;
        }
        else
        {
            src_folder_ok = true;
        }
    }

    //3. Does destination folder exist, and is it empty ?
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

    //4. save password ?
    if (m_chkbx_save_password->GetValue())
    {
        if (!m_pass1->IsEmpty())
        {
            if (m_pass1->GetValue() == m_pass2->GetValue())
            {
                pw_ok = true;
            }
            else
            {
                errormsg << "- Passwords do not match\n";
            }
        }
        else
        {
            errormsg << "- Empty passwords are not allowed\n";
        }
    }
    else
    {
        pw_ok = true;
    }

    if (!volname_ok || !src_folder_ok || !dst_folder_ok || !pw_ok)
    {
        wxString title;
        title.Printf(wxT("Errors found:"));
        wxMessageDialog * dlg = new wxMessageDialog(this, errormsg, title, wxOK|wxCENTRE|wxICON_ERROR);
        dlg->ShowModal();
        dlg->Destroy();
    }
    else
    {
        // save new volume
        wxString config_volname;
        wxConfigBase *pConfig = wxConfigBase::Get();
        config_volname.Printf(wxT("/Volumes/%s"), newvolumename);
        pConfig->SetPath(config_volname);
        pConfig->Write(wxT("enc_path"), srcfolder);
        pConfig->Write(wxT("mount_path"), dstfolder);
        pConfig->Write(wxT("automount"), m_chkbx_automount->GetValue());
        pConfig->Write(wxT("preventautounmount"), m_chkbx_prevent_autounmount->GetValue());
        pConfig->Write(wxT("passwordsaved"), m_chkbx_save_password->GetValue());
        // save password in KeyChain, if needed
        if (m_chkbx_save_password->GetValue())
        {
            wxString cmd;
            wxString pwaddoutput;
            cmd.Printf(wxT("sh -c \"security add-generic-password -U -a 'EncFSGUI_%s' -s 'EncFSGUI_%s' -w '%s' login.keychain\""), newvolumename, newvolumename, m_pass1->GetValue());
            pwaddoutput = StrRunCMDSync(cmd);
        }   
        Close(true);
    }

}

// member functions
void frmOpenDialog::Create()
{
    wxSizer * const sizerMaster = new wxBoxSizer(wxVERTICAL);
    wxSizer * const sizerVolume = new wxStaticBoxSizer(wxVERTICAL, this, "Volume settings");
    
    // desired volume name
    sizerVolume->Add(new wxStaticText(this, wxID_ANY, "&Set unique volume name:"));
    m_volumename_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    sizerVolume->Add(m_volumename_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerVolume->AddSpacer(8);

    // encrypted folder
    sizerVolume->Add(new wxStaticText(this, wxID_ANY, "&Encrypted encfs source folder:"));

    wxSizer * const sizerSRC = new wxBoxSizer(wxHORIZONTAL);
    m_source_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(370,22));
    sizerSRC->Add(m_source_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerSRC->Add(new wxButton( this , ID_BTN_CHOOSE_SOURCE, wxT("Select")));
    sizerVolume->Add(sizerSRC,wxSizerFlags(1).Expand().Border());

    sizerVolume->AddSpacer(8);

    // mount point
    sizerVolume->Add(new wxStaticText(this, wxID_ANY, "&Destination (mount) folder:"));
    wxSizer * const sizerDST = new wxBoxSizer(wxHORIZONTAL);
    m_destination_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(370,22));
    sizerDST->Add(m_destination_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerDST->Add(new wxButton( this , ID_BTN_CHOOSE_DESTINATION, wxT("Select")));
    sizerVolume->Add(sizerDST,wxSizerFlags(1).Expand().Border());
    
    wxSizer * const sizerPassword = new wxStaticBoxSizer(wxVERTICAL, this, "Password options");
    // save password ?
    m_chkbx_save_password  = new wxCheckBox(this, wxID_ANY, "Save password in Keychain");
    m_chkbx_save_password->SetValue(false);
    sizerPassword->Add(m_chkbx_save_password);

    wxSizer * const sizerPW1 = new wxBoxSizer(wxHORIZONTAL);
    sizerPW1->Add(new wxStaticText(this, wxID_ANY, "Enter password:"));
    m_pass1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizerPW1->Add(m_pass1, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerPassword->Add(sizerPW1, wxSizerFlags(1).Expand().Border());

    wxSizer * const sizerPW2 = new wxBoxSizer(wxHORIZONTAL);
    sizerPW2->Add(new wxStaticText(this, wxID_ANY, "Enter password again:"));
    m_pass2 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizerPW2->Add(m_pass2, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 5).Expand());
    sizerPassword->Add(sizerPW2, wxSizerFlags(1).Expand().Border());
    
    wxSizer * const sizerOptions = new wxStaticBoxSizer(wxVERTICAL, this, "Mount options");
    // auto mount ?
    m_chkbx_automount  = new wxCheckBox(this, wxID_ANY, "Automatically mount this volume when application starts");
    m_chkbx_automount->SetValue(false);
    sizerOptions->Add(m_chkbx_automount);

    // prevent auto unmount
    m_chkbx_prevent_autounmount  = new wxCheckBox(this, wxID_ANY, "Prevent auto-unmounting this volume on application exit");
    m_chkbx_prevent_autounmount->SetValue(false);
    sizerOptions->Add(m_chkbx_prevent_autounmount);

    // glue together
    sizerMaster->Add(sizerVolume, wxSizerFlags(1).Expand().Border());
    sizerMaster->Add(sizerPassword, wxSizerFlags(1).Expand().Border());
    sizerMaster->Add(sizerOptions, wxSizerFlags(1).Expand().Border());

    // Add "Apply" and "Cancel"
    sizerMaster->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCANCEL), wxSizerFlags().Right().Border());

    CentreOnScreen();

    SetSizer(sizerMaster);
}


// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

void createNewEncFSFolder(wxWindow *parent)
{
    wxSize frmAddSize;
    frmAddSize.Set(600,680);
    long framestyle;
    framestyle = wxDEFAULT_FRAME_STYLE | wxFRAME_EX_METAL;

    wxString strTitle;
    strTitle.Printf( "Create a new EncFS folder");  
    
    frmAddDialog* dlg = new frmAddDialog(parent, 
                                           strTitle, 
                                           wxDefaultPosition, 
                                           frmAddSize, 
                                           framestyle);
    dlg->Create();
    dlg->ShowModal();

}


void openExistingEncFSFolder(wxWindow *parent)
{
    wxSize frmOpenSize;
    frmOpenSize.Set(500,500);
    long framestyle;
    framestyle = wxDEFAULT_FRAME_STYLE | wxFRAME_EX_METAL;

    wxString strTitle;
    strTitle.Printf( "Open an existing EncFS folder");  
    
    frmOpenDialog* dlg = new frmOpenDialog(parent, 
                                           strTitle, 
                                           wxDefaultPosition, 
                                           frmOpenSize, 
                                           framestyle);
    dlg->Create();
    dlg->ShowModal();
}



