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

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

Define_Module(SingleProtectionMechanism);

void SingleProtectionMechanism::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rateSelection = check_and_cast<IQosRateSelection*>(getModuleByPath(par("rateSelectionModule")));
    }
}


//
// For an RTS that is not part of a dual clear-to-send (CTS) exchange, the Duration/ID field is set
// to the estimated time, in microseconds, required to transmit the pending frame, plus one CTS
// frame, plus one ACK or BlockAck frame if required, plus any NDPs required, plus explicit
// feedback if required, plus applicable IFS durations.
//
simtime_t SingleProtectionMechanism::computeRtsDurationField(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader, TxopProcedure *txop, IRecipientQosAckPolicy *ackPolicy)
{
    // TODO: We assume that the RTS frame is not part of a dual clear-to-send
    auto pendingFrameMode = rateSelection->computeMode(pendingPacket, pendingHeader, txop);
    simtime_t pendingFrameDuration = pendingFrameMode->getDuration(pendingPacket->getDataLength());
    simtime_t ctsFrameDuration = rateSelection->computeResponseCtsFrameMode(rtsPacket, rtsFrame)->getDuration(LENGTH_CTS);
    simtime_t durationId = ctsFrameDuration + modeSet->getSifsTime() + pendingFrameDuration + modeSet->getSifsTime();
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(pendingHeader)) {
        if (ackPolicy->isAckNeeded(dataOrMgmtHeader)) {
            RateSelection::setFrameMode(pendingPacket, dataOrMgmtHeader, pendingFrameMode); // FIXME: KLUDGE
            simtime_t ackFrameDuration = rateSelection->computeResponseAckFrameMode(pendingPacket, dataOrMgmtHeader)->getDuration(LENGTH_ACK);
            durationId += ackFrameDuration + modeSet->getSifsTime();
        }
    }
    return durationId;
}

//
// For all CTS frames sent by STAs as the first frame in the exchange under EDCA and with the
// receiver address (RA) matching the MAC address of the transmitting STA, the Duration/ID
// field is set to one of the following:
//  i) If there is a response frame, the estimated time required to transmit the pending frame,
//     plus one SIFS interval, plus the response frame (ACK or BlockAck), plus any NDPs
//     required, plus explicit feedback if required, plus an additional SIFS interval
//  ii) If there is no response frame, the time required to transmit the pending frame, plus one
//      SIFS interval
//
simtime_t SingleProtectionMechanism::computeCtsDurationField(const Ptr<const Ieee80211CtsFrame>& ctsFrame)
{
    throw cRuntimeError("Self CTS is not supported");
}

//
// For a BlockAckReq frame, the Duration/ID field is set to the estimated time required to
// transmit one ACK or BlockAck frame, as applicable, plus one SIFS interval.
//
simtime_t SingleProtectionMechanism::computeBlockAckReqDurationField(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq)
{
    //  TODO: ACK or BlockAck frame, as applicable
    if (dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq)) {
        simtime_t blockAckFrameDuration = rateSelection->computeResponseBlockAckFrameMode(packet, blockAckReq)->getDuration(LENGTH_BASIC_BLOCKACK);
        simtime_t blockAckReqDurationPerId = blockAckFrameDuration + modeSet->getSifsTime();
        return blockAckReqDurationPerId;
    }
    else
        throw cRuntimeError("Compressed and Multi-Tid Block Ack Requests are not supported");
}

//
// For a BlockAck frame that is not sent in response to a BlockAckReq or an implicit Block Ack
// request, the Duration/ID field is set to the estimated time required to transmit an ACK frame
// plus a SIFS interval.
//
simtime_t SingleProtectionMechanism::computeBlockAckDurationField(const Ptr<const Ieee80211BlockAck>& blockAck)
{
    throw cRuntimeError("Unimplemented");
}

