
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
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
 *                         gtk-frontend.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : code to hook a gtk+ user interface to
 *                          the main program
 *
 */

#include <iostream>

#include <gtk/gtk.h>

#include "config.h"

#include "gtk-frontend.h"

#include "call-core.h"
#include "contact-core.h"
#include "presence-core.h"
#include "addressbook-window.h"
#include "chat-window.h"
#include "roster-view-gtk.h"

#include "gmwindow.h"


GtkFrontend::GtkFrontend (Ekiga::ServiceCore &core)
{ 
  sigc::connection conn;

  Ekiga::PresenceCore *presence_core = NULL;
  Ekiga::ContactCore *contact_core = NULL;
  Ekiga::CallCore *call_core = NULL;

  contact_core = dynamic_cast<Ekiga::ContactCore *>(core.get ("contact-core"));
  presence_core = dynamic_cast<Ekiga::PresenceCore *>(core.get ("presence-core"));
  call_core = dynamic_cast<Ekiga::CallCore *>(core.get ("call-core"));

  roster_view = roster_view_gtk_new (*presence_core);
  addressbook_window = 
    addressbook_window_new_with_key (*contact_core, "/apps/ekiga/general/user_interface/addressbook_window");
  chat_window = 
    chat_window_new_with_key (core, "/apps/ekiga/general/user_interface/chat_window");

  conn = call_core->new_chat.connect (sigc::mem_fun (this, &GtkFrontend::on_new_chat));
  connections.push_back (conn);
}


GtkFrontend::~GtkFrontend ()
{
  for (std::vector<sigc::connection>::iterator iter = connections.begin () ;
       iter != connections.end ();
       iter++)
    iter->disconnect ();
}


const std::string GtkFrontend::get_name () const
{ 
  return "gtk-frontend"; 
}


const std::string GtkFrontend::get_description () const
{ 
  return "\tGtk+ frontend support"; 
}


const GtkWidget *GtkFrontend::get_roster_view () const
{ 
  return roster_view; 
}


const GtkWidget *GtkFrontend::get_addressbook_window () const
{ 
  return addressbook_window; 
}


const GtkWidget *GtkFrontend::get_chat_window () const
{ 
  return chat_window; 
}


void GtkFrontend::on_new_chat (Ekiga::CallManager & /*manager*/,
                               std::string name,
                               std::string uri)
{
  chat_window_add_page (CHAT_WINDOW (chat_window), name, uri);
  gtk_widget_show_all (GTK_WIDGET (chat_window));
}


bool
gtk_frontend_init (Ekiga::ServiceCore &core,
		   int * /*argc*/,
		   char ** /*argv*/[])
{
  GtkFrontend *gtk_frontend = new GtkFrontend (core);
  core.add (*gtk_frontend);

  return true;
}