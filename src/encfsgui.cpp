/*
    encFSGui - encfsgui.cpp
    main source

    written by Peter Van Eeckhoutte

*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/toolbar.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/log.h>
#include <wx/utils.h>
#include <vector>
#include <map>
#include "wx/taskbar.h"

#include "encfsgui.h"
#include "version.h"

// keep config files local
#define wxCONFIG_USE_LOCAL_FILE 1
#define USE_LOCAL_FILE 1


// ----------------------------------------------------------------------------
// Resources 
// ----------------------------------------------------------------------------


#include "encfsgui.xpm" 

#include "bitmaps/createfolder.xpm"
#include "bitmaps/existingfolder.xpm"
#include "bitmaps/browsefolder.xpm"
#include "bitmaps/removefolder.xpm"
#include "bitmaps/editfolder.xpm"
#include "bitmaps/folderinfo.xpm"
#include "bitmaps/mountfolder.xpm"
#include "bitmaps/unmountfolder.xpm"
#include "bitmaps/unmountfolder_all.xpm"    
#include "bitmaps/settings.xpm"
#include "bitmaps/quit.xpm"
#include "bitmaps/ico_ok.xpm"
#include "bitmaps/ico_notok.xpm"



// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// only put toolbar at top
enum Positions
{
    TOOLBAR_TOP
};    


// globals to keep track of what we have selected in the ListCtrl
// messy, but convenient
int g_selectedIndex;
wxString g_selectedVolume;

// IDs for toolbar controls and the menu commands
enum
{
    // menu items
    ID_Menu_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    ID_Menu_About = wxID_ABOUT,
    // the others don't matter too much, just sequential numbers
    ID_Menu_New,
    ID_Menu_Existing,
    ID_Menu_Settings,
    //Toolbar stuff
    ID_Toolbar_Create,
    ID_Toolbar_Existing,
    ID_Toolbar_Browse,
    ID_Toolbar_Remove,
    ID_Toolbar_Edit,
    ID_Toolbar_Info,
    ID_Toolbar_Mount,
    ID_Toolbar_Unmount,
    ID_Toolbar_UnmountAll,
    ID_Toolbar_Settings,
    ID_Toolbar_Quit,
    ID_TOOLBAR,
    // list control
    ID_List_Ctrl                   = 1000,
    // taskbar icon
    // use higher range to avoid issues
    ID_Taskbar_ShowGUI             = 2000,
    ID_Taskbar_HideGUI,
    ID_Taskbar_Settings,
    ID_Taskbar_Exit,
    ID_Taskbar_Update,
    ID_List_Menu_Create         = 2500,
    ID_List_Menu_Open,
    ID_List_Menu_Mount,
    ID_List_Menu_Unmount,
    ID_List_Menu_Edit,
    ID_List_Menu_Info,
    ID_List_Menu_Browse,
    ID_List_Menu_ForceUnmountAll
};

// enum for return codes related with mount success
enum
{
    ID_MNT_OK,
    ID_MNT_PWDFAIL,
    ID_MNT_OTHER
};




// -----------------------------------------------
// global stuff to manage volumes
// vector of all volumes, fast lookup
std::vector<wxString> v_AllVolumes;
// map of all volumes, using volume name as key
std::map<wxString, DBEntry*> m_VolumeData;
//
// -----------------------------------------------

// keep ref to main form
frmMain * g_frmMain;


// ----------------------------------------------------------------------------
// event tables 
// ----------------------------------------------------------------------------

// frmMain

wxBEGIN_EVENT_TABLE(frmMain, wxFrame)
    EVT_MENU(ID_Menu_Quit,  frmMain::OnQuit)
    EVT_MENU(ID_Menu_About, frmMain::OnAbout)
    EVT_MENU(ID_Menu_New, frmMain::OnNewFolder)
    EVT_MENU(ID_Menu_Existing, frmMain::OnAddExistingFolder)
    EVT_MENU(ID_Menu_Settings, frmMain::OnSettings)
    EVT_MENU(wxID_ANY, frmMain::OnToolLeftClick)
wxEND_EVENT_TABLE()


// ListCTRL specific events
wxBEGIN_EVENT_TABLE(mainListCtrl, wxListCtrl)
    EVT_LIST_ITEM_SELECTED(ID_List_Ctrl, mainListCtrl::OnItemSelected)
    EVT_LIST_ITEM_DESELECTED(ID_List_Ctrl, mainListCtrl::OnItemDeSelected)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_List_Ctrl, mainListCtrl::OnRightClick)
    EVT_MENU(wxID_ANY, mainListCtrl::OnPopupMenuClick)
    EVT_LIST_ITEM_ACTIVATED(ID_List_Ctrl, mainListCtrl::OnItemActivated)    // double-click/enter
wxEND_EVENT_TABLE()


// TaskBarIcon events
wxBEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
    EVT_MENU(ID_Taskbar_Exit, TaskBarIcon::OnMenuExit)
    EVT_MENU(ID_Taskbar_ShowGUI, TaskBarIcon::OnMenuShow)
    EVT_MENU(ID_Taskbar_HideGUI, TaskBarIcon::OnMenuHide)
    EVT_MENU(ID_Taskbar_Settings, TaskBarIcon::OnMenuSettings)
    EVT_MENU(ID_Taskbar_Update, TaskBarIcon::OnMenuUpdate)
    EVT_MENU(wxID_ANY, TaskBarIcon::OnOtherMenuClick)
wxEND_EVENT_TABLE()


// ----------------------------------------------------------------------------
// IMPLEMENTATION
// ----------------------------------------------------------------------------

wxIMPLEMENT_APP(encFSGuiApp);


// ----------------------------------------------------------------------------
// 'Main program' equivalent: the program execution "starts" here
// ----------------------------------------------------------------------------

bool encFSGuiApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // init - no selection made at this point
    g_selectedIndex = -1;
    g_selectedVolume = "";

    // this will be the default config file, that we can Get() when needed
    wxConfigBase *pConfig = wxConfigBase::Create();    
    wxConfigBase::Set(pConfig);
   
    // create the main application window
    wxSize frmMainSize;
    frmMainSize.Set(920,340);
    long framestyle;

    framestyle = wxDEFAULT_FRAME_STYLE ^ wxRESIZE_BORDER | wxFRAME_EX_METAL;

    wxString title;
    title.Printf(wxT(":: [ EncFSGui v%s] ::"), g_encfsguiversion);

    frmMain *frame = new frmMain(title, 
                                 wxDefaultPosition, 
                                 frmMainSize, 
                                 framestyle );

    g_frmMain = frame;

    frame->EnableCloseButton(false);

    pConfig->SetPath(wxT("/Config"));
    bool checkupdates = pConfig->Read(wxT("checkupdates"), 0l);

    // check for updates ?
    if (checkupdates)
    {
        frame->CheckUpdates();
    }


    wxInitAllImageHandlers();
    
    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}


// taskbaricon constructors


TaskBarIcon::TaskBarIcon(wxTaskBarIconType iconType) : wxTaskBarIcon(iconType)
{

}

// Overridables
// this function gets called each time user clicks the taskbar icon
// in other words, menu gets populated dynamically every time

wxMenu *TaskBarIcon::CreatePopupMenu()
{
    wxMenu *menu = new wxMenu;
    menu->Append(ID_Taskbar_ShowGUI, wxT("&Show EncFSGui"));
    menu->Append(ID_Taskbar_HideGUI, wxT("&Hide EncFSGui"));
    menu->AppendSeparator();
    menu->Append(ID_Taskbar_Settings, wxT("S&ettings"));
    menu->AppendSeparator();


    if (g_frmMain->GetVisibleState())
    {
        menu->Enable(ID_Taskbar_ShowGUI, false);
        menu->Enable(ID_Taskbar_HideGUI, true);
    }
    else
    {
        menu->Enable(ID_Taskbar_ShowGUI, true);
        menu->Enable(ID_Taskbar_HideGUI, false);
    }
    

    wxMenu *volumesmenu = new wxMenu;
    int submenuid = 5555;
    for (std::vector<wxString>::iterator it = v_AllVolumes.begin(); it != v_AllVolumes.end(); ++it)
    {
        wxString voltitle;
        wxString volname;
        bool isMounted;
        volname.Printf(wxT("%s"), *it);
        DBEntry * thisvol = m_VolumeData[volname];
        isMounted = thisvol->getMountState();
        voltitle.Printf(wxT("Mount '%s'"), volname);
        volumesmenu->Append(submenuid, voltitle);
        if (isMounted)
        {
            volumesmenu->Enable(submenuid, false);
        }
        else
        {
            volumesmenu->Enable(submenuid, true);
        }
        ++submenuid;
        voltitle.Printf(wxT("Unmount '%s'"), volname);
        volumesmenu->Append(submenuid, voltitle);
        if (isMounted)
        {
            volumesmenu->Enable(submenuid, true);
        }
        else
        {
            volumesmenu->Enable(submenuid, false);
        }
        volumesmenu->AppendSeparator();
        ++submenuid;
    }
    menu->AppendSubMenu(volumesmenu, "&Volumes");
    menu->AppendSeparator();
    menu->Append(ID_Taskbar_Update, wxT("&Check for updates"));

    /* OSX has built-in quit menu for the dock menu, but not for the status item */
    
