
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
 *                         opal-call.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a call handled by 
 *                          Opal.
 *
 */


#include <cctype>
#include <algorithm>

#include "opal-call.h"
#include "call.h"

#include "ekiga.h" // FIXME Runtime

using namespace Opal;


Call::Call (OpalManager & _manager, Ekiga::ServiceCore & _core) 
: OpalCall (_manager), Ekiga::Call (), core (_core)  
{
  re_a_bytes = tr_a_bytes = re_v_bytes = tr_v_bytes = 0.0;
  last_tick = PTime ();
}


void Call::hangup ()
{
  if (!is_outgoing () && !IsEstablished ()) 
    Clear (OpalConnection::EndedByAnswerDenied);
  else
    Clear ();
}


void Call::answer ()
{
  PThread::Create (PCREATE_NOTIFIER (OnAnswer), NULL, 
                   PThread::AutoDeleteThread, PThread::NormalPriority, 
                   "Opal::Call Answer Thread");
}


void Call::transfer (std::string uri)
{
  //FIXME
  std::cout << "Should transfer to " << uri << std::endl << std::flush;
}


void Call::toggle_hold ()
{
  PSafePtr<OpalConnection> connection = NULL;
  
  bool on_hold = false;
  int i = 0;

  do {

    connection = GetConnection (i);
    i++;
  }  while (PIsDescendant(&(*connection), OpalPCSSConnection)); 

  if (connection) {

    on_hold = connection->IsConnectionOnHold ();
    if (!on_hold) 
      connection->HoldConnection ();
    else 
      connection->RetrieveConnection ();
  }
}


void Call::toggle_stream_pause (StreamType type)
{
  Ekiga::Runtime *runtime = dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"));
  OpalMediaStreamPtr stream = NULL;
  PSafePtr<OpalConnection> connection = NULL;
  PString codec_name;
  std::string stream_name;

  bool paused = false;

  int i = 0;

  do {

    connection = GetConnection (i);
    i++;
  }  while (PIsDescendant(&(*connection), OpalPCSSConnection)); 

  if (connection != NULL) {

    stream = connection->GetMediaStream ((type == Audio)?OpalMediaFormat::DefaultAudioSessionID:OpalMediaFormat::DefaultVideoSessionID, false); 
    if (stream != NULL) {

      stream_name = std::string ((const char *) stream->GetMediaFormat ().GetEncodingName ());
      std::transform (stream_name.begin (), stream_name.end (), stream_name.begin (), (int (*) (int)) toupper);
      paused = stream->IsPaused ();
      stream->SetPaused (!paused);

      if (paused)
        runtime->run_in_main (sigc::bind (stream_resumed, stream_name, type));
      else
        runtime->run_in_main (sigc::bind (stream_paused, stream_name, type));
    }
  }
}


void Call::send_dtmf (const char dtmf)
{
  PSafePtr<OpalConnection> connection = NULL;

  int i = 0;

  do {

    connection = GetConnection (i);
    i++;
  }  while (PIsDescendant(&(*connection), OpalPCSSConnection)); 

  if (connection != NULL) 
    connection->SendUserInputTone (dtmf, 180);
}


std::string Call::get_id ()
{
  return GetToken ();
}


std::string Call::get_remote_party_name ()
{
  return remote_party_name;
}


std::string Call::get_remote_application ()
{
  return remote_application;
}


std::string Call::get_remote_uri ()
{
  return remote_uri;
}


std::string Call::get_call_duration ()
{
  std::stringstream duration;
  PTimeInterval t = PTime () - start_time;

  duration << setfill ('0') << setw (2) << t.GetHours () << ":";
  duration << setfill ('0') << setw (2) << (t.GetMinutes () % 60) << ":";
  duration << setfill ('0') << setw (2) << (t.GetSeconds () % 60);

  return duration.str (); 
}


void Call::parse_info (OpalConnection & connection)
{
  char special_chars [] = "([@";
  int i = 0;
  std::string::size_type idx;
  std::string party_name;
  std::string app;
  std::string uri;

  if (!PIsDescendant(&connection, OpalPCSSConnection)) {

    party_name = (const char *) connection.GetRemotePartyName ();
    app = (const char *) connection.GetRemoteApplication (); 
    uri = (const char *) connection.GetRemotePartyCallbackURL ();
    start_time = connection.GetConnectionStartTime ();

    if (!party_name.empty ())
      remote_party_name = party_name;
    if (!app.empty ())
      remote_application = app;
    if (!uri.empty ())
      remote_uri = uri;

    while (i < 3) {

      idx = remote_party_name.find_first_of (special_chars [i]);
      if (idx != std::string::npos)
        remote_party_name = remote_party_name.substr (0, idx);

      idx = remote_application.find_first_of (special_chars [i]);
      if (idx != std::string::npos)
        remote_application = remote_application.substr (0, idx);

      i++;
    }
  }
}


