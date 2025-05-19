//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

    /**
     * Called by indirectly by a *ConnectionState, when an ack frame were received.
     *
     * @param ackFrame The received ack frame.
     */
    void onAckReceived(const Ptr<const AckFrameHeader>& ackFrame, PacketNumberSpace pnSpace);

    /**
     * Called by Connection, when a new packet were sent.
     *
     * @param packet The sent packet.
     */
    void onPacketSent(QuicPacket *packet, PacketNumberSpace pnSpace);

    /**
     * Called from EstablishedConnectionState when the loss detection timer fires.
     */
    void onLossDetectionTimeout();

    /**
     * Creates a vector of all ack eliciting packets that are outstanding.
     *
     * @return Pointer to a vector of QuicPackets, that are ack eliciting and outstanding. Might be empty.
     */
    std::vector<QuicPacket*> *getAckElicitingSentPackets(PacketNumberSpace pnSpace);

    QuicPacket *getSentPacket(PacketNumberSpace pnSpace, uint64_t droppedPacketNumber);

    simtime_t getPtoDuration(PacketNumberSpace pnSpace);

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
    int reducePacketPtoFactorThreshold;
    uint ptoCount;
    uint persistentCongestionCounter;

    Statistics *stats;
    //simsignal_t packetLostSignal = cComponent::registerSignal("packetLost");

    /**
     * Determines which packets were newly acked (not acked in a received ack frame before) by the given ack frame
     * and removes them from the sentPackets map.
     *
     * @param ackFrame The ack frame to analyze.
     * @return Pointer to a vector with all newly acked packets.
     */
    std::vector<QuicPacket*> *detectAndRemoveAckedPackets(const Ptr<const AckFrameHeader>& ackFrame, PacketNumberSpace pnSpace);

    /**
     * Update the RTT values upon a received ack.
     *
     * @param ackDelay The ack delay value as reported in the received ack.
     */
    void updateRtt(uint64_t ackDelay);

    /**
     * Checks if one of the given vector of packets is an ack eliciting packet.
     *
     * @param packets Vector of packets to check.
     * @return true if the given vector of packets contains at least one ack eliciting packet, false otherwise.
     */
    bool includesAckEliciting(std::vector<QuicPacket*> *packets);

    /**
     * Called for each packet that were newly acked in a received ack frame.
     *
     * @param ackedPacket The packet that were acked.
     */
    void onPacketAcked(QuicPacket *ackedPacket);

    /**
     * Detects packets as lost, if their sent time is old enough
     * or if enough new packets, sent after them, were already acked.
     *
     * @param pnSpace The packet number space of the packets that shall be considered
     * @return Newly created vector of QUIC packets that were detected as lost (needs to be deleted).
     */
    std::vector<QuicPacket*> *detectAndRemoveLostPackets(PacketNumberSpace pnSpace);

    /**
     * Resets the loss detection timer.
     */
    void setLossDetectionTimer();

    /**
     * Moves all packets with packet numbers from largestAck to smallestAck from the sentPackets map to the newlyAckedPackets vector.
     *
     * @param newlyAckedPackets Adds packets to this vector.
     * @param largestAck Packet number of the first packet to add.
     * @param smallestAck Packet number of the last packet to add.
     */
    void moveNewlyAckedPackets(std::vector<QuicPacket*> *newlyAckedPackets, uint64_t largestAck, uint64_t smallestAck, PacketNumberSpace pnSpace);

    /**
     * Checks if ack eliciting packets are outstanding.
     *
     * @return True if ack eliciting packets are outstanding, false otherwise.
     */
    bool areAckElicitingPacketsInFlight(PacketNumberSpace pnSpace);

    bool areAckElicitingPacketsInFlight();

    /**
     * Determines the loss time that expires next and the corresponding packet number space.
     *
     * @param retSpace Out parameter for the corresponding packet number space.
     * @return The loss time that expires next.
     */
    simtime_t getLossTimeAndSpace(PacketNumberSpace *retSpace);

    /**
     * Detects a persistent congestion situation.
     *
     * @param lostPackets All packets detected as lost in the current round.
     * @return true, if in persistent congestion, false otherwise.
     *
     * ASSUMPTIONS;
     * - lostPackets are sorted by sent times, from oldest to newest.
     * - only one packet number space is used (i.e. a gap in packet number space means some packets in between are not lost)
     */
    bool inPersistentCongestion(std::vector<QuicPacket *> *lostPackets, QuicPacket *&firstAckEliciting);

    simtime_t getPtoTimeAndSpace(PacketNumberSpace *retSpace);
    void onPacketsLost(std::vector<QuicPacket *> *lostPackets);

    void deleteOldNonInFlightPackets();

    SimTime getReducePacketSizeTime();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_RELIABILITYMANAGER_H_ */
