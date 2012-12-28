#include <map>
#include <vector>
#include <vsx_string.h>
#include <vsx_param.h>
#include <vsx_module.h>
#include <dlfcn.h>


#include "vsx_module_list_abs.h"
#include "vsx_module_list_linux.h"
#include "vsx_module_dll_info.h"

void vsx_module_list_linux::init(vsx_string args = "")
{
  // woops, looks like we already built the list
  if (module_list.size()) return;

  // statistics counter - how many modules are loaded in total?
  unsigned long total_num_modules = 0;

  // set up engine environment for later use (directories in which modules can look for config)
  vsx_engine_environment engine_environment;
  engine_environment.engine_parameter[0] = PLATFORM_SHARED_FILES+"plugin-config/";

  // recursively find the plugin so's from the plugins directory
  // store it in: mfiles
  std::list<vsx_string> mfiles;
  get_files_recursive
  (
    vsx_string(CMAKE_INSTALL_PREFIX)
    +
    "/"
    +
    vsx_string(VSXU_INSTALL_LIB_DIR)
    +
    "/vsxu/plugins"
    ,
    &mfiles
    ,
    ".so"
    ,
    ""
  );
  //-------------------------------------------------------------------------

  //-------------------------------------------------------------------------
  // Iterate through all the filenames, treat them as plugins with dlopen
  // and probe them to see if they are vsxu modules.
  for (std::list<vsx_string>::iterator it = mfiles.begin(); it != mfiles.end(); ++it)
  {
    vsx_string dynamic_object_file_name = (*it);
    vsx_avector<vsx_string> parts;
    void* plugin_handle;
    vsx_string deli = "/";
    explode((*it),deli,parts);

    // load the plugin
    plugin_handle = dlopen(
          dynamic_object_file_name.c_str(),
          RTLD_NOW
    );

    // if loading fails, print debug output
    if (!plugin_handle) {
      printf(
            "vsx_module_list init: Error: trying to load the plugin \"%s\"\n"
            "                      Cause: dlopen returned error: %s\n",
            dynamic_object_file_name.c_str(),
            dlerror()
      );
      continue; // try to load the next plugin
    }

    // add this module handle to our list of module handles
    module_handles.push_back(plugin_handle);

    //-------------------------------------------------------------------------
    // look for the REQUIRED constructor (factory) method
    if (dlsym(plugin_handle, "create_module") == 0)
    {
      printf(
            "vsx_module_list init: Error: trying to load the plugin \"%s\"\n"
            "                      Cause: dlsym could not find \"create_module\"\n",
            dynamic_object_file_name.c_str()
            );
      continue; // try to load the next plugin
    }
    // initialize constructor (factory) method
    vsx_module*(*create_new_module)(unsigned long) =
        (vsx_module*(*)(unsigned long))
        dlsym(
          plugin_handle,
          "create_new_module"
        );
    //-------------------------------------------------------------------------



    //-------------------------------------------------------------------------
    // look for the REQUIRED destructor method
    if (dlsym(plugin_handle, "destroy_module") == 0)
    {
      printf(
            "vsx_module_list init: Error: trying to load the plugin \"%s\"\n"
            "                      Cause: dlsym could not find \"destroy_module\"\n",
            dynamic_object_file_name.c_str()
            );
      continue; // try to load the next plugin
    }
    // init destructor method
    void(*destroy_module)(vsx_module*,unsigned long) =
        (void(*)(vsx_module*,unsigned long))
        dlsym(
          plugin_handle,
          "destroy_module"
        );
    //-------------------------------------------------------------------------



    //-------------------------------------------------------------------------
    // look for the REQUIRED get_num_modules method
    if (dlsym(plugin_handle, "get_num_modules") == 0)
    {
      printf(
            "vsx_module_list init: Error: trying to load the plugin \"%s\"\n"
            "                      Cause: dlsym could not find \"get_num_modules\"\n",
            dynamic_object_file_name.c_str()
            );
      continue; // try to load the next plugin
    }
    // init get_num_modules method
    unsigned long(*get_num_modules)(void) =
        (unsigned long(*)(void))
        dlsym(
          plugin_handle,
          "get_num_modules"
        );
    //-------------------------------------------------------------------------



    //-------------------------------------------------------------------------
    // check for and if found, set the optional environment_info support
    if (dlsym(plugin_handle,"set_environment_info"))
    {
      void(*set_env)(vsx_engine_environment*) =
          (void(*)(vsx_engine_environment*))
          dlsym(
            plugin_handle,
            "set_environment_info"
          );
      set_env(&engine_environment);
    }
    //-------------------------------------------------------------------------

    // get the number of modules in this plugin
    unsigned long num_modules_in_this_plugin = get_num_modules();

    // iterate through modules in this plugin
    for (
         size_t module_index_iterator = 0;
         module_index_iterator < num_modules_in_this_plugin;
         module_index_iterator++
    )
    {
      // ask the constructor / factory to create a module instance for us
      vsx_module* module_object =
          create_new_module(module_index_iterator);
      // check for error
      if (0x0 == module_object)
      {
        printf(
              "vsx_module_list init: Error: trying to load the plugin \"%s\"\n"
              "                      Cause: create_new_module returned 0x0 for module_index_iterator %d\n"
              "                      Hint: If you are developing, check to see that get_num_modules returns\n"
              "                            the correct module count!\n"
              ,
              dynamic_object_file_name.c_str(),
              module_index_iterator
              );
        continue; // try to load the next module
      }

      // ask the module to provide its module info
      vsx_module_info* module_info = new vsx_module_info;
      module_object->module_info( module_info );

      // check to see if this module can run on this system
      bool can_run = module_object->can_run();

      destroy_module( module_object, module_index_iterator );

      if (!can_run) continue; // try to load the next module

      module_info->location = "external";

      // create module_dll_info template
      vsx_module_dll_info module_dll_info_template;

      module_dll_info_template.create_new_module = create_new_module;
      module_dll_info_template.destroy_module = destroy_module;

      module_dll_info_template.module_id = module_index_iterator;

      // split the module identifier string into its individual names
      // a module can have multiple names (and locations in the gui tree)
      // some of these are hidden, thus the name begins with an exclamation mark - !
      // example module identifier string:
      //   examples;my_modules;my_module||!old_path;old_category;old_name
      // Only the first will show up in the gui. The second identifier is still usable in
      // old state files.
      vsx_string deli = "||";
      vsx_avector<vsx_string> parts;
      explode(module_info->identifier, deli, parts);
      vsx_module_dll_info* applied_dll_info = 0;

      // iterate through the individual names for this module
      for (unsigned long i = 0; i < parts.size(); ++i)
      {
        // create a copy of the template
        applied_dll_info = new vsx_module_dll_info;
        *applied_dll_info = module_dll_info_template;
        vsx_module_info* applied_module_info = new vsx_module_info;
        *applied_module_info = *module_info;
        applied_dll_info->module_info = applied_module_info;


        vsx_string module_identifier;
        if (parts[i][0] == '!')
        {
          // hidden from gui
          applied_dll_info->hidden_from_gui = true;
          module_identifier = parts[i].substr(1);
        } else
        {
          // normal
          applied_dll_info->hidden_from_gui = false;
          module_identifier = parts[i];
        }
        // set module info identifier
        applied_module_info->identifier = module_identifier;
        // add the applied_dll_info to module_dll_list
        module_dll_list[module_identifier] = applied_dll_info;

        // add the module info to the module list
        module_list[module_identifier] = module_info;
      } // iterate through the individual names for this module
      module_infos.push_back(module_info);
    } // iterate through modules in this plugin
  } // Iterate through all the filenames, treat them as plugins
}

