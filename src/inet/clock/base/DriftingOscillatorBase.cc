//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/base/DriftingOscillatorBase.h"

#include "inet/common/IPrintableObject.h"

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
        setDriftFactor(SimTimeScale::fromPpm(driftRate.get<ppm>()));
        numTicksAtOrigin = 0;
        simtime_t currentTickLength = getCurrentTickLength();
        simtime_t tickOffset = par("tickOffset");
        if (tickOffset < 0 || tickOffset >= currentTickLength)
            throw cRuntimeError("First tick offset must be in the range [0, currentTickLength)");
        nextTickFromOrigin = currentTickLength - tickOffset;
        emit(driftRateChangedSignal, driftRate.get<ppm>());
        WATCH(nominalTickLength);
        WATCH(driftRate);
        WATCH(driftFactor);
        WATCH(nextTickFromOrigin);
    }
}

void DriftingOscillatorBase::finish()
{
    emit(driftRateChangedSignal, driftRate.get<ppm>());
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
    EV_DEBUG << "Setting oscillator origin" << EV_FIELD(origin) << EV_ENDL;
    ASSERTCMP(<=, origin, simTime());
    ASSERTCMP(<=, this->origin, origin);
    numTicksAtOrigin += computeTicksForInterval(origin - this->origin);
    this->origin = origin;
}

void DriftingOscillatorBase::setDriftRate(ppm newDriftRate)
{
    Enter_Method("setDriftRate");
    if (newDriftRate != driftRate) {
        emit(preOscillatorStateChangedSignal, this);
        simtime_t currentSimTime = simTime();
        EV_INFO << "Setting oscillator drift rate from " << driftRate << " to " << newDriftRate << " at simtime " << currentSimTime << ".\n";
        simtime_t oldCurrentTickLength = getCurrentTickLength();
        simtime_t baseTickTime = origin + nextTickFromOrigin - oldCurrentTickLength;
        simtime_t elapsedTickTime = fmod(currentSimTime - baseTickTime, oldCurrentTickLength);
        SimTimeScale newDriftFactor = SimTimeScale::fromPpm(newDriftRate.get<ppm>());
        simtime_t remainingTickTime = oldCurrentTickLength - elapsedTickTime;
        nextTickFromOrigin = SimTime::fromRaw(remainingTickTime.raw() * driftFactor / newDriftFactor);
        driftRate = newDriftRate;
        setDriftFactor(newDriftFactor);
        setOrigin(currentSimTime);
        if (tickTimer) {
            cancelEvent(tickTimer);
            scheduleTickTimer();
        }
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
        EV_INFO << "Setting oscillator tick offset from " << oldTickOffset << " to " << newTickOffset << " at simtime " << currentSimTime << ".\n";
        setOrigin(currentSimTime);
        nextTickFromOrigin = currentTickLength - newTickOffset;
        if (tickTimer) {
            cancelEvent(tickTimer);
            scheduleTickTimer();
        }
        emit(postOscillatorStateChangedSignal, this);
    }
}

void DriftingOscillatorBase::setDriftFactor(SimTimeScale driftFactor)
{
    const simtime_t roundTripNominalTickLengthLowerBound = SimTime::fromRaw(driftFactor.mulCeil(driftFactor.divFloor(nominalTickLength.raw())));
    const simtime_t roundTripNominalTickLengthUpperBound = SimTime::fromRaw(driftFactor.mulFloor(driftFactor.divCeil(nominalTickLength.raw())));
    ASSERTCMP(<=, roundTripNominalTickLengthLowerBound, nominalTickLength);
    ASSERTCMP(<=, nominalTickLength, roundTripNominalTickLengthUpperBound);
    this->driftFactor = driftFactor;
}

simtime_t DriftingOscillatorBase::getCurrentTickLength() const
{
    if (driftFactor.raw() > 0) // driftFactor > 1
        return SimTime::fromRaw(driftFactor.divFloor(nominalTickLength.raw()));
    else  if (driftFactor.raw() < 0) // driftFactor < 1
        return SimTime::fromRaw(driftFactor.divCeil(nominalTickLength.raw()));
    else
        return nominalTickLength; // driftFactor == 1
}

int64_t DriftingOscillatorBase::doComputeTicksForInterval(simtime_t timeInterval) const
{
    DEBUG_ENTER(false, timeInterval);
    DEBUG_OUT << DEBUG_FIELD(driftFactor) << DEBUG_FIELD(nextTickFromOrigin) << DEBUG_FIELD(nominalTickLength) << std::endl;
    int64_t result;
    if (timeInterval == 0)
        result = 0;
    else {
        auto divFloor = [] (simtime_raw_t a, simtime_raw_t b) -> simtime_raw_t {
            ASSERT(b > 0);
            simtime_raw_t q = a / b;
            simtime_raw_t r = a % b; // r has sign of a in C++
            if (r != 0 && a < 0) --q; // adjust trunc->floor for negative a
            return q;
        };
        result = divFloor(driftFactor.mulFloor((timeInterval - nextTickFromOrigin).raw()), nominalTickLength.raw()) + 1;
        if (result < 0)
        result = 0;
    }
    DEBUG_LEAVE(result);
    return result;
}

simtime_t DriftingOscillatorBase::doComputeIntervalForTicks(int64_t numTicks) const
{
    DEBUG_ENTER(false, numTicks);
    DEBUG_OUT << DEBUG_FIELD(driftFactor) << DEBUG_FIELD(nextTickFromOrigin) << DEBUG_FIELD(nominalTickLength) << std::endl;
    simtime_t result;
    if (numTicks == 0)
        result = 0;
    else {
        result = nextTickFromOrigin + SimTime::fromRaw(driftFactor.divCeil(nominalTickLength.raw() * (numTicks - 1)));
        if (result < 0)
            result = 0;
    }
    DEBUG_LEAVE(result);
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
            return getCurrentTickLength().str() + " s";
        case 'd':
            return driftRate.str();
        default:
            return OscillatorBase::resolveDirective(directive);
    }
}

} // namespace inet

