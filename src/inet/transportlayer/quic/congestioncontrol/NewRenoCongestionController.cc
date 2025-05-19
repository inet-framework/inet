//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "NewRenoCongestionController.h"

namespace inet {
namespace quic {

NewRenoCongestionController::NewRenoCongestionController() {
    this->stats = nullptr;
    this->path = nullptr;

    bytesInFlight = 0;
    partialBytesAcked = 0;
    congestionRecoveryStartTime = SimTime::ZERO;
    congestionWindow = getInitialWindow();

    accurateIncreaseInNewRenoCongestionAvoidance = true;
    ssthresh = std::numeric_limits<uint32_t>::max();
    EV_DEBUG << "NewRenoCongestionController: congestionWindow=" << congestionWindow << ", bytesInFlight=" << bytesInFlight << endl;
}

NewRenoCongestionController::~NewRenoCongestionController() { }

void NewRenoCongestionController::readParameters(cModule *module)
{
    accurateIncreaseInNewRenoCongestionAvoidance = module->par("accurateIncreaseInNewRenoCongestionAvoidance");
}

void NewRenoCongestionController::setStatistics(Statistics *stats)
{
    this->stats = stats;
    cwndStat = stats->createStatisticEntry("cwnd");
    bytesInFlightStat = stats->createStatisticEntry("bytesInFlight");
    partialBytesAckedStat = stats->createStatisticEntry("partialBytesAcked");
    emitStatValue(cwndStat, congestionWindow);
    emitStatValue(bytesInFlightStat, bytesInFlight);
    emitStatValue(partialBytesAckedStat, partialBytesAcked);
}

void NewRenoCongestionController::setPath(Path *path)
{
    bool cwndIsInitial = congestionWindow == getInitialWindow();
    bool cwndIsMinimum = congestionWindow == getMinimumWindow();

    this->path = path;

    if (cwndIsInitial) {
        congestionWindow = getInitialWindow();
    }
    if (cwndIsMinimum) {
        congestionWindow = getMinimumWindow();
    }
}

void NewRenoCongestionController::onPacketAcked(QuicPacket *ackedPacket)
{
    if (!ackedPacket->countsAsInFlight()) {
        return;
    }
    // Remove from bytes_in_flight.
    if (ackedPacket->getSize() < bytesInFlight) {
        bytesInFlight = bytesInFlight - ackedPacket->getSize();
    } else {
        bytesInFlight = 0;
        // TODO: According to RFC4960, we need to set partialBytesAcked to 0, when bytesInFlight becomes 0
        // But, is this the right position or is it too early?
        partialBytesAcked = 0;
        emitStatValue(partialBytesAckedStat, partialBytesAcked);
    }
    emitStatValue(bytesInFlightStat, bytesInFlight);

    if (inCongestionRecovery(ackedPacket->getTimeSent())) {
        // Do not increase congestion window in recovery period.
        EV_DEBUG << "NewRenoCongestionController: inCongestionRecovery - congestionWindow=" << congestionWindow << ", bytesInFlight=" << bytesInFlight << endl;

        if (congestionWindow >= ssthresh && accurateIncreaseInNewRenoCongestionAvoidance) {
            // count acked bytes.
            partialBytesAcked += ackedPacket->getSize();
            emitStatValue(partialBytesAckedStat, partialBytesAcked);
        }
        return;
    }
    if (isAppLimited()) {
        // Do not increase congestion_window if application
        // limited.
        EV_DEBUG << "NewRenoCongestionController: AppLimited - congestionWindow=" << congestionWindow << ", bytesInFlight=" << bytesInFlight << endl;
        return;
    }

    if (congestionWindow < ssthresh) {
        // Slow start.
        congestionWindow += ackedPacket->getSize();
        EV_DEBUG << "NewRenoCongestionController: increase in slow start - congestionWindow=" << congestionWindow << ", bytesInFlight=" << bytesInFlight << endl;
        emitStatValue(cwndStat, congestionWindow);

        partialBytesAcked = 0;
        emitStatValue(partialBytesAckedStat, partialBytesAcked);
    } else {
        // Congestion avoidance.

        if (accurateIncreaseInNewRenoCongestionAvoidance) {
            // use the recommended way as described in 3.1. of RFC5681 (like SCTP (RFC4960) does it),
            // instead of the way shown in the pseudo-code of quic-recovery draft

            // count acked bytes.
            partialBytesAcked += ackedPacket->getSize();
            emitStatValue(partialBytesAckedStat, partialBytesAcked);

            if (partialBytesAcked > (congestionWindow-getMaxDatagramSize())) {
                // enough bytes acked, we are allowed to increase cwnd
                congestionWindow += getMaxDatagramSize();
                EV_DEBUG << "NewRenoCongestionController: accurate increase in congestion avoidance - congestionWindow=" << congestionWindow << ", bytesInFlight=" << bytesInFlight << endl;
                emitStatValue(cwndStat, congestionWindow);

                // reset acked bytes counter
                if (congestionWindow < partialBytesAcked) {
                    partialBytesAcked = partialBytesAcked - congestionWindow;
                } else {
                    partialBytesAcked = 0;
                }
                emitStatValue(partialBytesAckedStat, partialBytesAcked);
            }
        } else {
            //congestionWindow += maxDatagramSize * ackedPacket->getSize() / congestionWindow;
            static uint32_t remainder = 0;
            uint32_t addCongestionWindow = (getMaxDatagramSize() * ackedPacket->getSize() + remainder) / congestionWindow;
            remainder = (getMaxDatagramSize() * ackedPacket->getSize() + remainder) % congestionWindow;
            if (addCongestionWindow > 0) {
                congestionWindow += addCongestionWindow;
                EV_DEBUG << "NewRenoCongestionController: approximate increase in congestion avoidance - congestionWindow=" << congestionWindow << ", bytesInFlight=" << bytesInFlight << endl;
                emitStatValue(cwndStat, congestionWindow);
            }
        }
    }
}

void NewRenoCongestionController::onPacketSent(QuicPacket *sentPacket)
{
    bytesInFlight += sentPacket->getSize();
    emitStatValue(bytesInFlightStat, bytesInFlight);
    //emitStatValue(cwndStat, congestionWindow);
    EV_DEBUG << "NewRenoCongestionController: congestionWindow=" << congestionWindow << ", bytesInFlight=" << bytesInFlight << endl;
}

void NewRenoCongestionController::onPacketsLost(std::vector<QuicPacket *> *lostPackets, bool inPersistentCongestion)
{
    ASSERT(!lostPackets->empty());

    QuicPacket *largestLostPacket = nullptr;
    // Remove lost packets from bytes_in_flight.
    for (QuicPacket *lostPacket : *lostPackets) {
        if (lostPacket->countsAsInFlight()) {
            bytesInFlight -= lostPacket->getSize();

            // A DPLPMTUD probe packet should not trigger a congestion event
            if (!lostPacket->isDplpmtudProbePacket()) {
                if (largestLostPacket == nullptr || largestLostPacket->getPacketNumber() < lostPacket->getPacketNumber()) {
                    largestLostPacket = lostPacket;
                }
            }
        }
    }
    emitStatValue(bytesInFlightStat, bytesInFlight);
    if (largestLostPacket != nullptr) {
        congestionEvent(largestLostPacket->getTimeSent());
    }

    // Reset the congestion window if the loss of these
    // packets indicates persistent congestion.
    if (inPersistentCongestion) {
        congestionWindow = getMinimumWindow();
        congestionRecoveryStartTime = SimTime::ZERO;
        emitStatValue(cwndStat, congestionWindow);
    }
    EV_DEBUG << "NewRenoCongestionController: congestionWindow=" << congestionWindow << ", bytesInFlight=" << bytesInFlight << endl;
}

uint32_t NewRenoCongestionController::getRemainingCongestionWindow()
{
    if (allowOnePacket) {
        allowOnePacket = false;
        return getMaxDatagramSize();
    }
    if (bytesInFlight > congestionWindow) {
        return 0;
    }
    return (congestionWindow - bytesInFlight);
}

bool NewRenoCongestionController::inCongestionRecovery(simtime_t sentTime)
{
    return (sentTime < congestionRecoveryStartTime);
}

void NewRenoCongestionController::setAppLimited(bool appLimited)
{
    this->appLimited = appLimited;
}

bool NewRenoCongestionController::isAppLimited()
{
    return this->appLimited;
}

void NewRenoCongestionController::congestionEvent(simtime_t sentTime)
{
    // No reaction if already in a recovery period.
    if (inCongestionRecovery(sentTime)) {
        return;
    }

    // Enter recovery period.
    congestionRecoveryStartTime = simTime();
    ssthresh = congestionWindow * kLossReductionFactor;
    congestionWindow = std::max(ssthresh, getMinimumWindow());
    allowOnePacket = true;
    partialBytesAcked = 0;
    emitStatValue(partialBytesAckedStat, partialBytesAcked);
    emitStatValue(cwndStat, congestionWindow);
}

/* TODO
ProcessECN(ack):
    // If the ECN-CE counter reported by the peer has increased,
    // this could be a new congestion event.
    if (ack.ce_counter > ecn_ce_counter):
    ecn_ce_counter = ack.ce_counter
    CongestionEvent(sent_packets[ack.largest_acked].time_sent)
*/

void NewRenoCongestionController::emitStatValue(simsignal_t signal, uint32_t value)
{
    if (stats) {
        stats->getMod()->emit(signal, value);
    }
}

uint32_t NewRenoCongestionController::getMinimumWindow() {
    return 2 * getMaxDatagramSize();
}

uint32_t NewRenoCongestionController::getInitialWindow() {
    return std::min(10 * getMaxDatagramSize(), std::max(2 * getMaxDatagramSize(), 14720u));
}

uint32_t NewRenoCongestionController::getMaxDatagramSize()
{
    if (path) {
        return path->getMaxQuicPacketSize();
    } else {
        return kMaxDatagramSize;
    }
}

} /* namespace quic */
} /* namespace inet */
