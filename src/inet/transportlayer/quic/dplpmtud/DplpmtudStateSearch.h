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

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATESEARCH_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATESEARCH_H_

#include "DplpmtudState.h"
#include "DplpmtudProbes.h"
#include "../Timer.h"
#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

class DplpmtudStateSearch: public DplpmtudState {
public:
    DplpmtudStateSearch(Dplpmtud *context);
    virtual ~DplpmtudStateSearch();

    virtual DplpmtudState *onProbeAcked(int ackedProbeSize) override;
    virtual DplpmtudState *onProbeLost(int lostProbeSize) override;
    virtual DplpmtudState *onPtbReceived(int ptbMtu) override;
    virtual DplpmtudState *onPmtuInvalid(int largestAckedSinceLoss) override;
    virtual void onRaiseTimeout() override;

    virtual void onGotProbeSendPermission(int probeSizeLimit, int overhead) override;
    virtual bool canSendAnotherProbePacket(int probeSizeLimit, int overhead) override;

private:
    DplpmtudCandidateSequence *sequence;
    Timer *probeTimer;
    DplpmtudProbes outstandingProbes;
    int smallestExpiredProbeSize;
    std::set<int> testedCandidates;
    SimTime startSearchTime;
    SimTime endSearchTime;
    int networkLoad;

    void start();
    void sendProbe(int probeSize, bool triggerSendRoutine = true) override;
    void updateSmallestExpiredProbeSize();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATESEARCH_H_ */