//
// For management frames, non-QoS data frames (i.e., with bit 7 of the Frame Control field equal
// to 0), and individually addressed data frames with the Ack Policy subfield equal to Normal Ack
// only, the Duration/ID field is set to one of the following:
//
//  i) If the frame is the final fragment of the TXOP, the estimated time required for the
//     transmission of one ACK frame (including appropriate IFS values)
//  ii) Otherwise, the estimated time required for the transmission of one ACK frame, plus the
//      time required for the transmission of the following MPDU and its response if required,
//      plus applicable IFS durations.
//
// For individually addressed QoS data frames with the Ack Policy subfield equal to No Ack or
// Block Ack, for management frames of subtype Action No Ack, and for group addressed
// frames, the Duration/ID field is set to one of the following:
//
//  i) If the frame is the final fragment of the TXOP, 0
//  ii) Otherwise, the estimated time required for the transmission of the following frame and its
//      response frame, if required (including appropriate IFS values)
//
simtime_t SingleProtectionMechanism::computeDataOrMgmtFrameDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader, TxopProcedure *txop, IRecipientQosAckPolicy *ackPolicy)
{
    bool mgmtFrame = false;
    bool mgmtFrameWithNoAck = false;
    bool groupAddressed = dataOrMgmtHeader->getReceiverAddress().isMulticast();
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(dataOrMgmtHeader)) {
        mgmtFrame = true;
        mgmtFrameWithNoAck = false; // FIXME: ack policy?
    }
    bool nonQoSData = dataOrMgmtHeader->getType() == ST_DATA;
    bool individuallyAddressedDataWithNormalAck = false;
    bool individuallyAddressedDataWithNoAckOrBlockAck = false;
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(dataOrMgmtHeader)) {
        individuallyAddressedDataWithNormalAck = !groupAddressed && dataHeader->getAckPolicy() == AckPolicy::NORMAL_ACK;
        individuallyAddressedDataWithNoAckOrBlockAck = !groupAddressed && (dataHeader->getAckPolicy() == AckPolicy::NO_ACK || dataHeader->getAckPolicy() == AckPolicy::BLOCK_ACK);
    }
    if (mgmtFrame || nonQoSData || individuallyAddressedDataWithNormalAck) {
        simtime_t ackFrameDuration = rateSelection->computeResponseAckFrameMode(packet, dataOrMgmtHeader)->getDuration(LENGTH_ACK);
        if (txop->isFinalFragment(dataOrMgmtHeader)) {
            return ackFrameDuration + modeSet->getSifsTime();
        }
        else {
            simtime_t ackFrameDuration = rateSelection->computeResponseAckFrameMode(packet, dataOrMgmtHeader)->getDuration(LENGTH_ACK);
            simtime_t duration = ackFrameDuration + modeSet->getSifsTime();
            if (pendingHeader) {
                auto pendingFrameMode = rateSelection->computeMode(pendingPacket, pendingHeader, txop);
                simtime_t pendingFrameDuration = pendingFrameMode->getDuration(pendingPacket->getDataLength());
                duration += pendingFrameDuration + modeSet->getSifsTime();
                if (ackPolicy->isAckNeeded(pendingHeader)) {
                    RateSelection::setFrameMode(pendingPacket, pendingHeader, pendingFrameMode); // KLUDGE:
                    simtime_t ackToPendingFrameDuration = rateSelection->computeResponseAckFrameMode(pendingPacket, pendingHeader)->getDuration(LENGTH_ACK);
                    duration += ackToPendingFrameDuration + modeSet->getSifsTime();
                }
            }
            return duration;
        }
    }
    if (individuallyAddressedDataWithNoAckOrBlockAck || mgmtFrameWithNoAck || groupAddressed) {
        if (txop->isFinalFragment(dataOrMgmtHeader))
            return 0;
        else {
            simtime_t duration = 0;
            if (pendingHeader) {
                auto pendingFrameMode = rateSelection->computeMode(pendingPacket, pendingHeader, txop);
                simtime_t pendingFrameDuration = pendingFrameMode->getDuration(pendingPacket->getDataLength());
                duration = pendingFrameDuration + modeSet->getSifsTime();
                if (ackPolicy->isAckNeeded(pendingHeader)) {
                    RateSelection::setFrameMode(pendingPacket, pendingHeader, pendingFrameMode); // KLUDGE:
                    simtime_t ackToPendingFrameDuration = rateSelection->computeResponseAckFrameMode(pendingPacket, pendingHeader)->getDuration(LENGTH_ACK);
                    duration += ackToPendingFrameDuration + modeSet->getSifsTime();
                }
            }
            return duration;
        }
    }
    throw cRuntimeError("Unknown frame");
}

simtime_t SingleProtectionMechanism::computeDurationField(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader, TxopProcedure *txop, IRecipientQosAckPolicy *ackPolicy)
{
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(header))
        return computeRtsDurationField(packet, rtsFrame, pendingPacket, pendingHeader, txop, ackPolicy);
    else if (auto ctsFrame = dynamicPtrCast<const Ieee80211CtsFrame>(header))
        return computeCtsDurationField(ctsFrame);
    else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(header))
        return computeBlockAckReqDurationField(packet, blockAckReq);
    else if (auto blockAck = dynamicPtrCast<const Ieee80211BlockAck>(header))
        return computeBlockAckDurationField(blockAck);
    else if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return computeDataOrMgmtFrameDurationField(packet, dataOrMgmtHeader, pendingPacket, pendingHeader, txop, ackPolicy);
    else
        throw cRuntimeError("Unknown frame type");
}

} /* namespace ieee80211 */
} /* namespace inet */