#ifdef __WXOSX__ 
    if ( OSXIsStatusItem() )
#endif
    {
        menu->AppendSeparator();
        menu->Append(ID_Taskbar_Exit, wxT("E&xit"));
    }

    m_taskBarMenu = menu;
    m_taskBarVolumesMenu = volumesmenu;

    return menu;
}



// taskbaricon member functions
void TaskBarIcon::OnMenuExit(wxCommandEvent& event)
{
    g_frmMain->OnQuit(event);
}

void TaskBarIcon::OnMenuUpdate(wxCommandEvent& WXUNUSED(event))
{
    g_frmMain->CheckUpdates(true);
}


void TaskBarIcon::OnMenuShow(wxCommandEvent& WXUNUSED(event))
{
    g_frmMain->SetVisibleState(true);
}

void TaskBarIcon::OnMenuHide(wxCommandEvent& WXUNUSED(event))
{
    g_frmMain->SetVisibleState(false);
}


void TaskBarIcon::OnMenuSettings(wxCommandEvent& event)
{
    if (!g_frmMain->GetVisibleState())
    {
        g_frmMain->SetVisibleState(true);        
    }
    g_frmMain->OnSettings(event);
}

void TaskBarIcon::OnOtherMenuClick(wxCommandEvent& event)
{
    int clickedid = event.GetId();
    wxString clickedtext = m_taskBarVolumesMenu->GetLabel(clickedid);

    bool domount = false;
    wxString reststr = "";
    wxString volname = "";
    if (clickedtext.StartsWith("Mount '", &reststr))
    {
        domount = true;
        volname = reststr;
        volname.Replace("'","");
    }
    else if (clickedtext.StartsWith("Unmount '", &reststr))
    {
        domount = false;
        volname = reststr;
        volname.Replace("'","");
    }

    if (!volname.IsEmpty())
    {
        wxString prevselectedvol = g_selectedVolume;
        int prevselectedindex = g_selectedIndex;
        g_selectedVolume = volname;
        g_selectedIndex = g_frmMain->GetListCtrlIndex(volname);

        if (domount)
        {
            g_frmMain->OnMount(event);
        }
        else
        {
            g_frmMain->OnUnMount(event);

        }
        g_selectedVolume = prevselectedvol;
        g_selectedIndex = prevselectedindex;

    }

}



// main frame constructor, overload built-in wxFrame

frmMain::frmMain(const wxString& title, 
                 const wxPoint &pos, 
                 const wxSize &size, 
                 long style) : wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
    m_visible = true;
    wxStandardPathsBase& stdp = wxStandardPaths::Get();
    m_listCtrl = NULL;
    m_datadir = stdp.GetUserDataDir();

    m_statusBar = CreateStatusBar(2, wxSB_SUNKEN);

    // set the frame icon
    SetIcon(wxICON(encfsgui_ico));

    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(ID_Menu_About, "&About\tF1", "Show about dialog");

    // create application-specific menu items
    fileMenu->Append(ID_Menu_New, "&Create a new EncFS folder\tF2","Create a new EncFS folder");
    fileMenu->Append(ID_Menu_Existing, "&Open existing EncFS folder\tF4","Open an existing encFS folder");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_Menu_Settings, "&Settings\tF6","Edit global settings");
    fileMenu->Append(ID_Menu_Quit, "E&xit\tAlt-X", "Quit this program");

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);


    // update the StatusBar
    RecreateStatusbar();

    // Populate vector & map with volume information
    PopulateVolumes();

    m_rows = 1;
    // Create the toolbar
    CreateToolbar();
    
    // panel to be used as a container for the Toolbar
    m_panel = new wxPanel(this, wxID_ANY);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    m_panel->SetSizer(sizer);

    // check if we need to mount volumes at startup
    AutoMountVolumes();

    // next, create the actual list control and populate it
    //long flags = wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_ALIGN_LEFT | wxLC_SMALL_ICON | wxLC_HRULES;
    long flags = wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_ALIGN_LEFT;
    m_listCtrl = new mainListCtrl(m_panel, 
                                  ID_List_Ctrl, 
                                  wxDefaultPosition, 
                                  wxDefaultSize, 
                                  flags, 
                                  m_statusBar);
    
    RecreateList();

    m_listCtrl->LinkToolbar(GetToolBar());
    m_listCtrl->UpdateToolBarButtons();

    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    bool startasicon = pConfig->Read(wxT("startasicon"), 0l);

    if (startasicon)
    {
        SetVisibleState(false);
    }
    else
    {
        SetVisibleState(true);
    }

    // finally, add the app icon
    m_taskBarIcon = new TaskBarIcon(wxTBI_DEFAULT_TYPE);
    m_taskBarIcon->SetIcon(wxICON(encfsgui_ico),
                                 "EncFSGui"); 
    
    #if defined(__WXOSX__) && wxOSX_USE_COCOA
        m_dockIcon = new TaskBarIcon(wxTBI_DOCK);
    #endif

}


// constructor mainListCtrl

mainListCtrl::mainListCtrl(wxWindow *parent, 
                           const wxWindowID id, 
                           const wxPoint& pos, 
                           const wxSize& size, 
                           long style, 
                           wxStatusBar * statusbar) : wxListCtrl(parent, id, pos, size, style)
{
    m_statusBar = statusbar;
}


// member functions

bool frmMain::GetVisibleState()
{
    return m_visible;
}

