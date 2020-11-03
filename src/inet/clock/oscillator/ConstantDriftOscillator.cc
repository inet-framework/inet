//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/clock/oscillator/ConstantDriftOscillator.h"

namespace inet {

Define_Module(ConstantDriftOscillator);

void ConstantDriftOscillator::initialize(int stage)
{
    OscillatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        nominalTickLength = par("nominalTickLength");
        if (nominalTickLength == 0)
            nominalTickLength.setRaw(1);
        driftRate = par("driftRate").doubleValue() / 1E+6;
        relativeSpeed = 1.0 + driftRate;
        origin = simTime();
        simtime_t currentTickLength = getCurrentTickLength();
        simtime_t tickOffset = par("tickOffset");
        if (tickOffset < 0 || tickOffset >= currentTickLength)
            throw cRuntimeError("First tick offset must be in the range [0, currentTickLength)");
        if (tickOffset == 0)
            nextTickFromOrigin = 0;
        else
            nextTickFromOrigin = currentTickLength - tickOffset;
        WATCH(nominalTickLength);
        WATCH(driftRate);
        WATCH(nextTickFromOrigin);
    }
}

void ConstantDriftOscillator::setDriftRate(double newDriftRate)
{
    Enter_Method("setDriftRate");
    if (newDriftRate != driftRate) {
        emit(preOscillatorStateChangedSignal, this);
        simtime_t currentSimTime = simTime();
        EV_DEBUG << "Setting oscillator drift rate from " << driftRate << " to " << newDriftRate << " at simtime " << currentSimTime << ".\n";
        simtime_t currentTickLength = getCurrentTickLength();
        simtime_t baseTickTime = origin + nextTickFromOrigin - currentTickLength;
        simtime_t elapsedTickTime = fmod(currentSimTime - baseTickTime, currentTickLength);
        if (elapsedTickTime == SIMTIME_ZERO)
            nextTickFromOrigin = 0;
        else
            nextTickFromOrigin = (currentTickLength - elapsedTickTime) * (1.0 + driftRate) / (1.0 + newDriftRate);
        driftRate = newDriftRate;
        relativeSpeed = 1.0 + driftRate;
        origin = currentSimTime;
        emit(driftRateChangedSignal, driftRate);
        emit(postOscillatorStateChangedSignal, this);
        updateDisplayString();
    }
}

void ConstantDriftOscillator::setTickOffset(simtime_t newTickOffset)
{
    Enter_Method("setTickOffset");
    simtime_t currentSimTime = simTime();
    simtime_t currentTickLength = getCurrentTickLength();
    simtime_t baseTickTime = origin + nextTickFromOrigin - currentTickLength;
    simtime_t oldTickOffset = fmod(currentSimTime - baseTickTime, currentTickLength);
    if (newTickOffset != oldTickOffset) {
        emit(preOscillatorStateChangedSignal, this);
        EV_DEBUG << "Setting oscillator tick offset from " << oldTickOffset << " to " << newTickOffset << " at simtime " << currentSimTime << ".\n";
        origin = currentSimTime;
        if (newTickOffset == 0)
            nextTickFromOrigin = 0;
        else
            nextTickFromOrigin = currentTickLength - newTickOffset;
        emit(postOscillatorStateChangedSignal, this);
        updateDisplayString();
    }
}

int64_t ConstantDriftOscillator::computeTicksForInterval(simtime_t timeInterval) const
{
    ASSERT(timeInterval >= 0);
    return (int64_t)floor(((timeInterval + nextTickFromOrigin) / nominalTickLength) * relativeSpeed);
}

simtime_t ConstantDriftOscillator::computeIntervalForTicks(int64_t numTicks) const
{
    if (numTicks == 0)
        return 0;
    else if (nextTickFromOrigin == 0)
        return nominalTickLength * numTicks / relativeSpeed;
    else
        return nominalTickLength * (numTicks - 1) / relativeSpeed + nextTickFromOrigin;
}

void ConstantDriftOscillator::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-oscillator")) {
        if (const char *driftRateStr = node.getAttribute("drift-rate")) {
            double newDriftRate = strtod(driftRateStr, nullptr) / 1E+6;
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

const char *ConstantDriftOscillator::resolveDirective(char directive) const
{
    static std::string result;
    switch (directive) {
        case 'c':
            result = getCurrentTickLength().str() + " s";
            break;
        case 'd':
            result = std::to_string((int64_t)(driftRate * 1E+6)) + " ppm";
            break;
        default:
            return OscillatorBase::resolveDirective(directive);
    }
    return result.c_str();
}

} // namespace inet

