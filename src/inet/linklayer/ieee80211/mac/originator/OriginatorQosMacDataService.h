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

#ifndef __INET_ORIGINATORQOSMACDATASERVICE_H
#define __INET_ORIGINATORQOSMACDATASERVICE_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentation.h"
#include "inet/linklayer/ieee80211/mac/contract/IFragmentationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IMpduAggregationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IMsduAggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IMsduAggregationPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorMacDataService.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorQosMacDataService : public IOriginatorMacDataService, public cSimpleModule
{
    protected:
        // Figure 5-1—MAC data plane architecture
        // MsduRateLimiting *msduRateLimiting = nullptr;
        ISequenceNumberAssignment *sequenceNumberAssigment = nullptr;
        // MsduIntegrityAndProtection *msduIntegrityAndProtection = nullptr;
        // MpduEncryptionAndIntegrity *mpduEncryptionAndIntegrity = nullptr;
        // MpduHeaderPlusCrc *mpduHeaderPlusCrc = nullptr;
        IFragmentationPolicy *fragmentationPolicy = nullptr;
        IFragmentation *fragmentation = nullptr;
        IMsduAggregationPolicy *aMsduAggregationPolicy = nullptr;
        IMsduAggregation *aMsduAggregation = nullptr;
        // PsDeferQueueing *psDeferQueueing = nullptr;
        IMpduAggregationPolicy *aMpduAggregationPolicy = nullptr;
        IMpduAggregation *aMpduAggregation = nullptr;

    protected:
        virtual void initialize() override;

        virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header);
        virtual std::vector<Packet *> *fragmentIfNeeded(Packet *frame);
        virtual Packet *aMsduAggregateIfNeeded(queueing::IPacketQueue *pendingQueue);
        virtual Packet *aMpduAggregateIfNeeded(std::vector<Packet *> *fragments);

    public:
        virtual ~OriginatorQosMacDataService();

        virtual std::vector<Packet *> *extractFramesToTransmit(queueing::IPacketQueue *pendingQueue) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_ORIGINATORQOSMACDATASERVICE_H
