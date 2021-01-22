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

#include "inet/clock/oscillator/RandomDriftOscillator.h"

namespace inet {

Define_Module(RandomDriftOscillator);

void RandomDriftOscillator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        driftRateParameter = &par("driftRate");
        driftRateChangeParameter = &par("driftRateChange");
        driftRate = driftRateParameter->doubleValue() / 1E+6;
    }
    DriftingOscillatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        changeTimer = new cMessage("ChangeTimer");
        driftRateChangeLowerLimit = par("driftRateChangeLowerLimit").doubleValue() / 1E+6;
        driftRateChangeUpperLimit = par("driftRateChangeUpperLimit").doubleValue() / 1E+6;
        scheduleAfter(par("changeInterval"), changeTimer);
    }
}

void RandomDriftOscillator::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        driftRateChangeTotal += driftRateChangeParameter->doubleValue() / 1E+6;
        driftRateChangeTotal = std::max(driftRateChangeTotal, driftRateChangeLowerLimit);
        driftRateChangeTotal = std::min(driftRateChangeTotal, driftRateChangeUpperLimit);
        driftRate = driftRateParameter->doubleValue();
        setDriftRate(driftRate + driftRateChangeTotal);
        scheduleAfter(par("changeInterval"), changeTimer);
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet

