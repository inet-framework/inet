//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/channelaccess/Edca.h"

namespace inet {
namespace ieee80211 {

Define_Module(Edca);

void Edca::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {
        numEdcafs = par("numEdcafs");
        edcafs = new Edcaf *[numEdcafs];
        for (int ac = 0; ac < numEdcafs; ac++) {
            edcafs[ac] = check_and_cast<Edcaf *>(getSubmodule("edcaf", ac));
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

Edcaf *Edca::getChannelOwner()
{
    for (int ac = 0; ac < numEdcafs; ac++)
        if (edcafs[ac]->isOwning())
            return edcafs[ac];
    return nullptr;
}

std::vector<Edcaf *> Edca::getInternallyCollidedEdcafs()
{
    std::vector<Edcaf *> collidedEdcafs;
    for (int ac = 0; ac < numEdcafs; ac++)
        if (edcafs[ac]->isInternalCollision())
            collidedEdcafs.push_back(edcafs[ac]);
    return collidedEdcafs;
}

void Edca::requestChannelAccess(AccessCategory ac, IChannelAccess::ICallback *callback)
{
    edcafs[ac]->requestChannel(callback);
}

void Edca::releaseChannelAccess(AccessCategory ac, IChannelAccess::ICallback *callback)
{
    edcafs[ac]->releaseChannel(callback);
}

Edca::~Edca()
{
    delete[] edcafs;
}

} // namespace ieee80211
} // namespace inet

