#ifndef VSX_MODULE_LIST_LINUX_H
#define VSX_MODULE_LIST_LINUX_H

// Implementation of Module List Class for Linux

// See vsx_module_list_abs.h for reference documentation for this class

class vsx_module_list_linux : public vsx_module_list_abs
{
private:
  std::vector<void*> module_handles;
public:
  void init(vsx_string args = "");
  std::vector< vsx_module_info* > get_module_list( bool include_hidden = false);
  void* load_module_by_name(vsx_string name);
  bool find( const vsx_string &module_name_to_look_for) = 0;
};

#endif
