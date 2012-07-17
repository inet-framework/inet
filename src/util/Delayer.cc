//
// Copyright (C) 2012 OpenSim Ltd
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
// @author: Zoltan Bojthe
//


#include "Delayer.h"

Define_Module(Delayer);

simsignal_t Delayer::rcvdPkSignal;
simsignal_t Delayer::sentPkSignal;
simsignal_t Delayer::delaySignal;

void Delayer::initialize()
{
    delayPar = &par("delay");
    //statistics
    rcvdPkSignal = registerSignal("rcvdPk");
    sentPkSignal = registerSignal("sentPk");
    delaySignal = registerSignal("delay");
}

void Delayer::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        emit(sentPkSignal, msg);
        send(msg, "out");
    }
    else
    {
        emit(rcvdPkSignal, msg);

        simtime_t delay = delayPar->doubleValue();
        emit(delaySignal, delay);
        scheduleAt(simTime() + delay, msg);
    }
}
