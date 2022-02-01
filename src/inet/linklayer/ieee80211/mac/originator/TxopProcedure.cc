//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"

#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211DsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HrDsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

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

s TxopProcedure::getTxopLimit(const IIeee80211Mode *mode, AccessCategory ac)
{
    switch (ac) {
        case AC_BK: return s(0);
        case AC_BE: return s(0);
        case AC_VI:
            if (dynamic_cast<const Ieee80211DsssMode *>(mode) || dynamic_cast<const Ieee80211HrDsssMode *>(mode)) return ms(6.016);
            else if (dynamic_cast<const Ieee80211HtMode *>(mode) || dynamic_cast<const Ieee80211OfdmMode *>(mode)) return ms(3.008);
            else return s(0);
        case AC_VO:
            if (dynamic_cast<const Ieee80211DsssMode *>(mode) || dynamic_cast<const Ieee80211HrDsssMode *>(mode)) return ms(3.264);
            else if (dynamic_cast<const Ieee80211HtMode *>(mode) || dynamic_cast<const Ieee80211OfdmMode *>(mode)) return ms(1.504);
            else return s(0);
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
        auto referenceMode = modeSet->getSlowestMandatoryMode();
        limit = getTxopLimit(referenceMode, ac).get();
    }
    // The STA selects between single and multiple protection when it transmits the first frame of a TXOP.
    // All subsequent frames transmitted by the STA in the same TXOP use the same class of duration settings.
    protectionMechanism = selectProtectionMechanism(ac);
    start = simTime();
    emit(txopStartedSignal, this);
    EV_INFO << "Txop started: limit = " << limit << ".\n";
}

void TxopProcedure::endTxop()
{
    Enter_Method("endTxop");
    emit(txopEndedSignal, this);
    start = -1;
    protectionMechanism = ProtectionMechanism::UNDEFINED_PROTECTION;
    EV_INFO << "Txop ended.\n";
}

simtime_t TxopProcedure::getRemaining() const
{
    if (start == -1)
        throw cRuntimeError("Txop has not started yet");
    auto now = simTime();
    return now > start + limit ? 0 : now - start;
}

simtime_t TxopProcedure::getDuration() const
{
    if (start == -1)
        throw cRuntimeError("Txop has not started yet");
    return simTime() - start;
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

