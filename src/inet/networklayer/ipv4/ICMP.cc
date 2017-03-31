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

#include <string.h>

#include "inet/networklayer/ipv4/ICMP.h"

#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IPv4Header.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

Define_Module(ICMP);

void ICMP::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_NETWORK_LAYER) {
        registerProtocol(Protocol::icmpv4, gate("ipOut"));
        registerProtocol(Protocol::icmpv4, gate("transportOut"));
    }
}

void ICMP::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();

    // process arriving ICMP message
    if (arrivalGate->isName("ipIn")) {
        EV_INFO << "Received " << msg << " from network protocol.\n";
        processICMPMessage(check_and_cast<Packet *>(msg));
        return;
    }
    else
        throw cRuntimeError("Message %s(%s) arrived in unknown '%s' gate", msg->getName(), msg->getClassName(), msg->getArrivalGate()->getName());
}

void ICMP::sendErrorMessage(Packet *packet, int inputInterfaceId, ICMPType type, ICMPCode code)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    const auto& ipv4Header = packet->peekHeader<IPv4Header>();
    IPv4Address origSrcAddr = ipv4Header->getSrcAddress();
    IPv4Address origDestAddr = ipv4Header->getDestAddress();

    // don't send ICMP error messages in response to broadcast or multicast messages
    if (origDestAddr.isMulticast() || origDestAddr.isLimitedBroadcastAddress() || possiblyLocalBroadcast(origDestAddr, inputInterfaceId)) {
        EV_DETAIL << "won't send ICMP error messages for broadcast/multicast message " << ipv4Header << endl;
        delete packet;
        return;
    }

    // don't send ICMP error messages response to unspecified, broadcast or multicast addresses
    if ((inputInterfaceId != -1 && origSrcAddr.isUnspecified())
            || origSrcAddr.isMulticast()
            || origSrcAddr.isLimitedBroadcastAddress()
            || possiblyLocalBroadcast(origSrcAddr, inputInterfaceId)) {
        EV_DETAIL << "won't send ICMP error messages to broadcast/multicast address, message " << ipv4Header << endl;
        delete packet;
        return;
    }

    // do not reply with error message to error message
    if (ipv4Header->getTransportProtocol() == IP_PROT_ICMP) {
        const auto& recICMPMsg = packet->peekDataAt<ICMPHeader>(byte(ipv4Header->getHeaderLength()));
        if (!isIcmpInfoType(recICMPMsg->getType())) {
            EV_DETAIL << "ICMP error received -- do not reply to it" << endl;
            delete packet;
            return;
        }
    }

    // assemble a message name
    char msgname[80];
    static long ctr;
    sprintf(msgname, "ICMP-error-#%ld-type%d-code%d", ++ctr, type, code);

    // debugging information
    EV_DETAIL << "sending ICMP error " << msgname << endl;

    // create and send ICMP packet
    Packet *errorPacket = new Packet(msgname);
    const auto& icmpHeader = std::make_shared<ICMPHeader>();
    icmpHeader->setChunkLength(byte(8));      //FIXME second 4 byte in icmp header not represented yet
    icmpHeader->setType(type);
    icmpHeader->setCode(code);
    icmpHeader->markImmutable();
    errorPacket->append(icmpHeader);
    // ICMP message length: the internet header plus the first 8 bytes of
    // the original datagram's data is returned to the sender.
    errorPacket->append(packet->peekDataAt(byte(0), byte(ipv4Header->getHeaderLength() + 8)));
    errorPacket->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv4);

    // if srcAddr is not filled in, we're still in the src node, so we just
    // process the ICMP message locally, right away
    if (origSrcAddr.isUnspecified()) {
        // pretend it came from the IPv4 layer
        errorPacket->ensureTag<L3AddressInd>()->setDestAddress(IPv4Address::LOOPBACK_ADDRESS);    // FIXME maybe use configured loopback address

        // then process it locally
        processICMPMessage(errorPacket);
    }
    else {
        sendToIP(errorPacket, ipv4Header->getSrcAddress());
    }
    delete packet;
}

