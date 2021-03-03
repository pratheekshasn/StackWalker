/**********************************************************************
 *
 * main.cpp
 *
 *
 * History:
 *  2008-11-27   v1    - Header added
 *                       Samples for Exception-Crashes added...
 *  2009-11-01   v2    - Moved to stackwalker.codeplex.com
 *
 **********************************************************************/

#include "CallTreeNode.h"
#include "GraphCreator.h"
#include "Profiler.h"
#include "stackwalker.h"
#include <DbgHelp.h>
#include <chrono>
#include <iostream>
#include <map>
#include <stdio.h>
#include <string>
#include <tchar.h>
#include <thread>
#include <tlhelp32.h>
#include <windows.h>

#define UNHANDLED_EXCEPTION_TEST
#define EXCEPTION_FILTER_TEST

#define THREADCOUNT 5
HANDLE ghThreads[THREADCOUNT];
HANDLE profThread;

typedef std::vector<std::vector<std::string>> CallStackList;
typedef std::map<std::string, CallStackList>  VICallStackMap;

VICallStackMap             gCallTrees; // <threadName, callStackList>
std::map<std::string, int> gFunctionCounts;

//  Forward declarations:
HANDLE ListProcessThreads(DWORD dwOwnerPID);
void   printError(TCHAR* msg);
HANDLE GetMainThreadId(DWORD PID);
void   Func1();
void   StackWalkTest(HANDLE threadHandle, std::string threadName);

// secure-CRT_functions are only available starting with VC8
#if _MSC_VER < 1400
#define _tcscpy_s _tcscpy
#define _tcscat_s _tcscat
#define _stprintf_s _stprintf
#define strcpy_s(a, b, c) strcpy(a, c)
#endif

int iGlobalInt = 0;
void (*pGlobalFuncPtr)() = 0;

// Specialized stackwalker-output classes
// Console (printf):
class StackWalkerToConsole : public StackWalker
{
protected:
  virtual void OnOutput(LPCSTR szText) { printf("%s", szText); }
};

// Specialized stackwalker-output classes
// Console (printf):
class StackWalkerToConsole_DifferentProcess : public StackWalker
{
public:
  StackWalkerToConsole_DifferentProcess(DWORD dwProcessId, HANDLE hProcess)
      : StackWalker(dwProcessId, hProcess)
  {
  }
  StackWalkerToConsole_DifferentProcess() //DWORD dwProcessId, HANDLE hProcess)
      : StackWalker()                     //dwProcessId, hProcess)
  {
  }

protected:
  virtual void OnOutput(LPCSTR szText) { printf("%s", szText); }
};

DWORD WINAPI ThreadProc(LPVOID);
DWORD WINAPI StartProfile(LPVOID);

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
  UNREFERENCED_PARAMETER(lpParam);

  Func1();
  return 0;
}

