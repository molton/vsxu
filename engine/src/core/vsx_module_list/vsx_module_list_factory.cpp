#include <map>
#include <vector>
#include <vsx_string.h>
#include <vsx_param.h>
#include <vsx_module.h>


#include "vsx_module_list_abs.h"
#include "vsx_module_list_linux.h"
#include "vsx_module_list_win.h"
#include "vsx_module_list_osx.h"
#include <vsx_platform.h>

vsx_module_list_abs* vsx_module_list_factory_create()
{
  #if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
    vsx_module_list_linux* module_list = new vsx_module_list_linux();
    module_list->init();
    return module_list;
  #endif
  #if PLATFORM_FAMILY == PLATFORM_FAMILY_WINDOWS
  #endif
}

void vsx_module_list_factory_destroy( vsx_module_list_abs* object )
{
#if PLATFORM_FAMILY == PLATFORM_FAMILY_UNIX
  delete (vsx_module_list_linux*)object;
#endif
#if PLATFORM_FAMILY == PLATFORM_FAMILY_WINDOWS
#endif

}
