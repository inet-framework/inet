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

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/QosClassifier.h"
#include "inet/linklayer/common/UserPriority.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"

#ifdef WITH_IPv4
#  include "inet/networklayer/ipv4/IcmpHeader_m.h"
#  include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif
#ifdef WITH_IPv6
#  include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#  include "inet/networklayer/ipv6/Ipv6Header.h"
#endif
#ifdef WITH_TCP_COMMON
#  include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif
#ifdef WITH_UDP
#  include "inet/transportlayer/udp/UdpHeader_m.h"
#endif

namespace inet {

Define_Module(QosClassifier);

void QosClassifier::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        defaultUp = parseUserPriority(par("defaultUp"));
        parseUserPriorityMap(par("ipProtocolUpMap"), ipProtocolUpMap);
        parseUserPriorityMap(par("udpPortUpMap"), udpPortUpMap);
        parseUserPriorityMap(par("tcpPortUpMap"), tcpPortUpMap);
    }
}

void QosClassifier::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(getUserPriority(msg));
    send(msg, "out");
}

int QosClassifier::parseUserPriority(const char *text)
{
    if (!strcmp(text, "BE"))
        return UP_BE;
    else if (!strcmp(text, "BK"))
        return UP_BK;
    else if (!strcmp(text, "BK2"))
        return UP_BK2;
    else if (!strcmp(text, "EE"))
        return UP_EE;
    else if (!strcmp(text, "CL"))
        return UP_CL;
    else if (!strcmp(text, "VI"))
        return UP_VI;
    else if (!strcmp(text, "VO"))
        return UP_VO;
    else if (!strcmp(text, "NC"))
        return UP_NC;
    else
        throw cRuntimeError("Unknown user priority: '%s'", text);
}

void QosClassifier::parseUserPriorityMap(const char *text, std::map<int, int>& upMap)
{
    cStringTokenizer tokenizer(text);
    while (tokenizer.hasMoreTokens()) {
        const char *keyString = tokenizer.nextToken();
        const char *upString = tokenizer.nextToken();
        if (!keyString || !upString)
            throw cRuntimeError("Insufficient number of values in: '%s'", text);
        int key = std::atoi(keyString);
        int up = parseUserPriority(upString);
        upMap.insert(std::pair<int, int>(key, up));
    }
}

int QosClassifier::getUserPriority(cMessage *msg)
{
    int ipProtocol = -1;

#if defined(WITH_IPv4) || defined(WITH_IPv6) || defined(WITH_UDP) || defined(WITH_TCP_COMMON)
    auto packet = check_and_cast<Packet *>(msg);
    b ipHeaderLength = b(-1);
#endif

#ifdef WITH_IPv4
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv4) {
        const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
        ipProtocol = ipv4Header->getProtocolId();
        ipHeaderLength = ipv4Header->getChunkLength();
    }
#endif

#ifdef WITH_IPv6
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv6) {
        const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
        ipProtocol = ipv6Header->getProtocolId();
        ipHeaderLength = ipv6Header->getChunkLength();
    }
#endif

    if (ipProtocol == -1)
        return defaultUp;
    auto it = ipProtocolUpMap.find(ipProtocol);
    if (it != ipProtocolUpMap.end())
        return it->second;

#ifdef WITH_UDP
    if (ipProtocol == IP_PROT_UDP) {
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(ipHeaderLength);
        unsigned int srcPort = udpHeader->getSourcePort();
        unsigned int destPort = udpHeader->getDestinationPort();
        auto it = udpPortUpMap.find(destPort);
        if (it != udpPortUpMap.end())
            return it->second;
        it = udpPortUpMap.find(srcPort);
        if (it != udpPortUpMap.end())
            return it->second;
    }
#endif

#ifdef WITH_TCP_COMMON
    if (ipProtocol == IP_PROT_TCP) {
        const auto& tcpHeader = packet->peekDataAt<tcp::TcpHeader>(ipHeaderLength);
        unsigned int srcPort = tcpHeader->getSourcePort();
        unsigned int destPort = tcpHeader->getDestinationPort();
        auto it = tcpPortUpMap.find(destPort);
        if (it != tcpPortUpMap.end())
            return it->second;
        it = tcpPortUpMap.find(srcPort);
        if (it != tcpPortUpMap.end())
            return it->second;
    }
#endif

    return defaultUp;
}

void QosClassifier::handleRegisterService(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (!strcmp("out", g->getName()))
        registerService(protocol, gate("in"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", g->getName());
}

void QosClassifier::handleRegisterProtocol(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("in", g->getName()))
        registerProtocol(protocol, gate("out"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", g->getName());
}

} // namespace inet

