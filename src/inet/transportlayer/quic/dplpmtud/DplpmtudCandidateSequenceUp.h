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

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEUP_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEUP_H_

#include "DplpmtudCandidateSequence.h"

namespace inet {
namespace quic {

class DplpmtudCandidateSequenceUp: public DplpmtudCandidateSequence {
public:
    DplpmtudCandidateSequenceUp(int minPmtu, int maxPmtu, int stepSize);
    virtual ~DplpmtudCandidateSequenceUp();

    virtual int getNextCandidate(int probeSizeLimit) override;
    virtual void testFailed(int failedCandidate) override;
    virtual bool repeatOnTimeout(int size) override;

private:
    int currentCandidate;

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDCANDIDATESEQUENCEUP_H_ */
