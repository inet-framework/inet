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

#include "inet/linklayer/ieee80211/thenewmac/reception/Ieee80211MacChannelState.h"
#include "inet/linklayer/ieee80211/thenewmac/transmission/Ieee80211MacDataPump.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacChannelState);

void Ieee80211MacChannelState::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
    }
    else if (stage == INITSTAGE_LAST)
    {
        emitChannelStateSignalToDataPump(CHANNEL_STATE_BUSY);
    }
}

void Ieee80211MacChannelState::handleMessage(cMessage* msg)
{
}

void Ieee80211MacChannelState::emitChannelStateSignalToDataPump(ChannelState state) const
{
//    const cGate *gate = this->gate("toDataPump")->getPathEndGate();
//    IIeee80211MacDataPump *dataPump = dynamic_cast<IIeee80211MacDataPump *>(gate->getOwnerModule());
//    dataPump->handleFromCsSignal(CHANNEL_STATE_BUSY);
}

} /* namespace ieee80211 */
} /* namespace inet */

