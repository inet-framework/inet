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
#include "inet/linklayer/ieee80211/mac/contract/IRecipientMacDataService.h"
#include "inet/linklayer/ieee80211/mac/fragmentation/BasicReassembly.h"

namespace inet {
namespace ieee80211 {

//
// 5.1.5 MAC data service architecture
//
class INET_API RecipientMacDataService : public cSimpleModule, public IRecipientMacDataService
{
    protected:
        typedef std::vector<Ieee80211DataOrMgmtFrame *> Fragments;

    protected:
        BasicReassembly *basicReassembly = nullptr; // FIXME: use Defragmentation

        // MpduHeaderAndCrcValidation *mpduHeaderAndCrcValidation = nullptr;
        // Address1Filtering *address1Filtering = nullptr;
        IDuplicateRemoval *duplicateRemoval = nullptr;
        // MpduDecryptionAndIntegrity *mpduDecryptionAndIntegrity = nullptr;
        IDefragmentation *defragmentation = nullptr;
        // RxMsduRateLimiting *rxMsduRateLimiting = nullptr;

    protected:
        virtual void initialize() override;
        Ieee80211DataOrMgmtFrame* defragment(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame);
        virtual std::vector<Ieee80211Frame *> dataOrMgmtFrameReceived(Ieee80211DataOrMgmtFrame *frame);

    public:
        virtual ~RecipientMacDataService();

        virtual std::vector<Ieee80211Frame *> dataFrameReceived(Ieee80211DataFrame *dataFrame) override;
        virtual std::vector<Ieee80211Frame *> managementFrameReceived(Ieee80211ManagementFrame *mgmtFrame) override;
        virtual std::vector<Ieee80211Frame *> controlFrameReceived(Ieee80211Frame *controlFrame) override;

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_RECIPIENTMACDATASERVICE_H
