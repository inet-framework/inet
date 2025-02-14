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

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUD_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUD_H_

#include "DplpmtudState.h"
#include "../Path.h"
#include "../Statistics.h"
#include "DplpmtudCandidateSequence.h"

using namespace omnetpp;

namespace inet {
namespace quic {

class DplpmtudState;
class Path;

class Dplpmtud {
public:
    static const int MAX_IP_PACKET_SIZE = (1<<16);

    Dplpmtud(Path *path, int mtu, int overhead, Statistics *stats);
    virtual ~Dplpmtud();

    virtual void start(bool probeBase=true);
    virtual int getPlpmtu();
    virtual int getMinPlpmtu();
    virtual void prepareProbe(int probeSize);
    virtual void sendProbe(int probeSize);
    virtual void onProbePacketAcked(int quicPacketSize);
    virtual void onProbePacketLost(int quicPacketSize);
    virtual void onRaiseTimeout();
    virtual int getNextLargerPmtu();
    virtual simtime_t getRaiseTime();
    virtual DplpmtudCandidateSequence *createSearchAlgorithm();
    virtual void probePacketBuilt();
    virtual void onPtbReceived(int quicPacketSize, int ptbMtu);
    virtual void onPmtuInvalid(int largestAckedSinceLoss);
    virtual void giveProbeSendPermission(int probeSizeLimit);
    virtual bool canSendProbe(int probeSizeLimit);
    virtual void setPmtu(int pmtu);
    virtual void initPmtuValidator();
    virtual void destroyPmtuValidator();

    Path *getPath() {
        return path;
    }
    int getMinPmtu() {
        return minPmtu;
    }
    void setMinPmtu(int minPmtu) {
        this->minPmtu = minPmtu;
    }
    void resetMinPmtu() {
        minPmtu = initialMinPmtu;
    }
    int getMaxPmtu() {
        return maxPmtu;
    }
    void setMaxPmtu(int maxPmtu) {
        this->maxPmtu = maxPmtu;
    }
    void resetMaxPmtu() {
        maxPmtu = initialMaxPmtu;
    }
    int getPmtu() {
        return pmtu;
    }
    bool needToSendProbe() {
        return doSendProbe;
    }
    int getProbeSize() {
        return probeSize;
    }
    int getMaxProbes() {
        return maxProbes;
    }

    Statistics *stats;
    simsignal_t pmtuStat;
    simsignal_t numberOfTestsStat;
    simsignal_t timeStat;
    simsignal_t networkLoadStat;
    simsignal_t pmtuInvalidSignalsStat;
    simsignal_t pmtuNumOfProbesBeforeAckStat;

private:
    const int step = 4;
    const simtime_t RAISE_TIMEOUT = SimTime(600, SimTimeUnit::SIMTIME_S);
    const simtime_t kGranularity = SimTime(1, SimTimeUnit::SIMTIME_MS);

    int initialMinPmtu;
    int initialMaxPmtu;
    int minPmtu;
    int maxPmtu;
    int pmtu;
    int overhead;
    int probeSize;
    bool doSendProbe;
    bool usePtb;
    bool usePmtuValidator;
    int maxProbes;
    std::string candidateSequence;
    DplpmtudState *state;
    Path *path;

    void readParameters(cModule *module);
    void determinePmtuBounds(int mtu);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUD_H_ */