DWORD WINAPI StartProfile(LPVOID lpParam)
{
  UNREFERENCED_PARAMETER(lpParam);

  HANDLE thread = GetCurrentThread();

  for (size_t j = 0; j < 10; j++)
  {
    for (int i = 0; i < THREADCOUNT; i++)
    {
      /*char* intStr = (char*)malloc(2);
      intStr = itoa(i, intStr, 0);*/
      //sprintf(threadName, "%d", i);
      StackWalkTest(ghThreads[i], std::to_string(i));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return 0;
}

void CreateMultipleThreads()
{
  DWORD dwThreadID;

  for (int i = 0; i < THREADCOUNT; i++)
  {
    // TODO: More complex scenarios may require use of a parameter
    //   to the thread procedure, such as an event per thread to
    //   be used for synchronization.
    ghThreads[i] = CreateThread(NULL,       // default security
                                0,          // default stack size
                                ThreadProc, // name of the thread function
                                NULL,       // no thread parameters
                                0,          // default startup flags
                                &dwThreadID);

    if (ghThreads[i] == NULL)
    {
      printf("CreateThread failed (%d)\n", GetLastError());
      return;
    }
  }
}

void CreateProfilerThread()
{
  DWORD dwThreadID;

  // TODO: More complex scenarios may require use of a parameter
  //   to the thread procedure, such as an event per thread to
  //   be used for synchronization.
  profThread = CreateThread(NULL,         // default security
                            0,            // default stack size
                            StartProfile, // name of the thread function
                            NULL,         // no thread parameters
                            0,            // default startup flags
                            &dwThreadID);

  if (profThread == NULL)
  {
    printf("CreateThread failed (%d)\n", GetLastError());
    return;
  }
}

void Func5(HANDLE threadHandle, std::string threadName)
{
  StackWalkerToConsole sw;
  SuspendThread(threadHandle);
  CONTEXT c;
  memset(&c, 0, sizeof(CONTEXT));
  c.ContextFlags = CONTEXT_FULL;

  // TODO: Detect if you want to get a thread context of a different process, which is running a different processor architecture...
  // This does only work if we are x64 and the target process is x64 or x86;
  // It cannot work, if this process is x64 and the target process is x64... this is not supported...
  // See also: http://www.howzatt.demon.co.uk/articles/DebuggingInWin64.html
  if (GetThreadContext(threadHandle, &c) != FALSE)
  {
    //sw.gLogFile << "\"" << threadName << "\":[";
    sw.ShowCallstack(threadHandle, &c);
    //sw.gLogFile << "]";
    // Build tree. Will have to make this thread-safe.
    // Maintain a set - sorted list for arranging based on sample count.
    // GetCurrentEC - for executing thread - from this, get DSP and VI name -- Make this root of this sub-tree.
    // Merge this sub-tree with the main tree.
    // Secondary storage for the info.

    if (gCallTrees.find(threadName) != gCallTrees.end())
    {
      auto value = gCallTrees[threadName];
      value.push_back(sw.callStackList);
      gCallTrees.erase(threadName);
      gCallTrees.insert({threadName, value});

      int currentCount = gFunctionCounts[threadName];
      gFunctionCounts.erase(threadName);
      gFunctionCounts.insert({threadName, currentCount + 1});
    }
    else
    {
      std::vector<std::vector<std::string>> newValue;
      newValue.push_back(sw.callStackList);
      gCallTrees.insert({threadName, newValue});

      gFunctionCounts.insert({threadName, 1});
    }
  }

  ResumeThread(threadHandle);
}

HANDLE GetMainThreadId(DWORD PID)
{
  //const std::tr1::shared_ptr<void> hThreadSnapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0),
  //                                                 CloseHandle);
  //if (hThreadSnapshot.get() == INVALID_HANDLE_VALUE)
  //{
  //  throw std::runtime_error("GetMainThreadId failed");
  //}
  //THREADENTRY32 tEntry;
  //tEntry.dwSize = sizeof(THREADENTRY32);
  //DWORD threadID = 0;
  ////PID = GetCurrentProcessId();
  //for (BOOL success = Thread32First(hThreadSnapshot.get(), &tEntry);
  //     !threadID && success && GetLastError() != ERROR_NO_MORE_FILES;
  //     success = Thread32Next(hThreadSnapshot.get(), &tEntry))
  //{
  //  if (tEntry.th32OwnerProcessID == PID)
  //  {
  //    threadID = tEntry.th32ThreadID;
  //  }
  //}
  return OpenThread(THREAD_ALL_ACCESS, FALSE, 21768);
}

int WaitForAllThreads()
{
  // The handle for each thread is signaled when the thread is
  // terminated.
  DWORD dwWaitResult = WaitForMultipleObjects(THREADCOUNT, // number of handles in array
                                              ghThreads,   // array of thread handles
                                              TRUE,        // wait until all are signaled
                                              INFINITE);

  switch (dwWaitResult)
  {
      // All thread objects were signaled
    case WAIT_OBJECT_0:
      printf("All threads ended, cleaning up for application exit...\n");
      printf("Returned %d\n", dwWaitResult);
      break;

      // An error occurred
    default:
      printf("WaitForMultipleObjects failed (%d)\n", GetLastError());
      return 1;
  }
  return 0;
}

int WaitForProfilerThread()
{
  // The handle for each thread is signaled when the thread is
  // terminated.
  DWORD dwWaitResult = WaitForMultipleObjects(1,           // number of handles in array
                                              &profThread, // array of thread handles
                                              TRUE,        // wait until all are signaled
                                              INFINITE);

  switch (dwWaitResult)
  {
      // All thread objects were signaled
    case WAIT_OBJECT_0:
      printf("All threads ended, cleaning up for application exit...\n");
      printf("Returned %d\n", dwWaitResult);
      break;

      // An error occurred
    default:
      printf("WaitForMultipleObjects failed (%d)\n", GetLastError());
      return 1;
  }
  return 0;
}

HANDLE ListProcessThreads(DWORD dwOwnerPID)
{
  HANDLE        hThreadSnap = INVALID_HANDLE_VALUE;
  THREADENTRY32 te32;

  // Take a snapshot of all running threads
  hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hThreadSnap == INVALID_HANDLE_VALUE)
    return (INVALID_HANDLE_VALUE);

  // Fill in the size of the structure before using it.
  te32.dwSize = sizeof(THREADENTRY32);

  // Retrieve information about the first thread,
  // and exit if unsuccessful
  if (!Thread32First(hThreadSnap, &te32))
  {
    printError(TEXT("Thread32First")); // Show cause of failure
    CloseHandle(hThreadSnap);          // Must clean up the snapshot object!
    return (INVALID_HANDLE_VALUE);
  }

  // Now walk the thread list of the system,
  // and display information about each thread
  // associated with the specified process
  do
  {
    if (te32.th32OwnerProcessID == dwOwnerPID)
    {
      _tprintf(TEXT("\n     THREAD ID      = 0x%08X"), te32.th32ThreadID);
      _tprintf(TEXT("\n     base priority  = %d"), te32.tpBasePri);
      _tprintf(TEXT("\n     delta priority = %d"), te32.tpDeltaPri);
    }
  } while (Thread32Next(hThreadSnap, &te32));

  _tprintf(TEXT("\n"));

  //  Don't forget to clean up the snapshot object.
  //CloseHandle(hThreadSnap);
  return (hThreadSnap);
}

void printError(TCHAR* msg)
{
  DWORD  eNum;
  TCHAR  sysMsg[256];
  TCHAR* p;

  eNum = GetLastError();
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, eNum,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                sysMsg, 256, NULL);

  // Trim the end of the line and terminate it with a null
  p = sysMsg;
  while ((*p > 31) || (*p == 9))
    ++p;
  do
  {
    *p-- = 0;
  } while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

  // Display the message
  _tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}

