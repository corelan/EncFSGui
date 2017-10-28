/*
    encFSGui - encfsgui_helpers.cpp
    source file contains helper functions

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
#include <wx/stdpaths.h> 
#include <wx/dir.h>
#include <wx/tokenzr.h>
#include <map>

#include <fstream>

#include <curl/curl.h>

//
// globals
//
wxString g_latestversion;

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
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    return pConfig->Read(wxT("encfsbinpath"), "/usr/local/bin/encfs");
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
    return pConfig->Read(wxT("mountbinpath"), "/sbin/mount");
}

wxString getUMountBinPath()
{
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    return pConfig->Read(wxT("umountbinpath"), "/sbin/umount");
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
    // add a space to volpath, to make sure we have an exact match
    wxString volpathsearch = volpath + " ";
    for ( size_t n = 0; n < count; n++ )
    {
        signed int checkval = -1;
        wxString thisline;
        thisline = mountinfo[n];
        if (not thisline.IsEmpty())
        {
            signed int pos1;
            signed int pos2;
            pos1 = thisline.Find(volpathsearch);
            pos2 = thisline.Find(encmarker);
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

wxString getExpectScriptContents(bool insertbreak)
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

    if (insertbreak)
    {
        newline.Printf(wxT("break\n"));
        scriptcontents << newline;        
    }

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
    
    newline.Printf(wxT("sleep x\n"));  
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


wxString getLaunchAgentContents()
{
    const wxString scriptcontents =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
        "<plist version=\"1.0\">\n"
        "<dict>\n"
        "    <key>Label</key>\n"
        "    <string>org.corelan.encfsgui.LaunchAtLogin</string>\n"
        "    <key>ProgramArguments</key>\n"
        "    <array>\n"
        "        <string>/Applications/EncFSGui.app/Contents/MacOS/encfsgui</string>\n"
        "    </array>\n"
        "    <key>ProcessType</key>\n"
        "    <string>Interactive</string>\n"
        "    <key>RunAtLoad</key>\n"
        "    <true/>\n"
        "    <key>KeepAlive</key>\n"
        "    <false/>\n"
        "</dict>\n"
        "</plist>\n";
    return scriptcontents;
}


std::map<wxString, wxString> getEncodingCapabilities()
{
    std::map<wxString, wxString> encodingcaps;

    // first, check if we have discovered the capabilities already
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/FilenameEncoding"));
    wxString capname;
    long dummy;
    wxString capval;
    bool bCont = pConfig->GetFirstEntry(capname, dummy);
    while (bCont)
    {
        // read value for this capname
        capval = pConfig->Read(capname, "");
        if (!capname.IsEmpty() && !capval.IsEmpty())
        {
            encodingcaps[capname] = capval;
        }
        bCont = pConfig->GetNextEntry(capname, dummy);
    }


    if (encodingcaps.size() == 0)
    {
        // create 2 temporary dirs
        wxStandardPathsBase& stdp = wxStandardPaths::Get();
        wxString tmp_dir = stdp.GetTempDir();
        wxString enc_dir = "tmp_encfsgui_crypt";
        wxString plain_dir = "tmp_encfsgui_plain";
        wxString encfsbin = getEncFSBinPath();
        wxString msg="";
        wxString cmd="";

        wxString enc_path;
        wxString plain_path;
        enc_path.Printf(wxT("%s/%s"), tmp_dir, enc_dir);
        plain_path.Printf(wxT("%s/%s"), tmp_dir, plain_dir);

        // remove existing dirs, if they exist
        wxDir * dirEnc = new wxDir(enc_path);
        wxDir * dirPlain = new wxDir(plain_path);

        if (dirEnc->Exists(enc_path))
        {
            dirEnc->Remove(enc_path, wxPATH_RMDIR_RECURSIVE);
        }
        if (dirPlain->Exists(plain_path))
        {
            dirPlain->Remove(plain_path, wxPATH_RMDIR_RECURSIVE);
        }

        // create dirs
        dirEnc->Make(enc_path);
        dirPlain->Make(plain_path);

        // true means insert 'break' after listing the filename encoding options
        wxString scriptcontents = getExpectScriptContents(true); 

        // use valid, but non-important values
        scriptcontents.Replace("$ENCFSBIN", encfsbin);
        scriptcontents.Replace("$ENCPATH", enc_path);
        scriptcontents.Replace("$MOUNTPATH", plain_path);
        scriptcontents.Replace("$CIPHERALGO", "1");
        scriptcontents.Replace("$CIPHERKEYSIZE", "128");
        scriptcontents.Replace("$BLOCKSIZE", "1024");
        scriptcontents.Replace("$ENCODINGALGO", "1");
        scriptcontents.Replace("$IVCHAINING","");
        scriptcontents.Replace("$PERFILEIV","");
        scriptcontents.Replace("$FILETOIVHEADERCHAINING","");
        scriptcontents.Replace("$BLOCKAUTHCODEHEADERS","");
        scriptcontents.Replace("sleep x","expect eof");    

        // run encfs, just to capture the output related with filename encoding mechanisms
        // write script to disk
        wxString pw = "DefaultPassword";
        wxTempFile * tmpfile = new wxTempFile();
        wxString scriptfile;
        scriptfile.Printf(wxT("%screateencfs.exp"), tmp_dir );
        tmpfile->Open(scriptfile);
        if (tmpfile->IsOpened())
        {
            tmpfile->Write(scriptcontents);
        }
        tmpfile->Commit();

        cmd.Printf(wxT("expect '%s' '%s'"), scriptfile, pw);
        // run command synchronously this time, it shouldn't take long :)
        wxArrayString arroutput = ArrRunCMDSync(cmd);
        
        // parse the output, look for information about available file encoding mechanisms
        // and add them to map
        bool startfound = false;
        bool endfound = false;
        wxArrayString rawCaps;
        size_t count = arroutput.GetCount();
        for ( size_t n = 0; n < count; n++ )
        {
            wxString thisline = arroutput[n];
            if (!startfound)
            {
                if (thisline.Find("The following filename encoding algorithms are available") > -1)
                {
                    startfound = true;
                }
            }
            else
            {
                if (thisline.Find(".") == -1)
                {
                    endfound = true;
                }
                else
                {
                    rawCaps.Add(thisline);
                }
            }
            if (startfound && endfound)
            {
                break;
            }
        }

        for (size_t n = 0; n < rawCaps.GetCount(); n++)
        {
            wxString rawline = rawCaps[n];
            // Tokenize the string
            wxStringTokenizer tokenizer(rawline, " ");
            int tokenindex = 0;
            wxString encodingnr="";
            wxString encodingname="";
            while ( tokenizer.HasMoreTokens() )
            {
                wxString thistoken = tokenizer.GetNextToken();
                if (tokenindex == 0)
                {
                    thistoken.Replace(".","");
                    encodingnr = thistoken;
                }
                else if (tokenindex == 1)
                {
                    encodingname = thistoken;
                }
                tokenindex++;
            }
            // save into map
            if (!encodingnr.IsEmpty() && !encodingname.IsEmpty())
            {
                encodingcaps[encodingname] = encodingnr;
            }
        }

        
        // clean up again
        tmpfile->Open(scriptfile);
        if (tmpfile->IsOpened())
        {
            tmpfile->Write("#cleaned");
        }
        tmpfile->Commit();

        if (dirEnc->Exists(enc_path))
        {
            dirEnc->Remove(enc_path, wxPATH_RMDIR_RECURSIVE);
        }
        if (dirPlain->Exists(plain_path))
        {
            dirPlain->Remove(plain_path, wxPATH_RMDIR_RECURSIVE);
        }    
    } 

    // save/rewrite config
    pConfig->SetPath(wxT("/FilenameEncoding"));
    for (std::map<wxString, wxString>::iterator it= encodingcaps.begin(); it != encodingcaps.end(); it++)
    {
        wxString encodingname = it->first;
        wxString encodingval = it->second;
        pConfig->Write(encodingname, encodingval);
    }
    pConfig->Flush();
    return encodingcaps;
}


void renameVolume(wxString& oldname, wxString& newname)
{
    wxConfigBase *pConfig = wxConfigBase::Get();
    wxString currentvol;
    wxString newvol;
    currentvol.Printf(wxT("/Volumes/%s"), oldname);
    newvol.Printf(wxT("/Volumes/%s"), newname);
    pConfig->SetPath(currentvol);
    // get current values
    wxString enc_path = pConfig->Read(wxT("enc_path"), "");
    wxString mount_path = pConfig->Read(wxT("mount_path"), "");
    bool automount = pConfig->ReadBool(wxT("automount"), 0l);
    bool preventautounmount = pConfig->ReadBool(wxT("preventautounmount"), 0l);
    bool passwordsaved = pConfig->ReadBool(wxT("passwordsaved"), 0l);
    bool allowother = pConfig->ReadBool(wxT("allowother"), 0l);
    bool mountaslocal = pConfig->ReadBool(wxT("mountaslocal"), 0l);
    // delete old group
    pConfig->DeleteGroup(currentvol);
    // create a new one
    pConfig->SetPath(newvol);
    pConfig->Write("enc_path", enc_path);
    pConfig->Write("mount_path", mount_path);
    pConfig->Write("automount", automount);
    pConfig->Write("preventautounmount", preventautounmount);
    pConfig->Write("passwordsaved", passwordsaved);
    pConfig->Write(wxT("allowother"),allowother);
    pConfig->Write(wxT("mountaslocal"),mountaslocal);            
    
    pConfig->Flush();
}



// callback function to get curl content
static size_t getHTTPContent(void* ptr, size_t size, size_t nmemb)
{
    size_t data_size = size * nmemb;
    g_latestversion.Append((char *)ptr, data_size);
    return data_size;
}


wxString getLatestVersion()
{
    g_latestversion = "";
    wxString contentbuffer;
    CURL *pCurlHandle;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);

    pCurlHandle = curl_easy_init();
    if(pCurlHandle)
    {
        curl_easy_setopt(pCurlHandle, CURLOPT_URL, "https://github.com/corelan/EncFSGui/raw/master/release/version.txt");
        // force SSL peer verification
        curl_easy_setopt(pCurlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
        // force hostname verification
        curl_easy_setopt(pCurlHandle, CURLOPT_SSL_VERIFYHOST, 1L);
        // no progress meter
        curl_easy_setopt(pCurlHandle, CURLOPT_NOPROGRESS, 1L);
        // there might be a redirect
        curl_easy_setopt(pCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
        // go get the data
        curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, getHTTPContent);


        res = curl_easy_perform(pCurlHandle);
        /* Check for errors */ 
        if(res != CURLE_OK)
        {
            wxString msg;
            msg.Printf(wxT("curl_easy_perform() failed: %s"), curl_easy_strerror(res));
            wxLogDebug(msg);
        }
        curl_easy_cleanup(pCurlHandle);
    }
    curl_global_cleanup();

    g_latestversion.Replace(" ","");
    g_latestversion.Replace("\r","");
    g_latestversion.Replace("\n","");

    return g_latestversion;
}


