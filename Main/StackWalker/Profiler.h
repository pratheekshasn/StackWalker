#ifndef _Profiler_H_
#define _Profiler_H_

#include <thread>
#include <windows.h>

class Profiler
{
public:
  Profiler* GetInstance(bool enable);
  void             StartStopProfilerThread(bool enabled);

private:
  HANDLE profilerThread;
  static bool s_profActive;
  void        StartProfiler();
  int        StopProfiler();
  static void StackWalk(); //ThThread thread, std::string viPath);
  static DWORD WINAPI Profile(LPVOID lpParam); //ThProcArgType ecPtr);
  void        CreateProfilerThread();
};

#endif // _Profiler_H_