bool ICMP::possiblyLocalBroadcast(const IPv4Address& addr, int interfaceId)
{
    if ((addr.getInt() & 1) == 0)
        return false;

    IIPv4RoutingTable *rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
    if (rt->isLocalBroadcastAddress(addr))
        return true;

    // if the input interface is unconfigured, we won't recognize network-directed broadcasts because we don't what network we are on
    IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (interfaceId != -1) {
        InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
        bool interfaceUnconfigured = (ie->ipv4Data() == nullptr) || ie->ipv4Data()->getIPAddress().isUnspecified();
        return interfaceUnconfigured;
    }
    else {
        // if all interfaces are configured, we are OK
        bool allInterfacesConfigured = true;
        for (int i = 0; i < (int)ift->getNumInterfaces(); i++)
            if ((ift->getInterface(i)->ipv4Data() == nullptr) || ift->getInterface(i)->ipv4Data()->getIPAddress().isUnspecified())
                allInterfacesConfigured = false;

        return !allInterfacesConfigured;
    }
}

void ICMP::processICMPMessage(Packet *packet)
{
    const auto& icmpmsg = packet->peekHeader<ICMPHeader>();
    switch (icmpmsg->getType()) {
        case ICMP_REDIRECT:
            // TODO implement redirect handling
            delete packet;
            break;

        case ICMP_DESTINATION_UNREACHABLE:
        case ICMP_TIME_EXCEEDED:
        case ICMP_PARAMETER_PROBLEM: {
            // ICMP errors are delivered to the appropriate higher layer protocol
            const auto& bogusL3Packet = packet->peekDataAt<IPv4Header>(icmpmsg->getChunkLength());
            int transportProtocol = bogusL3Packet->getTransportProtocol();
            if (transportProtocol == IP_PROT_ICMP) {
                // received ICMP error answer to an ICMP packet:
                //FIXME should send up dest unreachable answers to pingapps
                errorOut(packet);
            }
            else {
                if (transportProtocols.find(transportProtocol) == transportProtocols.end()) {
                    EV_ERROR << "Transport protocol " << transportProtocol << " not registered, packet dropped\n";
                    delete packet;
                }
                else {
                    packet->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ipprotocol.getProtocol(transportProtocol));
                    packet->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv4);
                    send(packet, "transportOut");
                }
            }
            break;
        }

        case ICMP_ECHO_REQUEST:
            processEchoRequest(packet);
            break;

        case ICMP_ECHO_REPLY:
            delete packet;
            break;

        case ICMP_TIMESTAMP_REQUEST:
            processEchoRequest(packet);
            break;

        case ICMP_TIMESTAMP_REPLY:
            delete packet;
            break;

        default:
            throw cRuntimeError("Unknown ICMP type %d", icmpmsg->getType());
    }
}

void ICMP::errorOut(Packet *packet)
{
    delete packet;
}

void ICMP::processEchoRequest(Packet *request)
{
    // turn request into a reply
    const auto& icmpReq = request->popHeader<ICMPHeader>();
    Packet *reply = new Packet((std::string(request->getName()) + "-reply").c_str());
    const auto& icmpReply = std::make_shared<ICMPHeader>();
    auto addressInd = request->getMandatoryTag<L3AddressInd>();
    IPv4Address src = addressInd->getSrcAddress().toIPv4();
    IPv4Address dest = addressInd->getDestAddress().toIPv4();
    icmpReply->setType(ICMP_ECHO_REPLY);
    icmpReply->markImmutable();
    reply->append(icmpReply);
    reply->append(request->peekDataAt(byte(0), request->getDataLength()));

    // swap src and dest
    // TBD check what to do if dest was multicast etc?
    // A. Ariza Modification 5/1/2011 clean the interface id, this forces the use of routing table in the IPv4 layer
    auto addressReq = reply->ensureTag<L3AddressReq>();
    addressReq->setSrcAddress(addressInd->getDestAddress().toIPv4());
    addressReq->setDestAddress(addressInd->getSrcAddress().toIPv4());

    sendToIP(reply);
    delete request;
}

void ICMP::sendToIP(Packet *msg, const IPv4Address& dest)
{
    msg->ensureTag<L3AddressReq>()->setDestAddress(dest);
    sendToIP(msg);
}

void ICMP::sendToIP(Packet *msg)
{
    // assumes IPv4ControlInfo is already attached
    EV_INFO << "Sending " << msg << " to lower layer.\n";
    msg->ensureTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    msg->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::icmpv4);
    send(msg, "ipOut");
}

void ICMP::handleRegisterProtocol(const Protocol& protocol, cGate *gate)
{
    Enter_Method("handleRegisterProtocol");
    if (!strcmp("transportIn", gate->getBaseName())) {
        transportProtocols.insert(ProtocolGroup::ipprotocol.getProtocolNumber(&protocol));
    }
}

} // namespace inet

