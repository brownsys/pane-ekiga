
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras
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
 *                         xcap-core.h  -  description
 *                         ------------------------------------
 *   begin                : Mon 29 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Interface of the XCAP support code
 *
 */

#ifndef __XCAP_CORE_H__
#define __XCAP_CORE_H__

#include "services.h"

#include "xcap-path.h"

namespace XCAP
{

  class CoreImpl; // yes, I'm pimpling!

  class Core: public Ekiga::Service
  {
  public:

    Core ();

    ~Core ();

    typedef enum { SUCCESS, ERROR } ResultType;

    void read (gmref_ptr<Path>,
	       sigc::slot<void,ResultType,std::string> callback);

    /* implementation of the Ekiga::Service api */

    const std::string get_name () const
    { return "xcap-core"; }

    const std::string get_description () const
    { return "Service providing XCAP support"; }

  private:

    CoreImpl* impl;
  };
};

#endif