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

#include <wx/filename.h>    // wxFileName
#include <wx/fileconf.h>
#include <wx/xml/xml.h>
#include <wx/config.h>
#include <wx/log.h>
#include <wx/utils.h>       // wxExecute
#include <map>

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

wxString getEncFSCTLBinPath()
{
    wxString returnval;
    returnval = getEncFSBinPath();
    returnval << "ctl";
    return returnval;
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
wxString StrRunCMDSync(wxString & cmd)
{
    wxExecuteEnv env;
    wxArrayString output, errors;
    wxExecute(cmd, output, errors, 0, &env);
    wxString returnvalue = "";
    
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

wxArrayString ArrRunCMDASync(wxString & cmd)
{
    wxExecuteEnv env;
    wxArrayString output;
    wxExecute(cmd, output, wxEXEC_ASYNC, &env);
    return output;
}


wxArrayString ArrRunCMDSync(wxString & cmd)
{
    wxExecuteEnv env;
    wxArrayString output, errors;
    wxExecute(cmd, output, errors, 0, &env);
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

wxString getKeychainPassword(wxString & volumename)
{
    wxString cmd;
    wxString fullname;
    wxString output;
    fullname.Printf(wxT("EncFSGUI_%s"), volumename);
    cmd.Printf(wxT("sh -c 'security find-generic-password -a \"%s\" -s \"%s\" -w login.keychain'"), fullname, fullname);
    output = StrRunCMDSync(cmd);
    return output;
}

bool doesVolumeExist(wxString & volumename)
{
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Volumes"));
    wxString thisvolumename;
    long dummy;
    bool bCont = pConfig->GetFirstGroup(thisvolumename, dummy);
    while ( bCont ) { 
        if (thisvolumename == volumename)
        {
            return true;
        }
        bCont = pConfig->GetNextGroup(thisvolumename, dummy);
    }
    return false;
}

wxArrayString getEncFSVolumeInfo(wxString& encfs_volume)
{
    wxString cmd;
    wxString encfsctlbin = getEncFSCTLBinPath();
    wxArrayString cmdoutput;

    cmd.Printf(wxT("sh -c \"'%s' '%s'\""), encfsctlbin, encfs_volume);
    cmdoutput = ArrRunCMDSync(cmd);
    return cmdoutput;
}

wxString getExpectScriptContents()
{
    wxString newline;
    wxString scriptcontents;

    // header
    newline.Printf(wxT("#!/usr/bin/env expect\n"));
    scriptcontents << newline;

    newline.Printf(wxT("set passwd [lindex $argv 0]\n"));
    scriptcontents << newline;

    newline.Printf(wxT("set timeout 10\n"));
    scriptcontents << newline;

    // launch encfs
    newline.Printf(wxT("spawn \"$ENCFSBIN\" -v \"$ENCPATH\" \"$MOUNTPATH\"\n"));
    scriptcontents << newline;

    // activate expert mode
    newline.Printf(wxT("expect \"Please choose from one of the following options:\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("expect \"?>\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"x\\n\"\n"));
    scriptcontents << newline;

    // set cipher algorithm
    newline.Printf(wxT("expect \"Enter the number corresponding to your choice: \"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"$CIPHERALGO\\n\"\n"));
    scriptcontents << newline;

    // select cipher keysize
    newline.Printf(wxT("expect \"Selected key size:\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"$CIPHERKEYSIZE\\n\"\n"));
    scriptcontents << newline;

    // select filesystem block size
    newline.Printf(wxT("expect \"filesystem block size:\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"$BLOCKSIZE\\n\"\n"));
    scriptcontents << newline;

    // select encoding algo
    newline.Printf(wxT("expect \"Enter the number corresponding to your choice: \"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"$ENCODINGALGO\\n\"\n"));
    scriptcontents << newline;

    // filename IV chaining
    newline.Printf(wxT("expect \"Enable filename initialization vector chaining?\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"$IVCHAINING\\n\"\n"));
    scriptcontents << newline;

    // per filename IV
    newline.Printf(wxT("expect \"Enable per-file initialization vectors?\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"$PERFILEIV\\n\"\n"));
    scriptcontents << newline;

    // file to IV header chaining can only be used when both previous options are enabled
    // which means it might slide to the next option right away
    newline.Printf(wxT("expect {\n"));
    scriptcontents << newline;
    newline.Printf(wxT("\t\"Enable filename to IV header chaining?\" {\n"));
    scriptcontents << newline;
    newline.Printf(wxT("\t\tsend \"$FILETOIVHEADERCHAINING\\n\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("\t\texpect \"Enable block authentication code headers\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("\t\tsend \"$BLOCKAUTHCODEHEADERS\\n\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("\t\t}\n"));
    scriptcontents << newline;
    newline.Printf(wxT("\t\"Enable block authentication code headers\" {\n"));  //space matters
    scriptcontents << newline;
    newline.Printf(wxT("\t\tsend \"$BLOCKAUTHCODEHEADERS\\n\"\n"));
    scriptcontents << newline;   
    newline.Printf(wxT("\t\t}\n"));
    scriptcontents << newline;   
    newline.Printf(wxT("\t}\n"));
    scriptcontents << newline;    

    // add random bytes to each block header
    newline.Printf(wxT("expect \"Select a number of bytes, from 0 (no random bytes) to 8: \"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"0\\n\"\n"));
    scriptcontents << newline;

    // file-hole pass-through
    newline.Printf(wxT("expect \"Enable file-hole pass-through?\"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"\\n\"\n"));
    scriptcontents << newline;        

    // password
    newline.Printf(wxT("expect \"New Encfs Password: \"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"$passwd\\n\"\n"));
    scriptcontents << newline;    

    newline.Printf(wxT("expect \"Verify Encfs Password: \"\n"));
    scriptcontents << newline;
    newline.Printf(wxT("send \"$passwd\\n\"\n"));
    scriptcontents << newline;    

    newline.Printf(wxT("puts \"\\nDone.\\n\"\n"));
    scriptcontents << newline;

    newline.Printf(wxT("expect \"\\n\"\n"));
    scriptcontents << newline;    
    
    newline.Printf(wxT("sleep 2\n"));
    scriptcontents << newline;

    return scriptcontents;
}

wxString getChangePasswordScriptContents(wxString & enc_path)
{
    wxString scriptcontents;
    wxString newline;
    wxString encfsctlbin = getEncFSCTLBinPath();

    newline.Printf(wxT("#!/usr/bin/env expect\n"));
    scriptcontents << newline;

    newline.Printf(wxT("set oldpw [lindex $argv 0]\n"));
    scriptcontents << newline;

    newline.Printf(wxT("set newpw [lindex $argv 1]\n"));
    scriptcontents << newline;

    newline.Printf(wxT("spawn '%s' autopasswd '%s'\n"), encfsctlbin, enc_path);
    scriptcontents << newline;

    newline.Printf(wxT("expect \"Enter current Encfs password\"\n"));
    scriptcontents << newline;
    
    newline.Printf(wxT("send \"$oldpw\r\"\n"));
    scriptcontents << newline;

    newline.Printf(wxT("expect \"Enter new Encfs password\"\n"));
    scriptcontents << newline;
    
    newline.Printf(wxT("send \"$newpw\r\"\n"));
    scriptcontents << newline;

    return scriptcontents;

}


std::map<wxString, wxString> getEncodingCapabilities(wxString & encfsOutput)
{
    std::map<wxString, wxString> encodingcaps;

    return encodingcaps;
}

