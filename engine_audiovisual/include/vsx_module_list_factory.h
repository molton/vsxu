#ifndef VSX_MODULE_LIST_FACTORY_H
#define VSX_MODULE_LIST_FACTORY_H

#include <vsx_platform.h>


//#if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
//#define DLLIMPORT
//#else
#include <windows.h>
  #if defined(VSX_ENG_DLL)
    #define DLLIMPORT __declspec (dllexport)
  #else 
    #define DLLIMPORT __declspec (dllimport)
  #endif
//#endif



//VSX_MANAGER_DLLIMPORT vsx_module_list_abs* vsx_module_list_factory_create();
//VSX_MANAGER_DLLIMPORT void vsx_module_list_factory_destroy( vsx_module_list_abs* object );



#endif
extern "C" {

DLLIMPORT vsx_module_list_abs* vsx_module_list_factory_create();
DLLIMPORT void vsx_module_list_factory_destroy( vsx_module_list_abs* object );
}
