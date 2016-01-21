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

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

enum
{
	ID_BTN_CHOOSE_SOURCE,
	ID_BTN_CHOOSE_DESTINATION,
	ID_CHECK_AUTOMOUNT
};


// ----------------------------------------------------------------------------
// Classes
// ----------------------------------------------------------------------------



class frmAddDialog : public wxDialog
{
public:
	frmAddDialog(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style);
	void Create();

};

// constructor
frmAddDialog::frmAddDialog(wxWindow *parent, const wxString& title, const wxPoint &pos, const wxSize &size, long style) :  wxDialog(parent, wxID_ANY, title, pos, size, style)
{

}


// member functions

void frmAddDialog::Create()
{

}


//
//   ------------------------------------------


class frmOpenDialog : public wxDialog
{
public:
	frmOpenDialog(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style);
	void Create();
	void ChooseSourceFolder(wxCommandEvent &event);
	void ChooseDestinationFolder(wxCommandEvent &event);
	void SaveSettings(wxCommandEvent &event);
private:
	wxTextCtrl * m_source_field;
	wxTextCtrl * m_destination_field;
	wxTextCtrl * m_volumename_field;
	wxCheckBox * m_chkbx_automount;
	wxCheckBox * m_chkbx_prevent_autounmount;
	wxDECLARE_EVENT_TABLE();
};


// event table
wxBEGIN_EVENT_TABLE(frmOpenDialog, wxDialog)
    EVT_BUTTON(ID_BTN_CHOOSE_SOURCE,  frmOpenDialog::ChooseSourceFolder)
    EVT_BUTTON(ID_BTN_CHOOSE_DESTINATION,  frmOpenDialog::ChooseDestinationFolder)
    EVT_BUTTON(wxID_APPLY, frmOpenDialog::SaveSettings)
wxEND_EVENT_TABLE()

// constructor
frmOpenDialog::frmOpenDialog(wxWindow *parent, const wxString& title, const wxPoint &pos, const wxSize &size, long style) :  wxDialog(parent, wxID_ANY, title, pos, size, style)
{
    
}

// event functions

void frmOpenDialog::ChooseSourceFolder(wxCommandEvent& WXUNUSED(event))
{
	wxString currentdir;
	currentdir = m_source_field->GetValue();
	wxDirDialog openDirDialog(this, "Select existing encfs encrypted folder", currentdir, wxDD_DEFAULT_STYLE);
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
	wxDirDialog openDirDialog(this, "Select destination mount point folder", currentdir, wxDD_DEFAULT_STYLE);
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
	wxString newvolumename = m_volumename_field->GetValue();

	// sanitize the name
	newvolumename.Replace("/","");
	newvolumename.Replace(" ","");
	newvolumename.Replace("'","");
	newvolumename.Replace('"',"");
	m_volumename_field->SetValue(newvolumename);

	//1. is volume name unique?
	wxConfigBase *pConfig = wxConfigBase::Get();
    std::vector<wxString> v_volnames;
    pConfig->SetPath(wxT("/Volumes"));
    wxString volumename;
    wxString allNames;
    long dummy;
    bool bCont = pConfig->GetFirstGroup(volumename, dummy);
    while ( bCont ) { 
        v_volnames.push_back(volumename); 
        if (volumename == newvolumename)
        {
        	volname_ok = false;
        }
        bCont = pConfig->GetNextGroup(volumename, dummy);
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

	//3. Does destination folder exist
	wxString dstfolder = m_destination_field->GetValue();
	if (!dstfolder.IsEmpty())
	{
		wxDir dir(dstfolder);
		if (!dir.IsOpened())
		{
			errormsg << "- Please specify a valid/existing destination mount point location\n";
			dst_folder_ok = false;
		}
		else
		{
			dst_folder_ok = true;
		}
	}

	if (!volname_ok || !src_folder_ok || !dst_folder_ok)
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
		config_volname.Printf(wxT("/Volumes/%s"), newvolumename);
		pConfig->SetPath(config_volname);
		pConfig->Write(wxT("enc_path"), srcfolder);
		pConfig->Write(wxT("mount_path"), dstfolder);
		pConfig->Write(wxT("automount"), m_chkbx_automount->GetValue());
		pConfig->Write(wxT("preventautounmount"), m_chkbx_prevent_autounmount->GetValue());
		Close(true);
	}

}

