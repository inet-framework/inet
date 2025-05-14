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

#ifndef INET_APPLICATIONS_QUIC_RECEIVEDPACKETSACCOUNTANT_H_
#define INET_APPLICATIONS_QUIC_RECEIVEDPACKETSACCOUNTANT_H_

#include "packet/QuicPacket.h"
#include "packet/FrameHeader_m.h"
#include "Timer.h"

namespace inet {
namespace quic {

class Timer;
class TransportParameters;

class GapRange {
public:
    uint64_t firstMissing;
    uint64_t lastMissing;
};

class ReceivedPacketsAccountant {
public:
    ReceivedPacketsAccountant(Timer *ackDelayTimer, TransportParameters *transportParameter);
    virtual ~ReceivedPacketsAccountant();

    void readParameters(cModule *module);

    /**
     * Account the received packet. Generate gap information, if the packet
     * received out of order.
     *
     * @param packetNumber The number of the received packet.
     * @param ackEliciting Indicates if the received packet is ack-eliciting or not.
     * @param isIBitSet Indicates if the I-Bit is set on the received packet.
     */
    void onPacketReceived(uint64_t packetNumber, bool ackEliciting, bool isIBitSet);

    /**
     * Generates an Ack Frame Header.
     *
     * @param maxSize The maximum size in bytes for the ack frame.
     * @return QuicFrame containing the generated Ack Frame Header
     */
    QuicFrame *generateAckFrame(size_t maxSize);

    /**
     * Informs that a sent packet was acked. If this packet contained an
     * ack frame, we can stop acking the packets that were acked in the
     * arrived ack frame.
     *
     * @param ackedPacket Packet we sent that were acked by the peer.
     */
    void onPacketAcked(QuicPacket *ackedPacket);

    void onAckDelayTimeout();

    bool hasNewAckInfo() {
        return newAckInfo;
    }
    bool hasNewAckInfoAboutAckElicitings() {
        return newAckInfoAboutAckElicitings;
    }
    bool wantsToSendAckImmediately() {
        return sendAckImmediately;
    }

private:
    int kNumReceivedAckElicitingsBeforeAck = 2;

    bool newAckInfo;
    bool newAckInfoAboutAckElicitings;
    bool sendAckImmediately;
    int numAckElicitingsReceivedSinceAck;
    uint64_t packetCounter;
    uint64_t largestAck;
    uint64_t smallestAck;
    uint64_t expectedPacketNumber;
    std::vector<GapRange> gapRanges;
    simtime_t timeLargestAckReceived;
    Timer *ackDelayTimer;
    TransportParameters *transportParameter;
    simtime_t timeLastAckElicitingReceivedOutOfOrder;
    bool useIBit = false;

    void removeOldGapRanges();
    bool hasGapsSince(uint64_t packetNumber);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_RECEIVEDPACKETSACCOUNTANT_H_ */