void frmMain::SetVisibleState(bool newstate)
{
    if (newstate)
    {
        if (!m_visible)
        {
            ShowWithEffect(wxSHOW_EFFECT_EXPAND);            
        }
        SetFocus();
        Raise();
    }
    else
    {
        HideWithEffect(wxSHOW_EFFECT_EXPAND);
    }
    m_visible = newstate;
}

int frmMain::GetListCtrlIndex(wxString& volname)
{
    int returnval = -1;
    int itemcount = m_listCtrl->GetItemCount();
    for (int i = 0; i < itemcount; ++i)
    {
        if (m_listCtrl->GetItemText(i, 1) == volname)
        {
            returnval = i;
            break;
        }
    }
    return returnval;
}

void frmMain::PopulateVolumes()
{
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    wxString mountbin = pConfig->Read(wxT("mountbin_path"), "/sbin/mount");

    // get info about already mounted volumes
    wxArrayString mount_output;
    mount_output = ArrRunCMDSync(mountbin);

    v_AllVolumes.clear();
    pConfig->SetPath(wxT("/Volumes"));
    wxString volumename;
    wxString allNames;
    long dummy;
    bool bCont = pConfig->GetFirstGroup(volumename, dummy);
    while ( bCont ) { 
        v_AllVolumes.push_back(volumename); 
        bCont = pConfig->GetNextGroup(volumename, dummy);
    }

    for (unsigned int i = 0; i < v_AllVolumes.size(); i++)
    {
        wxString currentPath;
        wxString enc_path;
        wxString mount_path;
        bool automount;
        bool alreadymounted;
        bool preventautounmount;
        bool pwsaved;
        bool allowother;
        bool mountaslocal;
        wxString volumename = v_AllVolumes.at(i);
        currentPath.Printf(wxT("/Volumes/%s"), volumename);
        pConfig->SetPath(currentPath);
        enc_path = pConfig->Read(wxT("enc_path"), "");
        mount_path = pConfig->Read(wxT("mount_path"), "");
        automount = pConfig->Read(wxT("automount"), 0l);
        preventautounmount = pConfig->Read(wxT("preventautounmount"), 0l);
        alreadymounted = IsVolumeSystemMounted(mount_path, mount_output);
        pwsaved = pConfig->Read(wxT("passwordsaved"), 0l);
        allowother = pConfig->Read(wxT("allowother"), 0l);
        mountaslocal = pConfig->Read(wxT("mountaslocal"), 0l);
        if (not enc_path.IsEmpty() && not mount_path.IsEmpty())
        {
            DBEntry* thisvolume = new DBEntry(volumename, 
                                              enc_path, 
                                              mount_path, 
                                              automount, 
                                              preventautounmount, 
                                              pwsaved,
                                              allowother,
                                              mountaslocal);
            thisvolume->setMountState(alreadymounted);
            // add to map
            m_VolumeData[volumename] = thisvolume;       
        }
    }

    // %u = unsigned int
    int nr_vols;
    nr_vols = v_AllVolumes.size();
    wxString statustxt = wxString::Format(wxT("Nr of volumes : %d"), nr_vols);
    SetStatusText(statustxt,0);
}

void frmMain::CheckUpdates()
{
    CheckUpdates(false);
}

void frmMain::CheckUpdates(bool showIfNoUpdate)
{
    wxString latestversion = getLatestVersion();     
    if (!latestversion.IsEmpty() && latestversion.Find("error") == -1)
    {
        // to do: implement proper version comparison check
        // ignore if you are running a newer version
        if (IsLatestVersionNewer(g_encfsguiversion, latestversion))
        {
            wxString dlurl = "https://github.com/corelan/EncFSGui/raw/master/release/EncFSGUI.dmg";
            wxMessageBox(wxString::Format
             (
                "You are running an outdated version of EncFSGui!\n"
                "Current version: %s\n"
                "Latest version released: %s\n"
                "\nYou can download the latest version from\n%s\n",
                g_encfsguiversion,
                latestversion,
                dlurl
             ),
             "An EncFSGui update was found",
             wxOK | wxICON_INFORMATION,
             this);
        }
        else if (showIfNoUpdate)
        {
            wxMessageBox(wxString::Format
             (
                "You are running the most up-to-date version of EncFSGui!\n"
                "Current version: %s\n"
                "Latest version released: %s\n",
                g_encfsguiversion,
                latestversion
             ),
             "All good",
             wxOK | wxICON_INFORMATION,
             this);
        }
    }
}


bool unmountVolume(wxString& volumename)
{
    DBEntry *thisvol = m_VolumeData[volumename];
    wxString mountvol = thisvol->getMountPath();
    wxString umountbin;
    wxString mountbin;
    bool beenmounted;
    mountbin = getMountBinPath();
    umountbin = getUMountBinPath();
    wxString cmd;
    cmd.Printf(wxT("'%s' '%s'"), umountbin, mountvol);
    wxString cmdoutput;
    cmdoutput = StrRunCMDSync(cmd);
    // get info about already mounted volumes
    wxArrayString mount_output;
    mount_output = ArrRunCMDSync(mountbin);
    wxString mountstr;
    mountstr = arrStrTowxStr(mount_output);
    
    beenmounted = IsVolumeSystemMounted(mountvol, mount_output);
    if (not beenmounted)
    {
        // it's gone - reset stuff
        thisvol->setMountState(false);
        return true;    // unmount success
    }
    return false;
}



void AutoUnmountVolumes(bool forced)
{
    for (std::map<wxString, DBEntry*>::iterator it= m_VolumeData.begin(); it != m_VolumeData.end(); it++)
    {
        wxString volumename = it->first;
        DBEntry * thisvol = it->second;
        if (thisvol->getMountState() && (!thisvol->getPreventAutoUnmount() || forced)) 
        {
            unmountVolume(volumename);
        }
    }
}


//
// event handlers
//

bool QuitApp(wxWindow * parent)
{
    // do we need to dismount all ?
    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    bool autounmount;
    bool nopromptonquit;
    autounmount = pConfig->Read(wxT("autounmount"), 0l);
    nopromptonquit =  pConfig->Read(wxT("nopromptonquit"), 0l);

    wxString hdr;
    hdr.Printf(wxT("Are you sure you want to exit this program?\n"));

    wxString msg;

    if (autounmount)
    {
        msg.Printf(wxT("\n*******************\nAll mounted volumes will be automatically unmounted, except for the ones that were configured to never auto-unmount.\nPlease close all open files first! \n******************* \n"));
    }
    else
    {
        msg.Printf(wxT("\nMounted volumes will not be unmounted!\n"));
    }
    hdr << msg;

    // ask if user is sure to exit

    int res;

    if (nopromptonquit)
    {
            res = wxYES;
    }
    else
    {
        res = wxMessageBox(hdr, wxT("Quit EncFSGui?"), wxYES_NO, parent);
    }

    
    if (res == wxYES)
    {
        delete wxConfigBase::Set((wxConfigBase *) NULL);
        // true is to force the frame to close

        // if autounmount, dismount volumes first
        if (autounmount)
        {
            // do not force
            AutoUnmountVolumes(false);
        }
        return true;
    }
    return false;
}

void frmMain::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    if (QuitApp(this))
    {
        Close(true);
    }
}



int frmMain::OnExit(wxCommandEvent& WXUNUSED(event))
{
    if (QuitApp(this))
    {
        Close(true);
        return 1;
    }
    return 0;
}

