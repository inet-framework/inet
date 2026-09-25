//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/common/TxopAdmissionDetails.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;
using PhyType = Ieee80211ModeSet::PhyType;

simsignal_t TxopProcedure::txopStartedSignal = cComponent::registerSignal("txopStarted");
simsignal_t TxopProcedure::txopEndedSignal = cComponent::registerSignal("txopEnded");

simsignal_t TxopProcedure::txopExchangeAdmittedSignal = cComponent::registerSignal("txopExchangeAdmitted");
simsignal_t TxopProcedure::txopExchangeRejectedSignal = cComponent::registerSignal("txopExchangeRejected");

Define_Module(TxopProcedure);
Register_Class(TxopAdmissionDetails);

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
    if (acquired)
        throw cRuntimeError("TXOP is already acquired");
    if (limit == -1)
        limit = getTxopLimit(modeSet->getPhyType(), ac).get<s>();
    acquired = true;
    accessCategory = ac;
    start = -1;
    lastDuration = 0;
    dataOrManagementTransmissions = 0;
    hasIdentity = false;
    protectionMechanism = selectProtectionMechanism(ac);
}

void TxopProcedure::transmissionStarted()
{
    Enter_Method("transmissionStarted");
    if (!acquired)
        throw cRuntimeError("Holder transmission without an acquired TXOP");
    if (start == -1) {
        start = simTime();
        emit(txopStartedSignal, this);
    }
}

void TxopProcedure::recordTransmission(const TxopExchangePlan& plan, bool dataOrManagement)
{
    if (dataOrManagement) {
        ++dataOrManagementTransmissions;
        if (!hasIdentity) {
            identity = plan.identity;
            hasIdentity = true;
        }
    }
}

void TxopProcedure::endTxop()
{
    Enter_Method("endTxop");
    if (!acquired)
        return;
    bool started = hasStarted();
    lastDuration = started ? simTime() - start : SIMTIME_ZERO;
    acquired = false;
    start = -1;
    protectionMechanism = UNDEFINED_PROTECTION;
    if (started)
        emit(txopEndedSignal, this);
}

simtime_t TxopProcedure::getRemaining() const
{
    if (!acquired)
        throw cRuntimeError("TXOP is not acquired");
    return hasStarted() ? std::max(SIMTIME_ZERO, start + limit - simTime()) : limit;
}

simtime_t TxopProcedure::getDuration() const
{
    return hasStarted() ? simTime() - start : lastDuration;
}

TxopAdmissionDecision TxopProcedure::evaluate(const TxopExchangePlan& plan, simtime_t exchangeStart,
        simtime_t txnavEnd, bool continuation, const TxopExchangePlan *projectedCurrent) const
{
    TxopAdmissionDecision result;
    result.start = exchangeStart;
    result.end = exchangeStart + plan.getDuration();
    auto accountingStart = hasStarted() ? start : simTime();
    result.remaining = std::max(SIMTIME_ZERO, accountingStart + limit - exchangeStart);
    if (!acquired)
        return result;
    if (plan.missingPriorControlMode) {
        result.reason = TxopAdmissionReason::NO_PRIOR_CONTROL_MODE;
        return result;
    }
    // IEEE Std 802.11-2024, 10.23.2.8: the TXNAV comparison is strict.
    // The prior Duration includes the leading SIFS, which is not part of F.
    if (continuation && plan.getDuration() - plan.getLeadingInterval() >= std::max(SIMTIME_ZERO, txnavEnd - exchangeStart)) {
        result.reason = TxopAdmissionReason::RESERVATION_EXPIRED;
        return result;
    }
    const TxopFrameIdentity *family = hasIdentity ? &identity :
            projectedCurrent && projectedCurrent->dataOrManagement ? &projectedCurrent->identity : nullptr;
    if (limit == SIMTIME_ZERO) {
        bool same = family == nullptr || (plan.dataOrManagement ? plan.identity == *family :
                plan.identity.receiver == family->receiver && plan.identity.tid == family->tid);
        result.accepted = same;
        result.reason = same ? TxopAdmissionReason::ZERO_LIMIT_FRAME : TxopAdmissionReason::DIFFERENT_ZERO_LIMIT_FRAME;
        return result;
    }
    if (result.end <= accountingStart + limit) {
        result.accepted = true;
        result.reason = TxopAdmissionReason::WITHIN_LIMIT;
        return result;
    }
    // IEEE Std 802.11-2024, 10.23.2.9: every overrun permits at most one
    // actual Data or Management transmission, including retries and fragments.
    int count = dataOrManagementTransmissions + (projectedCurrent && projectedCurrent->dataOrManagement ? 1 : 0) + (plan.dataOrManagement ? 1 : 0);
    result.reason = TxopAdmissionReason::LIMIT_EXCEEDED;
    if (count > 1)
        return result;
    if (!plan.dataOrManagement)
        result.reason = TxopAdmissionReason::CONTROL;
    else if (plan.unchangedRetry)
        result.reason = TxopAdmissionReason::UNCHANGED_RETRY;
    else if (plan.initialBlockAckMsdu)
        result.reason = TxopAdmissionReason::BLOCK_ACK_MSDU;
    else if (plan.initialFragmentAfterRetry)
        result.reason = TxopAdmissionReason::RETRIED_FRAGMENT_FAMILY;
    else if (plan.sixteenFragments)
        result.reason = TxopAdmissionReason::SIXTEEN_FRAGMENTS;
    else if (plan.group)
        result.reason = TxopAdmissionReason::GROUP;
    result.accepted = result.reason != TxopAdmissionReason::LIMIT_EXCEEDED;
    return result;
}

void TxopProcedure::commitAdmission(TxopExchangePlan& plan, const TxopAdmissionDecision& decision)
{
    Enter_Method("commitAdmission");
    plan.admission = decision;
    TxopAdmissionDetails details(accessCategory, plan);
    emit(decision.accepted ? txopExchangeAdmittedSignal : txopExchangeRejectedSignal, this, &details);
}

Register_ResultFilter("txopDuration", TxopDurationFilter);

void TxopDurationFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details)
{
    fire(this, t, check_and_cast<TxopProcedure *>(object)->getDuration(), details);
}

} // namespace ieee80211
} // namespace inet
