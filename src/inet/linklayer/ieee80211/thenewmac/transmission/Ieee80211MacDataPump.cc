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

#include "inet/linklayer/ieee80211/thenewmac/transmission/Ieee80211MacDataPump.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacDataPump);

void Ieee80211MacDataPump::handleMessage(cMessage* msg)
{
}

void Ieee80211MacDataPump::initialize(int stage)
{
}

void Ieee80211MacDataPump::handleFromCsSignal(IIeee80211MacChannelState::ChannelState channelState)
{
    std::cout << "Test: " << channelState << " signal has arrived." << endl;
    // TODO: do_something(); Implement Data_Pump process
}

void Ieee80211MacDataPump::handleTxrqSignal()
{
    // [TxRequest]
}

} /* namespace ieee80211 */
} /* namespace inet */