PBoolean Call::OnEstablished (OpalConnection & connection)
{
  Ekiga::Runtime *runtime = dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"));

  if (!PIsDescendant(&connection, OpalPCSSConnection)) {

    parse_info (connection);
    runtime->run_in_main (established.make_slot ());
  }
    
  return OpalCall::OnEstablished (connection);
}


void Call::OnReleased (OpalConnection & connection)
{
  std::string reason;

  /** TODO
   * the Call could be destroyed before the signal callback has been executed
   * maybe create a copy constructor
   */
  Ekiga::Runtime *runtime = dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"));

  if (!PIsDescendant(&connection, OpalPCSSConnection)) {

    if (!IsEstablished () 
        && !is_outgoing ()
        && connection.GetCallEndReason () != OpalConnection::EndedByAnswerDenied) {

      runtime->run_in_main (missed.make_slot ());
    }
    else {

      switch (connection.GetCallEndReason ()) {

      case OpalConnection::EndedByLocalUser :
        reason = _("Local user cleared the call");
        break;
      case OpalConnection::EndedByNoAccept :
        reason = _("Local user rejected the call");
        break;
      case OpalConnection::EndedByAnswerDenied :
        reason = _("Local user rejected the call");
        break;
      case OpalConnection::EndedByRemoteUser :
        reason = _("Remote user cleared the call");
        break;
      case OpalConnection::EndedByRefusal :
        reason = _("Remote user rejected the call");
        break;
      case OpalConnection::EndedByCallerAbort :
        reason = _("Remote user has stopped calling");
        break;
      case OpalConnection::EndedByTransportFail :
        reason = _("Abnormal call termination");
        break;
      case OpalConnection::EndedByConnectFail :
        reason = _("Could not connect to remote host");
        break;
      case OpalConnection::EndedByGatekeeper :
        reason = _("The Gatekeeper cleared the call");
        break;
      case OpalConnection::EndedByNoUser :
        reason = _("User not found");
        break;
      case OpalConnection::EndedByNoBandwidth :
        reason = _("Insufficient bandwidth");
        break;
      case OpalConnection::EndedByCapabilityExchange :
        reason = _("No common codec");
        break;
      case OpalConnection::EndedByCallForwarded :
        reason = _("Call forwarded");
        break;
      case OpalConnection::EndedBySecurityDenial :
        reason = _("Security check failed");
        break;
      case OpalConnection::EndedByLocalBusy :
        reason = _("Local user is busy");
        break;
      case OpalConnection::EndedByLocalCongestion :
        reason = _("Congested link to remote party");
        break;
      case OpalConnection::EndedByRemoteBusy :
        reason = _("Remote user is busy");
        break;
      case OpalConnection::EndedByRemoteCongestion :
        reason = _("Congested link to remote party");
        break;
      case OpalConnection::EndedByHostOffline :
        reason = _("Remote host is offline");
        break;
      case OpalConnection::EndedByTemporaryFailure :
      case OpalConnection::EndedByUnreachable :
      case OpalConnection::EndedByNoEndPoint :
      case OpalConnection::EndedByNoAnswer :
        if (connection.IsOriginating ())
          reason = _("Remote user is not available");
        else
          reason = _("Local user is not available");
        break;

      case OpalConnection::EndedByQ931Cause:
      case OpalConnection::EndedByDurationLimit:
      case OpalConnection::EndedByInvalidConferenceID:
      case OpalConnection::EndedByNoDialTone:
      case OpalConnection::EndedByNoRingBackTone:
      case OpalConnection::EndedByOutOfService:
      case OpalConnection::EndedByAcceptingCallWaiting:
      case OpalConnection::EndedWithQ931Code:
      case OpalConnection::NumCallEndReasons:
      default :
        reason = _("Call completed");
      }

      runtime->run_in_main (sigc::bind (cleared.make_slot (), reason));
    }
  }

  OpalCall::OnReleased (connection);
}


