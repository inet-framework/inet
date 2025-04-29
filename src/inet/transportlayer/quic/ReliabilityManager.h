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

#ifndef INET_APPLICATIONS_QUIC_RELIABILITYMANAGER_H_
#define INET_APPLICATIONS_QUIC_RELIABILITYMANAGER_H_

#include "packet/FrameHeader_m.h"
#include "packet/QuicPacket.h"
#include "ReceivedPacketsAccountant.h"
#include "Connection.h"
#include "Timer.h"
#include "congestioncontrol/ICongestionController.h"
#include "Statistics.h"
#include "TransportParameters.h"

namespace inet {
namespace quic {

class Timer;
class ReceivedPacketsAccountant;
class ICongestionController;

class ReliabilityManager {
public:
    ReliabilityManager(Connection *connection, TransportParameters *transportParameter, ReceivedPacketsAccountant *receivedPacketsAccountant, ICongestionController *congestionController, Statistics *stats);
    virtual ~ReliabilityManager();

    void onAckReceived(const Ptr<const AckFrameHeader>& ackFrame, PacketNumberSpace pnSpace);
    void onPacketSent(QuicPacket *packet, PacketNumberSpace pnSpace);
    void onLossDetectionTimeout();
    std::vector<QuicPacket*> *getAckElicitingSentPackets(PacketNumberSpace pnSpace);
    QuicPacket *getSentPacket(PacketNumberSpace pnSpace, uint64_t droppedPacketNumber);

    bool reducePacketSize(int minSize, int maxSize);

private:
    const int kMaxProbePackets = 10;
    const simtime_t kGranularity = SimTime(1, SimTimeUnit::SIMTIME_MS);
    const double kTimeThreshold = 9.0/8;
    const uint64_t kPacketThreshold = 3;
    const int kPersistentCongestionThreshold = 3;
    const simtime_t kInitialRtt = SimTime(333, SimTimeUnit::SIMTIME_MS);
    const simtime_t kOldNonInFlightPacketTimeThreshold = SimTime(5, SimTimeUnit::SIMTIME_S);

    Connection *connection;
    TransportParameters *transportParameter;
    ReceivedPacketsAccountant *receivedPacketsAccountant;
    ICongestionController *congestionController;
    Timer *lossDetectionTimer;
    std::map<uint64_t, QuicPacket*> sentPackets[3];
    uint64_t largestAckedPacketNumber[3];
    simtime_t lastSentAckElicitingPacketTime[3];
    simtime_t lossTime[3];
    simtime_t latestRtt;
    simtime_t minRtt;
    simtime_t firstRttSample;
    simtime_t reducePacketTimeThreshold;
    uint ptoCount;
    uint persistentCongestionCounter;

    Statistics *stats;
    //simsignal_t packetLostSignal = cComponent::registerSignal("packetLost");

    std::vector<QuicPacket*> *detectAndRemoveAckedPackets(const Ptr<const AckFrameHeader>& ackFrame, PacketNumberSpace pnSpace);
    void updateRtt(uint64_t ackDelay);
    bool includesAckEliciting(std::vector<QuicPacket*> *packets);
    void onPacketAcked(QuicPacket *ackedPacket);
    std::vector<QuicPacket*> *detectAndRemoveLostPackets(PacketNumberSpace pnSpace);
    void setLossDetectionTimer();

    void moveNewlyAckedPackets(std::vector<QuicPacket*> *newlyAckedPackets, uint64_t largestAck, uint64_t smallestAck, PacketNumberSpace pnSpace);
    bool areAckElicitingPacketsInFlight(PacketNumberSpace pnSpace);
    bool areAckElicitingPacketsInFlight();

    simtime_t getLossTimeAndSpace(PacketNumberSpace *retSpace);
    bool inPersistentCongestion(std::vector<QuicPacket *> *lostPackets, QuicPacket *&firstAckEliciting);
    simtime_t getPtoTimeAndSpace(PacketNumberSpace *retSpace);
    void onPacketsLost(std::vector<QuicPacket *> *lostPackets);

    void deleteOldNonInFlightPackets();

    SimTime getReducePacketSizeTime();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_RELIABILITYMANAGER_H_ */
