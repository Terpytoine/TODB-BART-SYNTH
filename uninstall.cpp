#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include "installer-support.hpp"
namespace fs=std::filesystem;
constexpr wchar_t registration[]=L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TODB BART SYNTH";
fs::path known(REFKNOWNFOLDERID id){PWSTR p=nullptr;if(FAILED(SHGetKnownFolderPath(id,0,nullptr,&p)))throw 1;fs::path value=p;CoTaskMemFree(p);return value;}
fs::path environment(const wchar_t* key){wchar_t value[32768];if(!GetEnvironmentVariableW(key,value,32768))throw 1;return value;}
int removeProduct(const fs::path& testRoot,bool quiet){
 const bool test=!testRoot.empty();
 fs::path local=test?fs::absolute(testRoot)/L"local":known(FOLDERID_LocalAppData);
 fs::path common=test?fs::absolute(testRoot)/L"common":environment(L"CommonProgramFiles");
 fs::path desktop=test?fs::absolute(testRoot)/L"desktop":known(FOLDERID_Desktop);
 fs::path programs=test?fs::absolute(testRoot)/L"programs":known(FOLDERID_Programs);
 install::Roots roots{local/L"Programs"/L"TODB",common/L"VST3"};
 auto userPluginParent=local/L"Programs"/L"Common"/L"VST3";
 std::vector<std::string> names;
 try{names=install::readManifest(roots.app()/install::manifestName);}catch(...){
  if(!quiet)MessageBoxW(nullptr,L"The installation file list is missing or damaged. Run the latest TODB installer once to repair the install, then uninstall again. No files were removed.",L"TODB BART SYNTH — repair needed",MB_OK|MB_ICONINFORMATION);
  return 8;
 }
 struct Owned{fs::path file,root;};std::vector<Owned>owned;
 for(auto&name:names){auto file=roots.file(name);owned.push_back({file,name.rfind("app/",0)==0?roots.appParent:roots.pluginParent});
  // Remove the same packaged files from an earlier per-user copy, preserving added user files.
  if(name.rfind("plugin/",0)==0){auto duplicate=userPluginParent/fs::u8path(name.substr(7));if(fs::exists(duplicate))owned.push_back({duplicate,userPluginParent});}
 }
 owned.push_back({desktop/L"TODB BART SYNTH.lnk",desktop});
 for(auto name:{L"TODB BART SYNTH.lnk",L"Uninstall TODB BART SYNTH.lnk"})owned.push_back({programs/L"TODB BART SYNTH"/name,programs});
 for(auto&item:owned){if(!install::safeTarget(item.file,item.root))return 2;if(!install::unlocked(item.file)){
  if(!quiet)MessageBoxW(nullptr,L"Close the TODB standalone and unload TODB BART SYNTH from FL Studio, then run Uninstall again. No files were removed.",L"TODB BART SYNTH — still open",MB_OK|MB_ICONINFORMATION);return 3;
 }}
 if(!test){HKEY key=nullptr;auto result=RegOpenKeyExW(HKEY_CURRENT_USER,registration,0,KEY_WRITE|KEY_WOW64_64KEY,&key);if(key)RegCloseKey(key);if(result!=ERROR_SUCCESS&&result!=ERROR_FILE_NOT_FOUND)return 4;}
 std::set<fs::path> directories;
 for(auto&item:owned){std::error_code error;fs::remove(item.file,error);if(error){
  if(!quiet)MessageBoxW(nullptr,L"Windows blocked removal. Close the instrument and retry. If the VST3 folder is protected, run the uninstaller as administrator.",L"TODB BART SYNTH",MB_OK|MB_ICONERROR);return 4;
 }for(auto folder=item.file.parent_path();folder!=item.root&&install::safeTarget(folder,item.root);folder=folder.parent_path())directories.insert(folder);}
 std::vector<fs::path> ordered(directories.begin(),directories.end());std::sort(ordered.begin(),ordered.end(),[](auto&a,auto&b){return a.native().size()>b.native().size();});
 // Only remove empty directories. User-created presets, files and folders are never in the manifest.
 for(auto&folder:ordered){std::error_code error;if(fs::is_directory(folder,error)&&fs::is_empty(folder,error))fs::remove(folder,error);}
 if(!test){auto result=RegDeleteTreeW(HKEY_CURRENT_USER,registration);if(result!=ERROR_SUCCESS&&result!=ERROR_FILE_NOT_FOUND)return 4;SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,nullptr,nullptr);}
 if(!quiet)MessageBoxW(nullptr,L"TODB BART SYNTH was removed. Your DAW projects, saved patches, original samples, exported audio and any files you added to its install folders were kept.",L"TODB BART SYNTH — removed",MB_OK|MB_ICONINFORMATION);
 return 0;
}

int WINAPI WinMain(HINSTANCE instance,HINSTANCE,LPSTR,int){
 int argc=0;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);bool quiet=false;for(int i=1;i<argc;i++)if(!wcscmp(argv[i],L"/silent"))quiet=true;
 try{
  if(argc>2&&!wcscmp(argv[1],L"/test")){auto root=fs::path(argv[2]);LocalFree(argv);return removeProduct(root,true);}
  if(argc>2&&!wcscmp(argv[1],L"/remove")){DWORD parent=wcstoul(argv[2],nullptr,10);auto process=OpenProcess(SYNCHRONIZE,FALSE,parent);if(process){WaitForSingleObject(process,10000);CloseHandle(process);}LocalFree(argv);return removeProduct({},quiet);}
  if(!quiet&&MessageBoxW(nullptr,L"Remove TODB BART SYNTH (VST3 instrument)? Saved patches, projects and original samples will be kept.",L"Uninstall TODB BART SYNTH",MB_YESNO|MB_ICONQUESTION)!=IDYES){LocalFree(argv);return 0;}
  wchar_t self[32768],temp[32768];GetModuleFileNameW(instance,self,32768);GetTempPathW(32768,temp);
  auto helper=fs::path(temp)/(L"TODB-Uninstall-"+std::to_wstring(GetCurrentProcessId())+L".exe");
  if(!CopyFileW(self,helper.c_str(),FALSE)){LocalFree(argv);return 5;}
  std::wstring command=L"\""+helper.wstring()+L"\" /remove "+std::to_wstring(GetCurrentProcessId())+(quiet?L" /silent":L"");
  STARTUPINFOW startup{};startup.cb=sizeof(startup);startup.dwFlags=STARTF_USESHOWWINDOW;startup.wShowWindow=SW_HIDE;PROCESS_INFORMATION info{};
  if(!CreateProcessW(helper.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&info)){LocalFree(argv);return 6;}
  CloseHandle(info.hThread);CloseHandle(info.hProcess);LocalFree(argv);return 0;
 }catch(...){LocalFree(argv);if(!quiet)MessageBoxW(nullptr,L"Uninstall could not finish. No other application folders are removed by this uninstaller.",L"TODB BART SYNTH",MB_OK|MB_ICONERROR);return 7;}
}
