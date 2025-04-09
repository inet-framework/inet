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
        computeAsSeparateTicks = par("computeAsSeparateTicks");
        double nominalTickLengthAsDouble = par("nominalTickLength");
        nominalTickLength = nominalTickLengthAsDouble;
        if (nominalTickLength == 0)
            nominalTickLength.setRaw(1);
        else if (std::abs(nominalTickLength.dbl() - nominalTickLengthAsDouble) / nominalTickLengthAsDouble > 1E-15)
            throw cRuntimeError("The nominalTickLength parameter value %lg cannot be accurately represented with the current simulation time precision, conversion result: %s", nominalTickLengthAsDouble, nominalTickLength.ustr().c_str());
        setOrigin(simTime());
        driftFactor = 1.0L + driftRate.get() / 1E+6L;
        // TODO check the relationship between nominalTickLength and simulation time precision and fail if they are too close (whatever that means) use a parameter for this? minimum

        // TODO check if tick becomes smaller than what can be represented in simtime and error out
        // TODO what if the precision is just not enough to represent the tick properly
        // TODO should there be some wrap around calculation to check that?
        simtime_t currentTickLength = getCurrentTickLength();
        if (driftRate != ppm(0)) {
            ASSERTCMP(!=, nominalTickLength, currentTickLength);
            ASSERTCMP(==, nominalTickLength, SimTime::fromRaw(roundl(currentTickLength.raw() * driftFactor)));
        }
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
    EV_DEBUG << "Setting oscillator origin" << EV_FIELD(origin) << EV_ENDL;
    ASSERTCMP(<=, origin, simTime());
    ASSERTCMP(<=, this->origin, origin);
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
        long double newDriftFactor = 1.0L + newDriftRate.get() / 1E+6L;
        simtime_t remainingTickTime = oldCurrentTickLength - elapsedTickTime;
        nextTickFromOrigin = SimTime::fromRaw(remainingTickTime.raw() * driftFactor / newDriftFactor);
        EV_DEBUG << "XXX: driftFactor = " << driftFactor << ", newDriftFactor = " << newDriftFactor << ", oldCurrentTickLength = " << oldCurrentTickLength << ", elapsedTickTime = " << elapsedTickTime << ", remainingTickTime = " << remainingTickTime << ", nextTickFromOrigin = " << nextTickFromOrigin << std::endl;
        driftRate = newDriftRate;
        driftFactor = newDriftFactor;
        EV_DEBUG << "YYY: newCurrentTickLength = " << getCurrentTickLength() << std::endl;
        if (driftRate != ppm(0)) {
            simtime_t newCurrentTickLength = getCurrentTickLength();
            ASSERTCMP(!=, nominalTickLength, newCurrentTickLength);
            ASSERTCMP(==, nominalTickLength, SimTime::fromRaw(roundl(newCurrentTickLength.raw() * driftFactor)));
        }
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

int64_t DriftingOscillatorBase::doComputeTicksForInterval(simtime_t timeInterval) const
{
    ClockCoutIndent indent;
    CLOCK_COUT << "-> computeTicksForInterval(" << timeInterval.raw() << ")\n";
    CLOCK_COUT << "   : " << "driftFactor = " << driftFactor << ", "
                          << "nextTickFromOrigin = " << nextTickFromOrigin << ", "
                          << "nominalTickLength = " << nominalTickLength << std::endl;
    if (timeInterval == 0)
        return 0;
    int64_t result;
    if (!computeAsSeparateTicks)
        result = floorl((timeInterval - nextTickFromOrigin).raw() * driftFactor / nominalTickLength.raw()) + 1;
    else {
        result = 1;
        simtime_t t = timeInterval - nextTickFromOrigin;
        simtime_t currentTickLength = getCurrentTickLength();
        if (t < 0) {
            int64_t count = ceil(-t / currentTickLength);
            result -= count;
            t += count * currentTickLength;
        }
        if (t >= currentTickLength) {
            int64_t count = floor(t / currentTickLength);
            result += count;
            t -= count * currentTickLength;
        }
    }
    if (result < 0)
        result = 0;
    CLOCK_COUT << "   computeTicksForInterval(" << timeInterval.raw() << ") -> " << result << std::endl;
    return result;
}

simtime_t DriftingOscillatorBase::doComputeIntervalForTicks(int64_t numTicks) const
{
    ClockCoutIndent indent;
    CLOCK_COUT << "-> computeIntervalForTicks(" << numTicks << ")\n";
    CLOCK_COUT << "   : " << "driftFactor = " << driftFactor << ", "
                          << "nextTickFromOrigin = " << nextTickFromOrigin << ", "
                          << "nominalTickLength = " << nominalTickLength << std::endl;
    if (numTicks == 0)
        return 0;
    simtime_t result;
    if (!computeAsSeparateTicks)
        result = SimTime::fromRaw(ceill((nominalTickLength.raw() * (numTicks - 1)) / driftFactor)) + nextTickFromOrigin;
    else
        result = nextTickFromOrigin + (numTicks - 1) * SimTime::fromRaw(roundl(nominalTickLength.raw() / driftFactor));
    if (result < 0)
        result = 0;
    CLOCK_COUT << "   computeIntervalForTicks(" << numTicks << ") -> " << result.raw() << std::endl;
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

