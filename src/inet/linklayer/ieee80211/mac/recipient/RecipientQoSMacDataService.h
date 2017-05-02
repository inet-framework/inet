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

#ifndef __INET_RECIPIENTQOSMACDATASERVICE_H
#define __INET_RECIPIENTQOSMACDATASERVICE_H

#include "inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h"
#include "inet/linklayer/ieee80211/mac/contract/IDefragmentation.h"
#include "inet/linklayer/ieee80211/mac/contract/IDuplicateRemoval.h"
#include "inet/linklayer/ieee80211/mac/contract/IMsduDeaggregation.h"
#include "inet/linklayer/ieee80211/mac/contract/IReassembly.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQoSMacDataService.h"

namespace inet {
namespace ieee80211 {

//
// Figure 5-1—MAC data plane architecture
//
class INET_API RecipientQoSMacDataService : public IRecipientQoSMacDataService, public cSimpleModule
{
    protected:
        IReassembly *basicReassembly = nullptr;

        // AMpduDeaggregation *aMpduDeaggregation = nullptr;
        // MpduHeaderAndCrcValidation *mpduHeaderAndCrcValidation = nullptr;
        // Address1Filtering *address1Filtering = nullptr;
        IDuplicateRemoval *duplicateRemoval = nullptr;
        // MpduDecryptionAndIntegrity *mpduDecryptionAndIntegrity = nullptr;
        BlockAckReordering *blockAckReordering = nullptr;
        IDefragmentation *defragmentation = nullptr;
        IMsduDeaggregation *aMsduDeaggregation = nullptr;
        // RxMsduRateLimiting *rxMsduRateLimiting = nullptr;

    protected:
        virtual ~RecipientQoSMacDataService();
        virtual void initialize() override;

        virtual Packet *defragment(std::vector<Packet *> completeFragments);
        virtual Packet *defragment(Packet *mgmtFragment);

    public:
        virtual std::vector<Packet *> dataFrameReceived(Packet *dataPacket, const Ptr<Ieee80211DataFrame>& dataFrame, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler) override;
        virtual std::vector<Packet *> controlFrameReceived(Packet *controlPacket, const Ptr<Ieee80211Frame>& controlFrame, IRecipientBlockAckAgreementHandler *blockAckAgreementHandler) override;
        virtual std::vector<Packet *> managementFrameReceived(Packet *mgmtPacket, const Ptr<Ieee80211ManagementHeader>& mgmtFrame) override;

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_RECIPIENTQOSMACDATASERVICE_H
