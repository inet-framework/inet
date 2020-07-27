//
// Copyright (C) 2016 OpenSim Ltd.
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

#include "inet/common/misc/SignalSource.h"

namespace inet {

Define_Module(SignalSource);

void SignalSource::initialize()
{
    startTime = par("startTime");
    endTime = par("endTime");
    signal = registerSignal(par("signalName"));
    scheduleAt(startTime, new cMessage("timer"));
}

void SignalSource::handleMessage(cMessage *msg)
{
    if (endTime < 0 || simTime() < endTime) {
        double value = par("value");
        emit(signal, value);
        scheduleAfter(par("interval"), msg);
    }
    else {
        delete msg;
    }
}

void SignalSource::finish()
{
}

} // namespace inet

