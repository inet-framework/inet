//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee80211/mac/ratecontrol/MinstrelHtRateControl.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <tuple>

#include "inet/linklayer/ieee80211/mac/ratecontrol/MinstrelRateControlTag_m.h"
#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace physicallayer;

Define_Module(MinstrelHtRateControl);

void MinstrelHtRateControl::initialize(int stage)
{
    RateControlBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this, "mibModule", true);
        radio.reference(this, "radioModule", true);
        interval = par("interval");
        retrySegmentDuration = par("retrySegmentDuration");
        packetLength = B(par("packetLength"));
        maxChannelWidth = Hz(par("maxChannelWidth"));
        maxRetryCount = par("maxRetryCount");
        maxNumSpatialStreams = par("maxNumSpatialStreams");
        probeEnabled = par("probeEnabled");
        if (interval <= SIMTIME_ZERO || retrySegmentDuration <= SIMTIME_ZERO || packetLength <= b(0) ||
                !std::isfinite(maxChannelWidth.get()) || maxChannelWidth <= Hz(0) ||
                maxRetryCount < 2 || maxRetryCount > 64 ||
                (maxNumSpatialStreams != -1 && maxNumSpatialStreams < 1))
            throw cRuntimeError("Invalid Minstrel interval, packet length, retry budget, channel width or stream limit");
        WATCH_EXPR("numStations", (int)stations.size());
    }
}

void MinstrelHtRateControl::handleMessage(cMessage *message)
{
    throw cRuntimeError("MinstrelHtRateControl does not handle messages");
}

void MinstrelHtRateControl::resetRateControl()
{
    stations.clear();
    // nextGeneration is deliberately not reset: old packet plans must expire.
}

const IIeee80211Mode *MinstrelHtRateControl::getLegacyMode() const
{
    const IIeee80211Mode *result = nullptr;
    for (auto mode : modeSet->getLegacyOperationalModes())
        if (modeSet->getIsMandatory(mode) &&
                (!result || mode->getDataMode()->getNetBitrate() > result->getDataMode()->getNetBitrate()))
            result = mode;
    if (!result)
        throw cRuntimeError("MinstrelHtRateControl requires a mandatory legacy mode for management and unnegotiated peers");
    return result;
}

std::vector<const IIeee80211Mode *> MinstrelHtRateControl::getSupportedModes(const MacAddress& receiver) const
{
    if (!modeSet || (modeSet->getPhyType() != Ieee80211ModeSet::PhyType::HT &&
                    modeSet->getPhyType() != Ieee80211ModeSet::PhyType::VHT))
        throw cRuntimeError("MinstrelHtRateControl requires an HT or VHT mode set");
    int streamLimit = radio->getAntenna()->getNumAntennas();
    if (maxNumSpatialStreams != -1)
        streamLimit = std::min(streamLimit, maxNumSpatialStreams);
    auto peer = mib->findPeerHtState(receiver);
    const auto& legacyModes = modeSet->getLegacyOperationalModes();
    std::vector<const IIeee80211Mode *> result;
    for (int i = 0; i < modeSet->getNumModes(); i++) {
        auto mode = modeSet->getMode(i);
        if (std::find(legacyModes.begin(), legacyModes.end(), mode) != legacyModes.end())
            continue;
        if (mode->getDataMode()->getNumberOfSpatialStreams() > streamLimit ||
                mode->getDataMode()->getBandwidth() > maxChannelWidth)
            continue;
        if (modeSet->getPhyType() == Ieee80211ModeSet::PhyType::HT) {
            // Reuse the same negotiated HT eligibility as the production selector.
            // HT duplicate/unequal-modulation MCS are outside Minstrel's groups.
            if (mode->getHtMcsIndex() < 0 || mode->getHtMcsIndex() >= 32 ||
                    selectPeerCompatibleMode(modeSet, peer, mode, receiver) != mode)
                continue;
        }
        result.push_back(mode);
    }
    // Pre-association data can use a safe legacy mode, but does not train HT.
    return result;
}

MinstrelHtRateControl::State& MinstrelHtRateControl::getState(const MacAddress& receiver)
{
    auto modes = getSupportedModes(receiver);
    auto& state = stations[receiver];
    bool changed = modes.size() != state.rates.size();
    for (size_t i = 0; !changed && i < modes.size(); i++)
        changed = modes[i] != state.rates[i].mode;
    if (state.generation == 0 || changed) {
        state = State();
        state.address = receiver;
        state.generation = ++nextGeneration;
        initializeState(state, modes);
        if (!state.rates.empty())
            emitDatarateChangedSignal(receiver, state.rates[state.bestThroughput[0]].mode);
    }
    return state;
}

