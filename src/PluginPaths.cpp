#include   <sstream>

#include   "config.h"
#include   "OCPNPlatform.h"
#include   "PluginPaths.h"
#include   "ocpn_plugin.h"

/*
 * The user-writable paths for libraries, binaries and plugin data, 
 * one path each. And the list of paths used fo loading plugin
 * plugin libraries, locating helper binaries and storing plugin
 * data.
 */


const char* const LINUX_DATA_PATH =
    "~/.local/share:/usr/local/share:/usr/share";

extern OCPNPlatform*  g_Platform;
extern bool           g_bportable;


static std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter)) {
      tokens.push_back(token);
   }
   return tokens;
}


static std::string expand(const std::string& s)
{
    wxFileName fn(s);
    fn.Normalize();
    return fn.GetFullPath().ToStdString();
}

PluginPaths* PluginPaths::getInstance()
{
    static PluginPaths* instance = 0;
    if (!instance) {
        instance = new(PluginPaths);
    }
    return instance;
}


void PluginPaths::initWindowsPaths()
{
    using namespace std;

    const string platform_dir = g_Platform->GetPluginDir().ToStdString();
    const string winPluginBaseDir =
        g_Platform->GetWinPluginBaseDir().ToStdString();
    m_userLibdir = winPluginBaseDir;
    m_userBindir = winPluginBaseDir;
    m_userDatadir = winPluginBaseDir;
    m_unknownPathDir = winPluginBaseDir + "\\unknown-prefix";

    m_libdirs.push_back(m_userLibdir);
    m_libdirs.push_back(g_Platform->GetPluginDir().ToStdString());
    m_bindirs = m_libdirs;

    m_datadirs.push_back(platform_dir + "\\plugins");
    m_datadirs.push_back(winPluginBaseDir);
}


void PluginPaths::initFlatpackPaths()
{
    using namespace std;

    const string flathome = m_home + "/.var/app/org.opencpn.OpenCPN";
    m_userLibdir = flathome + "/lib";
    m_userBindir = flathome + "/bin";
    m_userDatadir = flathome + "/data";
    m_unknownPathDir = flathome + "/unknown-prefix";

    m_libdirs.push_back(flathome + "/lib");
    m_libdirs.push_back("/app/extensions/lib/opencpn");
    m_libdirs.push_back("/app/lib/opencpn");

    m_bindirs.push_back(flathome + "/bin");
    m_bindirs.push_back("/app/extensions/bin");
    m_bindirs.push_back("/app/bin");

    m_datadirs.push_back(flathome + "/data/plugins");
    m_datadirs.push_back("/app/extensions/share/opencpn/plugins");
    m_datadirs.push_back("/app/share/opencpn/plugins");
}


