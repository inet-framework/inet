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
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <omnetpp.h>
#include "INETDefs.h"

/**
 * A module that just deletes every packet it receives, and collects
 * basic statistics (packet count, bit count, packet rate, bit rate).
 */
class INET_API Sink : public cSimpleModule
{
  protected:
    int numPackets;
    long numBits;
    double throughput; // bit/sec
    double packetPerSec;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

Define_Module(Sink);

void Sink::initialize()
{
    numPackets = 0;
    numBits = 0;
    throughput = 0;
    packetPerSec = 0;

    WATCH(numPackets);
    WATCH(numBits);
    WATCH(throughput);
    WATCH(packetPerSec);
}

void Sink::handleMessage(cMessage *msg)
{
    numPackets++;
    numBits += msg->length();

    throughput = numBits / simTime();
    packetPerSec = numPackets / simTime();

    delete msg;
}

void Sink::finish()
{
    recordScalar("numPackets", numPackets);
    recordScalar("numBits", numBits);
    recordScalar("throughput", throughput);
    recordScalar("packetPerSec", packetPerSec);
}


