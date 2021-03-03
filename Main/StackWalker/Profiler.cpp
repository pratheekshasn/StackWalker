#include "Profiler.h"

//#include "BaseInclude.h"
//#include "LVBaseComponent/EnforceNonConditionalCompiling.h" // This must happen before BaseInclude.h
//
//DefineFile(__FILE__);
//DefineRCSID("$Id: //labview/branches/2020/dev/source/profiler/Profiler.cpp#none $");

#include "GraphCreator.h"
#include "Profiler.h"
#include "StackWalker.h"
#include <iostream>

//static ThReaderWriterMutex gDynamicESystemEContextsReaderWriterRights = NULL;
//static ThReaderWriterMutex gProfilerReaderWriterRights = NULL;
bool                       g_profActive = false;

//std::map<std::string, std::vector<std::vector<std::string>>>
//                           gCallTrees; // This could probably be changed to class members as well, but maybe on the second pass, not right now.
//std::map<std::string, int> gFunctionCounts;

Profiler* Profiler::GetInstance(bool enable)
{
  static Profiler s_instance;
  //Profiler::s_profActive = enable;
  if (enable)
      StartProfiler();
  return &s_instance;
}

void Profiler::StartStopProfilerThread(bool enabled)
{
  //Profiler::s_profActive = enabled;
  if (enabled)
    StartProfiler();
  else
    StopProfiler();
}

void Profiler::StartProfiler()
{
  g_profActive = true;
  //s_profActive = true;
  CreateProfilerThread();
}

int Profiler::StopProfiler()
{
  g_profActive = false;
  //s_profActive = false;
  DWORD dwWaitResult = WaitForMultipleObjects(1,           // number of handles in array
                                              &profilerThread, // array of thread handles
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

void Profiler::StackWalk() //ThThread thread, std::string viPath)
{
 
}

DWORD WINAPI Profiler::Profile(LPVOID lpParam) //ThProcArgType ecPtr)
{
  while (g_profActive)
    std::cout << "Active" <<std::endl;
  std::cout << "Not active";
  return 0;
}

void Profiler::CreateProfilerThread()
{
  //MgErr err;
  DWORD dwThreadID;

  profilerThread = CreateThread(NULL,         // default security
                                  0,            // default stack size
                                  Profiler::Profile, // name of the thread function
                                  NULL,         // no thread parameters
                                  0,            // default startup flags
                                  &dwThreadID);

  if (profilerThread == NULL)
  {
    printf("CreateThread failed (%d)\n", GetLastError());
    return;
  }
}
