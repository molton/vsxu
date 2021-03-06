/**
* Project: VSXu: Realtime visual programming language, music/audio visualizer, animation tool and much much more.
*
* @author Jonatan Wallmander, Robert Wenzel, Vovoid Media Technologies Copyright (C) 2003-2011
* @see The GNU Public License (GPL)
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "vsx_gl_global.h"
#include <map>
#include <list>
#include <vector>
#include "vsx_command.h"
#include "vsx_math_3d.h"
#include "vsx_texture_info.h"
#include "vsx_texture.h"
#include "vsx_font.h"
#include "../vsx_widget_base.h"
#include "vsx_widget_lib.h"
#include "vsx_widget_panel.h"
#include "vsx_widget_base_edit.h"
#include <stdlib.h>
#ifndef _WIN32
#include "GL/glfw.h"
#endif


vsx_widget_base_edit::vsx_widget_base_edit() {
  enable_syntax_highlighting = true;
  enable_line_action_buttons = false;
  title ="edit";
  widget_type = VSX_WIDGET_TYPE_EDITOR;
  render_type = VSX_WIDGET_RENDER_2D;
  coord_type = VSX_WIDGET_COORD_CENTER;
  keywords["{"] = 3;
  keywords["}"] = 3;
  keywords["("] = 3;
  keywords[")"] = 3;
  keywords["<"] = 3;
  keywords[">"] = 3;
  keywords["."] = 3;
  keywords["["] = 3;
  keywords["]"] = 3;
  keywords[";"] = 3;
  keywords["-"] = 3;
  keywords["="] = 3;
  keywords["*"] = 3;
  keywords["/"] = 3;
  keywords[""]  = 3;
  keywords[";"] = 3;
  keywords["const"] = 4;
  keywords["varying"] = 4;
  keywords["uniform"] = 4;
  keywords["void"] = 4;
  keywords["float"] = 4;
  keywords["vec2"] = 4;
  keywords["vec3"] = 4;
  selected_line = -1;
  selected_line_highlight = false;
  editing_enabled = true;
  single_row = false;

  mirror_keystrokes_object = 0;
  mirror_mouse_move_object = 0;
  mirror_mouse_move_passive_object = 0;
  mirror_mouse_down_object = 0;
  mirror_mouse_up_object = 0;
  mirror_mouse_double_click_object = 0;
  filter_string_enabled = false;
  support_interpolation = false;

  font_size = 0.004;
  caretx = 0;
  carety = 0;
  scroll_x = 0;
  scroll_y = 0;
  scrollbar_pos_x = 0;
  scrollbar_pos_y = 0;
  longest_line = 0;
  longest_y = 0;
  process_characters = true;
  num_hidden_lines = 0;
  myf.syntax_colors[0] = skin_color[14]; // base color
  myf.syntax_colors[1] = vsx_color__(0.8,0.8,0.8,1); // comment
  myf.syntax_colors[2] = vsx_color__(0,1,1,1);
  myf.syntax_colors[3] = vsx_color__(153.0f/255.0f,204.0f/255.0f,1,1);

  menu = add(new vsx_widget_popup_menu,".edit_menu");
  menu->commands.adds(VSX_COMMAND_MENU, "clear", "clear","a"); //build a menu
  menu->commands.adds(VSX_COMMAND_MENU, "font_size>;very small", "font_size","0.002"); //build a menu
  menu->commands.adds(VSX_COMMAND_MENU, "font_size>;small", "font_size","0.004"); //build a menu
  menu->commands.adds(VSX_COMMAND_MENU, "font_size>;normal", "font_size","0.008"); //build a menu
  menu->commands.adds(VSX_COMMAND_MENU, "font_size>;large", "font_size","0.016"); //build a menu
  menu->commands.adds(VSX_COMMAND_MENU, "font_size>;larger", "font_size","0.022"); //build a menu
  menu->commands.adds(VSX_COMMAND_MENU, "font_size>;largest", "font_size","0.032"); //build a menu
  menu->size.x = 0.2;
  menu->size.y = 0.5;
  menu->init();
  //lines.push_back("");
  //lines.push_back("test22223423498llkjsalfjaskdfoi2u348239429342934782347292384293842849237978");
}

void vsx_widget_base_edit::vsx_command_process_b(vsx_command_s *t) {
  updates++;
  if (t->cmd == "action")
  {
    // TODO: take into account hidden lines
    backwards_message("editor_action "+i2s(id)+" "+lines[scroll_y+s2i(t->parts[1])]);
  }
  else
  if (t->cmd == "font_size")
  {
    font_size = s2f(t->parts[1]);
  }
  else
  if (t->cmd == "clear") {
    k_focus = this;
    set_string("");
  }
}

// PRIVATE
void vsx_widget_base_edit::process_line(int n_line) {
  if (!process_characters) return;
  int i = n_line;
  vsx_string b;

  char fill = (char)0x01;
  if (lines[i].size() >= 2)
  if (lines[i][0] == '/' && lines[i][1] == '/')
  fill = (char)0x02;

  for (size_t j = 0; j < lines[i].size(); ++j) {
    b = b + fill;
  }
  if (fill == (char)0x01)
  for (std::map<vsx_string,char>::iterator it = keywords.begin(); it != keywords.end(); ++it) {
    char f = (char)(*it).second;
    if (enable_syntax_highlighting)
    b = str_replace_char_pad((*it).first, vsx_string(f), lines[i], b);
  }
  if (n_line > (int)lines_p.size()-1) {

    lines_p.push_back(b);
  } else
  lines_p[n_line] = b;
}

void vsx_widget_base_edit::process_lines() {
  if (!process_characters) return;
  lines_p.clear();
  for (unsigned long i = 0; i < lines.size(); ++i) {
    vsx_string b;

    char fill = (char)0x01;
    if (lines[i].size() >= 2)
    if (lines[i][0] == '/' && lines[i][1] == '/')
    fill = (char)0x02;

    for (size_t j = 0; j < lines[i].size(); ++j) {
      b = b + fill;
    }
    if (fill == (char)0x01)
    for (std::map<vsx_string,char>::iterator it = keywords.begin(); it != keywords.end(); ++it) {
      char f = (char)(*it).second;
      b = str_replace_char_pad((*it).first, vsx_string(f), lines[i], b);
    }
    lines_p.push_back(b);
    //lines_p[i] = b;

    //printf(" line %d: %s\n",i,lines[i].c_str());
    //printf("pline %d: %s\n",i,b.c_str());
  }
}

void vsx_widget_base_edit::calculate_scroll_size() {
  size_t t_longest_line = 0;
  for (std::vector<vsx_string>::iterator it = lines.begin(); it != lines.end(); ++it) {
    if ((*it).size() > t_longest_line) t_longest_line = (*it).size();
  }
  t_longest_line += 3;
  if (t_longest_line > longest_line) longest_line = t_longest_line;
  characters_width = floor(target_size.x/(font_size*0.37));
  characters_height = floor(target_size.y/(font_size));

  //if (scroll_x_max
  //if (longest_line < scroll_x+characters_width) scroll_x_max = scroll_x+characters_width;
  //longest_line;//+characters_width-1;
  //scroll_x_max = longest_line;

  //float s = longest_line;
  //float s = scroll_x+characters_width;
  //if (longest_line < scroll_x+characters_width) {
//      scroll_x_max = scroll_x+3;
//      scroll_x_size = 1-(float)(scroll_x+3)/(scroll_x+characters_width+3);
//    }
//    else {
    scroll_x_max = longest_line;
//      scroll_x_size = 1-(float)(longest_line - characters_width)/characters_width;
    scroll_x_size = 1-(float)(longest_line - characters_width)/longest_line;
//    }

  longest_y = 0;
  if (longest_y < (float)lines.size()+3) longest_y = (float)lines.size()+3-num_hidden_lines;

  if (scroll_x_size > 1) scroll_x_size = 1;

  scroll_y_max = longest_y;
  float ff = longest_y - characters_height;
  if (ff < 0) ff = 0;
  scroll_y_size = 1-(float)(ff)/longest_y;
  if (scroll_y_size > 1) scroll_y_size = 1;
  //scroll_y_size = (float)characters_height/(float)lines.size();//+(float)characters_height);
}

void vsx_widget_base_edit::set_string(const vsx_string& str) {
  lines.clear();
  vsx_string deli = "\n";
  vsx_string f = str_replace("\r","",str);
  explode(f, deli, lines);
  lines_visible.clear();
  for (unsigned long i = 0; i < lines.size(); i++) lines_visible.push_back(0);
  longest_line = 0;
  scrollbar_pos_x = 0;
  scrollbar_pos_y = 0;
  for (std::vector<vsx_string>::iterator it = lines.begin(); it != lines.end(); ++it) {
    if ((*it).size() > longest_line) longest_line = (*it).size();
  }
  // hide eventual action buttons
  for (size_t i = 0; i < action_buttons.size(); i++)
  {
    action_buttons[i]->visible = 0.0f;
  }
  process_lines();
  calculate_scroll_size();
}

void vsx_widget_base_edit::set_filter_string(vsx_string &filter_string)
{
  if (filter_string.size() == 0) {
    if (filter_string_enabled) lines_visible = lines_visible_stack;
    filter_string_enabled = false;
    return;
  }

  if (!filter_string_enabled) lines_visible_stack = lines_visible;

  for (unsigned long i = 0; i < lines.size(); ++i) {
    if (lines[i].find(filter_string) != -1) lines_visible[i] = 0;
    else lines_visible[i] = 1;
  }
  scroll_y = 0;
  scrollbar_pos_y = 0;
  filter_string_enabled = true;
}

void vsx_widget_base_edit::caret_goto_end() {
#ifdef _WIN32
  event_key_down(-35,false,false,false);
#else
#if defined(_LINUX) || defined (__APPLE__)
  event_key_down(-GLFW_KEY_END,false,false,false);
#endif
#endif
}

void vsx_widget_base_edit::event_mouse_down(vsx_widget_distance distance,vsx_widget_coords coords,int button) {
  vsx_widget::event_mouse_down(distance,coords,button);
  if (!lines.size()) return;
  if (button == 0) {
    m_focus = this;
//      caretx = floor(distance.corner.x/(font_size*0.37));
//printf("distance.corner: %f\n",distance.corner.x);
    caretx = (int)floor(distance.corner.x/(font_size*0.37));
    carety = (int)floor(((target_size.y-distance.corner.y)/font_size));
    //printf("cx: %d, cy: %d\n",caretx, carety);
    if (carety < 0) carety = 0;
    if (carety > lines.size()-num_hidden_lines-1-scroll_y) carety = (int)floor(lines.size()-num_hidden_lines-1-scroll_y);

    if (lines_visible.size() == lines.size())
    {
      if (selected_line_highlight) {
        int clicked_line = 0;
        clicked_line = carety + (int)scroll_y;
        int real_line = 0;
        int num_visible_found = 0;
        while (num_visible_found < clicked_line && real_line < (int)(lines.size()-1))
        {
          if (lines_visible[real_line] == 0) num_visible_found++;
          real_line++;
        }
        while (lines_visible[real_line] != 0 && real_line < (int)(lines.size()-1)) real_line++;
        selected_line = real_line;
    }
    }
    if (caretx > lines[(int)(carety+scroll_y)].size()-scroll_x) {
#ifdef _WIN32
    event_key_down(-35,false,false,false);
#else
#if defined(_LINUX) || defined (__APPLE__)
    event_key_down(-GLFW_KEY_END,false,false,false);
#endif
#endif
    }
    //printf("carety: %d\n", carety);
  }
  if (mirror_mouse_down_object != 0) mirror_mouse_down_object->event_mouse_down(distance, coords, button);
}

void vsx_widget_base_edit::event_mouse_up(vsx_widget_distance distance,vsx_widget_coords coords,int button)
{
  if (mirror_mouse_up_object != 0) mirror_mouse_up_object->event_mouse_up(distance, coords,button);
}

void vsx_widget_base_edit::event_mouse_move(vsx_widget_distance distance,vsx_widget_coords coords)
{
  if (mirror_mouse_move_object != 0) mirror_mouse_move_object->event_mouse_move(distance, coords);
}

void vsx_widget_base_edit::event_mouse_move_passive(vsx_widget_distance distance,vsx_widget_coords coords)
{
  if (mirror_mouse_move_passive_object != 0) mirror_mouse_move_passive_object->event_mouse_move_passive(distance, coords);

}

void vsx_widget_base_edit::event_mouse_wheel(float y)
{
  float ff = longest_y - characters_height;
  scrollbar_pos_y -= y / ff;
  if (scrollbar_pos_y < 0) scrollbar_pos_y = 0;
  if (scrollbar_pos_y > 1.0f) scrollbar_pos_y = 1.0f;
}

int count_whitespaces(vsx_string& s)
{
  size_t i = 0;
  while (i < s.size() && s[i] == ' ')
  {
    //printf("line: %d\n",__LINE__);
    i++;
  }
  return i;
}

void vsx_widget_base_edit::event_mouse_double_click(vsx_widget_distance distance,vsx_widget_coords coords,int button)
{
  if (mirror_mouse_double_click_object) {
    mirror_mouse_double_click_object->event_mouse_double_click(distance,coords,button);
  }

  if (button == 0)
  {
    // 0 0 a c
    // 1 0  +b
    // 2 0    +c
    // 3 0    +d
    // 4 0  +e
    // 5 0   +f
    // 6 1    +g
    // 7 1    +g
    // 8 0   +g
    // 9 0  +g
    // A 0 g

    if (filter_string_enabled) return;



    event_mouse_down(distance, coords,button);
    // 1. locate real line

    int real_line = selected_line;
    if (real_line < 0) return;
    /*int clicked_line = carety + (int)scroll_y;
    int real_line = 0;
    int num_visible_found = 0;

    while (num_visible_found < clicked_line) {
      if (lines_visible[real_line] == 0) num_visible_found++;
      real_line++;
    }
    while (lines_visible[real_line] != 0) real_line++;
    */
    //printf("real_line: %d\n", real_line);
    if (real_line >= (int)lines.size()-1) return;
    int real_white = count_whitespaces(lines[real_line]);
    if (real_white < count_whitespaces(lines[real_line+1]))
    {
      real_line++;
      int new_state = lines_visible[real_line]; // if next line is hidden (>1) set it to 0
      while (real_line < (int)lines.size() && count_whitespaces(lines[real_line]) > real_white)
      {
        //printf("real line: %d\n",real_line);
        if (!new_state) // next line is not hidden!
        {
          if (lines_visible[real_line] == 0) num_hidden_lines++;  // a previously unhidden line is now hidden
          lines_visible[real_line]++; // hide line

          //float ff = longest_y - characters_height;
          //scrollbar_pos_y += (1.0 / ff) * 0.10f;
          //if (scrollbar_pos_y > 1.0f) scrollbar_pos_y = 1.0f;
        } else
        {
          lines_visible[real_line]--;
          if (lines_visible[real_line] == 0) num_hidden_lines--; // a previously hidden line is now unhidden
          //float ff = longest_y - characters_height;
          //scrollbar_pos_y -= 1.0 / ff;
          //if (scrollbar_pos_y < 0.0f) scrollbar_pos_y = 0.0f;
        }
        real_line++;
      }
    }
    calculate_scroll_size();
    // 2. check next line if it's more indented
    // 3. if it's not - do nothing, we're at lowest level
    // 4. else hide lines until end or less indented
  }
}

