//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_DETAILEDIEEE80211RADIO_H_
#define __INET_DETAILEDIEEE80211RADIO_H_

#include "DetailedRadio.h"

class INET_API DetailedIeee80211Radio : public DetailedRadio
{
  protected:
    virtual DetailedRadioSignal *createSignal(cPacket *macFrame);

    virtual simtime_t packetDuration(cPacket *macFrame, double bitrate);

    virtual DetailedRadioSignal *createSignal(simtime_t_cref start, simtime_t_cref length, double power, double bitrate);
};

#endif
