//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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

//  Cleanup and rewrite: Andras Varga, 2004

#include "inet/networklayer/ipv4/ErrorHandling.h"

#include "inet/networklayer/ipv4/ICMPMessage.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

namespace inet {

Define_Module(ErrorHandling);

void ErrorHandling::initialize()
{
    numReceived = 0;
    numHostUnreachable = 0;
    numTimeExceeded = 0;

    WATCH(numReceived);
    WATCH(numHostUnreachable);
    WATCH(numTimeExceeded);
}

void ErrorHandling::handleMessage(cMessage *msg)
{
    numReceived++;

    ICMPMessage *icmpMsg = check_and_cast<ICMPMessage *>(msg);
    // Note: we must NOT use decapsulate() because payload in ICMP is conceptually truncated
    IPv4Datagram *d = check_and_cast<IPv4Datagram *>(icmpMsg->getEncapsulatedPacket());

    EV_WARN << "Error Handler: ICMP message received:\n";
    EV_WARN << " Type: " << (int)icmpMsg->getType()
            << " Code: " << (int)icmpMsg->getCode()
            << " Bytelength: " << d->getByteLength()
            << " Src: " << d->getSrcAddress()
            << " Dest: " << d->getDestAddress()
            << " Time: " << simTime()
            << "\n";

    switch (icmpMsg->getType()) {
        case ICMP_DESTINATION_UNREACHABLE:
            numHostUnreachable++;
            break;

        case ICMP_TIME_EXCEEDED:
            numTimeExceeded++;
            break;
    }

    delete icmpMsg;

    if (ev.isGUI()) {
        char buf[80];
        sprintf(buf, "errors: %ld", numReceived);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

} // namespace inet