// destructor
frmMain::~frmMain()
{
    delete m_taskBarIcon;
    this->Destroy();
    Close(true);
}


void frmMain::OnAbout(wxCommandEvent& WXUNUSED(event))
{

    wxString encfsbinpath = getEncFSBinPath();
    wxString msg = "";
    if (encfsbinpath.IsEmpty())
    {
        msg = "** EncFS not found. Go to Settings & set encfs path **";
    }
    else
    {
        msg = encfsbinpath;
    }

    wxString latestversion;
    wxString versionmessage;
    latestversion = getLatestVersion();

    wxStandardPathsBase& stdp = wxStandardPaths::Get();

    wxMessageBox(wxString::Format
                 (
                    "EncFSGui - GUI Wrapper around encfs, for OSX\n"
                    "Current version: %s\n"
                    "Latest version: %s\n"
                    "written by Peter Van Eeckhoutte (Corelan GCV)\n"
                    "@corelanc0d3r\n\n"
                    "Project repository:\nhttps://github.com/corelan/EncFSGui\n\n"
                    "You are running %s\n\n"
                    "EncFS used: %s\n"
                    "EncFS version: %s\n"
                    "Config Folder: %s\n",
                    g_encfsguiversion,
                    latestversion,
                    wxGetOsDescription(),
                    msg,
                    getEncFSBinVersion(),
                    stdp.GetConfigDir()
                 ),
                 "About EncFSGui",
                 wxOK | wxICON_INFORMATION,
                 this);
}


void frmMain::OnNewFolder(wxCommandEvent& WXUNUSED(event))
{
    SetVisibleState(true);
    createNewEncFSFolder(this);
    RefreshAll();
}


void frmMain::OnAddExistingFolder(wxCommandEvent& WXUNUSED(event))
{
    SetVisibleState(true);
    openExistingEncFSFolder(this);
    RefreshAll();
}



void frmMain::OnBrowseFolder(wxCommandEvent& WXUNUSED(event))
{
    if (not g_selectedVolume.IsEmpty())
    {
        DBEntry * thisvol = m_VolumeData[g_selectedVolume];
        wxString mountpath = thisvol->getMountPath();
        BrowseFolder(mountpath);
    }
}



// mount folder - generic routine
int frmMain::mountFolder(wxString& volumename, wxString& pw)
{
    wxString mountvol;
    wxString encvol;
    bool allowother;
    bool mountaslocal;
    wxString buf;
    
    bool beenmounted;
    DBEntry *thisvol = m_VolumeData[volumename];
    mountvol = thisvol->getMountPath();
    encvol = thisvol->getEncPath();
    allowother = thisvol->getAllowOther();
    mountaslocal = thisvol->getMountAsLocal();

    // run encfs command
    wxString cmd;
    wxString cmdoutput;
    wxString encfsbin = getEncFSBinPath();
    wxString extra_osxfuse_opts = "";
    if (allowother)
    {
        extra_osxfuse_opts << "-o allow_other ";
    }
    if (mountaslocal)
    {
        extra_osxfuse_opts << "-o local ";
    }

    // first, create mount point if necessary
    cmd.Printf(wxT("mkdir -p '%s'"), mountvol);
    cmdoutput = StrRunCMDSync(cmd);

    // mount
    cmd.Printf(wxT("sh -c \"echo '%s' | %s -v -S %s -o volname='%s' '%s' '%s'\""), pw, encfsbin, extra_osxfuse_opts, volumename, encvol, mountvol);

    cmdoutput = StrRunCMDSync(cmd);

    // check if mount was successful
    wxString errmsg;
    errmsg = "Error decoding volume key, password incorrect";

    //wxLogDebug(wxT("----------------------------"));
    //wxLogDebug(cmdoutput);
    //wxLogDebug(wxT("----------------------------"));
    if (cmdoutput.Find(errmsg) > -1)
    {
        return ID_MNT_PWDFAIL;
    }

    // check mount list, to be sure
    wxString mountbin = getMountBinPath();

    // get info about already mounted volumes
    wxArrayString mount_output;
    mount_output = ArrRunCMDSync(mountbin);
    wxString mountstr;
    mountstr = arrStrTowxStr(mount_output);
    
    beenmounted = IsVolumeSystemMounted(mountvol, mount_output);
    if (beenmounted)
    {
        thisvol->setMountState(true);
        return ID_MNT_OK;
    }
    return ID_MNT_OTHER;
}




// unmount folder, generic routine
bool frmMain::unmountVolumeAsk(wxString& volumename)
{
    wxString msg;
    wxString title;
    wxString mountvol;
    bool unmountok;
    unmountok = false;

    DBEntry *thisvol = m_VolumeData[volumename];
    mountvol = thisvol->getMountPath();

    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    bool skippromptunmount = pConfig->Read(wxT("nopromptonunmount"), 0l);

    if (skippromptunmount)
    {
        unmountok = unmountVolume(volumename);
    }
    else
    {
        msg.Printf(wxT("Are you sure you want to unmount\n'%s' ?\n\nNote: make sure to close all open files\nbefore clicking 'Yes'."),mountvol);
        title.Printf(wxT("Unmount '%s' ?"), volumename);

        wxMessageDialog * dlg = new wxMessageDialog(this, 
                                                    msg, 
                                                    title, 
                                                    wxYES_NO|wxCENTRE|wxNO_DEFAULT|wxICON_QUESTION);
        if (dlg->ShowModal() == wxID_YES)
        {
            unmountok = unmountVolume(volumename);
        }
        dlg->Destroy();
    }
    return unmountok; // unmount did not work, or not selected 
}




int frmMain::mountSelectedFolder(wxString& pw)
{
    wxString buf;   
    // update statustext
    wxString msg;
    msg.Printf(wxT("Mounting '%s'"), g_selectedVolume);
    PushStatusText(msg,0);

    buf.Printf(wxT("%s"), ".....");
    m_listCtrl->SetItem(g_selectedIndex, 0, buf);

    int mountstatus = mountFolder(g_selectedVolume, pw);

    if (mountstatus == ID_MNT_OK)
    {
        buf.Printf(wxT("%s"), "YES");
        wxColour itemColour = wxColour(*wxRED);
        m_listCtrl->SetItemTextColour(g_selectedIndex, itemColour);
        m_listCtrl->SetItem(g_selectedIndex, 0, buf);
        SetToolBarButtonState(ID_Toolbar_Mount, false);
        SetToolBarButtonState(ID_Toolbar_Unmount, true);
        SetToolBarButtonState(ID_Toolbar_Browse, true);
        SetToolBarButtonState(ID_Toolbar_Edit, true);   // limited edits allowed
    }
    else
    {
        buf.Printf(wxT("%s"), "NO");
        wxColour itemColour = wxColour(*wxBLUE);
        m_listCtrl->SetItemTextColour(g_selectedIndex, itemColour);
        m_listCtrl->SetItem(g_selectedIndex, 0, buf);
        SetToolBarButtonState(ID_Toolbar_Unmount, false);
        SetToolBarButtonState(ID_Toolbar_Mount, true);
        SetToolBarButtonState(ID_Toolbar_Browse, false);
        SetToolBarButtonState(ID_Toolbar_Edit, true);
    }
    PopStatusText(0);
    return mountstatus;
}


