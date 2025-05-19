//
// Copyright (C) 2019-2024 Timo Völker
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_PMTUVALIDATOR_H_
#define INET_TRANSPORTLAYER_QUIC_PMTUVALIDATOR_H_

#include "packet/QuicPacket.h"
#include "dplpmtud/Dplpmtud.h"
#include "Path.h"
#include "Statistics.h"

namespace inet {
namespace quic {

class Path;
class Dplpmtud;

struct timeSize {
    SimTime at;
    int size;
};

class PmtuValidator {
public:
    PmtuValidator(Path *path, Statistics *stats);
    PmtuValidator(PmtuValidator *copy);
    virtual ~PmtuValidator();

    void onPacketsAcked(std::vector<QuicPacket*> *ackedPackets);
    void onPacketsLost(std::vector<QuicPacket*> *lostPackets, bool checkPmtuInvalidCriterium=true);
    void onPersistentCongestion(SimTime firstPacketSentTime, uint persistentCongestionCounter);
    bool doLostPacketsFulfillPmtuInvalidCriterium(int &largestAckedSinceFirst);

private:
    Path *path;
    Statistics *stats;
    Dplpmtud *dplpmtud;
    std::list<struct timeSize> largestPacketSizeAcked;
    std::list<struct timeSize> lostPacketsBySize;
    SimTime timeLastPmtuInvalid;

    int lostPacketsThreshold;
    SimTime pmtuInvalidTimeThreshold;
    int pmtuInvalidSrttFactorThreshold;
    int invalidOnPersistentCongestion;

    simsignal_t numberOfLostPacketsStat;
    simsignal_t timeBetweenLostPacketsStat;
    simsignal_t persistentCongestionsStat;

    /**
     * Delete all entries in list with an earlier sent time and a smaller packet size.
     *
     * @param list The list to delete entries from.
     * @param packet The reference packet.
     * @return The iterator to the first entry with a later time sent.
     */
    std::list<struct timeSize>::iterator deleteEarlierAndSmallerEntries(std::list<struct timeSize> *list, QuicPacket *packet);

    /**
     * @param since The time from when to look for an acked packet size.
     * @return The largest packet size acknowledged since the time given.
     */
    int largestAckedSince(SimTime since);

    void pmtuInvalid(int largestAckedSinceLoss);
    simtime_t getPmtuInvalidTimeThreshold();
    void readParameters(cModule *module);

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_PMTUVALIDATOR_H_ */
