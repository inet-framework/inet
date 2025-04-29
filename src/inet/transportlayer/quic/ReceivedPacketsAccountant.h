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
    void onPacketReceived(uint64_t packetNumber, bool ackEliciting, bool isIBitSet);
    QuicFrame *generateAckFrame(size_t maxSize);
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
