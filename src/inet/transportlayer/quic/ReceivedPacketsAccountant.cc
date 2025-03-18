//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "ReceivedPacketsAccountant.h"

namespace inet {
namespace quic {

ReceivedPacketsAccountant::ReceivedPacketsAccountant(Timer *ackDelayTimer, TransportParameter *transportParameter) {
    this->ackDelayTimer = ackDelayTimer;
    this->transportParameter = transportParameter;
    newAckInfo = false;
    newAckInfoAboutAckElicitings = false;
    sendAckImmediately = false;
    numAckElicitingsReceivedSinceAck = 0;
    largestAck = 0;
    smallestAck = 0;
    packetCounter = 0;
    expectedPacketNumber = 0;
    timeLastAckElicitingReceivedOutOfOrder = SimTime::ZERO;
}

ReceivedPacketsAccountant::~ReceivedPacketsAccountant() {
    if (ackDelayTimer != nullptr) {
        delete ackDelayTimer;
    }
}

void ReceivedPacketsAccountant::readParameters(cModule *module)
{
    // add upper limit for ack frame size as parameter?
    kNumReceivedAckElicitingsBeforeAck = module->par("numReceivedAckElicitingsBeforeAck");
    useIBit = module->par("useIBit");
}

/**
 * Account the received packet. Generate gap informationen, if the packet
 * received out of order.
 * \param packetNumber The number of the received packet.
 *
 * TODO: consider different packet number spaces (possible with multiple objects of this same class?)
 * TODO: Distinguish between ack-eliciting packets and non ack-eliciting packets
 */
void ReceivedPacketsAccountant::onPacketReceived(uint64_t packetNumber, bool ackEliciting, bool isIBitSet)
{
    packetCounter++;
    if (packetCounter == 1) { // first packet
        expectedPacketNumber = 0;
    }

    if (packetNumber > largestAck || largestAck == 0) {
        largestAck = packetNumber;
        timeLargestAckReceived = simTime();
    }

    bool packetReceivedOutOfOrder = (packetNumber != expectedPacketNumber);

    newAckInfo = true;
    if (ackEliciting) {
        static uint64_t lastReceivedAckEliciting = 0;
        newAckInfoAboutAckElicitings = true;
        numAckElicitingsReceivedSinceAck++;

        if (ackDelayTimer == nullptr) {
            // Packet number space is Initial or Handshake -> no ack delay
            sendAckImmediately = true;
        } else {

            // send ack immediately ...
            if (numAckElicitingsReceivedSinceAck >= kNumReceivedAckElicitingsBeforeAck // if kNumReceivedAckElicitingsBeforeAck (default 2) ack-elicitings were received
             || packetReceivedOutOfOrder // if this packet received out of order
             || hasGapsSince(lastReceivedAckEliciting) // if there were gaps in the sequence of received packet numbers since the last ack-eliciting packet were received
             || (useIBit && isIBitSet)) { // ... if the IBit is set
             //|| (packetNumber == 0)) { // ... if it is the first packet (helps the sender to measure a more accurate RTT. Normally the first RTT measurement would be after the handshake which isn't delayed)

                ackDelayTimer->cancel();
                sendAckImmediately = true;
                if (packetReceivedOutOfOrder) {
                    timeLastAckElicitingReceivedOutOfOrder = simTime();
                }
            } else {
                if (!ackDelayTimer->isScheduled()) {
                    ackDelayTimer->scheduleAt(simTime() + transportParameter->maxAckDelay);
                }
            }
        }
        lastReceivedAckEliciting = packetNumber;
    }

    if (!packetReceivedOutOfOrder) {
        expectedPacketNumber = packetNumber + 1;
        return;
    }

    // packet with an unexpected packet number received

    if (packetNumber > expectedPacketNumber) { // Loss
        if (!gapRanges.empty()) {
            auto lastGapRange = gapRanges.back();
            if (lastGapRange.lastMissing - packetNumber == 1) { // extend existing Block
                lastGapRange.lastMissing++;
                expectedPacketNumber = packetNumber + 1;
                return;
            }
        }

        // or create new block
        auto gapRange = GapRange();
        gapRange.firstMissing = expectedPacketNumber;
        gapRange.lastMissing = packetNumber -1;
        gapRanges.push_back(gapRange);

        expectedPacketNumber = packetNumber + 1;
        return;
    }

    for (auto gapRange = gapRanges.begin(); gapRange != gapRanges.end(); gapRange++) { // out of order delivery
        if (packetNumber == gapRange->firstMissing && packetNumber == gapRange->lastMissing) {
            gapRanges.erase(gapRange);
            return;
        }

        if (packetNumber == gapRange->firstMissing) {
            gapRange->firstMissing = packetNumber + 1;
            return;
        }

        if (packetNumber == gapRange->lastMissing) {
            gapRange->lastMissing = packetNumber - 1;
            return;
        }

        if (packetNumber > gapRange->firstMissing && packetNumber < gapRange->lastMissing) {
            auto newGapRange = GapRange();
            newGapRange.firstMissing = packetNumber + 1;
            newGapRange.lastMissing = gapRange->lastMissing;

            gapRange->lastMissing = packetNumber - 1;
            gapRanges.insert( gapRange+1 , newGapRange);
            return;
        }
    }

    if (packetNumber < smallestAck) { // there might have been a gap that is no longer reported
        if (packetNumber+1 != smallestAck) { // create a new gap, if there is one
            auto gapRange = GapRange();
            gapRange.firstMissing = packetNumber + 1;
            gapRange.lastMissing = smallestAck - 1;
            gapRanges.push_back(gapRange);
        }
        smallestAck = packetNumber;
        return;
    }

    // we shouldn't run into this code
    throw cRuntimeError("packetNumber %lu received, but is completely unexpected.", packetNumber);
}

/**
 * Generates an Ack Frame Header.
 * \return QuicFrame containing the generated Ack Frame Header
 */
QuicFrame *ReceivedPacketsAccountant::generateAckFrame(size_t maxSize)
{
    if (ackDelayTimer != nullptr) {
        ackDelayTimer->cancel();
    }
    newAckInfo = false;
    newAckInfoAboutAckElicitings = false;
    sendAckImmediately = false;
    numAckElicitingsReceivedSinceAck = 0;

    Ptr<AckFrameHeader> ackFrame = makeShared<AckFrameHeader>();
    ackFrame->setLargestAck(largestAck);
    int64_t ackDelay = (simTime() - timeLargestAckReceived).inUnit(SIMTIME_US);
    ackDelay = ackDelay >> transportParameter->ackDelayExponent; // use this endpoint's transport parameter
    ackFrame->setAckDelay(ackDelay);
    ackFrame->setAckRangeCount(gapRanges.size()); // set ack range count to the possible maximum

    if (gapRanges.empty()) { // no gaps
        ackFrame->setFirstAckRange(largestAck - smallestAck);
    } else { // mind the gaps
        ackFrame->setFirstAckRange(largestAck - gapRanges.back().lastMissing - 1);
    }

    ackFrame->setAckRangeArraySize(0); // set gap-ack-range array size to 0 to calculate the frame size without gap-ack-ranges
    ackFrame->calcChunkLength();
    size_t currentSize = B(ackFrame->getChunkLength()).get();
    if (currentSize > maxSize) {
        // maxSize is too small. Not even an ACK frame without gap-ack-ranges fits.
        return nullptr;
    }

    if (!gapRanges.empty()) {
        // generate ack ranges for the ack frame
        std::vector<AckRange> ackRanges;
        for (int i = gapRanges.size() - 1; i >= 0; i--) {
            GapRange currentGapRange = gapRanges[i];

            if (currentGapRange.firstMissing == 0) {
                // there are no packets to ack after this gap
                continue;
            }

            AckRange ackRange;
            ackRange.gap = currentGapRange.lastMissing - currentGapRange.firstMissing;

            // Set ack blocks as number of contiguous received packets
            if (i > 0) {
                // There are gaps left - set number of contiguous packets to next gap
                GapRange previousGapRange = gapRanges[i - 1];
                ackRange.ackRange = (currentGapRange.firstMissing - 1) - (previousGapRange.lastMissing + 1);
            } else {
                // No gaps left - set number of contiguous packets to beginning of transmission
                ackRange.ackRange = (currentGapRange.firstMissing - 1) - smallestAck;
            }

            currentSize += getVariableLengthIntegerSize(ackRange.ackRange) + getVariableLengthIntegerSize(ackRange.gap);
            if (currentSize > maxSize) {
                // we cannot add any more gap-ack-ranges, not even this one
                break;
            }

            ackRanges.push_back(ackRange);
        }

        // set ack range count and ack range array size to the actual number of elements
        ackFrame->setAckRangeCount(ackRanges.size());
        ackFrame->setAckRangeArraySize(ackRanges.size());
        // add all generated ack ranges to the ack frame
        for (int i = 0; i < ackRanges.size(); i++) {
            ackFrame->setAckRange(i, ackRanges[i]);
        }

        ackFrame->calcChunkLength();
    }

    ASSERT(B(ackFrame->getChunkLength()).get() <= maxSize);
    return new QuicFrame(ackFrame);
}

Ptr<const AckFrameHeader> findAckFrame(QuicPacket *packet)
{
    for (QuicFrame *frame : *packet->getFrames()) {
        Ptr<const AckFrameHeader> ackFrame = dynamicPtrCast<const AckFrameHeader>(frame->getHeader());
        if (ackFrame != nullptr) {
            return ackFrame;
        }
    }
    return nullptr;
}

void ReceivedPacketsAccountant::removeOldGapRanges()
{
    int removeBefore = 0;
    for (GapRange gapRange : gapRanges) {
        if (gapRange.firstMissing > smallestAck) {
            break;
        }
        removeBefore++;
    }

    gapRanges.erase(gapRanges.begin(), gapRanges.begin()+removeBefore);
}

bool ReceivedPacketsAccountant::hasGapsSince(uint64_t packetNumber) {
    for (GapRange gapRange : gapRanges) {
        if (gapRange.firstMissing >= packetNumber) {
            return true;
        }
    }
    return false;
}

/**
 * Informs that a sent packet was acked. If this packet contained an
 * ack frame, we can stop acking the packets that were acked in the
 * arrived ack frame.
 *
 * \param ackedPacket Packet we sent that were acked by the peer.
 */
void ReceivedPacketsAccountant::onPacketAcked(QuicPacket *ackedPacket)
{
    // Set smallestAck to the largestAck of the ack frame that were received by the peer
    // and remove gapRanges with smaller packet numbers.

    Ptr<const AckFrameHeader> ackFrame = findAckFrame(ackedPacket);
    if (ackFrame != nullptr) {
        uint64_t ackFrameLargestAck = ackFrame->getLargestAck();
        if (ackFrameLargestAck > smallestAck) {
            smallestAck = ackFrameLargestAck;
            removeOldGapRanges();
        }
    }
}

void ReceivedPacketsAccountant::onAckDelayTimeout()
{
    sendAckImmediately = true;
}

} /* namespace quic */
} /* namespace inet */
