
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
 *                         opal-call.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a call handled by 
 *                          Opal.
 *
 */

#include <opal/buildopts.h>
#include <ptbuildopts.h>

#include <ptlib.h>
#include <opal/manager.h>

#include "services.h"
#include "call.h"

#ifndef __OPAL_CALL_H__
#define __OPAL_CALL_H__

class GMManager;

namespace Opal {

  class Call
    : public OpalCall,
      public Ekiga::Call
    {

  public:

      Call (OpalManager & _manager, Ekiga::ServiceCore & _core);

      ~Call () { };

      /* 
       * Call Management
       */

      /** Hangup the call
       */
      void hangup (); 

      /** Answer an incoming call
       */
      void answer (); 

      /** Transfer the call to the specified uri
       * @param: uri: where to transfer the call
       */
      void transfer (std::string uri);

      /** Put the call on hold or retrieve it
       */
      void toggle_hold ();

      /** Toggle stream transmission (if any)
       * @param type the stream type
       */
      void toggle_stream_pause (StreamType type);

      /** Send the given DTMF
       * @param the dtmf (one char)
       */
      void send_dtmf (const char dtmf);


      /* 
       * Call Information
       */

      /** Return the call id
       * @return: the call id
       */
      std::string get_id ();
      
      /** Return the remote party name
       * @return: the remote party name
       */
      std::string get_remote_party_name ();

      /** Return the remote application
       * @return: the remote application
       */
      std::string get_remote_application ();


      /** Return the remote callback uri
       * @return: the remote uri
       */
      std::string get_remote_uri ();


      /** Return the call duration
       * @return: the current call duration
       */
      std::string get_call_duration ();


  public:

      /*
       * Opal Callbacks 
       */
      void OnHold (bool on_hold);

      void OnOpenMediaStream (OpalMediaStream & stream);

      void OnClosedMediaStream (OpalMediaStream & stream);

      void OnRTPStatistics (const OpalConnection & connection, const RTP_Session & session);

  private:

      PBoolean OnEstablished (OpalConnection & connection);

      void OnReleased (OpalConnection & connection);

      OpalConnection::AnswerCallResponse OnAnswerCall (OpalConnection & connection, const PString & caller);

      PBoolean OnSetUp (OpalConnection & connection);

      PDECLARE_NOTIFIER (PThread, Opal::Call, OnAnswer);

      
      /*
       * Helper methods
       */
      void parse_info (OpalConnection & connection);


      /*
       * Variables
       */
      Ekiga::ServiceCore & core;
      double re_a_bytes;
      double tr_a_bytes;
      double re_v_bytes;
      double tr_v_bytes;
      PTime last_tick;
      PTime start_time;
    };
};

#endif