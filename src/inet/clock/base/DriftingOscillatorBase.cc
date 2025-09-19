//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/base/DriftingOscillatorBase.h"

namespace inet {

void DriftingOscillatorBase::initialize(int stage)
{
    OscillatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        double nominalTickLengthAsDouble = par("nominalTickLength");
        nominalTickLength = nominalTickLengthAsDouble;
        if (nominalTickLength == 0)
            nominalTickLength.setRaw(1);
        else if (std::abs(nominalTickLength.dbl() - nominalTickLengthAsDouble) / nominalTickLengthAsDouble > 1E-15)
            throw cRuntimeError("The nominalTickLength parameter value %lg cannot be accurately represented with the current simulation time precision, conversion result: %s", nominalTickLengthAsDouble, nominalTickLength.ustr().c_str());
        setOrigin(simTime());
        driftFactor = 1.0L + driftRate.get() / 1E+6L;
        simtime_t currentTickLength = getCurrentTickLength();
        simtime_t tickOffset = par("tickOffset");
        if (tickOffset < 0 || tickOffset >= currentTickLength)
            throw cRuntimeError("First tick offset must be in the range [0, currentTickLength)");
        nextTickFromOrigin = currentTickLength - tickOffset;
        WATCH(nominalTickLength);
        WATCH(driftRate);
        WATCH(driftFactor);
        WATCH(nextTickFromOrigin);
    }
}

void DriftingOscillatorBase::handleTickTimer()
{
    setOrigin(simTime());
    nextTickFromOrigin = getCurrentTickLength();
    OscillatorBase::handleTickTimer();
}

void DriftingOscillatorBase::scheduleTickTimer()
{
    scheduleAfter(nextTickFromOrigin, tickTimer);
}

void DriftingOscillatorBase::setOrigin(simtime_t origin)
{
    ASSERTCMP(<=, origin, simTime());
    ASSERTCMP(<=, origin, newOrigin);
    this->origin = origin;
}

void DriftingOscillatorBase::setDriftRate(ppm newDriftRate)
{
    Enter_Method("setDriftRate");
    if (newDriftRate != driftRate) {
        emit(preOscillatorStateChangedSignal, this);
        simtime_t currentSimTime = simTime();
        EV_DEBUG << "Setting oscillator drift rate from " << driftRate << " to " << newDriftRate << " at simtime " << currentSimTime << ".\n";
        simtime_t currentTickLength = getCurrentTickLength();
        simtime_t baseTickTime = origin + nextTickFromOrigin - currentTickLength;
        simtime_t elapsedTickTime = fmod(currentSimTime - baseTickTime, currentTickLength);
        double long newDriftFactor = 1.0L + newDriftRate.get() / 1E+6L;
        nextTickFromOrigin = SimTime::fromRaw((currentTickLength.raw() - elapsedTickTime.raw()) * driftFactor / newDriftFactor);
        driftRate = newDriftRate;
        driftFactor = newDriftFactor;
        if (driftRate != ppm(0)) {
            ASSERTCMP(!=, nominalTickLength, currentTickLength);
            ASSERTCMP(==, nominalTickLength, currentTickLength * driftFactor);
        }
        setOrigin(currentSimTime);
        emit(driftRateChangedSignal, driftRate.get<ppm>());
        emit(postOscillatorStateChangedSignal, this);
    }
}

void DriftingOscillatorBase::setTickOffset(simtime_t newTickOffset)
{
    Enter_Method("setTickOffset");
    simtime_t currentSimTime = simTime();
    simtime_t currentTickLength = getCurrentTickLength();
    simtime_t baseTickTime = origin + nextTickFromOrigin - currentTickLength;
    simtime_t oldTickOffset = fmod(currentSimTime - baseTickTime, currentTickLength);
    if (newTickOffset != oldTickOffset) {
        emit(preOscillatorStateChangedSignal, this);
        EV_DEBUG << "Setting oscillator tick offset from " << oldTickOffset << " to " << newTickOffset << " at simtime " << currentSimTime << ".\n";
        setOrigin(currentSimTime);
        nextTickFromOrigin = currentTickLength - newTickOffset;
        emit(postOscillatorStateChangedSignal, this);
    }
}

int64_t DriftingOscillatorBase::doComputeTicksForInterval(simtime_t timeInterval) const
{
    int64_t result = floor((timeInterval - nextTickFromOrigin).raw() * driftFactor / nominalTickLength.raw()) + 1;
    return result;
}

simtime_t DriftingOscillatorBase::doComputeIntervalForTicks(int64_t numTicks) const
{
    simtime_t result = SimTime::fromRaw(ceil((nominalTickLength.raw() * (numTicks - 1)) / driftFactor)) + nextTickFromOrigin;
    if (result < 0)
        result = 0;
    return result;
}

int64_t DriftingOscillatorBase::computeTicksForInterval(simtime_t timeInterval) const
{
    ASSERTCMP(>=, timeInterval, 0);
    int64_t result = doComputeTicksForInterval(timeInterval);
    ASSERTCMP(>=, result, 0);
    ASSERTCMP(<=, doComputeIntervalForTicks(result), timeInterval);
    return result;
}

simtime_t DriftingOscillatorBase::computeIntervalForTicks(int64_t numTicks) const
{
    ASSERTCMP(>=, numTicks, 0);
    simtime_t result = doComputeIntervalForTicks(numTicks);
    ASSERTCMP(>=, result, 0);
    ASSERTCMP(==, doComputeTicksForInterval(result), numTicks);
    return result;
}

void DriftingOscillatorBase::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-oscillator")) {
        if (const char *driftRateStr = node.getAttribute("drift-rate")) {
            ppm newDriftRate = ppm(strtod(driftRateStr, nullptr));
            setDriftRate(newDriftRate);
        }
        if (const char *tickOffsetStr = node.getAttribute("tick-offset")) {
            simtime_t newTickOffset = SimTime::parse(tickOffsetStr);
            setTickOffset(newTickOffset);
        }
    }
    else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

std::string DriftingOscillatorBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'c':
            return std::to_string(getCurrentTickLength()) + " s";
        case 'd':
            return driftRate.str();
        default:
            return OscillatorBase::resolveDirective(directive);
    }
}

} // namespace inet

