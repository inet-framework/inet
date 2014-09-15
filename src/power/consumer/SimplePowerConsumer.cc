//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "SimplePowerConsumer.h"

namespace inet {

namespace power {

Define_Module(SimplePowerConsumer);

SimplePowerConsumer::SimplePowerConsumer() :
    powerConsumerId(-1),
    powerSource(NULL),
    timer(NULL)
{
}

SimplePowerConsumer::~SimplePowerConsumer()
{
    cancelAndDelete(timer);
}

void SimplePowerConsumer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        timer = new cMessage("timer");
        updatePowerConsumption();
        scheduleIntervalTimer();
        powerSource = dynamic_cast<IPowerSource *>(getModuleByPath(par("powerSource")));
        if (powerSource)
            powerConsumerId = powerSource->addPowerConsumer(this);
    }
}

void SimplePowerConsumer::handleMessage(cMessage *message)
{
    if (message == timer) {
        updatePowerConsumption();
        scheduleIntervalTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void SimplePowerConsumer::updatePowerConsumption()
{
    powerConsumption = W(par("powerConsumption"));
    powerSource->setPowerConsumption(powerConsumerId, powerConsumption);
}

void SimplePowerConsumer::scheduleIntervalTimer()
{
    scheduleAt(simTime() + par("interval"), timer);
}

} // namespace power

} // namespace inet

