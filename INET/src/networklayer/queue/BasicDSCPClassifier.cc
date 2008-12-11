//
// Copyright (C) 2005 Andras Varga
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


#include <omnetpp.h>
#include "BasicDSCPClassifier.h"
#include "IPDatagram.h"
#ifndef WITHOUT_IPv6
#include "IPv6Datagram.h"
#endif

Register_Class(BasicDSCPClassifier);

#define BEST_EFFORT 1

int BasicDSCPClassifier::getNumQueues()
{
    return 2;
}

int BasicDSCPClassifier::classifyPacket(cMessage *msg)
{
    if (dynamic_cast<IPDatagram *>(msg))
    {
        // IPv4 QoS: map DSCP to queue number
        IPDatagram *datagram = (IPDatagram *)msg;
        int dscp = datagram->getDiffServCodePoint();
        return classifyByDSCP(dscp);
    }
#ifndef WITHOUT_IPv6
    else if (dynamic_cast<IPv6Datagram *>(msg))
    {
        // IPv6 QoS: map Traffic Class to queue number
        IPv6Datagram *datagram = (IPv6Datagram *)msg;
        int dscp = datagram->getTrafficClass();
        return classifyByDSCP(dscp);
    }
#endif
    else
    {
        return BEST_EFFORT; // lowest priority ("best effort")
    }
}

int BasicDSCPClassifier::classifyByDSCP(int dscp)
{
    // DSCP is 6 bits, mask out all others
    dscp = (dscp & 0x3f);

    // DSCP format:
    //    xxxxx0: used by standards; see RFC 2474
    //    xxxxx1: experimental or local use
    // source: Stallings, High-Speed Networks, 2nd Ed, p494

    // all-zero DSCP maps to Best Effort (lowest priority)
    if (dscp==0)
        return BEST_EFFORT;

    // assume Best Effort service for experimental or local DSCP's too
    if (dscp & 1)
        return BEST_EFFORT;

    // from here on, we deal with non-zero standardized DSCP values only
    int upper3bits = (dscp & 0x3c) >> 3;
    //int lower3bits = (dscp & 0x07);  -- we'll ignore this

    // rfc 2474, section 4.2.2: at least two independently forwarded classes of traffic have to be created
    if (upper3bits & 0x04)
        return 0; // highest priority
    else
        return 1; // low priority (best effort)
}

