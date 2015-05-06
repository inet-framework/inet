//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "inet/linklayer/ieee80211/thenewmac/reception/Ieee80211MacReception.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacReception);

void Ieee80211MacReception::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
//        channelState = dynamic_cast<IIeee80211ChannelState *>(getSubmodule("channelState"));
//        mpduFilter = dynamic_cast<IIeee80211MpduFilter *>(getSubmodule("mpduFilter"));
//        mpduValidator = dynamic_cast<IIeee80211MpduValidator *>(getSubmodule("mpduValidator"));
//        defragment = dynamic_cast<IIeee80211Defragment *>(getSubmodule("defragment"));
    }
}

void Ieee80211MacReception::handleMessage(cMessage* msg)
{
}

} /* namespace ieee80211 */
} /* namespace inet */

