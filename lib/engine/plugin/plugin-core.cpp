
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         plugin-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2009 by Julien Puydt
 *   copyright            : (c) 2009 by Julien Puydt
 *   description          : Implementation of the object responsible of plugin loads
 *
 */

#include "plugin-core.h"

#include <gmodule.h>
#include <iostream>

// Here is what a trivial plugin looks like :
//
// #include "kickstart.h"
// #include <iostream>
//
// extern "C" void
// ekiga_plugin_init (Ekiga::KickStart& /*kickstart*/)
// {
//   std::cout << "Hello!" << std::endl;
// }
//
// which can be compiled with :
// gcc -o hello.so hello.cpp -shared -export-dynamic -I$(PATH_TO_EKIGA_SOURCES)/lib/engine/framework `pkg-config --cflags sigc++-2.0`

void
plugin_init (Ekiga::KickStart& kickstart)
{
  std::cout << "Trying to load the hello plugin... ";

  GModule* plugin = g_module_open ("/tmp/hello.so", G_MODULE_BIND_LOCAL);

  if (plugin != 0) {

    std::cout << "loaded... ";
    gpointer init_func = NULL;

    if (g_module_symbol (plugin, "ekiga_plugin_init", &init_func)) {

      std::cout << "valid, running:" << std::endl;
      g_module_make_resident (plugin);
      ((void (*)(Ekiga::KickStart&))init_func) (kickstart);
    } else {

      std::cout << "invalid: " << g_module_error () << std::endl;
      g_module_close (plugin);
    }
  } else {

    std::cout << "failed to load the module" << std::endl;
  }
}