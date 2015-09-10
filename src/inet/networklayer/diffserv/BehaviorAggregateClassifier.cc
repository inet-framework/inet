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

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#endif // ifdef WITH_IPv6

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UDPPacket.h"
#endif // ifdef WITH_UDP

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#endif // ifdef WITH_TCP_COMMON

#include "inet/networklayer/diffserv/BehaviorAggregateClassifier.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"

namespace inet {

using namespace DiffservUtil;

Define_Module(BehaviorAggregateClassifier);

simsignal_t BehaviorAggregateClassifier::pkClassSignal = registerSignal("pkClass");

void BehaviorAggregateClassifier::initialize()
{
    numOutGates = gateSize("outs");
    std::vector<int> dscps;
    parseDSCPs(par("dscps"), "dscps", dscps);
    int numDscps = (int)dscps.size();
    if (numDscps > numOutGates)
        throw cRuntimeError("%s dscp values are given, but the module has only %d out gates",
                numDscps, numOutGates);
    for (int i = 0; i < numDscps; ++i)
        dscpToGateIndexMap[dscps[i]] = i;

    numRcvd = 0;
    WATCH(numRcvd);
}

void BehaviorAggregateClassifier::handleMessage(cMessage *msg)
{
    cPacket *packet = check_and_cast<cPacket *>(msg);
    numRcvd++;
    int clazz = classifyPacket(packet);
    emit(pkClassSignal, clazz);

    if (clazz >= 0)
        send(packet, "outs", clazz);
    else
        send(packet, "defaultOut");

    if (hasGUI()) {
        char buf[20] = "";
        if (numRcvd > 0)
            sprintf(buf + strlen(buf), "rcvd:%d ", numRcvd);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

int BehaviorAggregateClassifier::classifyPacket(cPacket *packet)
{
    int dscp = getDscpFromPacket(packet);
    if (dscp >= 0) {
        auto it = dscpToGateIndexMap.find(dscp);
        if (it != dscpToGateIndexMap.end())
            return it->second;
    }
    return -1;
}

int BehaviorAggregateClassifier::getDscpFromPacket(cPacket *packet)
{
    for ( ; packet; packet = packet->getEncapsulatedPacket()) {
#ifdef WITH_IPv4
        IPv4Datagram *ipv4Datagram = dynamic_cast<IPv4Datagram *>(packet);
        if (ipv4Datagram)
            return ipv4Datagram->getDiffServCodePoint();
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        IPv6Datagram *ipv6Datagram = dynamic_cast<IPv6Datagram *>(packet);
        if (ipv6Datagram)
            return ipv6Datagram->getDiffServCodePoint();
#endif // ifdef WITH_IPv6
    }
    return -1;
}

} // namespace inet

