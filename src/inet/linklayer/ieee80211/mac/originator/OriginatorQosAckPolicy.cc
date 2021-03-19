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

#include <tuple>

#include "inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorQosAckPolicy);

void OriginatorQosAckPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IQosRateSelection*>(getModuleByPath(par("rateSelectionModule")));
        maxBlockAckPolicyFrameLength = par("maxBlockAckPolicyFrameLength");
        blockAckReqThreshold = par("blockAckReqThreshold");
        blockAckTimeout = par("blockAckTimeout");
        ackTimeout = par("ackTimeout");
    }
}

bool OriginatorQosAckPolicy::isAckNeeded(const Ptr<const Ieee80211MgmtHeader>& header) const
{
    return !header->getReceiverAddress().isMulticast();
}

std::map<MacAddress, std::vector<Packet *>> OriginatorQosAckPolicy::getOutstandingFramesPerReceiver(InProgressFrames *inProgressFrames) const
{
    auto outstandingFrames = inProgressFrames->getOutstandingFrames();
    std::map<MacAddress, std::vector<Packet *>> outstandingFramesPerReceiver;
    for (auto frame : outstandingFrames)
        outstandingFramesPerReceiver[frame->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress()].push_back(frame);
    return outstandingFramesPerReceiver;
}


SequenceNumberCyclic OriginatorQosAckPolicy::computeStartingSequenceNumber(const std::vector<Packet *>& outstandingFrames) const
{
    ASSERT(outstandingFrames.size() > 0);
    auto startingSequenceNumber = outstandingFrames[0]->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber();
    for (size_t i = 1; i < outstandingFrames.size(); i++) {
        auto seqNum = outstandingFrames[i]->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber();
        if (seqNum < startingSequenceNumber)
            startingSequenceNumber = seqNum;
    }
    return startingSequenceNumber;
}

bool OriginatorQosAckPolicy::isCompressedBlockAckReq(const std::vector<Packet *>& outstandingFrames, int startingSequenceNumber) const
{
    // The Compressed Bitmap subfield of the BA Control field or BAR Control field shall be set to 1 in all
    // BlockAck and BlockAckReq frames sent from one HT STA to another HT STA and shall be set to 0 otherwise.
    return false; // non-HT STA
//    for (auto frame : outstandingFrames)
//        if (frame->getSequenceNumber() >= startingSequenceNumber && frame->getFragmentNumber() > 0)
//            return false;
//    return true;
}

// FIXME
bool OriginatorQosAckPolicy::isBlockAckReqNeeded(InProgressFrames* inProgressFrames, TxopProcedure* txopProcedure) const
{
    auto outstandingFramesPerReceiver = getOutstandingFramesPerReceiver(inProgressFrames);
    for (auto outstandingFrames : outstandingFramesPerReceiver) {
        if ((int)outstandingFrames.second.size() >= blockAckReqThreshold)
            return true;
    }
    return false;
}

// FIXME
std::tuple<MacAddress, SequenceNumberCyclic, Tid> OriginatorQosAckPolicy::computeBlockAckReqParameters(InProgressFrames *inProgressFrames, TxopProcedure* txopProcedure) const
{
    auto outstandingFramesPerReceiver = getOutstandingFramesPerReceiver(inProgressFrames);
    for (auto outstandingFrames : outstandingFramesPerReceiver) {
        if ((int)outstandingFrames.second.size() >= blockAckReqThreshold) {
            auto largestOutstandingFrames = outstandingFramesPerReceiver.begin();
            for (auto it = outstandingFramesPerReceiver.begin(); it != outstandingFramesPerReceiver.end(); it++) {
                if (it->second.size() > largestOutstandingFrames->second.size())
                    largestOutstandingFrames = it;
            }
            MacAddress receiverAddress = largestOutstandingFrames->first;
            SequenceNumberCyclic startingSequenceNumber = computeStartingSequenceNumber(largestOutstandingFrames->second);
            Tid tid = largestOutstandingFrames->second.at(0)->peekAtFront<Ieee80211DataHeader>()->getTid();
            return std::make_tuple(receiverAddress, startingSequenceNumber, tid);
        }
    }
    return std::make_tuple(MacAddress::UNSPECIFIED_ADDRESS, SequenceNumberCyclic(), -1);
}

AckPolicy OriginatorQosAckPolicy::computeAckPolicy(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const
{
    if (agreement == nullptr)
        return AckPolicy::NORMAL_ACK;
    if (agreement->getIsAddbaResponseReceived() && isBlockAckPolicyEligibleFrame(packet, header)) {
        if (checkAgreementPolicy(header, agreement))
            return AckPolicy::BLOCK_ACK;
        else
            return AckPolicy::NORMAL_ACK;
    }
    else
        return AckPolicy::NORMAL_ACK;
}

bool OriginatorQosAckPolicy::isBlockAckPolicyEligibleFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) const
{
    return header->getType() == ST_DATA_WITH_QOS && packet->getByteLength() < maxBlockAckPolicyFrameLength;
}

bool OriginatorQosAckPolicy::checkAgreementPolicy(const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const
{
    bool bufferFull = agreement->getBufferSize() == agreement->getNumSentBaPolicyFrames();
    bool aMsduOk = agreement->getIsAMsduSupported() || !header->getAMsduPresent();
    // TODO: bool baPolicy = agreement->getIsDelayedBlockAckPolicySupported() || !frame->getAckPolicy();
    return !bufferFull && aMsduOk && (header->getSequenceNumber() >= agreement->getStartingSequenceNumber()); // TODO: && baPolicy
}

//
// After transmitting an MPDU that requires an ACK frame as a response (see Annex G), the STA shall wait for an
// ACKTimeout interval, with a value of aSIFSTime + aSlotTime + aPHY-RX-START-Delay, starting at the
// PHY-TXEND.confirm primitive. If a PHY-RXSTART.indication primitive does not occur during the
// ACKTimeout interval, the STA concludes that the transmission of the MPDU has failed, and this STA shall
// invoke its backoff procedure upon expiration of the ACKTimeout interval.
//
simtime_t OriginatorQosAckPolicy::getAckTimeout(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const
{
    return ackTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + rateSelection->computeResponseAckFrameMode(packet, dataOrMgmtHeader)->getPhyRxStartDelay() : ackTimeout;
}

simtime_t OriginatorQosAckPolicy::getBlockAckTimeout(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const
{
    return blockAckTimeout == -1 ? modeSet->getSifsTime() + modeSet->getSlotTime() + rateSelection->computeResponseBlockAckFrameMode(packet, blockAckReq)->getPhyRxStartDelay() : blockAckTimeout;
}

} /* namespace ieee80211 */
} /* namespace inet */
