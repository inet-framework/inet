//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#endif // ifdef WITH_IPv6

#include "inet/networklayer/diffserv/DSCP_m.h"
#include "inet/networklayer/diffserv/DSCPMarker.h"

#include "inet/networklayer/diffserv/DiffservUtil.h"

namespace inet {

using namespace DiffservUtil;

Define_Module(DSCPMarker);

simsignal_t DSCPMarker::markPkSignal = registerSignal("markPk");

void DSCPMarker::initialize()
{
    parseDSCPs(par("dscps"), "dscps", dscps);
    if (dscps.empty())
        dscps.push_back(DSCP_BE);
    while ((int)dscps.size() < gateSize("in"))
        dscps.push_back(dscps.back());

    numRcvd = 0;
    numMarked = 0;
    WATCH(numRcvd);
    WATCH(numMarked);
}

void DSCPMarker::handleMessage(cMessage *msg)
{
    cPacket *packet = dynamic_cast<cPacket *>(msg);
    if (packet) {
        numRcvd++;
        int dscp = dscps.at(msg->getArrivalGate()->getIndex());
        if (markPacket(packet, dscp)) {
            emit(markPkSignal, packet);
            numMarked++;
        }

        send(packet, "out");
    }
    else
        throw cRuntimeError("DSCPMarker expects cPackets");

    if (ev.isGUI()) {
        char buf[50] = "";
        if (numRcvd > 0)
            sprintf(buf + strlen(buf), "rcvd: %d ", numRcvd);
        if (numMarked > 0)
            sprintf(buf + strlen(buf), "mark:%d ", numMarked);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

bool DSCPMarker::markPacket(cPacket *packet, int dscp)
{
    EV_DETAIL << "Marking packet with dscp=" << dscpToString(dscp) << "\n";

    for ( ; packet; packet = packet->getEncapsulatedPacket()) {
#ifdef WITH_IPv4
        IPv4Datagram *ipv4Datagram = dynamic_cast<IPv4Datagram *>(packet);
        if (ipv4Datagram) {
            ipv4Datagram->setDiffServCodePoint(dscp);
            return true;
        }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        IPv6Datagram *ipv6Datagram = dynamic_cast<IPv6Datagram *>(packet);
        if (ipv6Datagram) {
            ipv6Datagram->setDiffServCodePoint(dscp);
            return true;
        }
#endif // ifdef WITH_IPv6
    }

    return false;
}

} // namespace inet

