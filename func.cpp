#include <tchar.h>

#include <windows.h>

#include <stdio.h>

#include <iostream>

#include <string>

#include <process.h>

#include <cstdio>

#include "resource.h"

#include <ios>

#include <fstream>

#include "funcs.h"

#include <cstring>

#include <stdexcept>

using namespace std;

// global vars

char *basePath = NULL;

bool exist(char *filename)
{
  return GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES;
}

void setBasePath(char *path)
{
  basePath = path;
}

BOOL closeWindowByTitle(char *title)
{

  auto hwnd = FindWindow(nullptr, title);

  if (hwnd != nullptr)
  {
    PostMessage(hwnd, WM_CLOSE, 0, 0);
    // MessageBoxA(NULL, "success", title, MB_OK | MB_ICONERROR);
    return true;
  }
  else
  {
    // MessageBoxA(NULL, "failed", title, MB_OK | MB_ICONERROR);
    return false;
  }
}

uint32_t fnv1a_hash(const char *data, size_t size)
{
  uint32_t hash = 2166136261u; // FNV offset basis
  for (size_t i = 0; i < size; i++)
  {
    hash ^= data[i];
    hash *= 16777619u; // FNV prime
  }
  return hash;
}

std::string hashString(std::string original, std::string prefix)
{
  std::string result = prefix;
  uint32_t hash = fnv1a_hash(original.c_str(), original.size());
  string substr = std::to_string(hash);
  result.append(substr);
  return result;
}

std::string exec(const char *cmd)
{
  char buffer[128];
  std::string result = "";
  FILE *pipe = popen(cmd, "r");
  if (!pipe)
    throw std::runtime_error("popen() failed!");
  try
  {
    while (fgets(buffer, sizeof buffer, pipe) != NULL)
    {
      result += buffer;
    }
  }
  catch (...)
  {
    pclose(pipe);
    throw;
  }
  pclose(pipe);
  return result;
}

void msgbox(char *message, char *title)
{
  MessageBoxA(NULL, message, title, MB_OK | MB_ICONERROR);
}

void msgbox(std::string message, char *title)
{
  MessageBoxA(NULL, message.c_str(), title, MB_OK | MB_ICONERROR);
}

void msgbox(int message, char *title)
{
  string msg = std::to_string(message);
  MessageBoxA(NULL, msg.c_str(), title, MB_OK | MB_ICONERROR);
}

void CreateFolder(const char *path)
{
  if (!CreateDirectory(path, NULL))
  {
    return;
  }
}

void makeTempFolder()
{
  // create temp folder
  TCHAR szTempFolder[MAX_PATH];
  sprintf(szTempFolder, "%s%s", basePath, "tmp");
  CreateFolder(szTempFolder);
}

