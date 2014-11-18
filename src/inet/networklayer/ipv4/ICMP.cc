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

#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(ICMP);

void ICMP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_NETWORK_LAYER_2) {
        IPSocket socket(gate("sendOut"));
        socket.registerProtocol(IP_PROT_ICMP);
    }
}

void ICMP::handleMessage(cMessage *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();

    // process arriving ICMP message
    if (!strcmp(arrivalGate->getName(), "localIn")) {
        EV_INFO << "Received " << msg << " from network protocol.\n";
        processICMPMessage(check_and_cast<ICMPMessage *>(msg));
        return;
    }

    // request from application
    if (!strcmp(arrivalGate->getName(), "pingIn")) {
        EV_INFO << "Received " << msg << " from upper layer.\n";
        sendEchoRequest(check_and_cast<PingPayload *>(msg));
        return;
    }
}

void ICMP::sendErrorMessage(IPv4Datagram *origDatagram, int inputInterfaceId, ICMPType type, ICMPCode code)
{
    Enter_Method("sendErrorMessage(datagram, type=%d, code=%d)", type, code);

    // get ownership
    take(origDatagram);

    // don't send ICMP error messages in response to broadcast or multicast messages
    IPv4Address origDestAddr = origDatagram->getDestAddress();
    if (origDestAddr.isMulticast() || origDestAddr.isLimitedBroadcastAddress() || possiblyLocalBroadcast(origDestAddr, inputInterfaceId)) {
        EV_DETAIL << "won't send ICMP error messages for broadcast/multicast message " << origDatagram << endl;
        delete origDatagram;
        return;
    }

    // don't send ICMP error messages response to unspecified, broadcast or multicast addresses
    IPv4Address origSrcAddr = origDatagram->getSrcAddress();
    if (origSrcAddr.isUnspecified() || origSrcAddr.isMulticast() || origSrcAddr.isLimitedBroadcastAddress() || possiblyLocalBroadcast(origSrcAddr, inputInterfaceId)) {
        EV_DETAIL << "won't send ICMP error messages to broadcast/multicast address, message " << origDatagram << endl;
        delete origDatagram;
        return;
    }

    // do not reply with error message to error message
    if (origDatagram->getTransportProtocol() == IP_PROT_ICMP) {
        ICMPMessage *recICMPMsg = check_and_cast<ICMPMessage *>(origDatagram->getEncapsulatedPacket());
        if (recICMPMsg->getType() < 128) {
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
    if (origDatagram->getSrcAddress().isUnspecified()) {
        // pretend it came from the IPv4 layer
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
        controlInfo->setSrcAddr(IPv4Address::LOOPBACK_ADDRESS);    // FIXME maybe use configured loopback address
        controlInfo->setProtocol(IP_PROT_ICMP);
        errorMessage->setControlInfo(controlInfo);

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
    int inputInterfaceId = ctrl->getInterfaceId();
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
        bool interfaceUnconfigured = (ie->ipv4Data() == NULL) || ie->ipv4Data()->getIPAddress().isUnspecified();
        return interfaceUnconfigured;
    }
    else {
        // if all interfaces are configured, we are OK
        bool allInterfacesConfigured = true;
        for (int i = 0; i < (int)ift->getNumInterfaces(); i++)
            if ((ift->getInterface(i)->ipv4Data() == NULL) || ift->getInterface(i)->ipv4Data()->getIPAddress().isUnspecified())
                allInterfacesConfigured = false;

        return !allInterfacesConfigured;
    }
}

void ICMP::processICMPMessage(ICMPMessage *icmpmsg)
{
    switch (icmpmsg->getType()) {
        case ICMP_DESTINATION_UNREACHABLE:
            errorOut(icmpmsg);
            break;

        case ICMP_REDIRECT:
            errorOut(icmpmsg);
            break;

        case ICMP_TIME_EXCEEDED:
            errorOut(icmpmsg);
            break;

        case ICMP_PARAMETER_PROBLEM:
            errorOut(icmpmsg);
            break;

        case ICMP_ECHO_REQUEST:
            processEchoRequest(icmpmsg);
            break;

        case ICMP_ECHO_REPLY:
            processEchoReply(icmpmsg);
            break;

        case ICMP_TIMESTAMP_REQUEST:
            processEchoRequest(icmpmsg);
            break;

        case ICMP_TIMESTAMP_REPLY:
            processEchoReply(icmpmsg);
            break;

        default:
            throw cRuntimeError("Unknown ICMP type %d", icmpmsg->getType());
    }
}

void ICMP::errorOut(ICMPMessage *icmpmsg)
{
    send(icmpmsg, "errorOut");
}

void ICMP::processEchoRequest(ICMPMessage *request)
{
    // turn request into a reply
    ICMPMessage *reply = request;
    reply->setName((std::string(request->getName()) + "-reply").c_str());
    reply->setType(ICMP_ECHO_REPLY);

    // swap src and dest
    // TBD check what to do if dest was multicast etc?
    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo *>(reply->getControlInfo());
    IPv4Address src = ctrl->getSrcAddr();
    IPv4Address dest = ctrl->getDestAddr();
    // A. Ariza Modification 5/1/2011 clean the interface id, this forces the use of routing table in the IPv4 layer
    ctrl->setInterfaceId(-1);
    ctrl->setSrcAddr(dest);
    ctrl->setDestAddr(src);

    sendToIP(reply);
}

void ICMP::processEchoReply(ICMPMessage *reply)
{
    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo *>(reply->removeControlInfo());
    PingPayload *payload = check_and_cast<PingPayload *>(reply->decapsulate());
    payload->setControlInfo(ctrl);
    delete reply;
    long originatorId = payload->getOriginatorId();
    PingMap::iterator i = pingMap.find(originatorId);
    if (i != pingMap.end()) {
        EV_INFO << "Sending " << payload << " to upper layer.\n";
        send(payload, "pingOut", i->second);
    }
    else {
        EV_WARN << "Received ECHO REPLY has an unknown originator ID: " << originatorId << ", packet dropped." << endl;
        delete payload;
    }
}

void ICMP::sendEchoRequest(PingPayload *msg)
{
    cGate *arrivalGate = msg->getArrivalGate();
    int i = arrivalGate->getIndex();
    pingMap[msg->getOriginatorId()] = i;

    IPv4ControlInfo *ctrl = check_and_cast<IPv4ControlInfo *>(msg->removeControlInfo());
    ctrl->setProtocol(IP_PROT_ICMP);
    ICMPMessage *request = new ICMPMessage(msg->getName());
    request->setType(ICMP_ECHO_REQUEST);
    request->encapsulate(msg);
    request->setControlInfo(ctrl);
    sendToIP(request);
}

void ICMP::sendToIP(ICMPMessage *msg, const IPv4Address& dest)
{
    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setDestAddr(dest);
    controlInfo->setProtocol(IP_PROT_ICMP);
    msg->setControlInfo(controlInfo);
    sendToIP(msg);
}

void ICMP::sendToIP(ICMPMessage *msg)
{
    // assumes IPv4ControlInfo is already attached
    EV_INFO << "Sending " << msg << " to lower layer.\n";
    send(msg, "sendOut");
}

} // namespace inet