OpalConnection::AnswerCallResponse Call::OnAnswerCall (OpalConnection & connection, const PString & caller)
{
  remote_party_name = (const char *) caller;

  parse_info (connection);

  outgoing = false;

  return OpalCall::OnAnswerCall (connection, caller);
}


PBoolean Call::OnSetUp (OpalConnection & connection)
{
  Ekiga::Runtime *runtime = dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"));

  parse_info (connection);

  outgoing = PIsDescendant(&connection, OpalPCSSConnection);

  runtime->run_in_main (setup.make_slot ());

  return OpalCall::OnSetUp (connection);
}


void Call::OnHold (bool on_hold)
{
  Ekiga::Runtime *runtime = dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"));

  if (on_hold)
    runtime->run_in_main (held.make_slot ());
  else
    runtime->run_in_main (retrieved.make_slot ());
}


void Call::OnOpenMediaStream (OpalMediaStream & stream)
{
  Ekiga::Runtime *runtime = dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"));

  StreamType type = (stream.GetSessionID () == OpalMediaFormat::DefaultAudioSessionID) ? Audio : Video;
  bool is_transmitting = false;
  std::string stream_name;

  stream_name = std::string ((const char *) stream.GetMediaFormat ().GetEncodingName ());
  std::transform (stream_name.begin (), stream_name.end (), stream_name.begin (), (int (*) (int)) toupper);
  is_transmitting = !stream.IsSource (); 

  runtime->run_in_main (sigc::bind (stream_opened, stream_name, type, is_transmitting));
}


void Call::OnClosedMediaStream (OpalMediaStream & stream)
{
  Ekiga::Runtime *runtime = dynamic_cast<Ekiga::Runtime *> (core.get ("runtime"));

  StreamType type = (stream.GetSessionID () == OpalMediaFormat::DefaultAudioSessionID) ? Audio : Video;
  bool is_transmitting = false;
  std::string stream_name;

  stream_name = std::string ((const char *) stream.GetMediaFormat ().GetEncodingName ());
  std::transform (stream_name.begin (), stream_name.end (), stream_name.begin (), (int (*) (int)) toupper);
  is_transmitting = !stream.IsSource (); 

  runtime->run_in_main (sigc::bind (stream_closed, stream_name, type, is_transmitting));
}


void Call::OnRTPStatistics (const OpalConnection & /*connection*/, const RTP_Session & session)
{
  double octets_received = 0.0;
  double octets_sent = 0.0;

  PTimeInterval t = PTime () - last_tick;
  unsigned elapsed_seconds = max ((unsigned long) t.GetMilliSeconds (), (unsigned long) 1);

  octets_received = session.GetOctetsReceived ();
  octets_sent = session.GetOctetsSent ();

  if (session.GetSessionID () == OpalMediaFormat::DefaultAudioSessionID) {

    re_a_bw = max ((octets_received - re_a_bytes) / elapsed_seconds, 0.0);
    tr_a_bw = max ((octets_sent - tr_a_bytes) / elapsed_seconds, 0.0);

    jitter = session.GetJitterBufferSize () / max ((unsigned) session.GetJitterTimeUnits (), (unsigned) 8);
  }
  else {

    re_v_bw = max ((octets_received - re_v_bytes) / elapsed_seconds, 0.0);
    tr_v_bw = max ((octets_sent - tr_v_bytes) / elapsed_seconds, 0.0);
  }

  lost_packets = session.GetPacketsLost () / max (session.GetPacketsReceived (), (DWORD) 1);
  late_packets = session.GetPacketsTooLate () / max (session.GetPacketsReceived (), (DWORD) 1);
  out_of_order_packets = session.GetPacketsOutOfOrder () / max (session.GetPacketsReceived (), (DWORD) 1);

  re_a_bytes = octets_received; 
  tr_a_bytes = octets_sent;
  last_tick = PTime ();
}


void Call::OnAnswer (PThread &, INT /*param*/)
{
  PSafePtr<OpalConnection> connection = NULL;
  PSafePtr<OpalPCSSConnection> conn = NULL;

  int i = 0;

  if (!is_outgoing () && !IsEstablished ()) {

    do {

      connection = GetConnection (i);
      i++;
    }  while (!PIsDescendant(&(*connection), OpalPCSSConnection)); 

    if (PIsDescendant(&(*connection), OpalPCSSConnection))
      PDownCast (OpalPCSSConnection, &(*connection))->AcceptIncoming ();
  }
}