void vsx_widget_base_edit::fold_all()
{
  int prev_white = 0;
  int val = 0;
  int white_frac = 0;
  for (unsigned long i = 0; i < lines_visible.size(); i++)
  {
    int white = count_whitespaces(lines[i]);
    if (white_frac == 0 && white > prev_white) white_frac = white;
    //if (white < prev_white) val--;
    //prev_white = white;
    //if (val < 0) val = 0;
    //if (white == 0) val = 0;
    if (white_frac == 0) val = 0;
    else
    val = white / white_frac;
    //printf("%d %s\n", val,lines[i].c_str());
    lines_visible[i] = val;
  }
  calculate_scroll_size();
}

/*******************************************************************************
        _               _
       (_)             | |
  _  _  _            _ | |  ____   ____  _ _ _
 (_)(_)| |          / || | / ___) / _  || | | |
  _  _ | | _______ ( (_| || |    ( ( | || | | |
 (_)(_)|_|(_______) \____||_|     \_||_| \____|
********************************************************************************/
void vsx_widget_base_edit::i_draw() {
  if (!lines.size()) return;

  allow_move_y = allow_move_x = false;
  //base_draw();
  scroll_x = round(scrollbar_pos_x*(longest_line-characters_width));
  if (scroll_x < 0) scroll_x = 0;
  float ff = longest_y - characters_height;
  if (ff < 0) ff = 0;
  scroll_y = round(scrollbar_pos_y*(ff));
  vsx_vector p = get_pos_p();
  p.x -= target_size.x*0.5;
  p.y -= target_size.y*0.5;
  if (render_type == VSX_WIDGET_RENDER_3D) {
    p.z = pos.z;
  } else {
    p.z = 0.0f;
  }
  skin_color[18].gl_color();
  draw_box(p, target_size.x, target_size.y);
  //int ypos = 0;
  p.y += target_size.y-font_size;
  calculate_scroll_size();
  bool run = true;

  int num_visible_found = 0;
  int real_line = 0;
  while (num_visible_found < scroll_y && real_line < (int)(lines.size()-1)) {
    //printf("traversing visible found\n");
    if (lines_visible[real_line] == 0) num_visible_found++;
    real_line++;
  }
  while (lines_visible[real_line] != 0 && real_line < (int)(lines.size()-1)) real_line++;

  int curline = real_line;
  vsx_vector pp = p;
  //std::vector<vsx_string>::iterator it = lines.begin();
  myf.syntax_colors[0] = skin_color[14];
  int cur_render_line = 0;
  if (selected_line_highlight) myf.color = skin_color[14];
  if (scroll_y < lines.size())
  if (curline < (int)lines.size())
  while (run)
  {
    if (lines_visible[curline] == 0)
    {
      //printf("rendering %d\n",curline);
      // save current size for speed in a variable
      long cursize = (long)lines[curline].size();
      //printf("scroll_x: %d\n",scroll_x);
      //printf("curline: %d\n",curline);
      //printf("cursize: %d\n",cursize);
      if (selected_line_highlight && curline == selected_line)
      {
        skin_color[15].gl_color();
        myf.syntax_colors[0] = skin_color[16]; // base color
        draw_box(pp,target_size.x,font_size);
      }
      if (cursize-(long)scroll_x >= (long)characters_width) {
        myf.print(pp,lines[curline].substr((int)scroll_x,(int)characters_width),font_size,lines_p[curline].substr((int)scroll_x,(int)characters_width));
      } else
      if (scroll_x < cursize)
      myf.print(pp,lines[curline].substr((int)scroll_x,(int)(lines[curline].size()-floor(scroll_x))),font_size,lines_p[curline].substr((int)scroll_x,(int)(lines[curline].size()-scroll_x)));

      if (enable_line_action_buttons)
      {
        if (cur_render_line+1 > (int)action_buttons.size())
        {
          vsx_widget* new_action_button = add(new vsx_widget_button,"ab_"+i2s(cur_render_line));
          new_action_button->init();
          new_action_button->set_size( vsx_vector(0.005f, font_size) );
          new_action_button->coord_related_parent = false;
          new_action_button->render_type = this->render_type;
          new_action_button->title = "x";
          new_action_button->commands.adds(4,"","action",i2s(cur_render_line));
          action_buttons.push_back(new_action_button);
        }
        action_buttons[cur_render_line]->set_pos(pp + vsx_vector(target_size.x-0.0025f,font_size*0.5f));
        action_buttons[cur_render_line]->visible = 1.0f;
      }

      pp.y -= font_size;

      if (selected_line_highlight && curline == selected_line)
      {
        myf.syntax_colors[0] = skin_color[14]; // base color
      }
      cur_render_line++;
    }
//    ++it;
    ++curline;
    //printf("%d %d %d %d\n",cur_render_line,(int)(characters_height-1), (int)curline, (lines.size()-1));
    if (cur_render_line > (long)characters_height-1 || (long)curline > (long)lines.size()-1)
    run = false;
  //  if (it == lines.end()) run = false;
  }
  if (k_focus == this && editing_enabled) {
    pp.x = p.x+(float)caretx*font_size*0.37;
    pp.y = p.y-font_size*(float)(carety);
    float tt = (float)((int)(time*3000) % 1000)*0.001;
    if (selected_line_highlight) myf.color = skin_color[17];
    myf.color.a = 1-tt;
    myf.print(pp,"_",font_size);
    myf.color.a = 1;
  }
  myf.color = vsx_color__(1,1,1,1);
}


