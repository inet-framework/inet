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

#include "DplpmtudCandidateSequenceUp.h"

#include "Dplpmtud.h"

namespace inet {
namespace quic {

DplpmtudCandidateSequenceUp::DplpmtudCandidateSequenceUp(int minPmtu, int maxPmtu, int stepSize) : DplpmtudCandidateSequence(minPmtu, maxPmtu, stepSize) {
    currentCandidate = minPmtu;
}
DplpmtudCandidateSequenceUp::~DplpmtudCandidateSequenceUp() { }

void DplpmtudCandidateSequenceUp::testFailed(int candidate) {
    maxPmtu = std::min(maxPmtu, candidate);
}

int DplpmtudCandidateSequenceUp::getNextCandidate(int probeSizeLimit) {
    int next = currentCandidate + stepSize;
    EV_DEBUG << "DplpmtudSearchAlgorithmUp::getNextCandidate currentCandidate=" << currentCandidate << ", next=" << next << ", probeSizeLimit=" << probeSizeLimit << endl;
    if (next >= smallestExpiredProbeSize || next > maxPmtu || next > probeSizeLimit) {
        return 0;
    }
    currentCandidate = next;
    return next;
}

bool DplpmtudCandidateSequenceUp::repeatOnTimeout(int size) {
    return true;
}

} /* namespace quic */
} /* namespace inet */
