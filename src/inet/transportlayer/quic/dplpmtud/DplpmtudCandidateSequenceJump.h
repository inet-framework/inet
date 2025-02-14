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

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEJUMP_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEJUMP_H_

#include <vector>
#include <set>
#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

class DplpmtudCandidateSequenceJump: public DplpmtudCandidateSequence {
public:
    DplpmtudCandidateSequenceJump(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequenceJump();

    virtual int getNextCandidate(int probeSizeLimit) override;
    virtual void testSucceeded(int succeededCandidate) override;
    virtual void testFailed(int failedCandidate) override;
    virtual bool repeatOnTimeout(int size) override;
    virtual void smallestProbeTimedOut(int size) override;
    virtual void ptbReceived(int ptbMtu) override;

private:
    std::vector<int> candidates;
    std::vector<int>::iterator currentIt;
    //std::set<int> usedCandidates;
    bool downward;

    void addCandidates(std::vector<int>::iterator &after);
    std::vector<int>::iterator getIterator(int value);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEJUMP_H_ */
