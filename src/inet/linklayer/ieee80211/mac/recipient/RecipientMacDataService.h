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
        typedef std::vector<Ieee80211DataOrMgmtHeader*> Fragments;

    protected:
        IReassembly *basicReassembly = nullptr; // FIXME: use Defragmentation

        // MpduHeaderAndCrcValidation *mpduHeaderAndCrcValidation = nullptr;
        // Address1Filtering *address1Filtering = nullptr;
        IDuplicateRemoval *duplicateRemoval = nullptr;
        // MpduDecryptionAndIntegrity *mpduDecryptionAndIntegrity = nullptr;
        IDefragmentation *defragmentation = nullptr;
        // RxMsduRateLimiting *rxMsduRateLimiting = nullptr;

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

#endif // __INET_RECIPIENTMACDATASERVICE_H
