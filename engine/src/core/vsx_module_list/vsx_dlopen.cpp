
#include <vsx_platform.h>


#if PLATFORM_FAMILY == PLATFORM_FAMILY_WINDOWS
  #include <io.h>
  #include <windows.h>
  #include <stdio.h>
#endif

#if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
  #include <dlfcn.h>
  #include <syslog.h>
#endif


#if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
  #include <stdio.h>
  #include <stdlib.h>
  #include <string>
  #include <sys/stat.h>
  #include <unistd.h>
#endif

#include "vsx_dlopen.h"



//edited
#if PLATFORM_FAMILY == PLATFORM_FAMILY_WINDOWS
HMODULE vsx_dlopen::open(const char *filename)
{
  HMODULE winlibrary = LoadLibrary( filename );
  return winlibrary;
}
#endif
#if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
void* vsx_dlopen::open(const char *filename)
{
  return dlopen( filename, RTLD_NOW );
}
#endif


#if PLATFORM_FAMILY == PLATFORM_FAMILY_WINDOWS
int  vsx_dlopen::close(HMODULE handle)
{
  return FreeLibrary( handle );
}
#endif
#if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
  int  vsx_dlopen::close(void* handle)
{
  return dlclose( handle );
}
#endif


char*  vsx_dlopen::error()
{
#if PLATFORM_FAMILY == PLATFORM_FAMILY_WINDOWS
  DWORD dw = GetLastError();
  char* message = (char*)malloc(sizeof(char)*64);
  sprintf(message, "windows error code %d", dw);
  return message;
#endif
#if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
  return dlerror();
#endif
}


#if PLATFORM_FAMILY == PLATFORM_FAMILY_WINDOWS
void* vsx_dlopen::sym(HMODULE handle, const char *symbol)
{
  return GetProcAddress( handle, symbol );
}
#endif
#if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
void*  vsx_dlopen::sym(void *handle, const char *symbol)
{
  return dlsym( handle, symbol );
}
#endif

