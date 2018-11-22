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

#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "QosClassifier.h"
#include "UserPriority.h"

#ifdef WITH_IPv4
#  include "inet/networklayer/ipv4/Ipv4Header_m.h"
#  include "inet/networklayer/ipv4/IcmpHeader_m.h"
#endif
#ifdef WITH_IPv6
#  include "inet/networklayer/ipv6/Ipv6Header.h"
#  include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#endif
#ifdef WITH_UDP
#  include "inet/transportlayer/udp/UdpHeader_m.h"
#endif
#ifdef WITH_TCP_COMMON
#  include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif

namespace inet {

Define_Module(QosClassifier);

void QosClassifier::initialize()
{
    //TODO parameters
        const char *defaultAcString = par("defaultAc");
        if(strcmp(defaultAcString,"BK") == 0)
            defaultAc = UP_BK;
        else if(strcmp(defaultAcString,"BE") == 0)
            defaultAc = UP_BE;
        else if(strcmp(defaultAcString,"VI") == 0)
            defaultAc = UP_VI;
        else if(strcmp(defaultAcString,"VO") == 0)
            defaultAc = UP_VO;
        else
            throw cRuntimeError("Wrong access category for default ac");

        cStringTokenizer tokenizerUdp(par("udpPortAcMap"));
        while (tokenizerUdp.hasMoreTokens()) {
            const char *portString = tokenizerUdp.nextToken();
            const char *acString = tokenizerUdp.nextToken();
            int port = std::atoi(portString);
            int ac = -1;
            if(strcmp(acString,"BK") == 0)
                ac = UP_BK;
            else if(strcmp(acString,"BE") == 0)
                ac = UP_BE;
            else if(strcmp(acString,"VI") == 0)
                ac = UP_VI;
            else if(strcmp(acString,"VO") == 0)
                ac = UP_VO;
            else
                throw cRuntimeError("Wrong access category for udp");

            if (!portString || !acString)
                throw cRuntimeError("Insufficient number of values");
            udpPortMap.insert(std::pair<int, int>(port, ac));
        }

        cStringTokenizer tokenizerTcp(par("tcpPortAcMap"));
        while (tokenizerUdp.hasMoreTokens()) {
            const char *portString = tokenizerTcp.nextToken();
            const char *acString = tokenizerTcp.nextToken();
            int port = std::atoi(portString);
            int ac = -1;
            if(strcmp(acString,"BK") == 0)
                ac = UP_BK;
            else if(strcmp(acString,"BE") == 0)
                ac = UP_BE;
            else if(strcmp(acString,"VI") == 0)
                ac = UP_VI;
            else if(strcmp(acString,"VO") == 0)
                ac = UP_VO;
            else
                throw cRuntimeError("Wrong access category for tcp");

            if (!portString || !acString)
                throw cRuntimeError("Insufficient number of values");
            tcpPortMap.insert(std::pair<int, int>(port, ac));
        }
}

void QosClassifier::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(getUserPriority(msg));
    send(msg, "out");
}

int QosClassifier::getUserPriority(cMessage *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    int ipProtocol = -1;
    b ipHeaderLength = b(-1);

#ifdef WITH_IPv4
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv4) {
        const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
        if (ipv4Header->getProtocolId() == IP_PROT_ICMP)
            return UP_BE; // ICMP class
        ipProtocol = ipv4Header->getProtocolId();
        ipHeaderLength = ipv4Header->getChunkLength();
    }
#endif

#ifdef WITH_IPv6
    if (packet->getTag<PacketProtocolTag>()->getProtocol() == &Protocol::ipv6) {
        const auto& ipv6Header = packet->peekAtFront<Ipv6Header>();
        if (ipv6Header->getProtocolId() == IP_PROT_IPv6_ICMP)
            return UP_BE; // ICMPv6 class
        ipProtocol = ipv6Header->getProtocolId();
        ipHeaderLength = ipv6Header->getChunkLength();
    }
#endif

    if (ipProtocol == -1)
        return defaultAc;

#ifdef WITH_UDP
    if (ipProtocol == IP_PROT_UDP) {
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(ipHeaderLength);
        unsigned int srcPort = udpHeader->getSourcePort();
        unsigned int destPort = udpHeader->getDestinationPort();
        auto it = udpPortMap.find(destPort);
        if(it != udpPortMap.end())
            return it->second;
        it = udpPortMap.find(srcPort);
        if(it != udpPortMap.end())
            return it->second;
        // this is needed for fingerprints to remain the same:
        if (destPort == 6000 || srcPort == 6000) // not classified
                    return -1;
    }
#endif

#ifdef WITH_TCP_COMMON
    if (ipProtocol == IP_PROT_TCP) {
        const auto& tcpHeader = packet->peekDataAt<tcp::TcpHeader>(ipHeaderLength);
        unsigned int srcPort = tcpHeader->getSourcePort();
        unsigned int destPort = tcpHeader->getDestinationPort();
        auto it = tcpPortMap.find(destPort);
        if(it != tcpPortMap.end())
            return it->second;
        it = tcpPortMap.find(srcPort);
        if(it != tcpPortMap.end())
            return it->second;
        // this is needed for fingerprints to remain the same:
        if (destPort == 6000 || srcPort == 6000) // not classified
                    return -1;
    }
#endif

    return defaultAc;
}

void QosClassifier::handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (!strcmp("out", out->getName()))
        registerService(protocol, gate("in"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", out->getName());
}

void QosClassifier::handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("in", in->getName()))
        registerProtocol(protocol, gate("out"), servicePrimitive);
    else
        throw cRuntimeError("Unknown gate: %s", in->getName());
}

} // namespace inet

