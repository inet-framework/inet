//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee80211Mgmt, Ieee80211MgmtProtocolDissector);

void Ieee80211MgmtProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::ieee80211Mgmt);
    // TODO popHeader<Ieee80211MgmtHeader>
    callback.visitChunk(packet->peekData(), &Protocol::ieee80211Mgmt);
    packet->setFrontOffset(packet->getBackOffset());
    callback.endProtocolDataUnit(&Protocol::ieee80211Mgmt);
}

} // namespace inet

