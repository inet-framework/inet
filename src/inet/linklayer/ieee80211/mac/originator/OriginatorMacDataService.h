//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORMACDATASERVICE_H
#define __INET_ORIGINATORMACDATASERVICE_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentation.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

//
// 5.1.5 MAC data service architecture
//
class INET_API OriginatorMacDataService : public IOriginatorMacDataService, public cSimpleModule
{
  protected:
    // Figure 5-1—MAC data plane architecture
//    MsduRateLimiting *msduRateLimiting = nullptr;
    ISequenceNumberAssignment *sequenceNumberAssignment = nullptr;
//    MsduIntegrityAndProtection *msduIntegrityAndProtection = nullptr;
    IFragmentationPolicy *fragmentationPolicy = nullptr;
    IFragmentation *fragmentation = nullptr;
//    MpduEncryptionAndIntegrity *mpduEncryptionAndIntegrity = nullptr;
//    MpduHeaderPlusCrc *mpduHeaderPlusCrc = nullptr;

  protected:
    virtual void initialize() override;

    virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header);
    virtual std::vector<Packet *> *fragmentIfNeeded(Packet *frame);

  public:
    virtual ~OriginatorMacDataService();

    virtual std::vector<Packet *> *extractFramesToTransmit(queueing::IPacketQueue *pendingQueue) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

