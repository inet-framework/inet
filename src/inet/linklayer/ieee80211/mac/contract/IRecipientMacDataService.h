//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECIPIENTMACDATASERVICE_H
#define __INET_IRECIPIENTMACDATASERVICE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecipientMacDataService
{
  public:
    static simsignal_t packetDefragmentedSignal;

  public:
    virtual ~IRecipientMacDataService() {}

    virtual std::vector<Packet *> dataFrameReceived(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader) = 0;
    virtual std::vector<Packet *> controlFrameReceived(Packet *controlPacket, const Ptr<const Ieee80211MacHeader>& controlHeader) = 0;
    virtual std::vector<Packet *> managementFrameReceived(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

