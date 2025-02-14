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

#include "DplpmtudCandidateSequenceOptBinary.h"

namespace inet {
namespace quic {

DplpmtudCandidateSequenceOptBinary::DplpmtudCandidateSequenceOptBinary(int minPmtu, int maxPmtu, int stepSize) : DplpmtudCandidateSequenceBinary(minPmtu, maxPmtu, stepSize) { }
DplpmtudCandidateSequenceOptBinary::~DplpmtudCandidateSequenceOptBinary() { }

int DplpmtudCandidateSequenceOptBinary::getNextCandidate(int probeSizeLimit) {
    if (firstCandidate) {
        firstCandidate = false;
        return maxPmtu;
    }
    int next = DplpmtudCandidateSequenceBinary::getNextCandidate(probeSizeLimit);
    if (next == maxPmtu) {
        return 0;
    }
    return next;
}


} /* namespace quic */
} /* namespace inet */