void Func6()
{
  std::cout << "Blah blah" << std::endl;
}

void Func4()
{
  Func6();
}

void Func3()
{
  for (size_t i = 0; i < 200; i++)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << i;
  }
  Func4();
}

void Func2()
{
  for (size_t i = 0; i < 200; i++)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    Func3();
  }
  //Func5();
}

void Func1()
{
  for (size_t i = 0; i < 200; i++)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    Func2();
  }
  //Func5();
}

void StackWalkTest(HANDLE threadHandle, std::string threadName)
{
  Func5(threadHandle, threadName);
}

void GlobalIntTest()
{
  StackWalkerToConsole sw;
  sw.ShowObject(&iGlobalInt);
}

void GlobalFunctionPointerTest()
{
  StackWalkerToConsole sw;
  sw.ShowObject(&pGlobalFuncPtr);
}

#ifdef UNHANDLED_EXCEPTION_TEST

// For more info about "PreventSetUnhandledExceptionFilter" see:
// "SetUnhandledExceptionFilter" and VC8
// http://blog.kalmbachnet.de/?postid=75
// and
// Unhandled exceptions in VC8 and above… for x86 and x64
// http://blog.kalmbach-software.de/2008/04/02/unhandled-exceptions-in-vc8-and-above-for-x86-and-x64/
// Even better: http://blog.kalmbach-software.de/2013/05/23/improvedpreventsetunhandledexceptionfilter/

