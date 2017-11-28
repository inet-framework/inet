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

#ifndef __INET_RECIPIENTQOSACKPOLICY_H
#define __INET_RECIPIENTQOSACKPOLICY_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQosAckPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API RecipientQosAckPolicy : public ModeSetListener, public IRecipientAckPolicy, public IRecipientQosAckPolicy
{
    protected:
        IQosRateSelection *rateSelection = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        simtime_t computeBasicBlockAckDuration(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const;
        simtime_t computeAckDuration(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const;

    public:
        virtual bool isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const override;
        virtual bool isBlockAckNeeded(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, RecipientBlockAckAgreement *agreement) const override;

        virtual simtime_t computeAckDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const override;
        virtual simtime_t computeBasicBlockAckDurationField(Packet *packet, const Ptr<const Ieee80211BasicBlockAckReq>& basicBlockAckReq) const override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RECIPIENTQOSACKPOLICY_H
