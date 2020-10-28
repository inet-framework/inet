//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/linklayer/ieee80211/mac/channelaccess/Edca.h"

namespace inet {
namespace ieee80211 {

Define_Module(Edca);

void Edca::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        numEdcafs = par("numEdcafs");
        edcafs = new Edcaf*[numEdcafs];
        for (int ac = 0; ac < numEdcafs; ac++) {
            edcafs[ac] = check_and_cast<Edcaf*>(getSubmodule("edcaf", ac));
        }
        mgmtAndNonQoSRecoveryProcedure = check_and_cast<NonQosRecoveryProcedure *>(getSubmodule("mgmtAndNonQoSRecoveryProcedure"));
    }
}

AccessCategory Edca::classifyFrame(const Ptr<const Ieee80211DataHeader>& header)
{
    return mapTidToAc(header->getTid());
}

AccessCategory Edca::mapTidToAc(Tid tid)
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

Edcaf* Edca::getChannelOwner()
{
    for (int ac = 0; ac < numEdcafs; ac++)
        if (edcafs[ac]->isOwning())
            return edcafs[ac];
    return nullptr;
}

std::vector<Edcaf*> Edca::getInternallyCollidedEdcafs()
{
    std::vector<Edcaf*> collidedEdcafs;
    for (int ac = 0; ac < numEdcafs; ac++)
        if (edcafs[ac]->isInternalCollision())
            collidedEdcafs.push_back(edcafs[ac]);
    return collidedEdcafs;
}

void Edca::requestChannelAccess(AccessCategory ac, IChannelAccess::ICallback* callback)
{
    edcafs[ac]->requestChannel(callback);
}

void Edca::releaseChannelAccess(AccessCategory ac, IChannelAccess::ICallback* callback)
{
    edcafs[ac]->releaseChannel(callback);
}

Edca::~Edca()
{
#if OMNETPP_BUILDNUM < 1505   //OMNETPP_VERSION < 0x0600    // 6.0 pre9
    for (int i = 0; i < numEdcafs; i++)
        edcafs[i]->deleteModule();
#endif
    delete[] edcafs;
}

} // namespace ieee80211
} // namespace inet