std::vector< vsx_module_info* >* vsx_module_list_linux::get_module_list( bool include_hidden = false)
{
  std::vector< vsx_module_info* >* result = new std::vector< vsx_module_info* >;
  for (std::map< vsx_string, void* >::const_iterator it = module_dll_list.begin(); it != module_dll_list.end(); it++)
  {
    vsx_module_dll_info* dll_info = (vsx_module_dll_info*)((*it).second);
    if
    (
        include_hidden && dll_info->hidden_from_gui
        ||
        !dll_info->hidden_from_gui
    )
    {
      //printf("module_ident: %s\n", dll_info->module_info->identifier.c_str() );
      result->push_back( dll_info->module_info );
    }
  }
  return result;
}

vsx_module* vsx_module_list_linux::load_module_by_name(vsx_string name)
{
  if ( module_dll_list.find(name) == module_dll_list.end() )
  {
    return 0x0;
  }

  // call constrcuction factory
  vsx_module* module =
    ((vsx_module_dll_info*)module_dll_list[ name ])
    ->
    create_new_module
    (
      ((vsx_module_dll_info*)module_dll_list[ name ])->module_id
    )
  ;
  module->module_id = ((vsx_module_dll_info*)module_dll_list[ name ])->module_id;
  module->module_identifier = ((vsx_module_dll_info*)module_dll_list[ name ])->module_info->identifier;
  return module;
}

void vsx_module_list_linux::unload_module( vsx_module* module_pointer )
{
  // call destrcuction factory
  ((vsx_module_dll_info*)module_dll_list[ module_pointer->module_identifier ])
  ->
  destroy_module
  (
    module_pointer,
    module_pointer->module_id
  );
}

bool vsx_module_list_linux::find( const vsx_string &module_name_to_look_for)
{
  if (!(module_list.find(module_name_to_look_for) != module_list.end()))
  {
    return false;
  }
  return true;
}



/* destroy
  #ifdef VSXU_DEBUG
    printf("engine destroy\n");
  #endif
  // unload module handles
  for (size_t i = 0; i < module_handles.size(); i++)
  {
    #ifdef _WIN32
      FreeLibrary(module_handles[i]);
    #endif
    #if defined(__linux__) || defined(__APPLE__)
      //void* oulp = (void*)dlsym(module_handles[i], "on_unload_library");
      if (dlsym(module_handles[i], "on_unload_library"))
      {
        void(*on_unload_library)(void) = (void(*)(void))dlsym(module_handles[i], "on_unload_library");
        on_unload_library();
      }
      dlclose(module_handles[i]);
    #endif
  }*/
