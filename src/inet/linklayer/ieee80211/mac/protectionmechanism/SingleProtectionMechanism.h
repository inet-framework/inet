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

#ifndef __INET_SINGLEPROTECTIONMECHANISM_H
#define __INET_SINGLEPROTECTIONMECHANISM_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientQosAckPolicy.h"

namespace inet {
namespace ieee80211 {

//
// 8.2.5 Duration/ID field (QoS STA)
//   8.2.5.1 General
//    The value in the Duration/ID field in a frame transmitted by a QoS STA is defined in 8.2.5.2 through 8.2.5.8.
//    All times are calculated in microseconds. If a calculated duration includes a fractional microsecond, that
//    value inserted in the Duration/ID field is rounded up to the next higher integer.
//
//   8.2.5.2 Setting for single and multiple protection under enhanced distributed channel access (EDCA)
//
class INET_API SingleProtectionMechanism : public ModeSetListener
{
    protected:
        IQosRateSelection *rateSelection = nullptr;
        // TODO: IRateSelection *nonQoSrateSelection = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        virtual simtime_t computeRtsDurationField(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader, TxopProcedure *txop, IRecipientQosAckPolicy *ackPolicy);
        virtual simtime_t computeCtsDurationField(const Ptr<const Ieee80211CtsFrame>& ctsFrame);
        virtual simtime_t computeBlockAckReqDurationField(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq);
        virtual simtime_t computeBlockAckDurationField(const Ptr<const Ieee80211BlockAck>& blockAck);
        virtual simtime_t computeDataOrMgmtFrameDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader, TxopProcedure *txop, IRecipientQosAckPolicy *ackPolicy);

    public:
        virtual ~SingleProtectionMechanism() { }

        // TODO: QoSAckPolicy, IQosRateSelection may give wrong answers when communicating with a Non-QoS STA.
        virtual simtime_t computeDurationField(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader, TxopProcedure *txop, IRecipientQosAckPolicy *ackPolicy);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_SINGLEPROTECTIONMECHANISM_H
