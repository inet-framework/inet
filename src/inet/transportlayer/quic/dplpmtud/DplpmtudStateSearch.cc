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

#include "DplpmtudStateSearch.h"
#include "DplpmtudStateComplete.h"
#include "DplpmtudStateBase.h"
#include "ProbeTimerMessage_m.h"

namespace inet {
namespace quic {

DplpmtudStateSearch::DplpmtudStateSearch(Dplpmtud *context) : DplpmtudState(context) {
    testedCandidates.clear();
    networkLoad = 0;
    start();
}
DplpmtudStateSearch::~DplpmtudStateSearch() {
    delete sequence;
    if (probeTimer != nullptr) {
        delete probeTimer;
    }
    context->stats->getMod()->emit(context->numberOfTestsStat, testedCandidates.size());
    context->stats->getMod()->emit(context->timeStat, endSearchTime - startSearchTime);
    context->stats->getMod()->emit(context->networkLoadStat, networkLoad);
}

void DplpmtudStateSearch::start() {
    sequence = context->createSearchAlgorithm();
    probeTimer = nullptr;
    startSearchTime = simTime();
    endSearchTime = startSearchTime;
    smallestExpiredProbeSize = context->MAX_IP_PACKET_SIZE;
    int probeSize = sequence->getNextCandidate();
    EV_DEBUG << "DPLPMTUD in SEARCH: start, send probe for " << probeSize << endl;
    sendProbe(probeSize);
}

DplpmtudState *DplpmtudStateSearch::onProbeAcked(int ackedProbeSize) {
    EV_DEBUG << "DPLPMTUD in SEARCH: probe for " << ackedProbeSize << "B acked" << endl;
    if (ackedProbeSize < context->getPmtu()) {
        return this;
    }
    if (ackedProbeSize > context->getPmtu()) {
        endSearchTime = simTime();
    }
    context->setPmtu(ackedProbeSize);
    DplpmtudProbe *ackedProbe = outstandingProbes.getBySize(ackedProbeSize);
    if (ackedProbe != nullptr) {
        context->stats->getMod()->emit(context->pmtuNumOfProbesBeforeAckStat, ackedProbe->getProbeCount());
    }
    if (ackedProbeSize == context->getMaxPmtu()) {
        EV_DEBUG << "DPLPMTUD in SEARCH: max PMTU acked, transition to COMPLETE" << endl;
        return newState(new DplpmtudStateComplete(context));
    }
    outstandingProbes.removeAllEqualOrSmaller(ackedProbeSize);
    // update smallestExpiredProbeSize if necessary
    if (ackedProbeSize >= smallestExpiredProbeSize) {
        updateSmallestExpiredProbeSize();
    }
    sequence->testSucceeded(ackedProbeSize);
    // force to continue with a new test if no larger one is outstanding
    if (!outstandingProbes.containsProbesNotLost()) {
        int probeSize = sequence->getNextCandidate();
        if (probeSize > 0) {
            // send probe for next candidate
            sendProbe(probeSize);
        } else {
            DplpmtudProbe *smallest = outstandingProbes.getSmallest();
            if (smallest != nullptr) {
                // repeat smallest outstanding probe since it is outstanding for at least the expected response time
                sendProbe(smallest->getSize());
            } else {
                // no more candidates to test
                EV_DEBUG << "DPLPMTUD in SEARCH: no more candidates to test, transition to COMPLETE" << endl;
                return newState(new DplpmtudStateComplete(context));
            }
        }
    }
    return this;
}

DplpmtudState *DplpmtudStateSearch::onProbeLost(int lostProbeSize) {
    EV_DEBUG << "DPLPMTUD in SEARCH: probe for " << lostProbeSize << "B lost." << endl;
    DplpmtudProbe *lostProbe = outstandingProbes.getBySize(lostProbeSize);
    if (lostProbe == nullptr) {
        EV_DEBUG << "DPLPMTUD in SEARCH: couldn't find outstanding probe packet for it." << endl;
        return this;
    }
    lostProbe->lost();
    if (lostProbeSize < smallestExpiredProbeSize) {
        smallestExpiredProbeSize = lostProbeSize;
        sequence->setSmallestExpiredProbeSize(lostProbeSize);
    }

    DplpmtudProbe *smallestProbe = outstandingProbes.getSmallest();
    //if (lostProbe == outstandingProbes.getSmallest()) {
    if (smallestProbe->isLost()) {
        int smallestProbeSize = smallestProbe->getSize();
        sequence->smallestProbeTimedOut(smallestProbeSize);
        bool repeat = true;
        if (smallestProbe->getProbeCount() == context->getMaxProbes()) {
            // too many probe packets sent without a response for this candidate, let the test fail
            sequence->testFailed(smallestProbeSize);
            if (smallestProbeSize <= context->getMinPmtu()) {
                throw cRuntimeError("DPLPMTUD: Test for minPmtu failed in search state");
            }
            outstandingProbes.removeAllEqualOrLarger(smallestProbeSize);
            repeat = false;
        }
        // try to send a new probe packet since this was the smallest outstanding
        int probeSize = sequence->getNextCandidate();
        if (probeSize == 0) {
            if (repeat) {
                EV_DEBUG << "DPLPMTUD in SEARCH: no smaller candidates left, repeat" << endl;
                sendProbe(smallestProbeSize);
            } else {
                EV_DEBUG << "DPLPMTUD in SEARCH: no smaller candidates left. Transition to COMPLETE." << endl;
                return newState(new DplpmtudStateComplete(context));
            }
        } else {
            sendProbe(probeSize);
        }
    }

    return this;
}

DplpmtudState *DplpmtudStateSearch::onPtbReceived(int ptbMtu) {
    EV_DEBUG << "DPLPMTUD in SEARCH: PTB received" << endl;

    if (ptbMtu < context->getPmtu()) {
        EV_DEBUG << "DPLPMTUD in SEARCH: reported MTU is smaller than a previously successful probed size. Go back to BASE." << endl;
        context->setMaxPmtu(ptbMtu);
        return newState(new DplpmtudStateBase(context));
    }

    if (ptbMtu == context->getPmtu()) {
        EV_DEBUG << "DPLPMTUD in SEARCH: reported MTU confirmed current PMTU. Transition to COMPLETE." << endl;
        return newState(new DplpmtudStateComplete(context));
    }

    // PMTU < PTB_MTU < MAX_PMTU
    EV_DEBUG << "DPLPMTUD in SEARCH: use reported MTU for a new probe." << endl;
    // use reported MTU for a new probe
    if (probeTimer != nullptr && probeTimer->isScheduled()) {
        probeTimer->cancel();
    }
    context->setMaxPmtu(ptbMtu);
    outstandingProbes.removeAllLarger(ptbMtu);
    sequence->ptbReceived(ptbMtu);
    if (outstandingProbes.getBySize(ptbMtu) == nullptr) {
        sendProbe(ptbMtu);
    }
    return this;
}

void DplpmtudStateSearch::sendProbe(int probeSize, bool triggerSendRoutine) {
    DplpmtudProbe *oldProbe = outstandingProbes.getBySize(probeSize);
    DplpmtudProbe *newProbe = nullptr;
    if (oldProbe == nullptr) {
        newProbe = new DplpmtudProbe(probeSize, simTime(), 1);
    } else {
        newProbe = new DplpmtudProbe(probeSize, simTime(), oldProbe->getProbeCount()+1);
        outstandingProbes.erase(probeSize);
        delete oldProbe;
    }
    outstandingProbes.add(newProbe);
    testedCandidates.insert(probeSize);
    networkLoad += probeSize;
    EV_DEBUG << "DPLPMTUD in SEARCH: send probe for " << probeSize << "B, probeCount=" << newProbe->getProbeCount() << endl;
    if (triggerSendRoutine) {
        context->sendProbe(probeSize);
    } else {
        context->prepareProbe(probeSize);
    }
}

DplpmtudState *DplpmtudStateSearch::onPmtuInvalid(int largestAckedSinceLoss) {
    return newState(new DplpmtudStateBase(context));
}

void DplpmtudStateSearch::onRaiseTimeout() {
    throw cRuntimeError("DPLPMTUD: Raise Timeout in search state should not happen");
    //return this;
}

void DplpmtudStateSearch::updateSmallestExpiredProbeSize() {
    DplpmtudProbe *probe = outstandingProbes.getSmallestLost();
    if (probe == nullptr) {
        smallestExpiredProbeSize = Dplpmtud::MAX_IP_PACKET_SIZE;
    } else {
        smallestExpiredProbeSize = probe->getSize();
    }
    sequence->setSmallestExpiredProbeSize(smallestExpiredProbeSize);
}

void DplpmtudStateSearch::onGotProbeSendPermission(int probeSizeLimit, int overhead) {
    EV_DEBUG << "DPLPMTUD in SEARCH: got probe send permission for " << probeSizeLimit << "B" << endl;
    int probeSize = 0;
    while ( (probeSize = sequence->getNextCandidate(probeSizeLimit+overhead)) > 0 ) {
        sendProbe(probeSize);
        probeSizeLimit -= (probeSize-overhead);
    }
}

bool DplpmtudStateSearch::canSendAnotherProbePacket(int probeSizeLimit, int overhead) {
    EV_DEBUG << "DPLPMTUD in SEARCH: got permission to send another probe packet for " << probeSizeLimit << "B" << endl;
    int probeSize = sequence->getNextCandidate(probeSizeLimit+overhead);
    if (probeSize > 0) {
        sendProbe(probeSize, false);
        return true;
    }
    return false;
}

} /* namespace quic */
} /* namespace inet */
