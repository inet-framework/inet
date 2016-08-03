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
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
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
        processICMPMessage(check_and_cast<ICMPMessage *>(msg));
        return;
    }
    else
        throw cRuntimeError("Message %s(%s) arrived in unknown '%s' gate", msg->getName(), msg->getClassName(), msg->getArrivalGate()->getName());
}

void ICMP::sendErrorMessage(IPv4Datagram *origDatagram, int inputInterfaceId, ICMPType type, ICMPCode code)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    // get ownership
    take(origDatagram);

    IPv4Address origSrcAddr = origDatagram->getSrcAddress();
    IPv4Address origDestAddr = origDatagram->getDestAddress();

    // don't send ICMP error messages in response to broadcast or multicast messages
    if (origDestAddr.isMulticast() || origDestAddr.isLimitedBroadcastAddress() || possiblyLocalBroadcast(origDestAddr, inputInterfaceId)) {
        EV_DETAIL << "won't send ICMP error messages for broadcast/multicast message " << origDatagram << endl;
        delete origDatagram;
        return;
    }

    // don't send ICMP error messages response to unspecified, broadcast or multicast addresses
    if (origSrcAddr.isMulticast() || origSrcAddr.isLimitedBroadcastAddress() || possiblyLocalBroadcast(origSrcAddr, inputInterfaceId)) {
        EV_DETAIL << "won't send ICMP error messages to broadcast/multicast address, message " << origDatagram << endl;
        delete origDatagram;
        return;
    }

    // do not reply with error message to error message
    if (origDatagram->getTransportProtocol() == IP_PROT_ICMP) {
        ICMPMessage *recICMPMsg = check_and_cast<ICMPMessage *>(origDatagram->getEncapsulatedPacket());
        if (!isIcmpInfoType(recICMPMsg->getType())) {
            EV_DETAIL << "ICMP error received -- do not reply to it" << endl;
            delete origDatagram;
            return;
        }
    }

    // assemble a message name
    char msgname[32];
    static long ctr;
    sprintf(msgname, "ICMP-error-#%ld-type%d-code%d", ++ctr, type, code);

    // debugging information
    EV_DETAIL << "sending ICMP error " << msgname << endl;

    // create and send ICMP packet
    ICMPMessage *errorMessage = new ICMPMessage(msgname);
    errorMessage->setType(type);
    errorMessage->setCode(code);
    errorMessage->encapsulate(origDatagram);

    // ICMP message length: the internet header plus the first 8 bytes of
    // the original datagram's data is returned to the sender.
    //
    // NOTE: since we just overwrite the errorMessage length without actually
    // truncating origDatagram, one can get "packet length became negative"
    // error when decapsulating the origDatagram on the receiver side.
    // A workaround is to avoid decapsulation, or to manually set the
    // errorMessage length to be larger than the encapsulated message.
    int dataLength = origDatagram->getByteLength() - origDatagram->getHeaderLength();
    int truncatedDataLength = dataLength <= 8 ? dataLength : 8;
    errorMessage->setByteLength(8 + origDatagram->getHeaderLength() + truncatedDataLength);

    // if srcAddr is not filled in, we're still in the src node, so we just
    // process the ICMP message locally, right away
    if (origSrcAddr.isUnspecified()) {
        // pretend it came from the IPv4 layer
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
        errorMessage->setControlInfo(controlInfo);
        errorMessage->ensureTag<L3AddressInd>()->setDestination(IPv4Address::LOOPBACK_ADDRESS);    // FIXME maybe use configured loopback address

        // then process it locally
        processICMPMessage(errorMessage);
    }
    else {
        sendToIP(errorMessage, origDatagram->getSrcAddress());
    }
}

void ICMP::sendErrorMessage(cPacket *transportPacket, IPv4ControlInfo *ctrl, ICMPType type, ICMPCode code)
{
    Enter_Method("sendErrorMessage(transportPacket, ctrl, type=%d, code=%d)", type, code);

    IPv4Datagram *datagram = ctrl->removeOrigDatagram();
    int inputInterfaceId = transportPacket->getMandatoryTag<InterfaceInd>()->getInterfaceId();
    delete ctrl;
    take(transportPacket);
    take(datagram);
    datagram->encapsulate(transportPacket);
    sendErrorMessage(datagram, inputInterfaceId, type, code);
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

void ICMP::processICMPMessage(ICMPMessage *icmpmsg)
{
    switch (icmpmsg->getType()) {
        case ICMP_REDIRECT:
            // TODO implement redirect handling
            delete icmpmsg;
            break;

        case ICMP_DESTINATION_UNREACHABLE:
        case ICMP_TIME_EXCEEDED:
        case ICMP_PARAMETER_PROBLEM: {
            // ICMP errors are delivered to the appropriate higher layer protocol
            IPv4Datagram *bogusL3Packet = check_and_cast<IPv4Datagram *>(icmpmsg->getEncapsulatedPacket());
            int transportProtocol = bogusL3Packet->getTransportProtocol();
            if (transportProtocol == IP_PROT_ICMP) {
                // received ICMP error answer to an ICMP packet:
                errorOut(icmpmsg);
            }
            else {
                if (transportProtocols.find(transportProtocol) == transportProtocols.end()) {
                    EV_ERROR << "Transport protocol " << transportProtocol << " not registered, packet dropped\n";
                    delete icmpmsg;
                }
                else {
                    icmpmsg->ensureTag<DispatchProtocolReq>()->setProtocol(ProtocolGroup::ipprotocol.getProtocol(transportProtocol));
                    send(icmpmsg, "transportOut");
                }
            }
            break;
        }

        case ICMP_ECHO_REQUEST:
            processEchoRequest(icmpmsg);
            break;

        case ICMP_ECHO_REPLY:
            delete icmpmsg;
            break;

        case ICMP_TIMESTAMP_REQUEST:
            processEchoRequest(icmpmsg);
            break;

        case ICMP_TIMESTAMP_REPLY:
            delete icmpmsg;
            break;

        default:
            throw cRuntimeError("Unknown ICMP type %d", icmpmsg->getType());
    }
}

void ICMP::errorOut(ICMPMessage *icmpmsg)
{
    delete icmpmsg;
}

void ICMP::processEchoRequest(ICMPMessage *request)
{
    // turn request into a reply
    ICMPMessage *reply = request;
    auto addressInd = request->getMandatoryTag<L3AddressInd>();
    IPv4Address src = addressInd->getSource().toIPv4();
    IPv4Address dest = addressInd->getDestination().toIPv4();
    reply->clearTags();
    reply->setName((std::string(request->getName()) + "-reply").c_str());
    reply->setType(ICMP_ECHO_REPLY);

    // swap src and dest
    // TBD check what to do if dest was multicast etc?
    // A. Ariza Modification 5/1/2011 clean the interface id, this forces the use of routing table in the IPv4 layer
    auto addressReq = request->ensureTag<L3AddressReq>();
    addressReq->setSource(dest);
    addressReq->setDestination(src);

    sendToIP(reply);
}

void ICMP::sendToIP(ICMPMessage *msg, const IPv4Address& dest)
{
    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    msg->setControlInfo(controlInfo);
    msg->ensureTag<L3AddressReq>()->setDestination(dest);
    sendToIP(msg);
}

void ICMP::sendToIP(ICMPMessage *msg)
{
    // assumes IPv4ControlInfo is already attached
    EV_INFO << "Sending " << msg << " to lower layer.\n";
    msg->ensureTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    msg->ensureTag<ProtocolTag>()->setProtocol(&Protocol::icmpv4);
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

