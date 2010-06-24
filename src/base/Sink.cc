//
// Copyright (C) 1992-2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
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

#include "Sink.h"


Define_Module(Sink);


void Sink::initialize()
{
    numPackets = 0;
    numBits = 0;
    throughput = 0;
    packetPerSec = 0;
    receivedBitsSignal = registerSignal("receivedBits");

    WATCH(numPackets);
    WATCH(numBits);
    WATCH(throughput);
    WATCH(packetPerSec);
}

void Sink::handleMessage(cMessage *msg)
{
    numPackets++;
    long receivedBits = PK(msg)->getBitLength();
    numBits += receivedBits;
    emit(receivedBitsSignal, receivedBits);
    throughput = numBits / simTime();
    packetPerSec = numPackets / simTime();

    delete msg;
}

void Sink::finish()
{
    recordScalar("throughput", throughput);
    recordScalar("packetPerSec", packetPerSec);
}