void LoadSplashImage(HWND hWnd)
{
  RECT rect;
  HDC hdc = GetDC(hWnd);

  // TCHAR splashImage[ ] = "assets\\splashscreen.bmp";

  HBRUSH brush = CreatePatternBrush((HBITMAP)LoadImage(NULL, _T("assets\\splashscreen.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
  GetWindowRect(hWnd, &rect);

  rect.left = 0;
  rect.top = 0;
  // rect.right = 540;
  // rect.bottom = 356;

  std::cout << "Left: " << rect.left << "\n";
  std::cout << "Right: " << rect.right << "\n";
  std::cout << "Top: " << rect.top << "\n";
  std::cout << "Bottom: " << rect.bottom << "\n";

  FillRect(hdc, &rect, brush);
  DeleteObject(brush);
  ReleaseDC(hWnd, hdc);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) /* handle the messages */
  {
  case WM_DESTROY:
    PostQuitMessage(0); /* send a WM_QUIT to the message queue */
    break;
 case WM_KILLFOCUS:
      PostQuitMessage (0);
      break;
  default:
    /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  return 0;
}

/*  This function is called by the Windows function DispatchMessage()  */

WNDCLASSEX wincl; /* Data structure for the windowclass */

/*  Make the class name into a global variable  */
TCHAR szClassName[] = _T("CodeBlocksWindowsApp");

BOOL CreateApplicationWindow(HINSTANCE hThisInstance,
                             HINSTANCE hPrevInstance,
                             LPSTR lpszArgument,
                             int nCmdShow, HWND &hwnd)
{

  /* The Window structure */
  wincl.hInstance = hThisInstance;
  wincl.lpszClassName = szClassName;
  wincl.lpfnWndProc = WindowProcedure; /* This function is called by windows */
  wincl.style = CS_DBLCLKS;            /* Catch double-clicks */
  wincl.cbSize = sizeof(WNDCLASSEX);

  /* Use default icon and mouse-pointer */
  wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
  wincl.lpszMenuName = NULL; /* No menu */
  wincl.cbClsExtra = 0;      /* No extra bytes after the window class */
  wincl.cbWndExtra = 0;      /* structure or the window instance */
  /* Use Windows's default colour as the background of the window */
  wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

  /* Register the window class, and if it fails quit the program */
  if (!RegisterClassEx(&wincl))
    return FALSE;

  int width = 640;
  int height = 456;

  /* The class is registered, let's create the program*/
  hwnd = CreateWindowEx(
      WS_EX_TOOLWINDOW,                              /* Extended possibilites for variation */
      szClassName,                                   /* Classname */
      NULL,                                          /* Title Text */
      WS_CLIPSIBLINGS | WS_EX_TOOLWINDOW | WS_POPUP, /* default window */
      (GetSystemMetrics(SM_CXSCREEN) - width) / 2,
      (GetSystemMetrics(SM_CYSCREEN) - height) / 2,
      width,         /* The programs width */
      height,        /* and height in pixels */
      HWND_DESKTOP,  /* The window is a child-window to desktop */
      NULL,          /* No menu */
      hThisInstance, /* Program Instance handler */
      NULL           /* No Window Creation data */
  );

  DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE); // Get the window's extended style
  SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle);        // Set the window's extended style

  // you can load splash

  return TRUE;
}

void getEXEPath(char *&exePath,
                const char *szFileName)
{
  // Get the last position of '/'
  std::string aux(szFileName);

// get '/' or '\\' depending on unix/mac or windows.
#if defined(_WIN32) || defined(WIN32)
  int pos = aux.rfind('\\');
#else
  int pos = aux.rfind('/');
#endif

  // Get the path and the name
  std::string path = aux.substr(0, pos + 1);

  char *path_array = new char[path.length() + 1];
  strcpy(path_array, path.c_str());

  exePath = path_array;
}

BOOL execCommand(char *cmd, DWORD dwFlag, bool wait)
{
  DWORD size = 0;
  STARTUPINFO info = {
      sizeof(info)};
  PROCESS_INFORMATION processInfo;

  if (CreateProcess(NULL, cmd, NULL, NULL, TRUE, dwFlag, NULL, NULL, &info, &processInfo))
  {
    if (wait)
    {
      WaitForSingleObject(processInfo.hProcess, INFINITE);
      CloseHandle(processInfo.hProcess);
      CloseHandle(processInfo.hThread);
    }
    return true;
  }
  return false;
}

BOOL execCommand2(char *cmd, DWORD dwFlag, bool wait)
{
  DWORD size = 0;
  STARTUPINFO info = {
      sizeof(info)};
  PROCESS_INFORMATION processInfo;

  if (CreateProcess(NULL, cmd, NULL, NULL, TRUE, dwFlag, NULL, NULL, &info, &processInfo))
  {
    if (wait)
    {
      //5 minutes wait max
      WaitForSingleObject(processInfo.hProcess, 60 * 5 * 1000);
      CloseHandle(processInfo.hProcess);
      CloseHandle(processInfo.hThread);
    }
    return true;
  }
  return false;
}



BOOL startWebkitEngine()
{
  DWORD size = 0;
  STARTUPINFO info = {
      sizeof(info)};

  if (CreateProcess(NULL, szbbWebKit, NULL, NULL, TRUE, 0, NULL, NULL, &info, &webkitProcessInfo))
  {
    return true;
  }

  MessageBoxA(NULL, "PHPBBWebkit could not be started for unknown reasons.", "Error", MB_OK | MB_ICONERROR);

  return false;
}

void waitForWebkitToExit() {
  WaitForSingleObject(webkitProcessInfo.hProcess, INFINITE);
  CloseHandle(webkitProcessInfo.hProcess);
  CloseHandle(webkitProcessInfo.hThread);
}



// registry check
BOOL DoesVCRedistNeedUpdate()
{
  BOOL requireUpdate = true;

  CHAR message[MAX_PATH];
  CHAR requiredVal[MAX_PATH] = "14.34.31938";
  CHAR currentVal[MAX_PATH];

  DWORD dataSize = MAXWORD;

  // Computer\HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\X64
  // Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Dependencies\Microsoft.VS.VC_RuntimeAdditionalVSU_amd64,v14
  // norm - v14.34.31938.00
  LONG result = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\Installer\\Dependencies\\Microsoft.VS.VC_RuntimeAdditionalVSU_amd64,v14", "Version", RRF_RT_REG_SZ, nullptr, &currentVal, &dataSize);
  if (result != ERROR_SUCCESS)
  {
    requireUpdate = true;
    strcpy(message, "No VCRedist Found");
  }
  else
  {
    // compare version
    std::string str_inp1(requiredVal);
    std::string str_inp2(currentVal);

    int res = str_inp1.compare(str_inp2);

    if (res == 0)
    {
      strcpy(message, "Exact match Found");
      requireUpdate = false;
    }
    else if (res < 0)
    {
      strcpy(message, "More up to date than required");
      requireUpdate = false;
    }
    else
    {
      strcpy(message, "Not up to date at all");
      requireUpdate = true;
    }
  }
  return requireUpdate;
}

void UpdateVCRedist(TCHAR path[])
{
  STARTUPINFO info = {
      sizeof(info)};
  PROCESS_INFORMATION processInfo;

  // char cmdArgs[] = "VC_redist.x64.exe /Q /norestart";
  char cmdArgs[] = "VC_redist.x64.exe /passive /norestart";
  //char cmdArgs[] = "VC_redist.x64.exe /norestart";
  char cmdArgs2[] = "VC_redist.x64.exe /repair /passive /norestart";

  //MessageBoxA(NULL, szVcRedistrMessage, szVcRedistrTitle, MB_OK);

  if (CreateProcess(path, cmdArgs, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
  {
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    // MessageBoxA(NULL, path, "Update Complete", MB_OK | MB_ICONERROR);
  }
  else
  {
    // MessageBoxA(NULL, path, "Update Failed", MB_OK | MB_ICONERROR);
  }
}