void frmMain::AutoMountVolumes()
{
    // iterate over volumes
    for (std::map<wxString, DBEntry*>::iterator it= m_VolumeData.begin(); it != m_VolumeData.end(); it++)
    {
        wxString volumename = it->first;
        DBEntry * thisvol = it->second;
        wxString mountvol = thisvol->getMountPath();
        wxString encvol = thisvol->getEncPath();
        wxString title;
        title.Printf(wxT("Automount '%s'"), volumename);
        if ((not thisvol->getMountState()) && (thisvol->getAutoMount()) )
        {
            bool trymount = true;
            int nrtries = 0;
            wxString extratxt = "";
            while (trymount && nrtries < 5)
            {
                wxString msg;
                msg.Printf(wxT("%sPlease enter password to auto-mount\n'%s'\nas\n'%s'"), extratxt, encvol, mountvol);
                wxString pw;
                if (thisvol->getPwSavedState())
                {
                    pw = getKeychainPassword(volumename);
                }
                else
                {
                    pw = getPassWord(title, msg);
                }
                 
                
                if (!pw.IsEmpty())
                {
                     // try mount
                    int mountstatus = mountFolder(volumename, pw);
                    // to do : instead of setting pw to a new value, clear out memory location directly 
                    pw = "GoodLuckWithThat";

                    if (mountstatus == ID_MNT_PWDFAIL)
                    {
                        extratxt.Printf(wxT("** You have entered an invalid password **\n\n"));
                    }
                    else if (mountstatus == ID_MNT_OK)
                    {
                        trymount = false;
                    }
                    else if (mountstatus == ID_MNT_OTHER)
                    {
                        // show message and then break
                        wxString errormsg;
                        wxString errortitle;
                        errormsg.Printf(wxT("Unable to mount volume '%s'\nEncfs folder: %s\nMount path: %s"), volumename, encvol, mountvol);
                        errortitle.Printf(wxT("Error found while mounting '%s'"), volumename);
                        wxMessageDialog * dlg = new wxMessageDialog(this, 
                                                                    errormsg, 
                                                                    errortitle, 
                                                                    wxOK|wxCENTRE|wxICON_ERROR);
                        dlg->ShowModal();
                        dlg->Destroy();
                        trymount = false;
                    }   
                }
                else
                {
                    // bail out
                    trymount = false;
                }
                nrtries++;
            }            
        }
    }
    // mount when needed
}

void frmMain::OnForceUnMountAll(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    wxString title;
    bool unmountok;
    int nrmounted = 0;
    unmountok = false;

    wxConfigBase *pConfig = wxConfigBase::Get();
    pConfig->SetPath(wxT("/Config"));
    bool skippromptunmount = pConfig->Read(wxT("nopromptonunmount"), 0l);


    // count how many volumes are mounted
    for (std::map<wxString, DBEntry*>::iterator it= m_VolumeData.begin(); it != m_VolumeData.end(); it++)
    {
        wxString volumename = it->first;
        DBEntry * thisvol = it->second;
        if (thisvol->getMountState()) 
        {
            ++nrmounted;
        }
    }

    if (nrmounted > 0)
    {
        if (skippromptunmount)
        {
            // force unmount
            AutoUnmountVolumes(true);
            RefreshAll();
        }
        else
        {
            msg.Printf(wxT("There are currently %d mounted volumes.\n\nAre you sure you want to unmount ALL those volumes at once?\n\nNote: make sure to close all open files\nbefore clicking 'Yes'."), nrmounted);
            title.Printf(wxT("Unmount ALL volumes?"));
            wxMessageDialog * dlg = new wxMessageDialog(this, 
                                                        msg, 
                                                        title, 
                                                        wxYES_NO|wxCENTRE|wxNO_DEFAULT|wxICON_QUESTION);
            if (dlg->ShowModal() == wxID_YES)
            {
                // force unmount on all mounted volumes
                AutoUnmountVolumes(true);
                RefreshAll();
            }
            dlg->Destroy();
        }   
    }
} 



void frmMain::OnUnMount(wxCommandEvent& WXUNUSED(event))
{
    bool beenunmounted;
    beenunmounted = unmountVolumeAsk(g_selectedVolume);
    if (beenunmounted)
    {
        // it's gone - reset stuff
        wxString buf;
        buf.Printf(wxT("%s"), "NO");
        wxColour itemColour = wxColour(*wxBLUE);
        m_listCtrl->SetItemTextColour(g_selectedIndex, itemColour);
        m_listCtrl->SetItem(g_selectedIndex, 0, buf);
        SetToolBarButtonState(ID_Toolbar_Mount, true);
        SetToolBarButtonState(ID_Toolbar_Unmount, false);
        SetToolBarButtonState(ID_Toolbar_Browse, false);
        SetToolBarButtonState(ID_Toolbar_Edit, true);
    }
    else
    {
        // still mounted
        SetToolBarButtonState(ID_Toolbar_Mount, false);
        SetToolBarButtonState(ID_Toolbar_Unmount, true);
        SetToolBarButtonState(ID_Toolbar_Browse, true);
        SetToolBarButtonState(ID_Toolbar_Edit, true); // limited edits allowed
    }
}

void frmMain::OnInfo(wxCommandEvent& WXUNUSED(event))
{
    // get full encfpath for this volume
    DBEntry * thisvol = m_VolumeData[g_selectedVolume];
    wxString encvol = thisvol->getEncPath();
    wxArrayString volinfo = getEncFSVolumeInfo(encvol);
    wxString msg = arrStrTowxStr(volinfo);
    wxString title;
    title.Printf(wxT("EncFS information for '%s'"), g_selectedVolume);
    wxString msgbody;
    msgbody.Printf(wxT("Encrypted path: '%s'\n\n"), encvol);
    msgbody << msg;
    
    wxMessageDialog * dlg = new wxMessageDialog(this, msgbody, title, wxOK|wxCENTRE|wxICON_INFORMATION);
    dlg->ShowModal();
    dlg->Destroy();
}


void frmMain::OnMount(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    wxString extratxt = "";
    wxString mountvol;
    wxString encvol;
    wxString title;
    title.Printf(wxT("Enter password for '%s'"), g_selectedVolume);
    DBEntry * thisvol = m_VolumeData[g_selectedVolume];
    mountvol = thisvol->getMountPath();
    encvol = thisvol->getEncPath();
    bool trymount = true;
    int nrtries = 0;
    while (trymount && nrtries < 5)
    {
        msg.Printf(wxT("%sPlease enter password to mount\n'%s'\nas\n'%s'"), extratxt, encvol,mountvol);
        wxString pw;
        if (thisvol->getPwSavedState())
        {
            pw = getKeychainPassword(g_selectedVolume);
        }
        else
        {
            pw = getPassWord(title, msg);   
        }

        if (!pw.IsEmpty())
        {
            // try mount
            int successmount = mountSelectedFolder(pw);
            
            pw = "GoodLuckWithThat";
            if (successmount == ID_MNT_PWDFAIL)
            {
                extratxt.Printf(wxT("** You have entered an invalid password **\n\n"));
            }
            else if (successmount == ID_MNT_OK)
            {
                trymount = false;
            }
            else if (successmount == ID_MNT_OTHER)
            {
                wxString errormsg;
                wxString errortitle;
                errormsg.Printf(wxT("Unable to mount volume '%s'\nEncfs folder: %s\nMount path: %s"), g_selectedVolume, thisvol->getEncPath(), thisvol->getMountPath());
                errortitle.Printf(wxT("Error found while mounting '%s'"), g_selectedVolume);
                wxMessageDialog * dlg = new wxMessageDialog(this, errormsg, errortitle, wxOK|wxCENTRE|wxICON_ERROR);
                dlg->ShowModal();
                dlg->Destroy();
                trymount = false;
            }
        }
        else
        {
            // bail out
            trymount = false;
        }
        nrtries++;
    }
}


