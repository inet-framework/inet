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

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATE_H_

#include "Dplpmtud.h"

namespace inet {
namespace quic {

class Dplpmtud;

class DplpmtudState {
public:
    DplpmtudState(Dplpmtud *context);
    virtual ~DplpmtudState();

    virtual DplpmtudState *onProbeAcked(int ackedProbeSize) = 0;
    virtual DplpmtudState *onProbeLost(int lostProbeSize) = 0;
    virtual DplpmtudState *onPtbReceived(int ptbMtu) = 0;
    virtual DplpmtudState *onPmtuInvalid(int largestAckedSinceLoss) = 0;
    virtual void onRaiseTimeout() = 0;

    virtual void onGotProbeSendPermission(int probeSizeLimit, int overhead) { }
    virtual bool canSendAnotherProbePacket(int probeSizeLimit, int overhead) { return false; }

protected:
    Dplpmtud *context;
    int probeCount;

    virtual void sendProbe(int probeSize, bool triggerSendRoutine = true);
    DplpmtudState *newState(DplpmtudState *state);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDSTATE_H_ */
