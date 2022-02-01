//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTMACDATASERVICE_H
#define __INET_RECIPIENTMACDATASERVICE_H

#include "inet/linklayer/ieee80211/mac/contract/IDefragmentation.h"
#include "inet/linklayer/ieee80211/mac/contract/IDuplicateRemoval.h"
#include "inet/linklayer/ieee80211/mac/contract/IReassembly.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientMacDataService.h"

namespace inet {
namespace ieee80211 {

//
// 5.1.5 MAC data service architecture
//
class INET_API RecipientMacDataService : public cSimpleModule, public IRecipientMacDataService
{
  protected:
    typedef std::vector<Ieee80211DataOrMgmtHeader *> Fragments;

  protected:
    IReassembly *basicReassembly = nullptr; // FIXME use Defragmentation

//    MpduHeaderAndCrcValidation *mpduHeaderAndCrcValidation = nullptr;
//    Address1Filtering *address1Filtering = nullptr;
    IDuplicateRemoval *duplicateRemoval = nullptr;
//    MpduDecryptionAndIntegrity *mpduDecryptionAndIntegrity = nullptr;
    IDefragmentation *defragmentation = nullptr;
//    RxMsduRateLimiting *rxMsduRateLimiting = nullptr;

  protected:
    virtual void initialize() override;
    virtual Packet *defragment(Packet *dataOrMgmtFrame);
    virtual std::vector<Packet *> dataOrMgmtFrameReceived(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header);

  public:
    virtual ~RecipientMacDataService();

    virtual std::vector<Packet *> dataFrameReceived(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader) override;
    virtual std::vector<Packet *> managementFrameReceived(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader) override;
    virtual std::vector<Packet *> controlFrameReceived(Packet *controlPacket, const Ptr<const Ieee80211MacHeader>& controlHeader) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

