/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

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
 *                         audiooutput-manager-null.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a audiooutput core.
 *                          A audiooutput core manages AudioOutputManagers.
 *
 */

#include "audiooutput-manager-null.h"

#define DEVICE_TYPE   "NULL"
#define DEVICE_SOURCE "NULL"
#define DEVICE_DEVICE "NULL"

GMAudioOutputManager_null::GMAudioOutputManager_null (Ekiga::ServiceCore & _core)
:    core (_core), runtime (*(dynamic_cast<Ekiga::Runtime *> (_core.get ("runtime"))))
{
  current_state[Ekiga::primary].opened = false;
  current_state[Ekiga::secondary].opened = false;
}

void GMAudioOutputManager_null::get_devices(std::vector <Ekiga::AudioOutputDevice> & devices)
{
  Ekiga::AudioOutputDevice device;
  device.type   = DEVICE_TYPE;
  device.source = DEVICE_SOURCE;
  device.device = DEVICE_DEVICE;
  devices.push_back(device);
}


bool GMAudioOutputManager_null::set_device (Ekiga::AudioOutputPrimarySecondary primarySecondary, const Ekiga::AudioOutputDevice & device)
{
  if ( ( device.type   == DEVICE_TYPE ) &&
       ( device.source == DEVICE_SOURCE) &&
       ( device.device == DEVICE_DEVICE) ) {

    PTRACE(4, "GMAudioOutputManager_null\tSetting Device[" << primarySecondary << "] " << device.source << "/" <<  device.device);
    current_state[primarySecondary].device = device;
    return true;
  }
  return false;
}

bool GMAudioOutputManager_null::open (Ekiga::AudioOutputPrimarySecondary primarySecondary, unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  Ekiga::AudioOutputConfig audiooutput_config;

  PTRACE(4, "GMAudioOutputManager_null\tOpening Device[" << primarySecondary << "] " << current_state[primarySecondary].device.source << "/" <<  current_state[primarySecondary].device.device);
  PTRACE(4, "GMAudioOutputManager_null\tOpening Device with " << channels << "-" << samplerate << "/" << bits_per_sample);

  current_state[primarySecondary].channels        = channels;
  current_state[primarySecondary].samplerate      = samplerate;
  current_state[primarySecondary].bits_per_sample = bits_per_sample;
  current_state[primarySecondary].opened = true;

  adaptive_delay[primarySecondary].Restart();

  audiooutput_config.volume = 0;
  audiooutput_config.modifyable = false;

  runtime.run_in_main (sigc::bind (device_opened.make_slot (), primarySecondary, current_state[primarySecondary].device, audiooutput_config));

  return true;
}

void GMAudioOutputManager_null::close(Ekiga::AudioOutputPrimarySecondary primarySecondary)
{
  current_state[primarySecondary].opened = false;
  runtime.run_in_main (sigc::bind (device_closed.make_slot (), primarySecondary, current_state[primarySecondary].device));
}


bool GMAudioOutputManager_null::set_frame_data (Ekiga::AudioOutputPrimarySecondary primarySecondary, 
                     const char */*data*/, 
                     unsigned size,
		     unsigned & bytes_written)
{
  if (!current_state[primarySecondary].opened) {
    PTRACE(1, "GMAudioOutputManager_null\tTrying to get frame from closed device[" << primarySecondary << "]");
    return true;
  }

  bytes_written = size;

  adaptive_delay[primarySecondary].Delay(size * 8 / current_state[primarySecondary].bits_per_sample * 1000 / current_state[primarySecondary].samplerate);
  return true;
}

bool GMAudioOutputManager_null::has_device(const std::string & /*sink*/, const std::string & /*device_name*/, Ekiga::AudioOutputDevice & /*device*/)
{
  return false;
}