void MinstrelHtRateControl::initializeState(State& state, const std::vector<const IIeee80211Mode *>& modes)
{
    state.lastUpdate = simTime();
    // Sort groups by family, width, long before short GI, then stream count.
    // A group is identified by the full PHY tuple; equal bitrates are not aliases.
    using GroupKey = std::tuple<bool, Hz, bool, int>;
    std::map<GroupKey, std::vector<int>> groupedRates;
    auto ackDuration = getLegacyMode()->getDuration(LENGTH_ACK);
    for (auto mode : modes) {
        Rate rate;
        rate.mode = mode;
        rate.dataDuration = mode->getDataMode()->getDuration(packetLength);
        rate.exchangeDuration = mode->getDuration(packetLength) + mode->getSifsTime() + ackDuration;
        auto dataMode = mode->getDataMode();
        bool isVht = mode->getHtMcsIndex() < 0;
        // getSymbolInterval() names the long-GI timing constant even for SGI
        // modes. Read the mode's selected guard interval instead.
        bool shortGuardInterval = isVht ?
            check_and_cast<const Ieee80211VhtMode *>(mode)->getDataMode()->getGuardIntervalType() == Ieee80211VhtModeBase::HT_GUARD_INTERVAL_SHORT :
            mode->isHtShortGuardInterval();
        auto key = std::make_tuple(isVht, dataMode->getBandwidth(), shortGuardInterval, dataMode->getNumberOfSpatialStreams());
        groupedRates[key].push_back(state.rates.size());
        state.rates.push_back(rate);
    }
    for (const auto& entry : groupedRates) {
        Group group;
        group.rates = entry.second;
        group.bestProbability = group.rates.front();
        for (int index : group.rates)
            state.rates[index].group = state.groups.size();
        for (auto& column : group.samples) {
            column = group.rates;
            // Simulator-owned RNG keeps the table reproducible for a fixed seed.
            for (int i = column.size() - 1; i > 0; i--)
                std::swap(column[i], column[intuniform(0, i)]);
        }
        state.groups.push_back(group);
    }
    state.sampleCount = state.groups.size() * 8;
    if (!state.rates.empty())
        rankRates(state);
}

double MinstrelHtRateControl::computeThroughput(int probability, simtime_t duration)
{
    // Linux v4.19 minstrel_ht_get_tp_avg: reject <10%, cap probability at 90%.
    if (probability < PROBABILITY_SCALE / 10)
        return 0;
    return double(std::min(probability, PROBABILITY_SCALE * 90 / 100)) / PROBABILITY_SCALE / duration.dbl();
}

void MinstrelHtRateControl::rankRates(State& state)
{
    auto betterThroughput = [&state](int left, int right) {
        const auto& a = state.rates[left];
        const auto& b = state.rates[right];
        if (a.throughput != b.throughput)
            return a.throughput > b.throughput;
        if (a.probability != b.probability)
            return a.probability > b.probability;
        // Stable PHY ordering resolves unmeasured/equal rates conservatively.
        return left < right;
    };
    auto betterProbability = [&state](int candidate, int current) {
        const auto& a = state.rates[candidate];
        const auto& b = state.rates[current];
        return a.probability > PROBABILITY_SCALE * 75 / 100 ?
               a.throughput > b.throughput : a.probability > b.probability;
    };
    std::vector<int> sorted(state.rates.size());
    std::iota(sorted.begin(), sorted.end(), 0);
    std::stable_sort(sorted.begin(), sorted.end(), betterThroughput);
    state.bestThroughput = { state.rates[sorted[0]].throughput > 0 ? sorted[0] : 0,
                            sorted.size() > 1 && state.rates[sorted[1]].throughput > 0 ? sorted[1] : 0 };
    for (auto& group : state.groups) {
        auto groupSorted = group.rates;
        std::stable_sort(groupSorted.begin(), groupSorted.end(), betterThroughput);
        group.bestThroughput = { state.rates[groupSorted[0]].throughput > 0 ? groupSorted[0] : group.rates.front(),
                                groupSorted.size() > 1 && state.rates[groupSorted[1]].throughput > 0 ?
                                groupSorted[1] : group.rates.front() };
        for (int index : group.rates) {
            if (state.rates[index].throughput == 0)
                continue;
            if (betterProbability(index, group.bestProbability))
                group.bestProbability = index;
            if (betterProbability(index, state.bestProbability))
                state.bestProbability = index;
        }
    }
    // minstrel_ht_prob_rate_reduce_streams: prefer a working lower-NSS fallback.
    int streams = state.rates[state.bestThroughput[0]].mode->getDataMode()->getNumberOfSpatialStreams();
    double bestThroughput = 0;
    for (const auto& group : state.groups) {
        int index = group.bestProbability;
        const auto& rate = state.rates[index];
        if (rate.mode->getDataMode()->getNumberOfSpatialStreams() < streams && rate.throughput > bestThroughput) {
            state.bestProbability = index;
            bestThroughput = rate.throughput;
        }
    }
}

