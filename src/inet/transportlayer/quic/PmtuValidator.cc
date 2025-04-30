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

#include "PmtuValidator.h"

namespace inet {
namespace quic {

PmtuValidator::PmtuValidator(Path *path, Statistics *stats) {
    this->path = path;
    this->stats = stats;
    this->dplpmtud = path->getDplpmtud();
    timeLastPmtuInvalid = SimTime::ZERO;

    readParameters(path->getConnection()->getModule());

    numberOfLostPacketsStat = stats->createStatisticEntry("pmtuValidatorNumberOfLostPackets");
    timeBetweenLostPacketsStat = stats->createStatisticEntry("pmtuValidatorTimeBetweenLostPackets");
    persistentCongestionsStat = stats->createStatisticEntry("pmtuValidatorPersistentCongestions");
}

PmtuValidator::PmtuValidator(PmtuValidator *copy) {
    this->path = copy->path;
    this->stats = nullptr; // do not write stats in a copy
    this->dplpmtud = copy->dplpmtud;
    this->largestPacketSizeAcked = copy->largestPacketSizeAcked;
    this->lostPacketsBySize = copy->lostPacketsBySize;
    this->timeLastPmtuInvalid = copy->timeLastPmtuInvalid;

    this->lostPacketsThreshold = copy->lostPacketsThreshold;
    this->pmtuInvalidTimeThreshold = copy->pmtuInvalidTimeThreshold;
    this->pmtuInvalidSrttFactorThreshold = copy->pmtuInvalidSrttFactorThreshold;
    this->invalidOnPersistentCongestion = copy->invalidOnPersistentCongestion;
}

PmtuValidator::~PmtuValidator() { }

void PmtuValidator::readParameters(cModule *module)
{
    this->lostPacketsThreshold = module->par("pmtuValidatorLostPacketsThreshold");
    this->pmtuInvalidTimeThreshold = module->par("pmtuValidatorTimeThreshold");
    this->pmtuInvalidSrttFactorThreshold = module->par("pmtuValidatorSrttFactorThreshold");
    this->invalidOnPersistentCongestion = module->par("pmtuValidatorInvalidOnPersistentCongestion");
}

/*
std::string listToString(std::list<struct timeSize> &list) {
    std::stringstream ss;
    ss << "[";
    for (auto it: list) {
        ss << "(" << it.at << ", " << it.size << "), ";
    }
    ss << "]";
    return ss.str();
}
*/

void PmtuValidator::onPacketsAcked(std::vector<QuicPacket*> *ackedPackets)
{
    if (!path->usesDplpmtud()) {
        return;
    }

    for (QuicPacket *ackedPacket: *ackedPackets) {
        // the acknowledgement of the packet shows that the current PMTU is at least of the size of the acked packet
        // delete all lost packets that are smaller and sent earlier than the acked packet
        deleteEarlierAndSmallerEntries(&lostPacketsBySize, ackedPacket);

        // to determine the largestAckedSince, information about acked packets sent earlier with a smaller size are unnecessary
        // delete them
        //EV_DEBUG << "largestPacketSizeAcked before: " << listToString(largestPacketSizeAcked) << endl;
        auto it = deleteEarlierAndSmallerEntries(&largestPacketSizeAcked, ackedPacket);
        // add the current acked packet if there is not already a larger acked packet sent later in the list
        if (it == largestPacketSizeAcked.end() || it->size < ackedPacket->getSize()) {
            // add the current acked packet before it
            struct timeSize ts;
            ts.at = ackedPacket->getTimeSent();
            ts.size = ackedPacket->getSize();
            largestPacketSizeAcked.insert(it, ts);
        }
        //EV_DEBUG << "largestPacketSizeAcked after: " << listToString(largestPacketSizeAcked) << endl;
    }
}

bool PmtuValidator::doLostPacketsFulfillPmtuInvalidCriterium(int &largestAckedSinceFirst) {
    EV_DEBUG << "PmtuValidator: doLostPacketsFulfillPmtuInvalidCriterium()" << endl;
    for (auto it=lostPacketsBySize.begin(); it != lostPacketsBySize.end(); ++it) {
        EV_DEBUG << "* lost packet sent at " << it->at << " with size " << it->size << endl;
    }

    if (lostPacketsThreshold <= 0) {
        return true;
    }
    if (lostPacketsThreshold == 1) {
        if (lostPacketsBySize.size() >= 1) {
            largestAckedSinceFirst = largestAckedSince(lostPacketsBySize.front().at);
            return true;
        } else {
            return false;
        }
    }

    // lostPacketsThreshold > 1
    // find two packets whose sent time distance is at least getPmtuInvalidTimeThreshold()
    for (auto first=lostPacketsBySize.begin(); first != lostPacketsBySize.end(); ++first) {
        largestAckedSinceFirst = largestAckedSince(first->at);
        EV_DEBUG << "PmtuValidator: look at first lost packet sent at " << first->at << " with largestAckedSince=" << largestAckedSinceFirst << endl;

        // find the latest lost packet that is larger than the acked packet sent after the first packet
        auto lastLarger = lostPacketsBySize.end();
        --lastLarger;
        for (; lastLarger->size <= largestAckedSinceFirst; --lastLarger);

        if (lastLarger == first) {
            EV_DEBUG << "PmtuValidator: found no second lost packet." << endl;
            if (stats) {
                // this is not a copy, emit numberOfLostPacketsLarger (n) and lastLarger->at-first->at (t)
                stats->getMod()->emit(numberOfLostPacketsStat, 1);
                //stats->getMod()->emit(timeBetweenLostPacketsStat, -1);
            }
            return false;
        }
        EV_DEBUG << "PmtuValidator: found second lost packet with a size of " << lastLarger->size << " sent at " << lastLarger->at << endl;
        if ( (lastLarger->at - first->at) < getPmtuInvalidTimeThreshold() ) {
            // the time distance between the two packets first and lastLarger is too short
            EV_DEBUG << "PmtuValidator: found two lost packets sent at " << first->at << " and at " << lastLarger->at << ", but distance is smaller than " << getPmtuInvalidTimeThreshold() << endl;
            return false;
        }

        // do we have enough lost packets?
        // count the number of lost packets larger than the largest acked packet since the first packet was sent
        int numberOfLostPacketsLarger = 2; // first and lastLarger
        auto between=lastLarger;
        for (--between; numberOfLostPacketsLarger < lostPacketsThreshold && between != first; --between) {
            if (between->size > largestAckedSinceFirst) {
                numberOfLostPacketsLarger++;
            }
        }
        if (stats) {
            // this is not a copy, emit numberOfLostPacketsLarger (n) and lastLarger->at-first->at (t)
            stats->getMod()->emit(numberOfLostPacketsStat, numberOfLostPacketsLarger);
            stats->getMod()->emit(timeBetweenLostPacketsStat, lastLarger->at-first->at);
        }
        if (numberOfLostPacketsLarger >= lostPacketsThreshold) {
            EV_DEBUG << "PmtuValidator: found enough (" << numberOfLostPacketsLarger << ") lost packets that are larger than " << largestAckedSinceFirst << endl;
            return true;
        }
    }
    return false;
}

void PmtuValidator::onPacketsLost(std::vector<QuicPacket*> *lostPackets, bool checkPmtuInvalidCriterium)
{
    EV_DEBUG << "PmtuValidator: onPacketsLost(...)" << endl;
    if (!path->usesDplpmtud()) {
        return;
    }

    for (QuicPacket *lostPacket: *lostPackets) {
        if (lostPacket->isDplpmtudProbePacket()) {
            // ignore loss of DPLPMTUD probe packets
            continue;
        }
        if (lostPacket->getTimeSent() < timeLastPmtuInvalid) {
            // this lost packet was sent before the last time we invalidated the PMTU -> ignore
            continue;
        }
        if (lostPacket->getSize() <= dplpmtud->getMinPlpmtu()) {
            // this lost packet is smaller than the minimum PMTU -> ignore
            continue;
        }
        EV_DEBUG << "largestAckedSince(" << lostPacket->getTimeSent() << ") = " << largestAckedSince(lostPacket->getTimeSent()) << endl;
        if (lostPacket->getSize() <= largestAckedSince(lostPacket->getTimeSent())) {
            // an acked packet with at least the size of this packet was sent at the same time or later than this packet was sent
            continue;
        }

        // find the correct position to add the current lost packet
        auto it = lostPacketsBySize.begin();
        for (; it != lostPacketsBySize.end() && it->at < lostPacket->getTimeSent(); it++);

        // add information about this lost packet before it
        struct timeSize ts;
        ts.at = lostPacket->getTimeSent();
        ts.size = lostPacket->getSize();
        lostPacketsBySize.insert(it, ts);
    }

    int largestAckedSinceFirst = 0;
    if (checkPmtuInvalidCriterium && doLostPacketsFulfillPmtuInvalidCriterium(largestAckedSinceFirst)) {
        pmtuInvalid(largestAckedSinceFirst);
    }
}

void PmtuValidator::onPersistentCongestion(SimTime firstPacketSentTime, uint persistentCongestionCounter)
{
    if (!path->usesDplpmtud()) {
        return;
    }
    if (firstPacketSentTime < timeLastPmtuInvalid) {
        // the first lost packet that induce the persistent congestion was sent before the last time we invalidated the PMTU -> ignore
        return;
    }

    // determine largest acked later than firstPacketSentTime (not equal to firstPacketSentTime)
    int largestAcked = largestAckedSince(firstPacketSentTime + SimTime(1, SimTimeUnit::SIMTIME_NS));
    if (largestAcked >= path->getDplpmtud()->getPlpmtu()) {
        EV_DEBUG << "PmtuValidator: PMTU seems valid despite a persistent congestion because a packet with a size of PMTU was sent and acked later" << endl;
    } else {
        if (stats) {
            stats->getMod()->emit(persistentCongestionsStat, persistentCongestionCounter);
        }
        if (invalidOnPersistentCongestion != 0 && persistentCongestionCounter >= invalidOnPersistentCongestion) {
            EV_DEBUG << "PmtuValidator: PMTU seems invalid due to " << persistentCongestionCounter << " consecutive persistent congestions" << endl;
            pmtuInvalid(largestAcked);
        }
    }
}

simtime_t PmtuValidator::getPmtuInvalidTimeThreshold()
{
    if (pmtuInvalidTimeThreshold < SimTime::ZERO) {
        return pmtuInvalidSrttFactorThreshold * path->smoothedRtt;
    } else {
        return pmtuInvalidTimeThreshold;
    }
}

void PmtuValidator::pmtuInvalid(int largestAckedSinceLoss)
{
    lostPacketsBySize.clear();
    timeLastPmtuInvalid = simTime();
    dplpmtud->onPmtuInvalid(largestAckedSinceLoss);
}

/**
 * Delete all entries in list with an earlier sent time and a smaller packet size
 * \param list The list to delete entries from
 * \param packet The reference packet
 * \return The iterator to the first entry with a later time sent
 */
std::list<struct timeSize>::iterator PmtuValidator::deleteEarlierAndSmallerEntries(std::list<struct timeSize> *list, QuicPacket *packet)
{
    auto it = list->begin();
    while (it != list->end() && it->at <= packet->getTimeSent()) {
        if (it->size <= packet->getSize()) {
            it = list->erase(it);
        } else {
            it++;
        }
    }
    return it;
}

/**
 * \param since The time from when to look for an acked packet size.
 * \return The largest packet size acknowledged since the time given.
 */
int PmtuValidator::largestAckedSince(SimTime since) {
    for (auto it: largestPacketSizeAcked) {
        if (it.at >= since) {
//        if (it.at > since) {
            return it.size;
        }
    }
    return 0;
}

} /* namespace quic */
} /* namespace inet */