std::map<wxString, long> VersionTokenizerToVersionMap(wxStringTokenizer tokenizer)
{
    std::map<wxString, long> m_returnval;

    int tokenindex = 0;
    while ( tokenizer.HasMoreTokens() )
    {
        wxString thistoken = tokenizer.GetNextToken();
        wxString tokenkey = "";
        long value;
        if(!thistoken.ToLong(&value))
        {
            value = 0;
        }
        if (tokenindex == 0)
        {
            tokenkey = "major";
        }
        else if (tokenindex == 1)
        {
            tokenkey = "minor";
        }
        else if (tokenindex == 2)
        {
            tokenkey = "revision";
        }
        m_returnval[tokenkey] = value;
        tokenindex++;
    }
    return m_returnval;
}


bool IsLatestVersionNewer(const wxString& currentversion, wxString& latestversion)
{

    bool isnewer = false;

    if (!(currentversion == latestversion))
    {
        wxStringTokenizer tokenizer_current(currentversion, ".");
        wxStringTokenizer tokenizer_latest(latestversion, ".");
        // both should have at least 3 tokens
        // if not, one of them may not be a real version
        // which may indicate the app was not able to retrieve the latest version
        // in that case, we'll assume the current version is ok
        size_t nrTokens_current = tokenizer_current.CountTokens();
        size_t nrTokens_latest = tokenizer_latest.CountTokens();

        if ((nrTokens_current < 3) || (nrTokens_latest < 3))
        {
            return false;   // current version is considered to be up-to-date
        }

        // put tokens in maps first
        std::map<wxString, long> m_tokenscurrent = VersionTokenizerToVersionMap(tokenizer_current);
        std::map<wxString, long> m_tokenslatest = VersionTokenizerToVersionMap(tokenizer_latest);

        // check, starting with major first
        if ( (m_tokenscurrent.count("major") > 0) && (m_tokenslatest.count("major") > 0) )
        {
            long currmajor = m_tokenscurrent["major"];
            long latestmajor = m_tokenslatest["major"];
            if (latestmajor > currmajor)
            {
                return true;
            }
            else if (currmajor > latestmajor)
            {
                return false;
            }
        }

        if ( (m_tokenscurrent.count("minor") > 0) && (m_tokenslatest.count("minor") > 0) )
        {
            long currminor = m_tokenscurrent["minor"];
            long latestminor = m_tokenslatest["minor"];
            if (latestminor > currminor)
            {
                return true;
            }
            else if (currminor > latestminor)
            {
                return false;
            }
        }

        if ( (m_tokenscurrent.count("revision") > 0) && (m_tokenslatest.count("revision") > 0) )
        {
            long currrevision = m_tokenscurrent["revision"];
            long latestrevision = m_tokenslatest["revision"];
            if (latestrevision > currrevision)
            {
                return true;
            }
        }        

    }

    return isnewer;

}