void MinstrelHtRateControl::updateStatistics(State& state)
{
    // Linux v4.19 minstrel_calc_rate_stats. Seed the first measurement directly;
    // idle intervals preserve probability and age the rate for occasional probes.
    for (auto& rate : state.rates) {
        if (rate.attempts > 0) {
            int probability = (rate.successes * PROBABILITY_SCALE) / rate.attempts;
            rate.probability = rate.measured ? rate.probability + (probability - rate.probability) / 4 : probability;
            rate.measured = true;
            rate.skippedIntervals = 0;
        }
        else
            rate.skippedIntervals = std::min(rate.skippedIntervals + 1, 20);
        rate.attempts = rate.successes = 0;
        rate.throughput = computeThroughput(rate.probability, rate.exchangeDuration);
    }
    rankRates(state);
    state.lastUpdate = simTime();
    state.slowSamples = 0;
    state.sampleCount = state.groups.size() * 8;
}

int MinstrelHtRateControl::selectSample(State& state)
{
    if (!probeEnabled)
        return -1;
    if (state.sampleWait > 0) {
        state.sampleWait--;
        return -1;
    }
    if (state.sampleTries == 0)
        return -1;
    auto& group = state.groups[state.sampleGroup];
    int index = group.samples[group.column][group.index];
    if (++group.index == (int)group.rates.size()) {
        group.index = 0;
        group.column = (group.column + 1) % SAMPLE_COLUMNS;
    }
    state.sampleGroup = (state.sampleGroup + 1) % state.groups.size();
    const auto& rate = state.rates[index];
    if (index == state.bestThroughput[0] || index == state.bestProbability ||
            rate.probability > PROBABILITY_SCALE * 95 / 100)
        return -1;
    int faster = state.bestThroughput[0], slower = state.bestThroughput[1];
    if (state.rates[faster].dataDuration > state.rates[slower].dataDuration)
        std::swap(faster, slower);
    int streams = state.rates[faster].mode->getDataMode()->getNumberOfSpatialStreams();
    if (rate.dataDuration >= state.rates[slower].dataDuration &&
            (rate.mode->getDataMode()->getNumberOfSpatialStreams() >= streams ||
             rate.dataDuration >= state.rates[state.bestProbability].dataDuration)) {
        if (rate.skippedIntervals < 20 || state.slowSamples++ >= 3)
            return -1;
    }
    state.sampleTries--;
    return index;
}

int MinstrelHtRateControl::computeRetryCount(const Rate& rate) const
{
    // minstrel_ht_set_rate uses two attempts below 20% probability. For usable
    // rates, count attempts up to the segment budget, capped by maxRetryCount.
    if (rate.probability < PROBABILITY_SCALE / 5)
        return 2;
    int contentionWindow = modeSet->getCwMin();
    simtime_t duration;
    auto addAttempt = [&]() {
        duration += rate.exchangeDuration + modeSet->getSlotTime() * contentionWindow / 2;
        contentionWindow = std::min(2 * contentionWindow + 1, modeSet->getCwMax());
    };
    addAttempt();
    addAttempt();
    int count = 2;
    // The reference counts another attempt only if its total stays below the budget.
    while (count < maxRetryCount) {
        addAttempt();
        if (duration >= retrySegmentDuration)
            break;
        count++;
    }
    return count;
}

void MinstrelHtRateControl::downgradeRate(State& state, int slot)
{
    int index = state.bestThroughput[slot];
    const auto& rate = state.rates[index];
    if (rate.attempts <= 30 || rate.successes * 5 >= rate.attempts)
        return;
    int streams = rate.mode->getDataMode()->getNumberOfSpatialStreams();
    for (int group = rate.group - 1; group >= 0; group--) {
        int candidate = state.groups[group].bestThroughput[slot];
        if (state.rates[candidate].mode->getDataMode()->getNumberOfSpatialStreams() <= streams) {
            state.bestThroughput[slot] = candidate;
            return;
        }
    }
}