wxString frmMain::getPassWord(wxString& title, wxString& prompt)
{
    wxString pw = "";
    wxPasswordEntryDialog * dlg = new wxPasswordEntryDialog(this, 
                                                            prompt, 
                                                            title , 
                                                            wxEmptyString, 
                                                            wxTextEntryDialogStyle, 
                                                            wxDefaultPosition);
    if (dlg->ShowModal() == wxID_OK)
    {
        pw = dlg->GetValue();
        return pw;
    }
    dlg->Destroy();
    return "";
}


void frmMain::OnEditFolder(wxCommandEvent& WXUNUSED(event))
{
    editExistingEncFSFolder(this, g_selectedVolume, m_VolumeData);
    RefreshAll();
}

void frmMain::OnRemoveFolder(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    wxString title;
    bool deleted = false;
    title.Printf(wxT("Remove '%s' ?"), g_selectedVolume);
    msg.Printf(wxT("Are you really sure you want to remove volume '%s' from this application?\n"), g_selectedVolume);
    msg << "Notes:\n";
    msg << "1. This action will NOT remove the actual folders and/or data.  It will only cause this application to forget about this volume.\n";
    msg << "2. If you have removed a volume by mistake, you can simply add it back via 'Open existing encfs folder'.\n";
    msg << "3. Removing a mounted volume will NOT unmount it.\n";
    wxMessageDialog * dlg = new wxMessageDialog(this, 
                                                msg, 
                                                title, 
                                                wxYES_NO|wxCENTRE|wxNO_DEFAULT|wxICON_QUESTION);
    if (dlg->ShowModal() == wxID_YES)
    {
        // simply remove from config file by removing the config group
        wxConfigBase *pConfig = wxConfigBase::Get();
        wxString configgroup;
        configgroup.Printf(wxT("/Volumes/%s"), g_selectedVolume);
        pConfig->DeleteGroup(configgroup);
        deleted = true;
    }
    dlg->Destroy();
    if (deleted)
    {
        RefreshAll();
    }
}


void frmMain::OnSettings(wxCommandEvent& WXUNUSED(event))
{
    if (!m_visible)
    {
        SetVisibleState(true);        
    }
    openSettings(this);
    RefreshAll();
}

void frmMain::OnToolLeftClick(wxCommandEvent& event)
{
    if (event.GetId() == ID_Toolbar_Create)
    {
        createNewEncFSFolder(this);
        RefreshAll();
    }
    else if (event.GetId() == ID_Toolbar_Existing)
    {
        openExistingEncFSFolder(this);
        RefreshAll();
    }
    else if (event.GetId() == ID_Toolbar_Settings)
    {
        openSettings(this);
        RefreshAll();
    }
    else if (event.GetId() == ID_Toolbar_Edit)
    {
        editExistingEncFSFolder(this, g_selectedVolume, m_VolumeData);
        RefreshAll();
    }
    else if (event.GetId() == ID_Toolbar_Quit)
    {
        OnQuit(event);
    }
    else if (event.GetId() == ID_Toolbar_Browse)
    {
        OnBrowseFolder(event);
    }
    else if (event.GetId() == ID_Toolbar_Mount)
    {
        OnMount(event);
    }
    else if (event.GetId() == ID_Toolbar_Unmount)
    {
        OnUnMount(event);
    }
    else if (event.GetId() == ID_Toolbar_UnmountAll)
    {
        OnForceUnMountAll(event);
    }    
    else if (event.GetId() == ID_Toolbar_Info)
    {
        OnInfo(event);
    }
    else if (event.GetId() == ID_Toolbar_Remove)
    {
        OnRemoveFolder(event);
    }
}


//
// Statusbar
//

void frmMain::RecreateStatusbar()
{
    #if wxUSE_STATUSBAR
        
        wxFont font = m_statusBar->GetFont();
        font.SetWeight(wxFONTWEIGHT_BOLD);
        m_statusBar->SetFont(font);

        // check if encfs is installed
        if (isEncFSBinInstalled())
        {
            wxString statustxt;
            statustxt.Printf("encFS ready // %s", getEncFSBinVersion());
            SetStatusText(statustxt,1);  
        }
        else
        {
            SetStatusText("** encFS not found. Open 'Settings' and set path to encfs **",1);    
        }
        
    #endif // wxUSE_STATUSBAR
}


//
// Toolbar
//


void frmMain::CreateToolbar()
{
    // delete and recreate the toolbar
    wxToolBarBase *toolBar = GetToolBar();
    static const long TOOLBAR_STYLE = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT;
    long style = toolBar ? toolBar->GetWindowStyle() : TOOLBAR_STYLE;
    delete toolBar; // just in case
    SetToolBar(NULL);
    toolBar = CreateToolBar(style, ID_TOOLBAR);
    PopulateToolbar(toolBar);
}

