//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTQOSMACDATASERVICE_H
#define __INET_RECIPIENTQOSMACDATASERVICE_H

#include "inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h"
#include "inet/linklayer/ieee80211/mac/contract/IDefragmentation.h"
#include "inet/linklayer/ieee80211/mac/contract/IDuplicateRemoval.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IMsduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IReassembly.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQosMacDataService.h"

namespace inet {
namespace ieee80211 {

//
// Figure 5-1—MAC data plane architecture
//
class INET_API RecipientQosMacDataService : public IRecipientQosMacDataService, public cSimpleModule
{
  protected:
    IReassembly *basicReassembly = nullptr;

    IMpduDeaggregation *aMpduDeaggregation = nullptr;
//    MpduHeaderAndCrcValidation *mpduHeaderAndCrcValidation = nullptr;
//    Address1Filtering *address1Filtering = nullptr;
    IDuplicateRemoval *duplicateRemoval = nullptr;
//    MpduDecryptionAndIntegrity *mpduDecryptionAndIntegrity = nullptr;
    BlockAckReordering *blockAckReordering = nullptr;
    IDefragmentation *defragmentation = nullptr;
    IMsduDeaggregation *aMsduDeaggregation = nullptr;
//    RxMsduRateLimiting *rxMsduRateLimiting = nullptr;

  protected:
    virtual ~RecipientQosMacDataService();
    virtual void initialize() override;

    virtual Packet *defragment(std::vector<Packet *> completeFragments);
    virtual Packet *defragment(Packet *mgmtFragment);

  public:
    virtual std::vector<Packet *> dataFrameReceived(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler) override;
    virtual std::vector<Packet *> controlFrameReceived(Packet *controlPacket, const Ptr<const Ieee80211MacHeader>& controlHeader, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler) override;
    virtual std::vector<Packet *> managementFrameReceived(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