const IIeee80211Mode *MinstrelHtRateControl::getRate(const MacAddress& receiver)
{
    Enter_Method("getRate");
    if (receiver.isMulticast())
        return getLegacyMode();
    auto& state = getState(receiver);
    return state.rates.empty() ? getLegacyMode() : state.rates[state.bestThroughput[0]].mode;
}

const IIeee80211Mode *MinstrelHtRateControl::getRateForFrame(Packet *frame)
{
    Enter_Method("getRateForFrame");
    const auto& header = frame->peekAtFront<Ieee80211MacHeader>();
    auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header);
    if (!dataHeader || header->getReceiverAddress().isMulticast())
        return getLegacyMode();
    if (dataHeader->getAckPolicy() != NORMAL_ACK)
        throw cRuntimeError("MinstrelHtRateControl requires normal-ACK data feedback; Block Ack and no-ACK are unsupported");
    auto& state = getState(header->getReceiverAddress());
    if (state.rates.empty()) {
        frame->removeTagIfPresent<MinstrelRateControlTag>();
        return getLegacyMode();
    }
    const auto& plan = frame->addTagIfAbsent<MinstrelRateControlTag>();
    if (plan->getControllerId() != getId() || plan->getGeneration() != state.generation) {
        int sample = selectSample(state);
        // The probe replaces the primary stage, as in mac80211's rate table.
        std::array<int, 3> chain = { sample >= 0 ? sample : state.bestThroughput[0],
                                    state.bestThroughput[1], state.bestProbability };
        plan->setControllerId(getId());
        plan->setGeneration(state.generation);
        plan->setProbe(sample >= 0);
        plan->setStage(0);
        plan->setStageAttempts(0);
        for (int i = 0; i < 3; i++) {
            plan->setModes(i, state.rates[chain[i]].mode);
            plan->setAttempts(i, i == 0 && sample >= 0 ? 1 : computeRetryCount(state.rates[chain[i]]));
        }
    }
    return plan->getModes(plan->getStage());
}

void MinstrelHtRateControl::frameTransmitted(Packet *frame, int retryCount, bool successful, bool givenUp)
{
    Enter_Method("frameTransmitted");
    const auto& header = frame->peekAtFront<Ieee80211MacHeader>();
    if (!dynamicPtrCast<const Ieee80211DataHeader>(header) || header->getReceiverAddress().isMulticast())
        return;
    const auto& plan = frame->findTagForUpdate<MinstrelRateControlTag>();
    if (!plan || plan->getControllerId() != getId())
        return; // An explicit rate override or pre-association legacy frame.
    auto& state = getState(header->getReceiverAddress());
    if (plan->getGeneration() != state.generation)
        return; // Feedback from the previous mode set / negotiated rate set.
    const auto& modeRequest = frame->getTag<Ieee80211ModeReq>();
    auto actualMode = modeRequest->getMode();
    auto found = std::find_if(state.rates.begin(), state.rates.end(),
            [actualMode](const Rate& rate) { return rate.mode == actualMode; });
    if (found == state.rates.end())
        return;
    found->attempts++;
    found->successes += successful;
    if (successful || givenUp)
        frame->removeTag<MinstrelRateControlTag>();
    else {
        int attempts = plan->getStageAttempts() + 1;
        if (attempts >= plan->getAttempts(plan->getStage()) && plan->getStage() < 2) {
            plan->setStage(plan->getStage() + 1);
            attempts = 0;
        }
        plan->setStageAttempts(attempts);
    }
    // avg_ampdu_len is one for the available per-MPDU normal-ACK feedback.
    if (!state.sampleWait && !state.sampleTries && state.sampleCount > 0) {
        state.sampleWait = 18;
        state.sampleTries = 1;
        state.sampleCount--;
    }
    auto oldMode = state.rates[state.bestThroughput[0]].mode;
    downgradeRate(state, 0);
    downgradeRate(state, 1);
    if (simTime() - state.lastUpdate >= interval)
        updateStatistics(state);
    auto newMode = state.rates[state.bestThroughput[0]].mode;
    if (newMode != oldMode)
        emitDatarateChangedSignal(state.address, newMode);
}

void MinstrelHtRateControl::frameReceived(Packet *frame)
{
}

} // namespace ieee80211
} // namespace inet