#if defined(_M_X64) || defined(_M_IX86)
static BOOL PreventSetUnhandledExceptionFilter()
{
  HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
  if (hKernel32 == NULL)
    return FALSE;
  void* pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
  if (pOrgEntry == NULL)
    return FALSE;

#ifdef _M_IX86
  // Code for x86:
  // 33 C0                xor         eax,eax
  // C2 04 00             ret         4
  unsigned char szExecute[] = {0x33, 0xC0, 0xC2, 0x04, 0x00};
#elif _M_X64
  // 33 C0                xor         eax,eax
  // C3                   ret
  unsigned char szExecute[] = {0x33, 0xC0, 0xC3};
#else
#error "The following code only works for x86 and x64!"
#endif

  DWORD dwOldProtect = 0;
  BOOL  bProt = VirtualProtect(pOrgEntry, sizeof(szExecute), PAGE_EXECUTE_READWRITE, &dwOldProtect);

  SIZE_T bytesWritten = 0;
  BOOL   bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, szExecute, sizeof(szExecute),
                                 &bytesWritten);

  if ((bProt != FALSE) && (dwOldProtect != PAGE_EXECUTE_READWRITE))
  {
    DWORD dwBuf;
    VirtualProtect(pOrgEntry, sizeof(szExecute), dwOldProtect, &dwBuf);
  }
  return bRet;
}
#else
#pragma message("This code works only for x86 and x64!")
#endif

static TCHAR s_szExceptionLogFileName[_MAX_PATH] = _T("\\exceptions.log"); // default
static BOOL  s_bUnhandledExeptionFilterSet = FALSE;
static LONG __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs)
{
#ifdef _M_IX86
  if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
  {
    static char MyStack[1024 * 128]; // be sure that we have enough space...
    // it assumes that DS and SS are the same!!! (this is the case for Win32)
    // change the stack only if the selectors are the same (this is the case for Win32)
    //__asm push offset MyStack[1024*128];
    //__asm pop esp;
    __asm mov eax, offset MyStack[1024 * 128];
    __asm mov esp, eax;
  }
#endif

  StackWalkerToConsole sw; // output to console
  sw.ShowCallstack(GetCurrentThread(), pExPtrs->ContextRecord);
  TCHAR lString[500];
  _stprintf_s(lString,
              _T("*** Unhandled Exception! See console output for more infos!\n")
              _T("   ExpCode: 0x%8.8X\n")
              _T("   ExpFlags: %d\n")
#if _MSC_VER >= 1900
              _T("   ExpAddress: 0x%8.8p\n")
#else
              _T("   ExpAddress: 0x%8.8X\n")
#endif
              _T("   Please report!"),
              pExPtrs->ExceptionRecord->ExceptionCode, pExPtrs->ExceptionRecord->ExceptionFlags,
              pExPtrs->ExceptionRecord->ExceptionAddress);
  FatalAppExit(-1, lString);
  return EXCEPTION_CONTINUE_SEARCH;
}

static void InitUnhandledExceptionFilter()
{
  TCHAR szModName[_MAX_PATH];
  if (GetModuleFileName(NULL, szModName, sizeof(szModName) / sizeof(TCHAR)) != 0)
  {
    _tcscpy_s(s_szExceptionLogFileName, szModName);
    _tcscat_s(s_szExceptionLogFileName, _T(".exp.log"));
  }
  if (s_bUnhandledExeptionFilterSet == FALSE)
  {
    // set global exception handler (for handling all unhandled exceptions)
    SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
#if defined _M_X64 || defined _M_IX86
    PreventSetUnhandledExceptionFilter();
#endif
    s_bUnhandledExeptionFilterSet = TRUE;
  }
}
#endif // UNHANDLED_EXCEPTION_TEST

#ifdef EXCEPTION_FILTER_TEST
LONG WINAPI ExpFilter(EXCEPTION_POINTERS* pExp, DWORD dwExpCode)
{
  //StackWalker sw;  // output to default (Debug-Window)
  StackWalkerToConsole sw; // output to the console
  sw.ShowCallstack(GetCurrentThread(), pExp->ContextRecord);
  return EXCEPTION_EXECUTE_HANDLER;
}
void ExpTest5()
{
  char* p = NULL;
  p[0] = 0;
  printf(p);
}
void ExpTest4()
{
  ExpTest5();
}
void ExpTest3()
{
  ExpTest4();
}
void ExpTest2()
{
  ExpTest3();
}
void ExpTest1()
{
  ExpTest2();
}
void TestExceptionWalking()
{
  __try
  {
    ExpTest1();
  }
  __except (ExpFilter(GetExceptionInformation(), GetExceptionCode()))
  {
    printf("\n\nException-Handler called\n\n\n");
  }
}

