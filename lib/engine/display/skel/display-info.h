
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         display-info.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider 
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : declaration of structs and classes used for communication
 *                          with the DisplayManagers
 *
 */

#ifndef __DISPLAY_INFO_H__
#define __DISPLAY_INFO_H__

#ifdef WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#ifdef None
#undef None
#endif
#ifdef BadRequest
#undef BadRequest
#endif
#endif
//namespace Ekiga {

/**
 * @addtogroup display
 * @{
 */

  /* Video modes */
  typedef enum {
  
    LOCAL_VIDEO, 
    REMOTE_VIDEO, 
    PIP,
    PIP_WINDOW,
    FULLSCREEN,
    UNSET
  } DisplayMode;
  
  /* Toggle operations for Fullscreen */
  typedef enum {
  
    ON,
    OFF,
    TOGGLE
  } FSToggle;
  
  /* Video Acceleration Status */
  typedef enum {
  
    NONE,
    REMOTE_ONLY,
    ALL,
    NO_VIDEO
  } HwAccelStatus;


  typedef struct {
    unsigned rx_fps;
    unsigned rx_width;
    unsigned rx_height;
    unsigned rx_frames;
    unsigned tx_fps;
    unsigned tx_width;
    unsigned tx_height;
    unsigned tx_frames;
  } DisplayStats;

  class DisplayInfo
  {
  public:
    DisplayInfo() {
      widget_info_set = false;
      x = 0;
      y = 0;
  #ifdef WIN32
      hwnd = 0;
  #else
      gc = 0;
      window = 0;
      xdisplay = NULL;
  #endif
  
      config_info_set = false;
      on_top = false;
      disable_hw_accel = false;
      allow_pip_sw_scaling = true;
      sw_scaling_algorithm = 0;
  
      display = UNSET;
      zoom = 0;
    };
    
    void operator= ( const DisplayInfo& rhs) {
  
    if (rhs.widget_info_set) {
        widget_info_set = rhs.widget_info_set;
        x = rhs.x;
        y = rhs.y;
  #ifdef WIN32
        hwnd = rhs.hwnd;
  #else
        gc = rhs.gc;
        window = rhs.window;
        xdisplay = rhs.xdisplay;
  #endif
      }
  
      if (rhs.config_info_set) {
        config_info_set = rhs.config_info_set;
        on_top = rhs.on_top;
        disable_hw_accel = rhs.disable_hw_accel;
        allow_pip_sw_scaling = rhs.allow_pip_sw_scaling;
        sw_scaling_algorithm =  rhs.sw_scaling_algorithm;
      }
      if (rhs.display != UNSET) display = rhs.display;
      if (rhs.zoom != 0) zoom = rhs.zoom;
    };
  
    bool widget_info_set;
    int x;
    int y;
              
  #ifdef WIN32
    HWND hwnd;
  #else
    GC gc;
    Window window;
    Display* xdisplay;
  #endif
  
    bool config_info_set;
    bool on_top;
    bool disable_hw_accel;
    bool allow_pip_sw_scaling;
    unsigned int sw_scaling_algorithm;
  
    DisplayMode display;
    unsigned int zoom;
  };

/**
 * @}
 */

//};

#endif