void frmMain::PopulateToolbar(wxToolBarBase* toolBar)
{
     // Set up toolbar
    enum
    {
        Tool_createfolder,
        Tool_existingfolder,
        Tool_browsefolder,
        Tool_removefolder,
        Tool_editfolder,
        Tool_folderinfo,
        Tool_mountfolder,
        Tool_unmountfolder,
        Tool_unmountfolder_all,
        Tool_settings,
        Tool_quit,
        Tool_Max
    };


    wxBitmap toolBarBitmaps[Tool_Max];

    #define INIT_TOOL_BMP(bmp) toolBarBitmaps[Tool_##bmp] = wxBitmap(bmp)

    INIT_TOOL_BMP(createfolder);
    INIT_TOOL_BMP(existingfolder);
    INIT_TOOL_BMP(browsefolder);
    INIT_TOOL_BMP(removefolder);
    INIT_TOOL_BMP(editfolder);
    INIT_TOOL_BMP(folderinfo);
    INIT_TOOL_BMP(mountfolder);
    INIT_TOOL_BMP(unmountfolder);
    INIT_TOOL_BMP(unmountfolder_all);
    INIT_TOOL_BMP(settings);
    INIT_TOOL_BMP(quit);

    // get size from first bitmap
    int w = toolBarBitmaps[Tool_createfolder].GetWidth(),
        h = toolBarBitmaps[Tool_createfolder].GetHeight();

    // this call is actually unnecessary as the toolbar will adjust its tools
    // size to fit the biggest icon used anyhow but it doesn't hurt neither
    toolBar->SetToolBitmapSize(wxSize(w, h));
    

    toolBar->AddTool(ID_Toolbar_Create, wxT("Create"),
                     toolBarBitmaps[Tool_createfolder], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Create new encfs folder"), wxT("Create new encfs folder"));

    toolBar->AddTool(ID_Toolbar_Existing, wxT("Open Existing"),
                     toolBarBitmaps[Tool_existingfolder], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Open existing encfs folder"), wxT("Open existing encfs folder"));

    toolBar->AddSeparator();

    toolBar->AddTool(ID_Toolbar_Browse, wxT("Browse"),
                     toolBarBitmaps[Tool_browsefolder], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Browse mounted encfs folder"), wxT("Browse mounted encfs folder"));

    toolBar->AddTool(ID_Toolbar_Remove, wxT("Remove"),
                     toolBarBitmaps[Tool_removefolder], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Remove encfs folder"), wxT("Remove encfs folder from database"));

    toolBar->AddTool(ID_Toolbar_Info, wxT("Info"),
                     toolBarBitmaps[Tool_folderinfo], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Show info"), wxT("Show encfs related information about this folder"));


    toolBar->AddTool(ID_Toolbar_Edit, wxT("Edit"),
                     toolBarBitmaps[Tool_editfolder], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Edit encfs folder"), wxT("Edit encfs folder configuration"));

    toolBar->AddSeparator();


    toolBar->AddTool(ID_Toolbar_Mount, wxT("Mount && expose"),
                     toolBarBitmaps[Tool_mountfolder], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Mount and expose a protected encfs folder"), wxT("Mount and expose a protected encfs folder"));


    toolBar->AddTool(ID_Toolbar_Unmount, wxT("Unmount && lock"),
                     toolBarBitmaps[Tool_unmountfolder], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Unmount and protect a mounted encfs folder"), wxT("Unmount and protect a mounted encfs folder"));

    toolBar->AddTool(ID_Toolbar_UnmountAll, wxT("Force Unmount all"),
                     toolBarBitmaps[Tool_unmountfolder_all], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Unmount and protect all mounted encfs folders"), wxT("Unmount and protect all mounted encfs folders"));

    toolBar->AddSeparator();

    toolBar->AddTool(ID_Toolbar_Settings, wxT("Settings"),
                     toolBarBitmaps[Tool_settings], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Global settings"), wxT("Global settings"));

    toolBar->AddSeparator();

    toolBar->AddTool(ID_Toolbar_Quit, wxT("Quit"),
                     toolBarBitmaps[Tool_quit], wxNullBitmap, wxITEM_NORMAL,
                     wxT("Quit"), wxT("Quit this program"));


    // after adding the buttons to the toolbar, must call Realize() to reflect
    // the changes
    toolBar->Realize();

    toolBar->SetRows(toolBar->IsVertical() ? toolBar->GetToolsCount() / m_rows
                                           : m_rows);
}


void mainListCtrl::UpdateToolBarButtons()
{
    // keep certain buttons disabled when encfs is not found/installed
    bool encfsbininstalled = isEncFSBinInstalled();
    m_toolBar->EnableTool(ID_Toolbar_Create, encfsbininstalled);
    m_toolBar->EnableTool(ID_Toolbar_Existing, encfsbininstalled);
    m_toolBar->EnableTool(ID_Toolbar_Browse, encfsbininstalled);
    m_toolBar->EnableTool(ID_Toolbar_Remove, encfsbininstalled);
    m_toolBar->EnableTool(ID_Toolbar_Edit, encfsbininstalled);
    m_toolBar->EnableTool(ID_Toolbar_Info, encfsbininstalled);
    m_toolBar->EnableTool(ID_Toolbar_Mount, encfsbininstalled);
    m_toolBar->EnableTool(ID_Toolbar_Unmount, encfsbininstalled);

    if (encfsbininstalled)
    {
        if (g_selectedIndex > -1)  // a line was selected
        {
            m_toolBar->EnableTool(ID_Toolbar_Remove, true);
            m_toolBar->EnableTool(ID_Toolbar_Edit, true);
            m_toolBar->EnableTool(ID_Toolbar_Browse, true);
            m_toolBar->EnableTool(ID_Toolbar_Info, true);

            // to do - add logic to check if selected volume is mounted
            DBEntry *thisvolume = m_VolumeData[g_selectedVolume];
            if (thisvolume->getMountState())
            {
                m_toolBar->EnableTool(ID_Toolbar_Mount, false);
                m_toolBar->EnableTool(ID_Toolbar_Unmount, true);
                m_toolBar->EnableTool(ID_Toolbar_Browse, true);   
                m_toolBar->EnableTool(ID_Toolbar_Edit, true);   
            }
            else
            {
                m_toolBar->EnableTool(ID_Toolbar_Mount, true);
                m_toolBar->EnableTool(ID_Toolbar_Unmount, false); 
                m_toolBar->EnableTool(ID_Toolbar_Browse, false);
                m_toolBar->EnableTool(ID_Toolbar_Edit, true); 
            }    
        }
        else   // no selection in ListCtrl
        {
            m_toolBar->EnableTool(ID_Toolbar_Remove, false);
            m_toolBar->EnableTool(ID_Toolbar_Edit, false);
            m_toolBar->EnableTool(ID_Toolbar_Browse, false);
            m_toolBar->EnableTool(ID_Toolbar_Mount, false);
            m_toolBar->EnableTool(ID_Toolbar_Unmount, false);
            m_toolBar->EnableTool(ID_Toolbar_Info, false);    
        }    
    }

}



// in case we want to flip state of a specific button
void frmMain::SetToolBarButtonState(int ButtonID, bool newstate)
{
    wxToolBarBase *toolBar = GetToolBar();
    toolBar->EnableTool(ButtonID, newstate);
}


// ------------------------------------------------
// list
// ------------------------------------------------

void frmMain::DoSize()
{
    wxSize size = GetClientSize();
    wxCoord y = (2*size.y);
    m_listCtrl->SetSize(0, 0, size.x, y);
}

void frmMain::RecreateList()
{
    {        
        m_listCtrl->ClearAll();

        FillListWithVolumes();

        DoSize();
    }   
}

void frmMain::FillListWithVolumes()
{
    // global font settings
    wxFont font = this->GetFont();
    font.MakeSmaller();

    wxString columnHeader;

    // create columns
    
    columnHeader = "Mounted";
    m_listCtrl->AppendColumn(columnHeader);

    columnHeader = "Volume name";
    m_listCtrl->AppendColumn(columnHeader);

    columnHeader = "Encrypted folder";
    m_listCtrl->AppendColumn(columnHeader);

    columnHeader = "Mounted at";
    m_listCtrl->AppendColumn(columnHeader);

    columnHeader = "Automount";
    m_listCtrl->AppendColumn(columnHeader);


    // change Column width
    // Mounted
    m_listCtrl->SetColumnWidth(0,65);
    // Volume Name
    m_listCtrl->SetColumnWidth(1,120);
    // EncryptedFolder
    m_listCtrl->SetColumnWidth(2,320);
    // Mounted At
    m_listCtrl->SetColumnWidth(3,300);
    // Automount
    m_listCtrl->SetColumnWidth(4,70);


    
    // to speed up inserting we hide the control temporarily
    m_listCtrl->Hide();

    for (unsigned int rowindex = 0; rowindex < v_AllVolumes.size(); rowindex++)
    {
        wxString buf;
        wxString volumename;

        volumename = v_AllVolumes.at(rowindex);
        DBEntry * thisvol;
        thisvol = m_VolumeData[volumename];

        bool isMounted;
        isMounted = thisvol->getMountState();

        wxColour itemColour;
        // column[0]

        if (isMounted)
        {
            buf.Printf(wxT("%s"), "YES");
            itemColour = wxColour(*wxRED);
        }
        else
        {  
            buf.Printf(wxT("%s"), "NO"); 
            itemColour = wxColour(*wxBLUE);
        }

        long rid = m_listCtrl->InsertItem(rowindex, buf, 0);
        m_listCtrl->SetItemData(rid, rowindex);
        m_listCtrl->SetItemFont(rid, font);
        m_listCtrl->SetItemTextColour(rid, itemColour);


        // column[1]
        buf.Printf(wxT("%s"), volumename);
        m_listCtrl->SetItem(rid, 1, buf);

        // column[2]
        buf.Printf(wxT("%s"), thisvol->getEncPath());
        m_listCtrl->SetItem(rid, 2, buf);

        // column[3]
        buf.Printf(wxT("%s"), thisvol->getMountPath());
        m_listCtrl->SetItem(rid, 3, buf);

        // column[4]
        if (thisvol->getAutoMount())
        {
            buf.Printf(wxT("YES"));
        }
        else
        {
            buf.Printf(wxT("NO"));
        }
        m_listCtrl->SetItem(rid, 4, buf);
    }

    m_listCtrl->Show();
}


void frmMain::RefreshAll()
{
    PopulateVolumes();
    m_listCtrl->UpdateToolBarButtons();
    RecreateList();
    RecreateStatusbar();
}



// ----------------------------------------------------------------------------
// CDBEntry member functions
// ----------------------------------------------------------------------------

// DBENtry constructor
DBEntry::DBEntry(wxString volname, 
                 wxString enc_path, 
                 wxString mount_path, 
                 bool automount, 
                 bool preventautounmount, 
                 bool pwsaved,
                 bool allowother,
                 bool mountaslocal)
{
    m_automount = automount;
    m_volname = volname;
    m_enc_path = enc_path;
    m_mount_path = mount_path;
    m_preventautounmount = preventautounmount;
    m_pwsaved = pwsaved;
    m_allowother = allowother;
    m_mountaslocal = mountaslocal;
}


void DBEntry::setMountState(bool newstate)
{
    m_mountstate = newstate;
}

bool DBEntry::getMountState()
{
    return m_mountstate;
}

bool DBEntry::getPreventAutoUnmount()
{
    return m_preventautounmount;
}

bool DBEntry::getPwSavedState()
{
    return m_pwsaved;
}

wxString DBEntry::getEncPath()
{
    return m_enc_path;
}

bool DBEntry::getAutoMount()
{
    return m_automount;
}

wxString DBEntry::getMountPath()
{
    return m_mount_path;
}

wxString DBEntry::getVolName()
{
    return m_volname;
}

bool DBEntry::getAllowOther()
{
    return m_allowother;
}

bool DBEntry::getMountAsLocal()
{
    return m_mountaslocal;
}


// ----------------------------------------------------------------------------
// mainListCtrl member functions
// ----------------------------------------------------------------------------


void mainListCtrl::LinkToolbar(wxToolBarBase * toolbar)
{
    m_toolBar = toolbar;
}


void mainListCtrl::SetSelectedIndex(int index)
{
    g_selectedIndex = index;
    int nr_vols;
    nr_vols = v_AllVolumes.size();
    wxString selvol;

    if (index >= 0)
    {
        g_selectedVolume = GetItemText(index,1);
        selvol.Printf(wxT("// Selected volume: %s"),g_selectedVolume);
    }
    else
    {
        g_selectedVolume = "";
        selvol = "";
    }
    wxString statustxt = wxString::Format(wxT("Nr of volumes : %d %s"), nr_vols, selvol);
    m_statusBar->SetStatusText(statustxt,0);
    UpdateToolBarButtons();
}


void mainListCtrl::OnItemSelected(wxListEvent& WXUNUSED(event))
{
    long itemIndex = -1;

    while ((itemIndex = GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != wxNOT_FOUND) 
    {
        SetSelectedIndex(itemIndex);
    }
}

void mainListCtrl::OnPopupMenuClick(wxCommandEvent& event)
{
    // see what we have clicked
    if (event.GetId() == ID_List_Menu_Create)
    {
        g_frmMain->OnNewFolder(event);
    }
    else if (event.GetId() == ID_List_Menu_Open)
    {
        g_frmMain->OnAddExistingFolder(event);
    }
    else if (event.GetId() == ID_List_Menu_Unmount)
    {
        g_frmMain->OnUnMount(event);
    }
    else if (event.GetId() == ID_List_Menu_Mount)
    {
        g_frmMain->OnMount(event);
    }
    else if (event.GetId() == ID_List_Menu_Edit)
    {
        g_frmMain->OnEditFolder(event);
    }
    else if (event.GetId() == ID_List_Menu_Info)
    {
        g_frmMain->OnInfo(event);
    }
    else if (event.GetId() == ID_List_Menu_Browse)
    {
        g_frmMain->OnBrowseFolder(event);
    }
    else if (event.GetId() == ID_List_Menu_ForceUnmountAll)
    {
        g_frmMain->OnForceUnMountAll(event);
    }    
}

void mainListCtrl::OnRightClick(wxListEvent& event)
{
    wxMenu *menu = new wxMenu();
    if (g_selectedIndex > -1)
    {
        DBEntry * thisvol = m_VolumeData[g_selectedVolume];
        bool isMounted = thisvol->getMountState();
        wxString msg;
        if (isMounted)
        {
            msg.Printf(wxT("Unmount '%s'"), g_selectedVolume);
            menu->Append(ID_List_Menu_Unmount, msg);
            menu->AppendSeparator();
            msg.Printf(wxT("Browse '%s'"), g_selectedVolume);
            menu->Append(ID_List_Menu_Browse, msg);
        }
        else
        {
            msg.Printf(wxT("Mount '%s'"), g_selectedVolume);
            menu->Append(ID_List_Menu_Mount, msg);
            menu->AppendSeparator();
        }
        
        msg.Printf(wxT("Edit '%s'"), g_selectedVolume);
        menu->Append(ID_List_Menu_Edit, msg);
        msg.Printf(wxT("Show info about '%s'"), g_selectedVolume);
        menu->Append(ID_List_Menu_Info, msg);
        menu->AppendSeparator();
    }

    menu->Append(ID_List_Menu_Create, wxT("Create a new encfs folder"));
    menu->Append(ID_List_Menu_Open, wxT("Open an existing encfs folder"));
    menu->AppendSeparator();    
    menu->Append(ID_List_Menu_ForceUnmountAll, wxT("Force unmount all"));

    PopupMenu(menu, event.GetPoint());
}



void mainListCtrl::OnItemDeSelected(wxListEvent& WXUNUSED(event))
{
    // this allows us to disable toolbar buttons again when no selection is active in ListCtrl
    long itemIndex = -1;
    bool somethingselected = false;

    while ((itemIndex = GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != wxNOT_FOUND) 
    {
        somethingselected = true;
    }
    if (not somethingselected)
    {
        SetSelectedIndex(-1);
    }
}

void mainListCtrl::OnItemActivated(wxListEvent& WXUNUSED(event))
{
    if (g_selectedIndex > -1)
    {
        // if volume was mounted, browse the folder
        DBEntry * thisvol = m_VolumeData[g_selectedVolume];
        if (thisvol->getMountState())
        {
            // open
            wxString mountpath = thisvol->getMountPath();
            BrowseFolder(mountpath);
        }
        else
        {
            // edit
            editExistingEncFSFolder(this, g_selectedVolume, m_VolumeData);
            g_frmMain->RefreshAll();
        }
    }
}
