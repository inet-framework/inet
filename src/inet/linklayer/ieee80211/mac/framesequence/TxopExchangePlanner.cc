// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/linklayer/ieee80211/mac/framesequence/TxopExchangePlanner.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"

namespace inet {
namespace ieee80211 {

std::unique_ptr<TxopExchangePlan> TxopExchangePlanner::prepare(FrameSequenceContext *context,
        const TxopExchangePlan *projectedCurrent, bool forceBlockAck, bool useFastestMode) const
{
    auto qos = context->getQoSContext();
    auto frames = context->getInProgressFrames();
    auto excluded = projectedCurrent ? projectedCurrent->payload : nullptr;
    frames->prepareCandidate(excluded);
    auto candidate = frames->findPreparedCandidate(excluded);
    auto projectedBa = projectedCurrent && projectedCurrent->ackPolicy == AckPolicy::BLOCK_ACK ? excluded : nullptr;
    auto bar = qos->ackPolicy->projectBlockAckReq(frames, projectedBa,
            forceBlockAck || candidate == nullptr || qos->txopProcedure->getLimit() == SIMTIME_ZERO);
    bool needsBar = !std::get<0>(bar).isUnspecified();
    if (candidate == nullptr && !needsBar)
        return nullptr;
    if (forceBlockAck && !needsBar)
        return nullptr;
    auto plan = std::make_unique<TxopExchangePlan>();
    plan->rateRevision = qos->mib->getRateSetRevision();
    bool startsTxop = !qos->txopProcedure->hasStarted() && projectedCurrent == nullptr;
    auto sifs = context->getModeSet()->getSifsTime();
    auto interval = startsTxop ? SIMTIME_ZERO : sifs;
    std::unique_ptr<Packet> control;
    if (needsBar) {
        auto header = qos->blockAckProcedure->buildBasicBlockAckReqFrame(std::get<0>(bar), std::get<2>(bar), std::get<1>(bar));
        control = std::make_unique<Packet>("BasicBlockAckReq", header);
        control->insertAtBack(makeShared<Ieee80211MacTrailer>());
        candidate = control.get();
        plan->identity = {std::get<0>(bar), std::get<2>(bar), std::get<1>(bar).get()};
    }
    else {
        plan->payload = candidate;
        frames->describeTransmission(*plan);
        auto header = candidate->peekAtFront<Ieee80211DataOrMgmtHeader>();
        if (plan->group)
            plan->ackPolicy = AckPolicy::NO_ACK;
        else if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            auto agreement = qos->blockAckAgreementHandler ? qos->blockAckAgreementHandler->getAgreement(data->getReceiverAddress(), data->getTid()) : nullptr;
            plan->ackPolicy = qos->ackPolicy->computeAckPolicy(candidate, data, agreement);
            plan->initialBlockAckMsdu = !plan->retry && !data->getAMsduPresent() &&
                    data->getFragmentNumber() == 0 && !data->getMoreFragments() &&
                    agreement != nullptr && agreement->getIsAddbaResponseReceived() && plan->ackPolicy == AckPolicy::BLOCK_ACK;
        }
        else
            plan->ackPolicy = qos->ackPolicy->isAckNeeded(dynamicPtrCast<const Ieee80211MgmtHeader>(header)) ? AckPolicy::NORMAL_ACK : AckPolicy::NO_ACK;
    }
    auto header = candidate->peekAtFront<Ieee80211MacHeader>();
    plan->packetId = candidate->getId();
    auto receiver = header->getReceiverAddress();
    auto previous = context->getPreviousMode(receiver);
    if (projectedCurrent && projectedCurrent->identity.receiver == receiver)
        for (auto step : projectedCurrent->steps)
            if (step->getType() == IFrameSequenceStep::Type::TRANSMIT)
                previous = step->getPreparedMode();
    if (!needsBar && !startsTxop && previous == nullptr && context->getRtsPolicy()->isRtsNeeded(candidate, header)) {
        // 10.6.6.4 has no missing-history bound for RTS. A fresh TXOP uses 10.6.6.2.
        plan->missingPriorControlMode = true;
        return plan;
    }
    auto mode = qos->rateSelection->computeMode(candidate, header, startsTxop, previous, useFastestMode);
    auto own = [&](std::unique_ptr<IFrameSequenceStep> step) {
        auto pointer = context->ownPreparedStep(std::move(step));
        plan->steps.push_back(pointer);
        return pointer;
    };
    auto response = [&](ITransmitStep *tx, Ieee80211FrameType type, Ieee80211ResponseFrameKind kind, b length) {
        auto responseMode = qos->rateSelection->computeResponseMode(tx->getPreparedMode(), kind, receiver);
        auto timeout = type == ST_CTS ? context->getRtsPolicy()->getCtsTimeout(responseMode) :
                type == ST_BLOCKACK ? qos->ackPolicy->getBlockAckTimeout(responseMode) : qos->ackPolicy->getAckTimeout(responseMode);
        own(std::make_unique<ReceiveStep>(timeout, type, receiver, responseMode, length, sifs));
    };
    if (!needsBar && context->getRtsPolicy()->isRtsNeeded(candidate, header)) {
        auto rts = context->getRtsProcedure()->buildRtsFrame(candidate->peekAtFront<Ieee80211DataOrMgmtHeader>());
        auto packet = std::make_unique<Packet>("RTS", rts);
        packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
        auto rtsMode = qos->rateSelection->computeMode(packet.get(), rts, startsTxop, previous);
        auto step = std::make_unique<RtsTransmitStep>(candidate, packet.get(), interval, rtsMode);
        packet.release();
        auto tx = static_cast<ITransmitStep *>(own(std::move(step)));
        response(tx, ST_CTS, Ieee80211ResponseFrameKind::CTS, LENGTH_CTS);
        interval = sifs;
    }
    auto step = std::make_unique<TransmitStep>(candidate, interval, mode, needsBar);
    if (needsBar)
        control.release();
    auto tx = static_cast<ITransmitStep *>(own(std::move(step)));
    if (needsBar)
        response(tx, ST_BLOCKACK, Ieee80211ResponseFrameKind::BASIC_BLOCK_ACK, LENGTH_BASIC_BLOCKACK);
    else if (plan->ackPolicy == AckPolicy::NORMAL_ACK)
        response(tx, ST_ACK, Ieee80211ResponseFrameKind::ACK, LENGTH_ACK);
    if (!useFastestMode && qos->txopProcedure->getLimit() > SIMTIME_ZERO) {
        auto exchangeStart = simTime() + (projectedCurrent ? projectedCurrent->getDuration() : SIMTIME_ZERO);
        auto decision = qos->txopProcedure->evaluate(*plan, exchangeStart,
                exchangeStart + plan->getDuration(), projectedCurrent != nullptr, projectedCurrent);
        if (decision.accepted && decision.reason != TxopAdmissionReason::WITHIN_LIMIT)
            return prepare(context, projectedCurrent, forceBlockAck, true);
    }
    return plan;
}

void TxopExchangePlanner::prepareContinuation(FrameSequenceContext *context) const
{
    context->setContinuationPlan(nullptr);
    auto current = context->getExchangePlan();
    // A BAR reserves only its immediate BA response (9.2.5.2).
    if (!current->dataOrManagement)
        return;
    auto qos = context->getQoSContext();
    // IEEE Std 802.11-2024, 10.23.2.8: an IBSS backs off after each group frame.
    if (current->group && qos->mib->mode == Ieee80211Mib::INDEPENDENT)
        return;
    auto next = prepare(context, current, false);
    auto end = simTime() + current->getDuration();
    auto evaluate = [&](TxopExchangePlan& candidate) {
        // Trial extension: the current frame can reserve only a legal complete continuation.
        return qos->txopProcedure->evaluate(candidate, end, end + candidate.getDuration(), true, current);
    };
    if (next && !evaluate(*next).accepted) {
        auto bar = prepare(context, current, true);
        if (bar && evaluate(*bar).accepted)
            next = std::move(bar);
        else {
            qos->txopProcedure->commitAdmission(*next, evaluate(*next));
            return;
        }
    }
    if (next && evaluate(*next).accepted)
        context->setContinuationPlan(std::move(next));
}

} // namespace ieee80211
} // namespace inet
