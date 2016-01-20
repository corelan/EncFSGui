/*
	encFSGui - source file contains
	helper functions

	written by Peter Van Eeckhoutte

*/


// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/filename.h>	// wxFileName
#include <wx/fileconf.h>
#include <wx/xml/xml.h>
#include <wx/config.h>
#include <wx/log.h>
#include <wx/utils.h>		// wxExecute


#include <fstream>

// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

void ShowMsg(wxString msg)
{
	wxLogMessage(msg);
}


// get full path to encfs from config
// or resort to default value if config does not exist (yet)
// and save to config
wxString getEncFSBinPath()
{
	wxString defaultvalue;
	wxString configvalue;

	defaultvalue = "/usr/local/bin/encfs";

    wxConfigBase *pConfig = wxConfigBase::Get();

    pConfig->SetPath(wxT("/Config"));
    // read path from config, or return default value in case config 
    // doesn't exist yet
    configvalue = pConfig->Read(wxT("encfsbinpath"), defaultvalue );
	
	if (configvalue.IsEmpty())
	{
		configvalue = defaultvalue;
	}
    
    pConfig->Write(wxT("encfsbinpath"),configvalue);

	return configvalue;
}

wxString getMountBinPath()
{
	wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    return pConfig->Read(wxT("mountbin_path"), "/sbin/mount");
}

wxString getUMountBinPath()
{
	wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    return pConfig->Read(wxT("umountbin_path"), "/sbin/umount");
}


// check if binary file exists
bool isEncFSBinInstalled()
{
	wxString encfsbinpath = getEncFSBinPath();
	if (not encfsbinpath.IsEmpty())
	{
		wxFileName encfsbinfile;
		encfsbinfile = wxFileName::wxFileName(encfsbinpath);
		if (encfsbinfile.FileExists())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
    return true;
}


// convert output array into wxString
wxString arrStrTowxStr(wxArrayString & input)
{
	wxString returnval;
	returnval = "";
	size_t count = input.GetCount();
	if ( !count )
	    return returnval;

	if (count == 1)
	{
		returnval = input[0];
		return returnval;
	} 

	for ( size_t n = 0; n < count; n++ )
	{
	    returnval << input[n];
	    if (n < count-1)
	    {
	    	returnval << "\n";
	    }
	}

	return returnval;

}


// run a command (sync) and return output
wxString StrRunCMDSync(wxString cmd)
{
	
	wxArrayString output, errors;
	wxExecute(cmd, output, errors);
	wxString returnvalue;
	
	// command line output may end up in errors
	// depending on the exit code of the called app
	// so this is not necessarily a problem
	size_t count = output.GetCount();
	if (count)
	{
		returnvalue = arrStrTowxStr(output);
	
	}
	count = errors.GetCount();
	if (count)
	{	
		if (!returnvalue.IsEmpty())
		{
			returnvalue << "\n";
		}
		returnvalue << arrStrTowxStr(errors);
	}

	return returnvalue;
}

wxArrayString ArrRunCMDSync(wxString cmd)
{
	wxArrayString output, errors;
	wxExecute(cmd, output, errors);
	// command line output may end up in errors
	// depending on the exit code of the called app
	// so this is not necessarily a problem
	size_t count = output.GetCount();
	if (count)
	{
		return output;
	}
	return errors;
}


// Get EncFS Version by running encfs --version
wxString getEncFSBinVersion()
{
	wxString version = "<unable to get version>";

	if (isEncFSBinInstalled())
	{
		wxString encfsbinpath = getEncFSBinPath();
		wxString cmd = encfsbinpath + " --version";
		version = StrRunCMDSync(cmd);
	}

	return version;
}

void createPwFile(wxString& pwfile, wxString& pw)
{
	std::ofstream pfile;
	pfile.open(pwfile.mb_str());
	pfile << "echo ";
	pfile << pw;
	pfile.close();
}

void cleanPwFile(wxString& pwfile)
{
	std::ofstream pfile;
	pfile.open(pwfile.mb_str());
	pfile << "Good luck with that";
	pfile.close();
}


// Check if volumepath is in "/sbin/mount" output
// to determine if volume is mounted by encfs already
bool IsVolumeSystemMounted(wxString volpath, wxArrayString mountinfo)
{
	bool matchfound = false;
	size_t count = mountinfo.GetCount();
	wxString encmarker = "encfs";
	for ( size_t n = 0; n < count; n++ )
	{
		signed int checkval = -1;
		wxString thisline;
		thisline = mountinfo[n];
		if (not thisline.IsEmpty())
		{
			signed int pos1;
			signed int pos2;
			pos1 = thisline.find(volpath);
			pos2 = thisline.find(encmarker);
			if ( (pos1 > checkval) && (pos2 > checkval) )
			{
				return true;
			}
		}
	}
	return matchfound;
}


void BrowseFolder(wxString & mountpath)
{
    wxString cmd;
    cmd.Printf(wxT("open '%s'"), mountpath);
    //wxLogDebug(cmd);
    wxExecuteEnv env;
    wxExecute(cmd, wxEXEC_ASYNC, NULL, &env);
}