void PluginPaths::initLinuxPaths()
{
    using namespace std;

    if(g_bportable){
        m_userLibdir = g_Platform->GetPrivateDataDir().ToStdString() + "/plugins/lib";                  //m_home + "/.local/lib";
        m_libdirs.push_back(m_userLibdir);
        m_userBindir = g_Platform->GetPrivateDataDir().ToStdString() + "/plugins/bin";                          //m_home + "/.local/bin";
        m_bindirs = m_libdirs;
        m_userDatadir = g_Platform->GetPrivateDataDir().ToStdString() + "/plugins/share";                       //m_home + "/.local/share";
        m_datadirs.push_back(m_userDatadir);
        m_unknownPathDir = g_Platform->GetPrivateDataDir().ToStdString() + "/plugins/share/unknown-prefix";     //m_home + "/.local/share/opencpn/unknown-prefix";
        return;
    }
        
        
    m_userLibdir = m_home + "/.local/lib";
    m_userBindir = m_home + "/.local/bin";
    m_userDatadir = m_home + "/.local/share";
    m_unknownPathDir = m_home + "/.local/share/opencpn/unknown-prefix";

    const string platform_dir = g_Platform->GetPluginDir().ToStdString();
    const char* const envdirs = getenv("OPENCPN_PLUGIN_DIRS");
    string dirlist = envdirs ? envdirs : OCPN_LINUX_LOAD_PATH;
    m_libdirs = split(dirlist, ':');
    for (auto& dir: m_libdirs) {
        dir += "/opencpn";
        dir = expand(dir);
    }
    if (envdirs == 0 && dirlist.find(platform_dir) == string::npos) {
        m_libdirs.push_back(expand(platform_dir));
    }
    m_bindirs = m_libdirs;
    for (auto& dir: m_bindirs) {
        // Fails on Debian multilib paths like /usr/lib/x86_64-linux-gnu.
        // But we don't use those even on Debian.
        size_t pos = dir.rfind("/lib/opencpn");
        if (pos == string::npos) {
            pos = dir.rfind("/lib64/opencpn");
        }
        dir = pos == string::npos ? dir : dir.substr(0, pos) + "/bin";
    }
    const char* const xdg_data_dirs = getenv("XDG_DATA_DIRS");
    dirlist = xdg_data_dirs ? xdg_data_dirs : LINUX_DATA_PATH;
    m_datadirs = split(dirlist, ':');
    for (auto& dir: m_datadirs) {
        dir += "/opencpn/plugins";
    }
    if (xdg_data_dirs == 0 && dirlist.find(platform_dir) == string::npos) {
        m_datadirs.push_back(platform_dir + "/plugins");
    }
}


void PluginPaths::initApplePaths()
{
    using namespace std;

    const string mac_home = m_home + "/Library/Application Support/OpenCPN";
    m_userLibdir = mac_home + "/Contents/PlugIns";
    m_userBindir = m_userLibdir;
    m_userDatadir = mac_home + "/Contents";
    m_unknownPathDir = mac_home + "/Contents/unknown-paths";

    m_libdirs.push_back(m_userLibdir);
    wxFileName fn_exe(GetOCPN_ExePath());
    fn_exe.RemoveLastDir();
    string exeLibDir = fn_exe.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR).ToStdString() + "Plugins"; 
    m_libdirs.push_back(exeLibDir);
    //m_libdirs.push_back("/Applications/OpenCPN.app/Contents/Plugins");
    m_bindirs = m_libdirs;

    m_datadirs.push_back(m_userDatadir);
    m_datadirs.push_back( "/Applications/OpenCPN.app/Contents/PlugIns");
}

void PluginPaths::initAndroidPaths()
{
    using namespace std;

    // Initial Android implementation cannot use new PluginManager scheme
    // So we default to standard R/O location as used by system plugins.

    //m_userLibdir = m_home + "/.local/lib";
    //m_userBindir = m_home + "/.local/bin";
    //m_userDatadir = m_home + "/.local/share";
    //m_unknownPathDir = m_home + "/.local/share/opencpn/unknown-prefix";

    const string platform_dir = g_Platform->GetPluginDir().ToStdString();
    m_libdirs.push_back(expand(platform_dir));
}


PluginPaths::PluginPaths()
{
    using namespace std;

    wxString wxHome("unusable-$HOME");
    wxGetEnv("HOME", &wxHome);
    m_home = wxHome.ToStdString();

    auto osSystemId = wxPlatformInfo::Get().GetOperatingSystemId();
    if (osSystemId & wxOS_WINDOWS) {
        initWindowsPaths();
    }
    else if (g_Platform->isFlatpacked()) {
        initFlatpackPaths();
    }
    else if (osSystemId & wxOS_UNIX_LINUX) {
        initLinuxPaths();
    }
    else if (osSystemId & wxOS_MAC) {
        initApplePaths();
    }
    else {
        wxString os_name = wxPlatformInfo::Get().GetPortIdName();
        wxLogMessage(_T("OS_NAME: ") + os_name);
        if(os_name.Contains(_T("wxQT"))){
            initAndroidPaths();
        }
        else
            wxLogWarning("PluginPaths: Unknown platform");
    }
}
