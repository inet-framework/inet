//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;
using PhyType = Ieee80211ModeSet::PhyType;

simsignal_t TxopProcedure::txopStartedSignal = cComponent::registerSignal("txopStarted");
simsignal_t TxopProcedure::txopEndedSignal = cComponent::registerSignal("txopEnded");

Define_Module(TxopProcedure);

void TxopProcedure::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        limit = par("txopLimit");
        WATCH(start);
        WATCH(protectionMechanism);
    }
}

// IEEE Std 802.11-2024, Table 9-194 selects the default TXOP limit by the
// PHY type clause. The existing INET values are retained for compatibility;
// full Table 9-194/9-195 modernization is a separate change.
s TxopProcedure::getTxopLimit(PhyType phyType, AccessCategory ac)
{
    switch (ac) {
        case AC_BK: return s(0);
        case AC_BE: return s(0);
        case AC_VI:
            switch (phyType) {
                case PhyType::HR_DSSS: return ms(6.016);
                case PhyType::OFDM:
                case PhyType::ERP:
                case PhyType::HT:
                case PhyType::VHT: return ms(3.008);
                default: throw cRuntimeError("Unknown PHY type = %d", static_cast<int>(phyType));
            }
        case AC_VO:
            switch (phyType) {
                case PhyType::HR_DSSS: return ms(3.264);
                case PhyType::OFDM:
                case PhyType::ERP:
                case PhyType::HT:
                case PhyType::VHT: return ms(1.504);
                default: throw cRuntimeError("Unknown PHY type = %d", static_cast<int>(phyType));
            }
        default: throw cRuntimeError("Unknown access category = %d", ac);
    }
}

TxopProcedure::ProtectionMechanism TxopProcedure::selectProtectionMechanism(AccessCategory ac) const
{
    return ProtectionMechanism::SINGLE_PROTECTION;
}

simtime_t TxopProcedure::getStart() const
{
    return start;
}

simtime_t TxopProcedure::getLimit() const
{
    return limit;
}

void TxopProcedure::startTxop(AccessCategory ac)
{
    Enter_Method("startTxop");
    if (start != -1)
        throw cRuntimeError("Txop is already running");
    if (limit == -1) {
        limit = getTxopLimit(modeSet->getPhyType(), ac).get<s>();
    }
    // The STA selects between single and multiple protection when it transmits the first frame of a TXOP.
    // All subsequent frames transmitted by the STA in the same TXOP use the same class of duration settings.
    protectionMechanism = selectProtectionMechanism(ac);
    start = simTime();
    resetTransmissionHistory();
    emit(txopStartedSignal, this);
    EV_INFO << "Txop started: limit = " << limit << ".\n";
}

void TxopProcedure::endTxop()
{
    Enter_Method("endTxop");
    emit(txopEndedSignal, this);
    start = -1;
    resetTransmissionHistory();
    protectionMechanism = ProtectionMechanism::UNDEFINED_PROTECTION;
    EV_INFO << "Txop ended.\n";
}

simtime_t TxopProcedure::getRemaining() const
{
    if (start == -1)
        throw cRuntimeError("Txop has not started yet");
    auto now = simTime();
    return now > start + limit ? 0 : start + limit - now;
}

simtime_t TxopProcedure::getDuration() const
{
    if (start == -1)
        throw cRuntimeError("Txop has not started yet");
    return simTime() - start;
}

// IEEE Std 802.11-2024, 10.23.2.9: use retained frame facts and actual transmission history for overrun eligibility. The count includes a projected predecessor when the check considers a next exchange. For example, a group address can permit an overrun without an earlier transmission.
TxopProcedure::Admission TxopProcedure::admitExchange(simtime_t cost, const StagedFrameView& candidate,
        simtime_t at, const StagedFrameView *precedingCandidate) const
{
    if (start < SIMTIME_ZERO || cost < SIMTIME_ZERO || candidate.identity < 0)
        throw cRuntimeError("Invalid TXOP admission inputs");
    auto count = dataOrMgmtTransmissions + (precedingCandidate ? 1 : 0);
    auto unit = firstUnitIdentity != -1 ? firstUnitIdentity : (precedingCandidate ? precedingCandidate->unitIdentity : -1);
    if (limit == SIMTIME_ZERO)
        return count == 0 || unit == candidate.unitIdentity ? Admission::ZERO_LIMIT_UNIT : Admission::REFUSED;
    if (cost <= std::max(SIMTIME_ZERO, start + limit - at))
        return Admission::FIT;
    if (count != 0)
        return Admission::REFUSED;
    bool retransmission = candidate.transmissions > 0 && candidate.length == candidate.originalLength;
    bool laterFragment = candidate.transmissions == 0 && candidate.earlierFragmentRetransmitted;
    bool sixteenFragments = candidate.fragmentCount == 16;
    return retransmission || laterFragment || sixteenFragments || candidate.receiver.isMulticast() ?
        Admission::OVERRUN_EXCEPTION : Admission::REFUSED;
}

// IEEE Std 802.11-2024, 10.23.2.8: this strict comparison excludes SIFS before the next originator transmission. Hypothetical: an 84 us cost without that SIFS fails against an 84 us TXNAV interval. Equality can pass the separate TXOP budget check, which includes that SIFS.
bool TxopProcedure::fitsTxnav(simtime_t costWithoutLeadingIfs, simtime_t at) const
{
    return costWithoutLeadingIfs < std::max(SIMTIME_ZERO, txnavEnd - at);
}

void TxopProcedure::recordDataOrMgmtTransmission(const StagedFrameView& candidate)
{
    if (dataOrMgmtTransmissions++ == 0)
        firstUnitIdentity = candidate.unitIdentity;
}

void TxopProcedure::recordTransmittedReservation(simtime_t ppduEnd, simtime_t serializedDuration)
{
    txnavEnd = std::max(txnavEnd, ppduEnd + serializedDuration);
}

void TxopProcedure::resetTransmissionHistory()
{
    txnavEnd = SIMTIME_ZERO;
    dataOrMgmtTransmissions = 0;
    firstUnitIdentity = -1;
}

// FIXME implement!
bool TxopProcedure::isFinalFragment(const Ptr<const Ieee80211MacHeader>& header) const
{
    return false;
}

// FIXME implement!
bool TxopProcedure::isTxopInitiator(const Ptr<const Ieee80211MacHeader>& header) const
{
    return false;
}

// FIXME implement!
bool TxopProcedure::isTxopTerminator(const Ptr<const Ieee80211MacHeader>& header) const
{
    return false;
}

Register_ResultFilter("txopDuration", TxopDurationFilter);

void TxopDurationFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    fire(this, t, check_and_cast<TxopProcedure *>(object)->getDuration(), details);
}

} // namespace ieee80211
} // namespace inet