bool vsx_widget_base_edit::event_key_down(signed long key, bool alt, bool ctrl, bool shift) {
  if (!editing_enabled) return true;
  std::vector<vsx_string>::iterator it = lines.begin();
  std::vector<int>::iterator itlv = lines_visible.begin();

  std::vector<vsx_string>::iterator itp = lines_p.begin();
  int c2 = 0;
  scroll_x = floor(scroll_x);
  vsx_string tempstring;
  vsx_string tempstring2;
  //printf("key: %d\n",key);
  if (ctrl && !alt && !shift) {
    //printf("ctrl! %d\n",key);
    switch(key) {
      case 10:
        //save();
      break;
      case 'v':
      case 'V':
#ifdef _WIN32
        HANDLE hData;

        LPVOID pData;
        char* pszData = 0;
        HWND hwnd = GetForegroundWindow();
        if (!IsClipboardFormatAvailable(CF_TEXT)) return false;
        OpenClipboard(hwnd);
        hData = GetClipboardData(CF_TEXT);
        pData = GlobalLock(hData);
        if (pszData) free(pszData);
        pszData = (char*)malloc(strlen((char*)pData) + 1);
        strcpy(pszData, (LPSTR)pData);
        vsx_string res = pszData;
        GlobalUnlock(hData);
        CloseClipboard();
        res = str_replace("\n","",res);
        process_characters = false;
        for (int i = 0; i < res.size(); ++i) {
          event_key_down(res[i],false,false,false);
        }
        free(pszData);
        process_characters = true;
        process_lines();
// copying
/*HGLOBAL hData;
    LPVOID pData;
    OpenClipboard(hwnd);
    EmptyClipboard();
    hData = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,
                        strlen(pszData) + 1);
    pData = GlobalLock(hData);
    strcpy((LPSTR)pData, pszData);
    GlobalUnlock(hData);
    SetClipboardData(CF_TEXT, hData);
    CloseClipboard();*/
#endif
      break;
    }
  } else
  switch(key) {
    // arrow left
#ifdef _WIN32
    case -37:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_LEFT:
#endif
#endif
      --caretx;
      if (caretx < 0) {
        if (scroll_x) {
          ++caretx;
          --scroll_x;
          //fix_pos();
        } else
        if (carety) {
          --carety;
#ifdef _WIN32
          event_key_down(-35,false,false,false);
#else
#if defined(_LINUX) || defined (__APPLE__)
        event_key_down(-GLFW_KEY_END);
#endif
#endif
        } else caretx = 0;
      }
      break;
    // arrow right
#ifdef _WIN32
    case -39:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_RIGHT:
#endif
#endif
      ++caretx;
      if ((size_t)caretx > lines[carety+(int)scroll_y].size()-(int)scroll_x) {
#ifdef _WIN32
        event_key_down(-40,false,false,false);
        event_key_down(-36,false,false,false);
#else
#if defined(_LINUX) || defined (__APPLE__)
        event_key_down(-GLFW_KEY_DOWN);
        event_key_down(-GLFW_KEY_HOME);
#endif
#endif
      }
      if (caretx > characters_width-3) {
        --caretx;
        ++scroll_x;
      }
      break;
    // arrow up
#ifdef _WIN32
    case -38:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_UP:
#endif
#endif
      if (!single_row) {
        --carety;
        if (carety < 0) {
          carety = 0;
          if (scroll_y) {
            --scroll_y;
            //fix_pos();
          }
        }
        if ((size_t)caretx > lines[carety+(int)scroll_y].size()-(int)scroll_x)
#ifdef _WIN32
          event_key_down(-35);
#else
#if defined(_LINUX) || defined (__APPLE__)
        event_key_down(-GLFW_KEY_END);
#endif
#endif
         //caretx = lines[carety+(int)scroll_y].size()-(int)scroll_x;
        }
      break;
    // page up
#ifdef _WIN32
    case -33:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_PAGEUP:
#endif
#endif
      if (!single_row) {
        for (int zz = 0; zz < characters_height*0.95; ++zz) {
#ifdef _WIN32
          event_key_down(-38);
#else
#if defined(_LINUX) || defined (__APPLE__)
        event_key_down(-GLFW_KEY_UP);
#endif
#endif
        }
      }
      break;
    // arrow down
#ifdef _WIN32
    case -40:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_DOWN:
#endif
#endif
      if (!single_row) {
        ++carety;
        if (carety > lines.size()-1-scroll_y) carety = (int)((float)lines.size()-1.0f-scroll_y);
        if (carety > characters_height-2) {
          ++scroll_y;
          --carety;
        }
        if ((size_t)caretx > lines[carety+(int)scroll_y].size()-(int)scroll_x)
         caretx = lines[carety+(int)scroll_y].size()-(int)scroll_x;
      }
      break;
    // page down
#ifdef _WIN32
    case -34:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_PAGEDOWN:
#endif
#endif
      if (!single_row) {
        for (int zz = 0; zz < characters_height*0.95; ++zz) {
#ifdef _WIN32
          event_key_down(-40);
#else
#if defined(_LINUX) || defined (__APPLE__)
        event_key_down(-GLFW_KEY_DOWN,false,false,false);
#endif
#endif
        }
      }
      break;
    // home
#ifdef _WIN32
    case -36:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_HOME:
#endif
#endif
      scroll_x = 0;
      caretx = 0;
      //fix_pos();
      break;
    // end
#ifdef _WIN32
    case -35:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_END:
#endif
#endif
      caretx = lines[carety+(int)scroll_y].size()-(int)scroll_x;
      //if (caretx < 0) caretx = 0;
      if (caretx > characters_width-3) {
        scroll_x += caretx - characters_width+3;
        //fix_pos();
        caretx = (int)characters_width-3;
      }
      if (caretx < 0) {
        scroll_x += caretx-5;//lines[carety+(int)scroll_y].size()-5;
        if (scroll_x < 0) scroll_x = 0;
        caretx = lines[carety+(int)scroll_y].size()-(int)scroll_x;
      }
      //fix_pos();
    break;
    // backspace
#ifdef _WIN32
    case 8:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_BACKSPACE:
#endif
#endif
      if (caretx+(int)scroll_x) {
        lines[carety+(int)scroll_y].erase(caretx-1+(int)scroll_x,1);
        --caretx;
        if (caretx < 0) {--scroll_x; ++caretx;}
        process_line(carety+(int)scroll_y);
        //fix_pos();
      } else {
        if (scroll_y+carety) {
          while (c2 < carety+scroll_y) { ++c2; ++it; ++itp; ++itlv; }
          //++it;
          tempstring = lines[carety+(int)scroll_y];
          lines.erase(it);
          lines_p.erase(itp);
          lines_visible.erase(itlv);
#ifdef _WIN32
          event_key_down(-38,false,false,false);
          event_key_down(-35,false,false,false);
#else
#if defined(_LINUX) || defined (__APPLE__)
        event_key_down(-GLFW_KEY_UP,false,false,false);
        event_key_down(-GLFW_KEY_END,false,false,false);
#endif
#endif
          lines[carety+(int)scroll_y] += tempstring;
          lines_p[carety+(int)scroll_y] += tempstring;
          process_line(carety+(int)scroll_y);
          process_line(carety+(int)scroll_y+1);
          //fix_pos();
        }
      }
      if (mirror_keystrokes_object) mirror_keystrokes_object->event_key_down(key, alt, ctrl, shift);
    break;
    // delete
#ifdef _WIN32
    case -46:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_DEL:
#endif
#endif
#ifdef _WIN32
      event_key_down(-39,false,false,false);
      event_key_down(8,false,false,false);
#else
#if defined(_LINUX) || defined (__APPLE__)
        event_key_down(-GLFW_KEY_RIGHT,false,false,false);
        event_key_down(-GLFW_KEY_BACKSPACE,false,false,false);
#endif
#endif
      process_line(carety+(int)scroll_y);
      if (mirror_keystrokes_object) mirror_keystrokes_object->event_key_down(key, alt, ctrl, shift);
    break;
    // enter
#ifdef _WIN32
    case 13:
#else
#if defined(_LINUX) || defined (__APPLE__)
    case -GLFW_KEY_ENTER:
#endif
#endif
      if (single_row) {
        vsx_string d;
        if (command_prefix.size()) d = command_prefix+" ";
        command_q_b.add_raw(d+lines[0]);
        parent->vsx_command_queue_b(this);
      } else {
        if ((size_t)caretx+(size_t)scroll_x > lines[carety+(int)scroll_y].size()) event_key_down(-35,false,false,false);
        while (c2 < carety+(int)scroll_y) { ++c2; ++it; ++itp; ++itlv; }
        ++it;
        ++itp;
        ++itlv;
        tempstring = lines[carety+(int)scroll_y].substr(caretx+(int)scroll_x,lines[carety+(int)scroll_y].size()-(caretx+(int)scroll_x));
        tempstring2 = lines[carety+(int)scroll_y].substr(0,caretx+(int)scroll_x);
        lines[carety+(int)scroll_y] = tempstring2;
        lines.insert(it,tempstring);
        lines_visible.insert(itlv,0);

        tempstring = lines_p[carety+(int)scroll_y].substr(caretx+(int)scroll_x,lines_p[carety+(int)scroll_y].size()-(caretx+(int)scroll_x));
        tempstring2 = lines_p[carety+(int)scroll_y].substr(0,caretx+(int)scroll_x);
        lines_p[carety+(int)scroll_y] = tempstring2;
        lines_p.insert(itp,tempstring);

#ifdef _WIN32
        event_key_down(-40,false,false,false);
        event_key_down(-36,false,false,false);
#else
#if defined(_LINUX) || defined (__APPLE__)
        event_key_down(-GLFW_KEY_DOWN,false,false,false);
        event_key_down(-GLFW_KEY_HOME,false,false,false);
#endif
#endif

        process_line(carety-1+(int)scroll_y);
        process_line(carety+(int)scroll_y);
      }
      if (mirror_keystrokes_object) mirror_keystrokes_object->event_key_down(key, alt, ctrl, shift);
    break;
    // esc
#ifdef _WIN32
    case 27:
#else
#ifdef _LINUX
    case -GLFW_KEY_ESC:
#endif
#endif
    // da rest:
      if (single_row) {
        command_q_b.add_raw("cancel");
        parent->vsx_command_queue_b(this);
      } else
      a_focus = k_focus = parent;
    break;
    default:
      if (key > 0) {
        if (allowed_chars.size()) {
          if (allowed_chars.find(key) == -1) {
            break;
          }
        }
        lines[carety+(int)scroll_y].insert(caretx+(int)scroll_x,(char)key);
        updates++;
        ++caretx;
        if ((size_t)caretx > lines[carety+(int)scroll_y].size()-(int)scroll_x)
        caretx = lines[carety+(int)scroll_y].size()-(int)scroll_x;
        int t_scroll_x = (int)scroll_x;
        if (caretx+(int)scroll_x > characters_width) ++scroll_x;
        //fix_pos();
        //cout << scroll_x - t_scroll_x << endl;
        caretx -= (int)scroll_x - t_scroll_x;
        process_line(carety+(int)scroll_y);
        if (mirror_keystrokes_object) mirror_keystrokes_object->event_key_down(key, alt, ctrl, shift);
      }
    // FROO
  }
  calculate_scroll_size();
  //process_lines();
  if (longest_line-characters_width <= 0) {
    scrollbar_pos_x = 0;
  } else {
    scrollbar_pos_x = (float)scroll_x/(longest_line-characters_width);
  }
  if (longest_y-characters_height <= 0) {
    scrollbar_pos_y = 0;
  } else {
    scrollbar_pos_y = (float)scroll_y/(longest_y-characters_height);
  }
  //printf("scroll_x: %f scroll_y: %f\n caretx: %d  carety: %d\n",scroll_x,scroll_y,caretx,carety);
  return false;
}

