//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECIPIENTQOSMACDATASERVICE_H
#define __INET_IRECIPIENTQOSMACDATASERVICE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecipientQosMacDataService
{
  public:
    struct ManagementFrameReceptionResult {
        std::vector<Packet *> completeFrames;
        // IEEE Std 802.11-2024, 10.5: a management body becomes available to
        // the coordination function only after the complete MMPDU is present.
        Ptr<const Ieee80211MgmtHeader> completeHeader;
        bool duplicate = false;
    };

    static simsignal_t packetDefragmentedSignal;
    static simsignal_t packetDeaggregatedSignal;

  public:
    virtual ~IRecipientQosMacDataService() {}

    virtual std::vector<Packet *> dataFrameReceived(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler) = 0;
    virtual std::vector<Packet *> controlFrameReceived(Packet *controlPacket, const Ptr<const Ieee80211MacHeader>& controlHeader, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler) = 0;
    virtual ManagementFrameReceptionResult managementFrameReceived(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader) = 0;
    virtual void resetBlockAckReordering(Tid tid, MacAddress originatorAddr) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
