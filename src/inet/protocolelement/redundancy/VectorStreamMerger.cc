//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/protocolelement/redundancy/VectorStreamMerger.h"

#include "inet/common/SequenceNumberTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(VectorStreamMerger);

simsignal_t VectorStreamMerger::recoveryPassedPacketsSignal = registerSignal("recoveryPassedPackets");
simsignal_t VectorStreamMerger::recoveryDiscardedPacketsSignal = registerSignal("recoveryDiscardedPackets");
simsignal_t VectorStreamMerger::recoveryRoguePacketsSignal = registerSignal("recoveryRoguePackets");
simsignal_t VectorStreamMerger::recoveryOutOfOrderPacketsSignal = registerSignal("recoveryOutOfOrderPackets");
simsignal_t VectorStreamMerger::recoveryLostPacketsSignal = registerSignal("recoveryLostPackets");
simsignal_t VectorStreamMerger::recoveryTaglessPacketsSignal = registerSignal("recoveryTaglessPackets");
simsignal_t VectorStreamMerger::recoveryResetsSignal = registerSignal("recoveryResets");


VectorStreamMerger::~VectorStreamMerger()
{
    for (auto& entry : recoveryStates)
        cancelAndDelete(entry.second->timer);
}

void VectorStreamMerger::initialize(int stage)
{
    StreamMerger::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        historyLength = par("historyLength");
        // Validate even when no streams are configured.
        VectorSequenceRecovery validation(historyLength);
        int64_t resetMilliseconds = par("resetTimeout").intValue();
        int64_t ticksPerSecond = par("ticksPerSecond").intValue();
        if (resetMilliseconds < 0 || resetMilliseconds > UINT32_MAX || ticksPerSecond < 100 || ticksPerSecond > UINT32_MAX)
            throw cRuntimeError("resetTimeout must be 0..4294967295 ms and ticksPerSecond must be 100..4294967295");
        // IEEE 802.1CB-2017 7.4.3.4 rounds the timeout up to whole timer ticks.
        uint64_t ticks = (uint64_t(resetMilliseconds) * ticksPerSecond + 999) / 1000;
        resetInterval = SimTime(double(ticks) / ticksPerSecond);
        if (resetMilliseconds > 0 && resetInterval <= SIMTIME_ZERO)
            throw cRuntimeError("Recovery timer cannot be represented at the simulation time resolution");
        takeNoSequence = par("takeNoSequence");
        individualRecovery = par("individualRecovery");
        recoveryMapping = check_and_cast<cValueMap *>(par("recoveryMapping").objectValue());
        if (backpressure)
            throw cRuntimeError("VectorStreamMerger requires backpressure=false to discard rejected frames");
        // Publish zero for inactive groups too, so last-value statistics are defined.
        for (auto signal : {recoveryPassedPacketsSignal, recoveryDiscardedPacketsSignal, recoveryRoguePacketsSignal,
                            recoveryOutOfOrderPacketsSignal, recoveryLostPacketsSignal, recoveryTaglessPacketsSignal,
                            recoveryResetsSignal})
            emit(signal, 0L);
        initialized = true;
        configureRecoveryStates();
    }
}

void VectorStreamMerger::configureRecoveryStates()
{
    std::set<std::string> streams;
    for (const auto& entry : mapping->getFields()) {
        const char *inputStream = entry.first.c_str();
        streams.insert(recoveryMapping->containsKey(inputStream) ?
                recoveryMapping->get(inputStream).stringValue() : entry.second.stringValue());
    }
    // Individual recovery must have one member stream per recovery instance.
    if (individualRecovery && streams.size() != mapping->getFields().size())
        throw cRuntimeError("Individual recovery requires a distinct recovery instance for each input stream");
    for (auto it = recoveryStates.begin(); it != recoveryStates.end(); ) {
        if (streams.count(it->first) == 0) {
            cancelAndDelete(it->second->timer);
            it = recoveryStates.erase(it);
        }
        else
            ++it;
    }
    for (const auto& stream : streams) {
        if (recoveryStates.count(stream) == 0) {
            auto state = std::make_unique<RecoveryState>(historyLength);
            state->timer = new cMessage("sequenceRecoveryTimeout");
            state->timer->setContextPointer(state.get());
            // A timeout at a frame's arrival time is processed first.
            state->timer->setSchedulingPriority(-1);
            recoveryStates.emplace(stream, std::move(state));
            recoveryResets++;
            emit(recoveryResetsSignal, (intval_t)recoveryResets);
        }
    }
}

void VectorStreamMerger::handleParameterChange(const char *name)
{
    StreamMerger::handleParameterChange(name);
    if (initialized && !strcmp(name, "recoveryMapping"))
        recoveryMapping = check_and_cast<cValueMap *>(par("recoveryMapping").objectValue());
    if (initialized && (!strcmp(name, "mapping") || !strcmp(name, "recoveryMapping")))
        configureRecoveryStates();
}

void VectorStreamMerger::handleMessage(cMessage *message)
{
    if (message->isSelfMessage()) {
        auto state = static_cast<RecoveryState *>(message->getContextPointer());
        state->recovery.reset();
        recoveryResets++;
        emit(recoveryResetsSignal, (intval_t)recoveryResets);
    }
    else
        StreamMerger::handleMessage(message);
}

