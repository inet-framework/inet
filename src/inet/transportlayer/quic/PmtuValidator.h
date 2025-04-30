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

    std::list<struct timeSize>::iterator deleteEarlierAndSmallerEntries(std::list<struct timeSize> *list, QuicPacket *packet);
    int largestAckedSince(SimTime since);
    void pmtuInvalid(int largestAckedSinceLoss);
    simtime_t getPmtuInvalidTimeThreshold();
    void readParameters(cModule *module);

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_PMTUVALIDATOR_H_ */
