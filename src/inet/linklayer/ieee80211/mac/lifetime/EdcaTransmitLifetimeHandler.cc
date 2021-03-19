//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
// 

#include "inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

EdcaTransmitLifetimeHandler::EdcaTransmitLifetimeHandler(simtime_t bkLifetime, simtime_t beLifetime, simtime_t viLifetime, simtime_t voLifetime)
{
    msduLifetime[0] = bkLifetime;
    msduLifetime[1] = beLifetime;
    msduLifetime[2] = viLifetime;
    msduLifetime[3] = voLifetime;
}

void EdcaTransmitLifetimeHandler::frameGotInProgess(const Ptr<const Ieee80211DataHeader>& header)
{
    if (header->getFragmentNumber() == 0)
        lifetimes[header->getSequenceNumber().get()] = simTime();
}

void EdcaTransmitLifetimeHandler::frameTransmitted(const Ptr<const Ieee80211DataHeader>& header)
{
    // don't care
}

bool EdcaTransmitLifetimeHandler::isLifetimeExpired(const Ptr<const Ieee80211DataHeader>& header)
{
    ASSERT(header->getType() == ST_DATA_WITH_QOS);
    AccessCategory ac = mapTidToAc(header->getTid());
    auto it = lifetimes.find(header->getSequenceNumber().get());
    if (it == lifetimes.end())
        throw cRuntimeError("There is no lifetime entry for frame = %s", header->getName());
    return (simTime() - it->second) >= msduLifetime[ac];
}

// TODO: Copy!!!!!!
AccessCategory EdcaTransmitLifetimeHandler::mapTidToAc(int tid)
{
    // standard static mapping (see "UP-to-AC mappings" table in the 802.11 spec.)
    switch (tid) {
        case 1: case 2: return AC_BK;
        case 0: case 3: return AC_BE;
        case 4: case 5: return AC_VI;
        case 6: case 7: return AC_VO;
        default: throw cRuntimeError("No mapping from TID=%d to AccessCategory (must be in the range 0..7)", tid);
    }
}


} /* namespace ieee80211 */
} /* namespace inet */