const char *VectorStreamMerger::getOutputStream(const Packet *packet) const
{
    auto stream = packet->findTag<StreamInd>();
    if (stream == nullptr || !mapping->containsKey(stream->getStreamName()))
        return nullptr;
    return mapping->get(stream->getStreamName()).stringValue();
}

const char *VectorStreamMerger::getRecoveryStream(const Packet *packet) const
{
    auto outputStream = getOutputStream(packet);
    if (outputStream == nullptr)
        return nullptr;
    const char *inputStream = packet->getTag<StreamInd>()->getStreamName();
    return recoveryMapping->containsKey(inputStream) ? recoveryMapping->get(inputStream).stringValue() : outputStream;
}

int VectorStreamMerger::getSequenceNumber(const Packet *packet) const
{
    auto tag = packet->findTag<SequenceNumberInd>();
    int sequenceNumber = tag == nullptr ? -1 : tag->getSequenceNumber();
    return sequenceNumber >= 0 && sequenceNumber < VectorSequenceRecovery::SEQUENCE_NUMBER_SPACE
           ? sequenceNumber : -1;
}

bool VectorStreamMerger::matchesPacket(const Packet *packet) const
{
    auto stream = getRecoveryStream(packet);
    if (stream == nullptr)
        return true;
    int sequenceNumber = getSequenceNumber(packet);
    if (sequenceNumber < 0)
        return takeNoSequence;
    auto decision = recoveryStates.at(stream)->recovery.classify(sequenceNumber);
    return decision == VectorSequenceRecovery::ACCEPT || decision == VectorSequenceRecovery::ACCEPT_OUT_OF_ORDER;
}

void VectorStreamMerger::reloadTimer(RecoveryState& state)
{
    // With RemainingTicks=0 there is no 1-to-0 transition in 7.4.3.2.4.
    if (resetInterval == SIMTIME_ZERO)
        cancelEvent(state.timer);
    else
        rescheduleAt(simTime() + resetInterval, state.timer);
}

void VectorStreamMerger::processRecovery(Packet *packet, bool accepted)
{
    auto stream = getRecoveryStream(packet);
    if (stream == nullptr)
        return;
    auto& state = *recoveryStates.at(stream);
    int sequenceNumber = getSequenceNumber(packet);
    if (sequenceNumber < 0) {
        taglessPackets++;
        emit(recoveryTaglessPacketsSignal, (intval_t)taglessPackets);
        if (!accepted) {
            discardedPackets++;
            emit(recoveryDiscardedPacketsSignal, (intval_t)discardedPackets);
        }
    }
    else {
        auto result = state.recovery.process(sequenceNumber);
        if (result.lostPackets != 0) {
            lostPackets += result.lostPackets;
            emit(recoveryLostPacketsSignal, (intval_t)lostPackets);
        }
        if (result.decision == VectorSequenceRecovery::DUPLICATE) {
            discardedPackets++;
            emit(recoveryDiscardedPacketsSignal, (intval_t)discardedPackets);
        }
        else if (result.decision == VectorSequenceRecovery::ROGUE) {
            roguePackets++;
            emit(recoveryRoguePacketsSignal, (intval_t)roguePackets);
        }
        else if (result.decision == VectorSequenceRecovery::ACCEPT_OUT_OF_ORDER) {
            outOfOrderPackets++;
            emit(recoveryOutOfOrderPacketsSignal, (intval_t)outOfOrderPackets);
        }
        // Individual recovery reloads the timer on numbered discards as well.
        if (!accepted && individualRecovery)
            reloadTimer(state);
    }
    if (accepted) {
        passedPackets++;
        emit(recoveryPassedPacketsSignal, (intval_t)passedPackets);
        reloadTimer(state);
    }
}

void VectorStreamMerger::processPacket(Packet *packet)
{
    auto stream = getOutputStream(packet);
    if (stream != nullptr) {
        if (*stream != '\0')
            packet->getTagForUpdate<StreamInd>()->setStreamName(stream);
        else {
            packet->removeTag<StreamInd>();
            packet->removeTagIfPresent<SequenceNumberInd>();
        }
    }
}

void VectorStreamMerger::handlePacketFiltered(Packet *packet, bool matches)
{
    // Commit recovery once, before packet tags are changed or the packet is dropped.
    // The legacy StreamMerger hook maintains its separate list-based history.
    processRecovery(packet, matches);
}

void VectorStreamMerger::pushPacketStart(Packet *, const cGate *, bps)
{
    throw cRuntimeError("VectorStreamMerger requires complete frames; packet streaming is unsupported");
}

void VectorStreamMerger::pushPacketEnd(Packet *, const cGate *)
{
    throw cRuntimeError("VectorStreamMerger requires complete frames; packet streaming is unsupported");
}

void VectorStreamMerger::pushPacketProgress(Packet *, const cGate *, bps, b, b)
{
    throw cRuntimeError("VectorStreamMerger requires complete frames; packet streaming is unsupported");
}

} // namespace inet
