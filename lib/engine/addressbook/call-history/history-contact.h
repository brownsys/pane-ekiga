
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
 *                         history-contact.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a call history entry
 *
 */

#ifndef __HISTORY_CONTACT_H__
#define __HISTORY_CONTACT_H__

#include <libxml/tree.h>

#include "services.h"
#include "contact-core.h"

namespace History {

  typedef enum {

    RECEIVED,
    PLACED,
    MISSED
  } call_type;

  class Contact: public Ekiga::Contact
  {
  public:

    Contact (Ekiga::ServiceCore &_core,
		xmlNodePtr _node);

    Contact (Ekiga::ServiceCore &_core,
	     const std::string _name,
	     const std::string _uri,
	     const std::string _status,
	     call_type c_t);

    ~Contact ();

    /* generic presentity api */

    const std::string get_name () const;

    const std::set<std::string> get_groups () const;

    bool populate_menu (Ekiga::MenuBuilder &builder);

    /* more specific api */

    xmlNodePtr get_node ();

    const std::map<std::string,std::string> get_uris () const;

    bool is_found (std::string test) const;

  private:

    Ekiga::ServiceCore &core;
    Ekiga::ContactCore *contact_core;

    xmlNodePtr node;
    std::string name;
    std::string uri;
    std::string status;
    std::set<std::string> groups;
  };
};

#endif