// member functions
void frmOpenDialog::Create()
{
	wxSizer * const sizerTop = new wxBoxSizer(wxVERTICAL);
	wxSizer * const sizerGlobal = new wxStaticBoxSizer(wxVERTICAL, this, "Folder settings");

	// desired volume name
	sizerGlobal->Add(new wxStaticText(this, wxID_ANY, "&Set volume name:"));
	m_volumename_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
	sizerGlobal->Add(m_volumename_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 10).Expand());

	sizerGlobal->AddSpacer(8);

	// encrypted folder
	sizerGlobal->Add(new wxStaticText(this, wxID_ANY, "&Encrypted encfs source folder:"));
	m_source_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
	sizerGlobal->Add(m_source_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 10).Expand());
	sizerGlobal->Add(new wxButton( this , ID_BTN_CHOOSE_SOURCE, wxT("Select encrypted folder")));

	sizerGlobal->AddSpacer(8);

	// mount point
	sizerGlobal->Add(new wxStaticText(this, wxID_ANY, "&Destination (mount) folder:"));
	m_destination_field = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
	sizerGlobal->Add(m_destination_field, wxSizerFlags().Border(wxLEFT|wxBOTTOM|wxRIGHT, 10).Expand());
	sizerGlobal->Add(new wxButton( this , ID_BTN_CHOOSE_DESTINATION, wxT("Select mount folder")));

	sizerGlobal->AddSpacer(15);

	// auto mount ?
	m_chkbx_automount  = new wxCheckBox(this, wxID_ANY, "Automatically mount this volume when application starts");
	m_chkbx_automount->SetValue(false);
    sizerGlobal->Add(m_chkbx_automount);


    // prevent auto unmount
	m_chkbx_prevent_autounmount  = new wxCheckBox(this, wxID_ANY, "Prevent auto-unmounting this volume on application exit");
	m_chkbx_prevent_autounmount->SetValue(false);
    sizerGlobal->Add(m_chkbx_prevent_autounmount);


	// glue together
	sizerTop->Add(sizerGlobal, wxSizerFlags(1).Expand().Border());

	// Add "Apply" and "Cancel"
    sizerTop->Add(CreateStdDialogButtonSizer(wxAPPLY | wxCANCEL), wxSizerFlags().Right().Border());

	CentreOnScreen();

	SetSizer(sizerTop);
}


// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

void createNewEncFSFolder(wxWindow *parent)
{
	wxSize frmAddSize;
	frmAddSize.Set(500,640);
	long framestyle;
	framestyle = wxDEFAULT_FRAME_STYLE | wxFRAME_EX_METAL;

	wxString strTitle;
	strTitle.Printf( "Create a new EncFS folder");	
	
    frmOpenDialog* dlg = new frmOpenDialog(parent, strTitle, wxDefaultPosition, frmAddSize, framestyle);
 	dlg->Create();
    dlg->ShowModal();

}


void openExistingEncFSFolder(wxWindow *parent)
{
	wxSize frmOpenSize;
	frmOpenSize.Set(500,450);
	long framestyle;
	framestyle = wxDEFAULT_FRAME_STYLE | wxFRAME_EX_METAL;

	wxString strTitle;
	strTitle.Printf( "Open an existing EncFS folder");	
	
    frmOpenDialog* dlg = new frmOpenDialog(parent, strTitle, wxDefaultPosition, frmOpenSize, framestyle);
    dlg->Create();
    dlg->ShowModal();
}