int f(int i)
{
  if (i < 0)
    return i;
  return f(i + 1);
}
#endif // EXCEPTION_FILTER_TEST

//void map_custom_class_example();

// Simple implementation of an additional output to the console:
class MyStackWalker : public StackWalker
{
public:
  MyStackWalker() : StackWalker() {}
  MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}
  virtual void OnOutput(LPCSTR szText)
  {
    printf(szText);
    StackWalker::OnOutput(szText);
  }
};

// Test for callstack of threads for an other process:
void TestDifferentProcess(DWORD dwProcessId) // copied from demo project.
{
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);

  if (hProcess == NULL)
    return;

  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, dwProcessId);
  if (hSnap == INVALID_HANDLE_VALUE)
    return;

  THREADENTRY32 te;
  memset(&te, 0, sizeof(te));
  te.dwSize = sizeof(te);
  if (Thread32First(hSnap, &te) == FALSE)
  {
    CloseHandle(hSnap);
    return;
  }

  // Initialize StackWalker...
  MyStackWalker sw(dwProcessId, hProcess);
  sw.LoadModules();
  // now enum all thread for this processId
  do
  {
    if (te.th32OwnerProcessID != dwProcessId)
      continue;
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
    if (hThread == NULL)
      continue;
    char szTemp[100];
    sprintf(szTemp, "\r\nThreadID: %d\r\n", te.th32ThreadID);
    sw.OnOutput(szTemp); // output the threadId
    /*CONTEXT c;
    memset(&c, 0, sizeof(CONTEXT));
    c.ContextFlags = CONTEXT_FULL;
    SuspendThread(hThread);
    if (GetThreadContext(hThread, &c) != FALSE)
    {*/
    sw.ShowCallstack(
        hThread); //, &c); // Without passing context, it says filename not available, but shows filename, function name, line number.
    /*}
    ResumeThread(hThread);*/
    CloseHandle(hThread);
  } while (Thread32Next(hSnap, &te) != FALSE);
}

//void CollectCallStackForDifferentProcess()
//{
//  DWORD pid = 22296;
//  HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
//
//  DWORD  tid = 22296;
//  HANDLE threadHandle = OpenThread(, TRUE, tid);
//  //StackWalker sw(63, "C:\\LWOL\\Stuff\\Profiler\\Process\\ProfilerTarget\\Debug", pid, processHandle);
//  StackWalker sw(pid, processHandle);
//  sw.ShowCallstack();
//}

void CreateFakeCallTree()
{
  for (int i = 97; i < 103; i++)
  {
    std::vector<std::vector<std::string>> callStacks;
    for (int k = (i - 97); k < (i - 97 + 1); k++)
    {
      std::vector<std::string> callStack;
      for (int j = k; j < (k + 30); j++)
      {
        callStack.push_back(std::to_string(j)); // 5 elements in each call stack
      }
      for (size_t x = 0; x < 20; x++)
      {
        callStacks.push_back(callStack); // List of call stacks per key
      }
      callStack.push_back("42");
      callStacks.push_back(callStack); // List of call stacks per key
    }
    std::string key;
    key = char(i);
    gCallTrees.insert({key, callStacks});
  }
}

int gCount = 0;

int main(int argc, _TCHAR* argv[])
{
  //printf("\n\n\nShow a simple callstack of the current thread:\n\n\n");
  //CreateMultipleThreads(); // Create these as a placeholder for LV threads.

  //CreateProfilerThread();

  ////WaitForAllThreads();
  //WaitForProfilerThread();

  /*Profiler p;
  
  p.GetInstance(true);
  p.StartStopProfilerThread(true);

  for (size_t i = 0; i < 1000; i++)
  {
  }

  p.StartStopProfilerThread(false);*/

  CreateFakeCallTree();

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  CreateGraphAndJSON(gCallTrees);

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout << "Time difference = "
            << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]"
            << std::endl
            << "Total count: " << gCount;
  gCallTrees.clear();
  //CollectCallStackForDifferentProcess();
  //TestDifferentProcess(21924);
  return 0;
}