vsx_string vsx_widget_base_edit::get_string()
{
  return implode(lines,"\n");
}

vsx_string vsx_widget_base_edit::get_line(unsigned long line)
{
  if (line < lines.size()) {
    return lines[line];
  }
  return "";
}

//***************************************************************************************
//***************************************************************************************
//***************************************************************************************
//***************************************************************************************

vsx_widget_base_editor::vsx_widget_base_editor() {
  scrollbar_horiz = (vsx_widget_scrollbar*)add(new vsx_widget_scrollbar,"horiz");
  scrollbar_vert = (vsx_widget_scrollbar*)add(new vsx_widget_scrollbar,"vert");
  editor = (vsx_widget_base_edit*)add(new vsx_widget_base_edit,"edit");
  init_children();
  editor->size_from_parent = true;
  scrollbar_horiz->scroll_type = 0;
  scrollbar_vert->scroll_type = 1;
  scrollbar_horiz->control_value = &editor->scrollbar_pos_x;
  scrollbar_vert->control_value = &editor->scrollbar_pos_y;
  allow_move_y = allow_move_x = false;
}

void vsx_widget_base_editor::set_string(const vsx_string& str) {
  editor->set_string(str);
}
vsx_string vsx_widget_base_editor::get_string() {
  return editor->get_string();
}


void vsx_widget_base_editor::i_draw() {
  calc_pos();
  calc_size();
  //vsx_widget_panel::base_draw();
  float db15 = dragborder*2.5f;
  scrollbar_horiz->set_pos(vsx_vector(-size.x*0.5,-size.y*0.5));
  scrollbar_horiz->set_size(vsx_vector(target_size.x-db15, db15));
  scrollbar_horiz->scroll_window_size = editor->scroll_x_size;

  scrollbar_vert->set_pos(vsx_vector(size.x*0.5-db15,-size.y*0.5+db15));
  scrollbar_vert->set_size(vsx_vector(db15,target_size.y-scrollbar_horiz->size.y));
  scrollbar_vert->scroll_window_size = editor->scroll_y_size;
  editor->set_pos(vsx_vector(-scrollbar_vert->size.x*0.5f,scrollbar_horiz->size.y*0.5f));

  editor->target_size.x = target_size.x-scrollbar_vert->size.x;
  editor->target_size.y = target_size.y-scrollbar_horiz->size.y;
}



