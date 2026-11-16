// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsPathMtuState.h"

#include <algorithm>

namespace inet::dtls {

DtlsPathMtuState::DtlsPathMtuState(size_t base, size_t ceiling) : base(base), ceiling(ceiling), searchCeiling(ceiling), confirmed(base)
{
    if (base == 0 || ceiling < base)
        throw cRuntimeError("Invalid DTLS path MTU base or ceiling");
}

size_t DtlsPathMtuState::getProbeSize() const
{
    if (phase == Phase::UNSUPPORTED)
        return 0;
    if (phase == Phase::BASE || phase == Phase::ERROR)
        return base;
    if (phase == Phase::SEARCHING)
        return confirmed + std::min<size_t>(128, searchCeiling - confirmed);
    return confirmed;
}

void DtlsPathMtuState::reset()
{
    phase = Phase::BASE;
    confirmed = base;
    searchCeiling = ceiling;
}

void DtlsPathMtuState::unsupported()
{
    reset();
    phase = Phase::UNSUPPORTED;
}

void DtlsPathMtuState::raise()
{
    if (phase != Phase::SEARCH_COMPLETE)
        return;
    searchCeiling = ceiling;
    if (confirmed < searchCeiling)
        phase = Phase::SEARCHING;
}

void DtlsPathMtuState::succeeded()
{
    if (phase == Phase::UNSUPPORTED || phase == Phase::SEARCH_COMPLETE)
        return;
    if (phase == Phase::ERROR && searchCeiling < base)
        searchCeiling = ceiling;
    confirmed = getProbeSize();
    phase = confirmed < searchCeiling ? Phase::SEARCHING : Phase::SEARCH_COMPLETE;
}

void DtlsPathMtuState::failed()
{
    switch (phase) {
        case Phase::BASE:
        case Phase::ERROR:
            confirmed = base;
            phase = Phase::ERROR;
            break;
        case Phase::SEARCHING:
            phase = Phase::SEARCH_COMPLETE;
            break;
        case Phase::SEARCH_COMPLETE:
            confirmed = base;
            phase = Phase::BASE;
            break;
        case Phase::UNSUPPORTED:
            break;
    }
}

bool DtlsPathMtuState::packetTooBig(size_t udpBytes)
{
    if (phase == Phase::UNSUPPORTED || udpBytes >= getProbeSize())
        return false;
    searchCeiling = std::min(searchCeiling, udpBytes);
    if (udpBytes < base) {
        confirmed = base;
        phase = Phase::ERROR;
    }
    else if (udpBytes < confirmed) {
        confirmed = base;
        phase = Phase::BASE;
    }
    else if (phase == Phase::SEARCHING && confirmed == searchCeiling)
        phase = Phase::SEARCH_COMPLETE;
    return true;
}

} // namespace inet::dtls
