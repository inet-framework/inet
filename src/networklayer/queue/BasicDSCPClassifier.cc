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


#include "INETDefs.h"

#include "BasicDSCPClassifier.h"

#ifdef WITH_IPv4
#include "IPv4Datagram.h"
#endif

#ifdef WITH_IPv6
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
#ifdef WITH_IPv4
    if (dynamic_cast<IPv4Datagram *>(msg))
    {
        IPv4Datagram *datagram = (IPv4Datagram *)msg;
        int dscp = datagram->getTypeOfService() & 0x3f; // DSCP is the six least significant bits of ToS
        return classifyByDSCP(dscp);
    }
    else
#endif
#ifdef WITH_IPv6
    if (dynamic_cast<IPv6Datagram *>(msg))
    {
        IPv6Datagram *datagram = (IPv6Datagram *)msg;
        int dscp = datagram->getTrafficClass() & 0x3f; // DSCP is the six least significant bits of Traffic Class
        return classifyByDSCP(dscp);
    }
    else
#endif
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
    int upper3bits = (dscp & 0x38) >> 3;
    //int lower3bits = (dscp & 0x07);  -- we'll ignore this

    // rfc 2474, section 4.2.2: at least two independently forwarded classes of traffic have to be created
    if (upper3bits & 0x04)
        return 0; // highest priority
    else
        return 1; // low priority (best effort)
}

