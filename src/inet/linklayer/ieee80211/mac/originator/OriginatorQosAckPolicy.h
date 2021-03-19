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

#ifndef __INET_ORIGINATORQOSACKPOLICY_H
#define __INET_ORIGINATORQOSACKPOLICY_H

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorQoSAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorQosAckPolicy : public ModeSetListener, public IOriginatorQoSAckPolicy
{
    protected:
        IQosRateSelection *rateSelection = nullptr;
        int maxBlockAckPolicyFrameLength = -1;
        int blockAckReqThreshold = -1;

        simtime_t blockAckTimeout = -1;
        simtime_t ackTimeout = -1;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        virtual bool checkAgreementPolicy(const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const;
        virtual std::map<MacAddress, std::vector<Packet *>> getOutstandingFramesPerReceiver(InProgressFrames *inProgressFrames) const;
        virtual SequenceNumberCyclic computeStartingSequenceNumber(const std::vector<Packet *>& outstandingFrames) const;
        virtual bool isCompressedBlockAckReq(const std::vector<Packet *>& outstandingFrames, int startingSequenceNumber) const;

    public:
        virtual bool isAckNeeded(const Ptr<const Ieee80211MgmtHeader>& header) const override;
        virtual AckPolicy computeAckPolicy(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const override;
        virtual bool isBlockAckPolicyEligibleFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) const override;
        virtual bool isBlockAckReqNeeded(InProgressFrames *inProgressFrames, TxopProcedure *txopProcedure) const override;
        virtual std::tuple<MacAddress, SequenceNumberCyclic, Tid> computeBlockAckReqParameters(InProgressFrames *inProgressFrames, TxopProcedure* txopProcedure) const override;

        virtual simtime_t getAckTimeout(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const override;
        virtual simtime_t getBlockAckTimeout(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_ORIGINATORQOSACKPOLICY_H
