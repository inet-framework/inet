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

#include "DplpmtudState.h"

namespace inet {
namespace quic {

DplpmtudState::DplpmtudState(Dplpmtud *context) {
    this->context = context;
    probeCount = 0;
}

DplpmtudState::~DplpmtudState() { }

void DplpmtudState::sendProbe(int probeSize, bool triggerSendRoutine) {
    probeCount++;
    if (triggerSendRoutine) {
        context->sendProbe(probeSize);
    } else {
        context->prepareProbe(probeSize);
    }
}

DplpmtudState *DplpmtudState::newState(DplpmtudState *state) {
    delete this;
    return state;
}

} /* namespace quic */
} /* namespace inet */
