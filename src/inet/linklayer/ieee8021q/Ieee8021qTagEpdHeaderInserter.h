//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021QTAGEPDHEADERINSERTER_H
#define __INET_IEEE8021QTAGEPDHEADERINSERTER_H

#include "inet/common/Protocol.h"
#include "inet/linklayer/common/VlanTagType.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API Ieee8021qTagEpdHeaderInserter : public PacketFlowBase
{
  protected:
    VlanTagType vlanTagType = VLAN_TAG_TYPE_C;
    const Protocol *qtagProtocol = nullptr;
    const Protocol *nextProtocol = nullptr;
    int defaultVlanId = -1;
    int defaultPcp = -1;
    int defaultDropEligible = -1;
    bool userPriorityReqMappingEnabled = true;
    bool removeUserPriorityReq = false;
    bool pcpIndMappingEnabled = true;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;

    virtual void mapUserPriorityReqToPcpReq(Packet *packet);
    virtual void mapPcpIndToUserPriorityInd(Packet *packet);
};

} // namespace inet

#endif

