#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <dwmapi.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include "installer-payload.hpp"
HINSTANCE instanceHandle;HWND statusLabel;bool uiTest=false;std::filesystem::path selfPath;
HBRUSH setupBackground=CreateSolidBrush(RGB(29,12,42));HFONT setupFont=nullptr;
bool desktopShortcut(){HRESULT init=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);PWSTR desktop=nullptr,programs=nullptr;std::error_code error;if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop,0,nullptr,&desktop))){std::filesystem::remove(std::filesystem::path(desktop)/L"TODB BART SYNTH.lnk",error);CoTaskMemFree(desktop);}if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Programs,0,nullptr,&programs))){auto folder=std::filesystem::path(programs)/L"TODB BART SYNTH";std::filesystem::remove(folder/L"TODB BART SYNTH.lnk",error);std::filesystem::remove(folder/L"Uninstall TODB BART SYNTH.lnk",error);std::filesystem::remove(folder,error);CoTaskMemFree(programs);}if(SUCCEEDED(init))CoUninitialize();return true;}

bool registerUninstall(){
 wchar_t local[32768];if(!GetEnvironmentVariableW(L"LOCALAPPDATA",local,32768))return false;
 auto app=std::filesystem::path(local)/L"Programs"/L"TODB"/L"BART SYNTH";
 if(!std::filesystem::exists(app/L"Uninstall TODB BART SYNTH.exe"))return false;
 HKEY key=nullptr;if(RegCreateKeyExW(HKEY_CURRENT_USER,install::registryKey,0,nullptr,0,KEY_SET_VALUE|KEY_WOW64_64KEY,nullptr,&key,nullptr)!=ERROR_SUCCESS)return false;
 auto text=[&](const wchar_t*name,const std::wstring&value){return RegSetValueExW(key,name,0,REG_SZ,(const BYTE*)value.c_str(),DWORD((value.size()+1)*sizeof(wchar_t)))==ERROR_SUCCESS;};
 auto number=[&](const wchar_t*name,DWORD value){return RegSetValueExW(key,name,0,REG_DWORD,(const BYTE*)&value,sizeof(value))==ERROR_SUCCESS;};
 std::wstring command=L"\""+(app/L"Uninstall TODB BART SYNTH.exe").wstring()+L"\"";
 bool ok=text(L"DisplayName",L"TODB BART SYNTH")&&text(L"DisplayVersion",L"1.3.1")&&text(L"Publisher",L"TODB / Twon On Da Beat")&&text(L"InstallLocation",app.wstring())&&text(L"UninstallString",command)&&text(L"QuietUninstallString",command+L" /silent")&&text(L"DisplayIcon",L"\""+(app/L"Uninstall TODB BART SYNTH.exe").wstring()+L"\",0")&&text(L"Comments",L"Bay Area Rhythm & Tone — native VST3 instrument")&&number(L"NoModify",1)&&number(L"NoRepair",1)&&number(L"VersionMajor",1)&&number(L"VersionMinor",3);
 // This is a visible EXE install, not an MSI product or hidden system component.
 for(auto value:{L"WindowsInstaller",L"SystemComponent",L"ParentKeyName",L"ParentDisplayName"})RegDeleteValueW(key,value);
 uint64_t bytes=0;wchar_t common[32768];GetEnvironmentVariableW(L"CommonProgramFiles",common,32768);
 for(auto folder:{app,std::filesystem::path(common)/L"VST3"/L"TODB BART SYNTH.vst3"}){std::error_code ec;if(std::filesystem::exists(folder,ec))for(auto&entry:std::filesystem::recursive_directory_iterator(folder,std::filesystem::directory_options::skip_permission_denied,ec))if(entry.is_regular_file(ec))bytes+=entry.file_size(ec);}
 ok=number(L"EstimatedSize",DWORD(std::min<uint64_t>((bytes+1023)/1024,0xffffffff)))&&ok;
 SYSTEMTIME date;GetLocalTime(&date);wchar_t day[16];swprintf(day,16,L"%04u%02u%02u",date.wYear,date.wMonth,date.wDay);ok=text(L"InstallDate",day)&&ok;RegCloseKey(key);
 DWORD_PTR response=0;SendMessageTimeoutW(HWND_BROADCAST,WM_SETTINGCHANGE,0,(LPARAM)L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",SMTO_ABORTIFHUNG,500,&response);SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,nullptr,nullptr);return ok;
}

std::wstring installError;
bool payload(std::filesystem::path testRoot={},bool verify=false){
 try{
  auto items=install::readPayload(selfPath);if(verify)return true;
  install::Roots roots;if(!testRoot.empty())roots={std::filesystem::absolute(testRoot)/L"app",std::filesystem::absolute(testRoot)/L"plugin"};
  else{wchar_t common[32768],local[32768];if(!GetEnvironmentVariableW(L"CommonProgramFiles",common,32768)||!GetEnvironmentVariableW(L"LOCALAPPDATA",local,32768))return false;roots={std::filesystem::path(local)/L"Programs"/L"TODB",std::filesystem::path(common)/L"VST3"};}
  std::set<std::string> current;for(auto&i:items)current.insert(i.name);std::vector<std::filesystem::path> obsolete;auto manifest=roots.app()/install::manifestName;if(std::filesystem::exists(manifest))for(auto&name:install::readManifest(manifest))if(name.rfind("app/",0)==0&&!current.count(name)&&name!="app/BART SYNTH/.todb-install-files"){auto file=roots.file(name);if(!install::unlocked(file))throw std::runtime_error("Close BART SYNTH before removing the standalone");obsolete.push_back(file);}install::installFiles(roots,std::move(items));for(auto&file:obsolete){std::error_code ec;std::filesystem::remove(file,ec);if(ec)throw std::runtime_error("Could not remove an old standalone file");}if(!testRoot.empty())return true;
  if(!registerUninstall()){installError=L"Files installed, but Windows app registration could not finish. Run setup again to repair it.";return false;}
  if(!desktopShortcut()){installError=L"Installed and registered. Windows could not create the desktop shortcut; open BART SYNTH from its installed folder.";return false;}
  return true;
 }catch(const std::exception&error){auto detail=std::string(error.what());installError.assign(detail.begin(),detail.end());return false;}
}

LRESULT CALLBACK Proc(HWND w,UINT m,WPARAM wp,LPARAM lp){if(m==WM_CTLCOLORSTATIC){SetTextColor((HDC)wp,RGB(244,211,139));SetBkColor((HDC)wp,RGB(29,12,42));return(LRESULT)setupBackground;}if(m==WM_DRAWITEM){auto*d=(DRAWITEMSTRUCT*)lp;HBRUSH brush=CreateSolidBrush(IsWindowEnabled(d->hwndItem)?RGB(69,32,85):RGB(40,26,48));FillRect(d->hDC,&d->rcItem,brush);DeleteObject(brush);HPEN pen=CreatePen(PS_SOLID,1,RGB(175,135,67));auto oldPen=SelectObject(d->hDC,pen),oldBrush=SelectObject(d->hDC,GetStockObject(NULL_BRUSH));RoundRect(d->hDC,d->rcItem.left,d->rcItem.top,d->rcItem.right,d->rcItem.bottom,8,8);SelectObject(d->hDC,oldPen);SelectObject(d->hDC,oldBrush);DeleteObject(pen);wchar_t text[128];GetWindowTextW(d->hwndItem,text,128);SetBkMode(d->hDC,TRANSPARENT);SetTextColor(d->hDC,IsWindowEnabled(d->hwndItem)?RGB(248,218,146):RGB(141,128,148));SelectObject(d->hDC,setupFont);DrawTextW(d->hDC,text,-1,&d->rcItem,DT_CENTER|DT_VCENTER|DT_SINGLELINE);return TRUE;}if((m==WM_COMMAND&&LOWORD(wp)==1)||m==WM_APP+1){EnableWindow(GetDlgItem(w,1),FALSE);SetWindowTextW(statusLabel,L"Installing BART SYNTH…");try{bool ok=payload();SetWindowTextW(statusLabel,ok?L"VST3 installed. Uninstall through Windows Apps.\nFL Studio > Manage plugins > Find installed plugins.\nLoad the new TODB BART SYNTH entry.":L"Could not install. Unload the VST3 from FL Studio and retry. If Windows blocks the VST3 folder, run setup as administrator.");if(!ok){EnableWindow(GetDlgItem(w,1),TRUE);if(!installError.empty())SetWindowTextW(statusLabel,installError.c_str());}if(ok){SetWindowTextW(GetDlgItem(w,1),L"Installed ✓");wchar_t local[32768];GetEnvironmentVariableW(L"LOCALAPPDATA",local,32768);auto path=std::filesystem::path(local)/L"Programs"/L"TODB"/L"BART SYNTH"/L"BART SYNTH.exe";SetWindowLongPtrW(w,GWLP_USERDATA,1);if(uiTest)PostQuitMessage(0);}else if(uiTest)PostQuitMessage(5);}catch(...){SetWindowTextW(statusLabel,L"Installation could not finish. Close BART SYNTH and retry.");EnableWindow(GetDlgItem(w,1),TRUE);if(uiTest)PostQuitMessage(5);}}return DefWindowProcW(w,m,wp,lp);}
int WINAPI WinMain(HINSTANCE inst,HINSTANCE,LPSTR,int){instanceHandle=inst;wchar_t path[32768];GetModuleFileNameW(inst,path,32768);selfPath=path;int argc;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(argc>1&&!wcscmp(argv[1],L"/repairregistration")){bool ok=desktopShortcut()&&registerUninstall();LocalFree(argv);return ok?0:6;}if(argc>1&&!wcscmp(argv[1],L"/verify")){bool ok=payload({},true);LocalFree(argv);return ok?0:2;}if(argc>2&&!wcscmp(argv[1],L"/extracttest")){bool ok=payload(argv[2]);LocalFree(argv);return ok?0:3;}if(argc>1&&!wcscmp(argv[1],L"/install")){bool ok=payload();if(!ok){std::ofstream report(selfPath.parent_path()/L"Install diagnostic.txt");report<<std::string(installError.begin(),installError.end());}LocalFree(argv);return ok?0:4;}uiTest=argc>1&&!wcscmp(argv[1],L"/uitest");LocalFree(argv);HANDLE singleton=CreateMutexW(nullptr,FALSE,L"TODB_BART_SETUP_V7");if(GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(singleton);return 0;}WNDCLASSW wc={};wc.lpfnWndProc=Proc;wc.hInstance=inst;wc.hIcon=LoadIconW(inst,MAKEINTRESOURCEW(1));wc.lpszClassName=L"TODB_BART_Setup";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=setupBackground;RegisterClassW(&wc);HWND w=CreateWindowW(wc.lpszClassName,L"TODB BART SYNTH — Studio Edition 1.3.1 · Setup",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,CW_USEDEFAULT,CW_USEDEFAULT,620,285,nullptr,nullptr,inst,nullptr);CreateWindowW(L"STATIC",L"TODB / BART SYNTH — Bay Area Rhythm & Tone\n350 Bay Area sounds · Native 64-bit VST3 instrument",WS_CHILD|WS_VISIBLE,25,20,555,60,w,nullptr,inst,nullptr);statusLabel=CreateWindowW(L"STATIC",L"Installs for this Windows account. Unload the instrument from FL Studio first.\nVST3: Program Files > Common Files > VST3\nNo standalone app or desktop shortcut is installed.",WS_CHILD|WS_VISIBLE,25,87,555,85,w,nullptr,inst,nullptr);CreateWindowW(L"BUTTON",L"Install BART SYNTH",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,25,187,210,35,w,(HMENU)1,inst,nullptr);CreateWindowW(L"BUTTON",L"VST3 installed",WS_CHILD|WS_VISIBLE|WS_DISABLED|BS_OWNERDRAW,260,187,180,35,w,(HMENU)2,inst,nullptr);setupFont=CreateFontW(-15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");for(HWND child=GetWindow(w,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT))SendMessageW(child,WM_SETFONT,(WPARAM)setupFont,TRUE);BOOL dark=TRUE;DwmSetWindowAttribute(w,20,&dark,sizeof(dark));COLORREF caption=RGB(29,12,42);DwmSetWindowAttribute(w,35,&caption,sizeof(caption));ShowWindow(w,uiTest?SW_HIDE:SW_SHOW);PostMessageW(w,WM_APP+1,0,0);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}bool installed=GetWindowLongPtrW(w,GWLP_USERDATA)==1;DestroyWindow(w);return uiTest&&!installed?5:0;}




