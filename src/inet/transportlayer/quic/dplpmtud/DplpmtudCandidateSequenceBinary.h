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

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEBINARY_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEBINARY_H_

#include <vector>
#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

class DplpmtudCandidateSequenceBinary: public DplpmtudCandidateSequence {
public:
    DplpmtudCandidateSequenceBinary(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequenceBinary();

    virtual int getNextCandidate(int probeSizeLimit) override;
    virtual void testSucceeded(int succeededCandidate) override;
    virtual void testFailed(int failedCandidate) override;
    virtual bool repeatOnTimeout(int size) override;
    virtual void ptbReceived(int ptbMtu) override;

private:
    bool gotAck;
    std::vector<int> candidatesTree;
    std::vector<int> retainedCandidates;

    int calculateNextValue();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEBINARY_H_ */
