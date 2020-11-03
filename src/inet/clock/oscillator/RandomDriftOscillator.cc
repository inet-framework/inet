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
    ConstantDriftOscillator::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        timer = new cMessage("ChangeTimer");
        scheduleAfter(par("changeInterval"), timer);
    }
}

void RandomDriftOscillator::handleMessage(cMessage *message)
{
    if (message == timer) {
        double driftRateChange = par("driftRateChange").doubleValue() / 1E+6;
        setDriftRate(driftRate + driftRateChange);
        scheduleAfter(par("changeInterval"), timer);
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet

