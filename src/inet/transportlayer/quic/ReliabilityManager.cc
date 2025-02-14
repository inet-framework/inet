//
// Copyright (C) 2018 Julius Flohr, Benedikt Kentsch
// Copyright (C) 2019 Timo Voelker
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

#include "ReliabilityManager.h"
#include "PmtuValidator.h"
#include "NoResponseException.h"

namespace inet {
namespace quic {

ReliabilityManager::ReliabilityManager(Connection *connection, TransportParameter *transportParameter, ReceivedPacketsAccountant *receivedPacketsAccountant, ICongestionController *congestionController, Statistics *stats) {
    this->connection = connection;
    this->transportParameter = transportParameter;
    this->receivedPacketsAccountant = receivedPacketsAccountant;
    this->congestionController = congestionController;
    this->stats = stats;
    this->reducePacketTimeThreshold = connection->getModule()->par("reducePacketTimeThreshold");

    Path *path = connection->getPath();

    lossDetectionTimer = connection->createTimer(TimerType::LOSS_DETECTION_TIMER, "lossDetectionTimer");
    ptoCount = 0;
    persistentCongestionCounter = 0;
    latestRtt = SIMTIME_ZERO;
    path->smoothedRtt = kInitialRtt;
    path->rttVar = kInitialRtt / 2;
    minRtt = SIMTIME_ZERO;
    firstRttSample = SIMTIME_ZERO;

    // should be per packet number space
    largestAckedPacketNumber = std::numeric_limits<uint64_t>::max();
    lastSentAckElicitingPacketTime = SIMTIME_ZERO;
    lossTime = SIMTIME_ZERO;

    //lastSentCryptoPacketTime = SIMTIME_ZERO;
}

ReliabilityManager::~ReliabilityManager() {
    delete lossDetectionTimer;
    if (sentPackets.size() > 0) {
        int memSize = 0;
        for (auto& mapEntry: sentPackets) {
            QuicPacket *unacked = mapEntry.second;
            if (unacked->isAckEliciting()) {
                if (connection && connection->getModule() && connection->getModule()->getParentModule()) {
                    EV_WARN << connection->getModule()->getParentModule()->getName() << ": ";
                }
                EV_WARN << "ReliabilityManager still had the unacked ack-eliciting sentPacket " << mapEntry.first << " when destructed" << endl;
            }
            memSize += sizeof(mapEntry);
            memSize += unacked->getMemorySize();
            delete unacked;
        }
        if (memSize > 0 ) {
            if (connection && connection->getModule() && connection->getModule()->getParentModule()) {
                EV_INFO << connection->getModule()->getParentModule()->getName() << ": ";
            }
            EV_INFO << "ReliabilityManager stored unacked packets with a total memory size of " << memSize << " B before destruction" << endl;
        }
    }
}

void ReliabilityManager::deleteOldNonInFlightPackets()
{
    simtime_t oldSendTime = simTime() - kOldNonInFlightPacketTimeThreshold;

    // find old non in-flight packets
    std::vector<QuicPacket*> oldNonInFlightPackets;
    for (auto& mapEntry: sentPackets) {
        QuicPacket *unacked = mapEntry.second;
        if (!unacked->countsAsInFlight() && unacked->getTimeSent() <= oldSendTime) {
            oldNonInFlightPackets.push_back(unacked);
        }
    }
    // delete these non in-flight packets to free up memory
    for (QuicPacket *oldNonInFlightPacket: oldNonInFlightPackets) {
        sentPackets.erase(oldNonInFlightPacket->getPacketNumber());
        delete oldNonInFlightPacket;
    }
}

/**
 * Called by Connection, when a new packet were sent.
 * \param packet The sent packet.
 */
void ReliabilityManager::onPacketSent(QuicPacket *packet)
{
    sentPackets.insert({ packet->getPacketNumber(), packet });

    //static SimTime lastTimeSent = SimTime(-1, SimTimeUnit::SIMTIME_S);
    //static int packetsSentAtSameTime = 0;
    //SimTime timeSent = simTime();
    //if (timeSent == lastTimeSent) {
    //    packetsSentAtSameTime++;
    //} else {
    //    packetsSentAtSameTime = 0;
    //}
    //// add a little delay if we send multiple packets at the exact same simulation time
    //packet->setTimeSent(timeSent + SimTime(packetsSentAtSameTime, SimTimeUnit::SIMTIME_AS));
    //lastTimeSent = timeSent;
    packet->setTimeSent(simTime());
    if (packet->countsAsInFlight()) {
        if (packet->isAckEliciting()) {
            lastSentAckElicitingPacketTime = simTime();
        }
        congestionController->onPacketSent(packet);
        setLossDetectionTimer();
    }
    deleteOldNonInFlightPackets();
}

/**
 * Called by EstablishedConnectionState, when a ack frame were received.
 * \param ackFrame The received ack frame.
 * TODO: consider different packet number spaces
 */
void ReliabilityManager::onAckReceived(const Ptr<const AckFrameHeader>& ackFrame)
{
    EV_TRACE << "enter ReliabilityManager::onAckReceived" << endl;

    if (largestAckedPacketNumber == std::numeric_limits<uint64_t>::max()) {
        largestAckedPacketNumber = ackFrame->getLargestAck();
    } else {
        largestAckedPacketNumber = std::max(largestAckedPacketNumber, ackFrame->getLargestAck().getIntValue());
    }

    QuicPacket *largestAcknolwedNewlyAcked = getSentPacket(ackFrame->getLargestAck());
    std::vector<QuicPacket*> *newlyAckedPackets = detectAndRemoveAckedPackets(ackFrame);

    // Nothing to do if there are no newly acked packets.
    if (newlyAckedPackets->empty()) {
        EV_DEBUG << "no newly acked packets" << endl;
        delete newlyAckedPackets;
        return;
    }

    // If the largest acknowledged is newly acked and
    // at least one ack-eliciting was newly acked, update the RTT.
    if (largestAcknolwedNewlyAcked != nullptr && includesAckEliciting(newlyAckedPackets)) {
        static simsignal_t latestRttStat = stats->createStatisticEntry("latestRtt");
        latestRtt = simTime() - largestAcknolwedNewlyAcked->getTimeSent();
        stats->getMod()->emit(latestRttStat, latestRtt);
        updateRtt(ackFrame->getAckDelay() << transportParameter->ackDelayExponent); // use peer endpoint's transport parameter
    }

    /* TODO:
    // Process ECN information if present.
    if (ACK frame contains ECN information):
        ProcessECN(ack)
    */

    // report acked packets before reporting lost packets to the PMTU Validator
    if (connection->getPath()->hasPmtuValidator()) {
        connection->getPath()->getPmtuValidator()->onPacketsAcked(newlyAckedPackets);
    }

    std::vector<QuicPacket*> *lostPackets = detectAndRemoveLostPackets();
    if (!lostPackets->empty()) {
        onPacketsLost(lostPackets);
    } else {
        persistentCongestionCounter = 0;
    }
    delete lostPackets;

    for (QuicPacket* ackedPacket : *newlyAckedPackets) {
        onPacketAcked(ackedPacket);
    }
    delete newlyAckedPackets;


    // Reset pto_count unless the client is unsure if
    // the server has validated the client's address.
    // TODO:
    //if (PeerCompletedAddressValidation())
        ptoCount = 0;

    setLossDetectionTimer();

    EV_TRACE << "leave ReliabilityManager::onAckReceived" << endl;
}

/**
 * Determines which packets were newly acked (not acked in a received ack frame before) by the given ack frame
 * and removes them from the sentPackets map.
 * \param ackFrame The ack frame to analyze.
 * \return Pointer to a vector with all newly acked packets.
 */
std::vector<QuicPacket*> *ReliabilityManager::detectAndRemoveAckedPackets(const Ptr<const AckFrameHeader>& ackFrame)
{
    std::vector<QuicPacket*> *newlyAckedPackets = new std::vector<QuicPacket*>();
    uint64_t smallestUnack = sentPackets.begin()->first;

    uint64_t largestAck = ackFrame->getLargestAck();
    uint64_t smallestAck = ackFrame->getLargestAck() - ackFrame->getFirstAckRange();
    moveNewlyAckedPackets(newlyAckedPackets, largestAck, smallestAck);
    for (int i=0; i<ackFrame->getAckRangeArraySize(); i++) {
        const AckRange &ackRange = ackFrame->getAckRange(i);
        largestAck = smallestAck - ackRange.gap - 2;
        if (largestAck < smallestUnack) {
            // this ack range is below the lowest packet number of all outstanding packets
            // given that the ack ranges are in descending order, this and all following ranges will not add newly acked packets
            break;
        }
        smallestAck = largestAck - ackRange.ackRange;
        moveNewlyAckedPackets(newlyAckedPackets, largestAck, smallestAck);
    }

    return newlyAckedPackets;
}
/**
 * Moves all packets with packet numbers from largestAck to smallestAck from the sentPackets map to the newlyAckedPackets vector.
 * \param newlyAckedPackets Adds packets to this vector.
 * \param largestAck Packet number of the first packet to add.
 * \param smallestAck Packet number of the last packet to add.
 */
void ReliabilityManager::moveNewlyAckedPackets(std::vector<QuicPacket*> *newlyAckedPackets, uint64_t largestAck, uint64_t smallestAck) {
    for (uint64_t packetNumber=largestAck; packetNumber>=smallestAck; packetNumber--) {
        auto result = sentPackets.find(packetNumber);
        if (result != sentPackets.end()) {
            newlyAckedPackets->push_back(result->second);
            sentPackets.erase(packetNumber);
        }
        if (packetNumber == 0) {
            // if we don't leave the for loop now, we will never, since packetNumber is an unsigned int.
            break;
        }
    }
}

/**
 * Checks if one of the given vector of packets is an ack eliciting packet.
 * \param packets Vector of packets to check.
 * \return true if the given vector of packets contains at least one ack eliciting packet, false otherwise.
 */
bool ReliabilityManager::includesAckEliciting(std::vector<QuicPacket*> *packets)
{
    for (QuicPacket *packet : *packets) {
        if (packet->isAckEliciting()) {
            return true;
        }
    }
    return false;
}

/**
 * Update the RTT values upon a received ack.
 * \param ackDelay The ack delay value as reported in the received ack.
 */
void ReliabilityManager::updateRtt(uint64_t ackDelay)
{
    Path *path = connection->getPath();
    EV_DEBUG << "ReliabilityManager::updateRtt: latestRtt=" << latestRtt << "; old: minRtt=" << minRtt << ", smoothedRtt=" << path->smoothedRtt << ", rttVar=" << path->rttVar << "; ";
    if (firstRttSample == SIMTIME_ZERO) {
        minRtt = latestRtt;
        path->smoothedRtt = latestRtt;
        path->rttVar = latestRtt / 2;
        firstRttSample = simTime();
        EV_DEBUG << " new: minRtt=" << minRtt << ", smoothedRtt=" << path->smoothedRtt << ", rttVar=" << path->rttVar << endl;
        return;
    }

    // min_rtt ignores ack delay.
    minRtt = std::min(minRtt, latestRtt);
    // Limit ack_delay by max_ack_delay after handshake
    // confirmation. Note that ack_delay is 0 for
    // acknowledgements of Initial and Handshake packets.
    // TODO:
    // if (handshake confirmed):
        simtime_t ackDelayTime = SimTime(ackDelay, SimTimeUnit::SIMTIME_US);
        ackDelayTime = SimTime().setRaw(std::min(ackDelayTime.raw(), transportParameter->maxAckDelay.raw()));

    // Adjust for ack delay if plausible.
    simtime_t adjustedRtt = latestRtt;
    if (latestRtt >= minRtt + ackDelayTime) { //(latestRtt > minRtt + ackDelayTime) {
        adjustedRtt = latestRtt - ackDelayTime;
    }

    path->rttVar = 3.0/4 * path->rttVar + 1.0/4 * SimTime().setRaw(std::abs(path->smoothedRtt.raw() - adjustedRtt.raw()));
    path->smoothedRtt = 7.0/8 * path->smoothedRtt + 1.0/8 * adjustedRtt;
    EV_DEBUG << "new: minRtt=" << minRtt << ", smoothedRtt=" << path->smoothedRtt << ", rttVar=" << path->rttVar << "; ackDelayTime=" << ackDelayTime << ", adjustedRtt=" << adjustedRtt << endl;
}

/**
 * Called for each packet that were newly acked in a received ack frame.
 * \param ackedPacket The packet that were acked.
 */
void ReliabilityManager::onPacketAcked(QuicPacket *ackedPacket)
{
    EV_DEBUG << "packet " << ackedPacket->getPacketNumber() << " acked" << endl;
    congestionController->onPacketAcked(ackedPacket);
    receivedPacketsAccountant->onPacketAcked(ackedPacket);
    ackedPacket->onPacketAcked();
    delete ackedPacket;
}

/**
 * Determines the loss time that expires next.
 * Each packet number space has its own lossTime. One different packet number spaces are handled, this method becomes more complex.
 * \return The loss time that expires next.
 */
simtime_t ReliabilityManager::getEarliestLossTime()
{
    return lossTime;
}

simtime_t ReliabilityManager::getPtoTime()
{
    Path *path = connection->getPath();
    SimTime duration = (path->smoothedRtt + SimTime().setRaw(std::max(4 * path->rttVar.raw(), kGranularity.raw()))) * (1 << ptoCount);

    /* TODO:
    // Arm PTO from now when there are no inflight packets.
    if (no in-flight packets):
      assert(!PeerCompletedAddressValidation())
      if (has handshake keys):
        return (now() + duration), Handshake
      else:
        return (now() + duration), Initial
    */

    SimTime ptoTimeout;
    /* TODO:
    ptoTimeout = infinite
    ptoSpace = Initial
    for space in [ Initial, Handshake, ApplicationData ]:
       if (no in-flight packets in space):
           continue;
       if (space == ApplicationData):
         // Skip Application Data until handshake confirmed.
         if (handshake is not confirmed):
           return pto_timeout, pto_space
         // Include max_ack_delay and backoff for Application Data.
         duration += max_ack_delay * (2 ^ pto_count)

       t = time_of_last_ack_eliciting_packet[space] + duration
       if (t < pto_timeout):
     */

    duration += transportParameter->maxAckDelay * (1 << ptoCount);
    ptoTimeout = lastSentAckElicitingPacketTime + duration;
    return ptoTimeout;
}

/**
 * Resets the loss detection timer.
 */
void ReliabilityManager::setLossDetectionTimer()
{
    simtime_t earliestLossTime = getEarliestLossTime();
    if (earliestLossTime != SimTime::ZERO) {
        // Time threshold loss detection.
        lossDetectionTimer->update(earliestLossTime);
        return;
    }

    /*TODO:
    if (server is at anti-amplification limit):
       // The server's timer is not set if nothing can be sent.
       loss_detection_timer.cancel()
       return
    */

    // Don't arm timer if there are no ack-eliciting packets
    // in flight.
    if (!areAckElicitingPacketsInFlight()) { // TODO: && PeerCompletedAddressValidation()):
        EV_DEBUG << "no more ack eliciting packets in flight -> cancel lossDetectionTimer" << endl;
        lossDetectionTimer->cancel();
        return;
    }
    EV_DEBUG << "there are still ack eliciting packets in flight -> update lossDetectionTimer" << endl;

    SimTime timeout = getPtoTime();
    if (timeout < simTime()) {
        timeout = simTime();
    }
    lossDetectionTimer->update(timeout);
    EV_DEBUG << "lossDetectionTimer set to " << timeout << endl;
}

/**
 * Called from EstablishedConnectionState when the loss detection timer fires.
 */
void ReliabilityManager::onLossDetectionTimeout()
{
    SimTime earliestLossTime = getEarliestLossTime();
    if (earliestLossTime != SimTime::ZERO) {
        // Time threshold loss Detection
        std::vector<QuicPacket*> *lostPackets = detectAndRemoveLostPackets();
        ASSERT(!lostPackets->empty());
        onPacketsLost(lostPackets);
        delete lostPackets;
        setLossDetectionTimer();
        return;
    }

    /* TODO:
    if (bytes_in_flight > 0):
    */
        // PTO. Send new data if available, else retransmit old data.
        // If neither is available, send a single PING frame.
        if (ptoCount == kMaxProbePackets) {
            throw NoResponseException("ReliabilityManager: Sent already kMaxProbePackets (10?) probe packets. Won't send more. Peer isn't answering.");
        }
        connection->sendProbePacket(ptoCount+1);
    /*
    else:
       assert(!PeerCompletedAddressValidation())
       // Client sends an anti-deadlock packet: Initial is padded
       // to earn more anti-amplification credit,
       // a Handshake packet proves address ownership.
       if (has Handshake keys):
         SendOneAckElicitingHandshakePacket()
       else:
         SendOneAckElicitingPaddedInitialPacket()
    */

    ptoCount++;
    setLossDetectionTimer();
}

/**
 * Detects packets as lost, if their sent time is old enough
 * or if enough new packets, sent after them, were already acked.
 */
std::vector<QuicPacket*> *ReliabilityManager::detectAndRemoveLostPackets()
{
    ASSERT(largestAckedPacketNumber != std::numeric_limits<uint64_t>::max());
    Path *path = connection->getPath();

    lossTime = SimTime::ZERO;
    simtime_t lossDelay = kTimeThreshold * SimTime().setRaw(std::max(latestRtt.raw(), path->smoothedRtt.raw()));

    // Minimum time of kGranularity before packets are deemed lost.
    lossDelay = SimTime().setRaw(std::max(lossDelay.raw(), kGranularity.raw()));

    // Packets sent before this time are deemed lost.
    simtime_t lostSendTime = simTime() - lossDelay;

    // Packets with packet numbers before this are deemed lost.
    uint64_t lostPacketNumber = largestAckedPacketNumber - kPacketThreshold;
    bool lostPacketNumberOverflow = false;
    if (kPacketThreshold > largestAckedPacketNumber) {
        lostPacketNumberOverflow = true;
    }

    std::vector<QuicPacket*> *lostPackets = new std::vector<QuicPacket*>();
    for (auto& mapEntry: sentPackets) {
        QuicPacket *unacked = mapEntry.second;
        if (unacked->getPacketNumber() > largestAckedPacketNumber) {
            continue;
        }

        // Mark packet as lost, or set time when it should be marked.
        // Note: The use of kPacketThreshold here assumes that there
        // were no sender-induced gaps in the packet number space.
        if (unacked->getTimeSent() <= lostSendTime || (!lostPacketNumberOverflow && lostPacketNumber >= unacked->getPacketNumber())) {
            EV_DEBUG << "packet " << unacked->getPacketNumber() << " is now considered lost. "
                    << unacked->getTimeSent() << " <= " << lostSendTime << " || (" << !lostPacketNumberOverflow << " && " << unacked->getPacketNumber() << " <= " << lostPacketNumber << ")" << endl;
            lostPackets->push_back(unacked);
            //Packet *pkt = unacked->createOmnetPacket();
            //stats->getMod()->emit(packetLostSignal, pkt);
            //delete(pkt);
        } else {
            if (lossTime == SimTime::ZERO) {
                lossTime = unacked->getTimeSent() + lossDelay;
            } else {
                lossTime = SimTime().setRaw(std::min(lossTime.raw(), (unacked->getTimeSent() + lossDelay).raw()));
            }
        }
    }

    for (QuicPacket *lostPacket: *lostPackets) {
        sentPackets.erase(lostPacket->getPacketNumber());
    }

    return lostPackets;
}

/**
 * Detects a persistent congestion situation.
 * \param lostPackets All packets detected as lost in the current round.
 * \return true, if in persistent congestion, false otherwise.
 *
 * ASSUMPTIONS;
 * - lostPackets are sorted by sent times, from oldest to newest.
 * - only one packet number space is used (i.e. a gap in packet number space means some packets in between are not lost)
 */
bool ReliabilityManager::inPersistentCongestion(std::vector<QuicPacket *> *lostPackets, QuicPacket *&firstAckEliciting)
{
    Path *path = connection->getPath();
    QuicPacket *lastLostPacket = nullptr;

    if (firstRttSample == SimTime::ZERO) {
        return false;
    }

    SimTime pcDuration = (
            path->smoothedRtt
            + SimTime().setRaw(std::max(4 * path->rttVar.raw(), kGranularity.raw()))
            + transportParameter->maxAckDelay
        ) * kPersistentCongestionThreshold;

    for (QuicPacket *lostPacket: *lostPackets) {
        if (lostPacket->getTimeSent() <= firstRttSample) {
            continue;
        }

        if (lastLostPacket != nullptr && lostPacket->getPacketNumber() != (lastLostPacket->getPacketNumber()+1)) {
            // there is a gap in the sequence of packet numbers of lost packets
            // looks like packets in between are not lost -> reset firstAckEliciting
            firstAckEliciting = nullptr;
        }
        // for persistent congestion detection, the first and the last packet must be ack eliciting
        if (lostPacket->isAckEliciting()) {
            if (firstAckEliciting == nullptr) {
                firstAckEliciting = lostPacket;
            } else {
                // all packets sent between lostPacket and firstAckEliciting packet are lost
                // check if the sent times of these two packets exceed the persistent congestion duration
                if ((lostPacket->getTimeSent() - firstAckEliciting->getTimeSent()) > pcDuration) {
                    EV_DEBUG << "ReliabilityManager: persistent congestion detected" << endl;
                    static simsignal_t persistentCongestionPeriodStat = stats->createStatisticEntry("persistentCongestionPeriod");
                    stats->getMod()->emit(persistentCongestionPeriodStat, firstAckEliciting->getTimeSent());
                    stats->getMod()->emit(persistentCongestionPeriodStat, lostPacket->getTimeSent());
                    return true;
                }
            }
        }

        lastLostPacket = lostPacket;
    }

    return false;
}

/**
 * Checks if ack eliciting packets are outstanding.
 * \return True if ack eliciting packets are outstanding, false otherwise.
 */
bool ReliabilityManager::areAckElicitingPacketsInFlight()
{
    for (auto &mapEntry : sentPackets) {
        QuicPacket *packet = mapEntry.second;
        if (packet->isAckEliciting()) {
            return true;
        }
    }
    return false;
}

/**
 * Creates a vector of all ack eliciting packets that are outstanding.
 * \return Pointer to a vector of QuicPackets, that are ack eliciting and outstanding. Might be empty.
 */
std::vector<QuicPacket*> *ReliabilityManager::getAckElicitingSentPackets()
{
    std::vector<QuicPacket*> *ackElicitingSentPackets = new std::vector<QuicPacket*>();
    for (auto &mapEntry : sentPackets) {
        QuicPacket *packet = mapEntry.second;
        if (packet->isAckEliciting()) {
            ackElicitingSentPackets->push_back(packet);
        }
    }
    return ackElicitingSentPackets;
}

QuicPacket *ReliabilityManager::getSentPacket(uint64_t droppedPacketNumber)
{
    auto it = sentPackets.find(droppedPacketNumber);
    if (it == sentPackets.end()) {
        return nullptr;
    }
    return it->second;
}

void ReliabilityManager::onPacketsLost(std::vector<QuicPacket*> *lostPackets)
{
    Path *path = connection->getPath();
    QuicPacket *firstAckEliciting = nullptr;
    bool inPc = inPersistentCongestion(lostPackets, firstAckEliciting);
    if (inPc) {
        persistentCongestionCounter++;
        if (path->hasPmtuValidator()) {
            path->getPmtuValidator()->onPersistentCongestion(firstAckEliciting->getTimeSent(), persistentCongestionCounter);
        }
    } else {
        persistentCongestionCounter = 0;
    }
    if (path->hasPmtuValidator()) {
        path->getPmtuValidator()->onPacketsLost(lostPackets);
    }
    congestionController->onPacketsLost(lostPackets, inPc);
    for (QuicPacket* lostPacket : *lostPackets) {
        lostPacket->onPacketLost();
        delete lostPacket;
    }
}

SimTime ReliabilityManager::getReducePacketSizeTime() {
    if (reducePacketTimeThreshold < SimTime::ZERO) {
        Path *path = connection->getPath();
        //EV_DEBUG << "ReliabilityManager: E = " << path->smoothedRtt << " + " << SimTime().setRaw(std::max(4 * path->rttVar.raw(), kGranularity.raw())) << " + " << transportParameter->maxAckDelay << " = " << (path->smoothedRtt + SimTime().setRaw(std::max(4 * path->rttVar.raw(), kGranularity.raw()))) + transportParameter->maxAckDelay << endl;
        SimTime pto = (path->smoothedRtt + SimTime().setRaw(std::max(4 * path->rttVar.raw(), kGranularity.raw()))) + transportParameter->maxAckDelay;
        //EV_DEBUG << "ReliabilityManager: w = " << simTime() << " - (" << -reducePacketTimeThreshold.inUnit(SimTimeUnit::SIMTIME_S) << " * " << pto << ") = " << simTime() << " - " << (-reducePacketTimeThreshold.inUnit(SimTimeUnit::SIMTIME_S) * pto) << " = " << simTime() - (-reducePacketTimeThreshold.inUnit(SimTimeUnit::SIMTIME_S) * pto) << endl;
        return simTime() - (-reducePacketTimeThreshold.inUnit(SimTimeUnit::SIMTIME_S) * pto);
    } else {
        return simTime() - reducePacketTimeThreshold;
    }
}

bool ReliabilityManager::reducePacketSize(int minSize, int maxSize) {
    if (reducePacketTimeThreshold == SimTime::ZERO) {
        return false;
    }
    SimTime w = getReducePacketSizeTime();
    //EV_DEBUG << "ReliabilityManager: reducePacketSize with w = " << w << endl;
    for (auto &mapEntry : sentPackets) {
        QuicPacket *packet = mapEntry.second;
        //EV_DEBUG << "* packet sent at " << packet->getTimeSent() << ", PMTU probe: " << packet->isDplpmtudProbePacket() << ", packet number larger than largestAcked: " << (packet->getPacketNumber() > largestAckedPacketNumber) << ", size within limits: " << (minSize < packet->getSize() && packet->getSize() <= maxSize) << endl;
        if (packet->getTimeSent() > w) {
            break;
        }
        if (packet->isAckEliciting()
                && !packet->isDplpmtudProbePacket()
                && packet->getPacketNumber() > largestAckedPacketNumber
                && minSize < packet->getSize() && packet->getSize() <= maxSize) {
            return true;
        }
    }
    return false;
}

} /* namespace quic */
} /* namespace inet */
