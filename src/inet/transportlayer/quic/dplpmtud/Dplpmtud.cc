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

#include "Dplpmtud.h"

#include "DplpmtudCandidateSequenceBinary.h"
#include "DplpmtudCandidateSequenceDown.h"
#include "DplpmtudCandidateSequenceJump.h"
#include "DplpmtudCandidateSequenceOptBinary.h"
#include "DplpmtudCandidateSequenceOptUp.h"
#include "DplpmtudCandidateSequenceUp.h"
#include "DplpmtudStateBase.h"
#include "DplpmtudStateSearch.h"

namespace inet {
namespace quic {

Dplpmtud::Dplpmtud(Path *path, int mtu, int overhead, Statistics *stats) {
    this->path = path;
    this->overhead = overhead;
    this->stats = stats;
    doSendProbe = false;
    minPmtu = 0;
    maxPmtu = (1 << 16) - 1;
    readParameters(path->getConnection()->getModule());
    determinePmtuBounds(mtu);
    pmtu = minPmtu;
    state = nullptr;

    pmtuStat = stats->createStatisticEntry("dplpmtudSearchPmtu");
    numberOfTestsStat = stats->createStatisticEntry("dplpmtudSearchNumberOfTests");
    timeStat = stats->createStatisticEntry("dplpmtudSearchTime");
    networkLoadStat = stats->createStatisticEntry("dplpmtudSearchNetworkLoad");
    pmtuInvalidSignalsStat = stats->createStatisticEntry("dplpmtudInvalidSignals");
    pmtuNumOfProbesBeforeAckStat = stats->createStatisticEntry("dplpmtudNumOfProbesBeforeAck");
}

Dplpmtud::~Dplpmtud() {
    if (state != nullptr) {
        delete state;
    }
}

void Dplpmtud::start(bool probeBase) {
    if (probeBase) {
        state = new DplpmtudStateBase(this);
    } else {
        EV_DEBUG << "DPLPMTUD: skip probe for base, start in SEARCH" << endl;
        state = new DplpmtudStateSearch(this);
    }
}

void Dplpmtud::readParameters(cModule *module)
{
    this->usePtb = module->par("dplpmtudUsePtb");
    this->minPmtu = module->par("dplpmtudMinPmtu");
    this->candidateSequence = module->par("dplpmtudCandidateSequence").stdstringValue();
    this->usePmtuValidator = module->par("dplpmtudUsePmtuValidator");
    this->maxProbes = module->par("dplpmtudMaxProbes");
}

void Dplpmtud::determinePmtuBounds(int mtu) {
    maxPmtu = mtu;

    L3Address remoteAddr = path->getRemoteAddr();
    if (remoteAddr.getType() == L3Address::IPv4) {
        if (minPmtu < 68) minPmtu = 1200;
    } else if (remoteAddr.getType() == L3Address::IPv6) {
        if (minPmtu < 1280) minPmtu = 1280;
    } else {
        throw cRuntimeError("Unknown L3Address type");
    }

    if (maxPmtu < minPmtu) {
        throw cRuntimeError("DPLPMTUD: maxPmtu (%d) is smaller than minPmtu (%d).", maxPmtu, minPmtu);
    }

    initialMinPmtu = minPmtu;
    initialMaxPmtu = maxPmtu;
}

int Dplpmtud::getNextLargerPmtu() {
    if (pmtu == maxPmtu) {
        return 0;
    }
    return std::min(pmtu + step, maxPmtu);
}

simtime_t Dplpmtud::getRaiseTime() {
    return simTime() + RAISE_TIMEOUT;
}

DplpmtudCandidateSequence *Dplpmtud::createSearchAlgorithm() {
    if (candidateSequence == "Up") {
        return new DplpmtudCandidateSequenceUp(pmtu, maxPmtu, step);
    } else if (candidateSequence == "Down") {
        return new DplpmtudCandidateSequenceDown(pmtu, maxPmtu, step);
    } else if (candidateSequence == "OptUp") {
        return new DplpmtudCandidateSequenceOptUp(pmtu, maxPmtu, step);
    } else if (candidateSequence == "Binary") {
        return new DplpmtudCandidateSequenceBinary(pmtu, maxPmtu, step);
    } else if (candidateSequence == "OptBinary") {
        return new DplpmtudCandidateSequenceOptBinary(pmtu, maxPmtu, step);
    } else if (candidateSequence == "Jump") {
        return new DplpmtudCandidateSequenceJump(pmtu, maxPmtu, step);
    } else {
        throw cRuntimeError("Unknown DPLPMTUD candidate sequence specified");
    }
}

void Dplpmtud::onProbePacketAcked(int quicPacketSize) {
    state = state->onProbeAcked(quicPacketSize + overhead);
}

void Dplpmtud::onProbePacketLost(int quicPacketSize) {
    state = state->onProbeLost(quicPacketSize + overhead);
}

void Dplpmtud::onPtbReceived(int quicPacketSize, int ptbMtu) {
    if (!usePtb) {
        return;
    }

    int packetSize = quicPacketSize + overhead;

    if (ptbMtu >= packetSize) {
        EV_WARN << "PTB reports an MTU upon a packet that is equal or larger than the size of the packet. Ignore PTB." << endl;
        // Coalesed packet?
        return;
    }

    if (ptbMtu < minPmtu || ptbMtu > maxPmtu) {
        EV_WARN << "PTB reports an MTU that is either smaller than MIN_PMTU or larger than MAX_PMTU. Ignore PTB." << endl;
        return;
    }

    state = state->onPtbReceived(ptbMtu);
}

void Dplpmtud::onRaiseTimeout() {
    state->onRaiseTimeout();
}

void Dplpmtud::prepareProbe(int probeSize) {
    this->probeSize = probeSize - overhead;
}

void Dplpmtud::sendProbe(int probeSize) {
    prepareProbe(probeSize);
    doSendProbe = true;
    path->getConnection()->sendPackets();
}

void Dplpmtud::setPmtu(int pmtu) {
    this->pmtu = pmtu;
    stats->getMod()->emit(pmtuStat, pmtu);
}

int Dplpmtud::getPlpmtu() {
    return pmtu - overhead;
}

int Dplpmtud::getMinPlpmtu() {
    return minPmtu - overhead;
}

void Dplpmtud::probePacketBuilt() {
    doSendProbe = false;
}

void Dplpmtud::onPmtuInvalid(int largestAckedSinceLoss) {
    largestAckedSinceLoss += overhead;
    stats->getMod()->emit(pmtuInvalidSignalsStat, largestAckedSinceLoss);
    state = state->onPmtuInvalid(largestAckedSinceLoss);
}

void Dplpmtud::giveProbeSendPermission(int probeSizeLimit) {
    state->onGotProbeSendPermission(probeSizeLimit, overhead);
}

bool Dplpmtud::canSendProbe(int probeSizeLimit) {
    return state != nullptr && state->canSendAnotherProbePacket(probeSizeLimit, overhead);
}

void Dplpmtud::initPmtuValidator() {
    if (!usePmtuValidator) {
        return;
    }
    path->setPmtuValidator(new PmtuValidator(path, stats));
}

void Dplpmtud::destroyPmtuValidator() {
    if (!usePmtuValidator) {
        return;
    }
    path->setPmtuValidator(nullptr);
}

} /* namespace quic */
} /* namespace inet */
