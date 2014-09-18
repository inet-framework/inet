//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"

#include "inet/networklayer/ipv4/ICMPMessage.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

MessageHandler::MessageHandler(Router *containingRouter, cSimpleModule *containingModule) :
    IMessageHandler(containingRouter),
    ospfModule(containingModule),
    helloHandler(containingRouter),
    ddHandler(containingRouter),
    lsRequestHandler(containingRouter),
    lsUpdateHandler(containingRouter),
    lsAckHandler(containingRouter)
{
}

void MessageHandler::messageReceived(cMessage *message)
{
    if (message->isSelfMessage()) {
        handleTimer(message);
    }
    else if (dynamic_cast<ICMPMessage *>(message)) {
        EV_ERROR << "ICMP error received -- discarding\n";
        delete message;
    }
    else {
        OSPFPacket *packet = check_and_cast<OSPFPacket *>(message);
        EV_INFO << "Received packet: (" << packet->getClassName() << ")" << packet->getName() << "\n";
        if (packet->getRouterID() == IPv4Address(router->getRouterID())) {
            EV_INFO << "This packet is from ourselves, discarding.\n";
            delete message;
        }
        else {
            processPacket(packet);
        }
    }
}

void MessageHandler::handleTimer(cMessage *timer)
{
    switch (timer->getKind()) {
        case INTERFACE_HELLO_TIMER: {
            Interface *intf;
            if (!(intf = reinterpret_cast<Interface *>(timer->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid InterfaceHelloTimer.\n";
                delete timer;
            }
            else {
                printEvent("Hello Timer expired", intf);
                intf->processEvent(Interface::HELLO_TIMER);
            }
        }
        break;

        case INTERFACE_WAIT_TIMER: {
            Interface *intf;
            if (!(intf = reinterpret_cast<Interface *>(timer->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid InterfaceWaitTimer.\n";
                delete timer;
            }
            else {
                printEvent("Wait Timer expired", intf);
                intf->processEvent(Interface::WAIT_TIMER);
            }
        }
        break;

        case INTERFACE_ACKNOWLEDGEMENT_TIMER: {
            Interface *intf;
            if (!(intf = reinterpret_cast<Interface *>(timer->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid InterfaceAcknowledgementTimer.\n";
                delete timer;
            }
            else {
                printEvent("Acknowledgement Timer expired", intf);
                intf->processEvent(Interface::ACKNOWLEDGEMENT_TIMER);
            }
        }
        break;

        case NEIGHBOR_INACTIVITY_TIMER: {
            Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Neighbor *>(timer->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborInactivityTimer.\n";
                delete timer;
            }
            else {
                printEvent("Inactivity Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Neighbor::INACTIVITY_TIMER);
            }
        }
        break;

        case NEIGHBOR_POLL_TIMER: {
            Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Neighbor *>(timer->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborInactivityTimer.\n";
                delete timer;
            }
            else {
                printEvent("Poll Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Neighbor::POLL_TIMER);
            }
        }
        break;

        case NEIGHBOR_DD_RETRANSMISSION_TIMER: {
            Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Neighbor *>(timer->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborDDRetransmissionTimer.\n";
                delete timer;
            }
            else {
                printEvent("Database Description Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Neighbor::DD_RETRANSMISSION_TIMER);
            }
        }
        break;

        case NEIGHBOR_UPDATE_RETRANSMISSION_TIMER: {
            Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Neighbor *>(timer->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborUpdateRetransmissionTimer.\n";
                delete timer;
            }
            else {
                printEvent("Update Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Neighbor::UPDATE_RETRANSMISSION_TIMER);
            }
        }
        break;

        case NEIGHBOR_REQUEST_RETRANSMISSION_TIMER: {
            Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<Neighbor *>(timer->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborRequestRetransmissionTimer.\n";
                delete timer;
            }
            else {
                printEvent("Request Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(Neighbor::REQUEST_RETRANSMISSION_TIMER);
            }
        }
        break;

        case DATABASE_AGE_TIMER: {
            printEvent("Ageing the database");
            router->ageDatabase();
        }
        break;

        default:
            break;
    }
}

void MessageHandler::processPacket(OSPFPacket *packet, Interface *unused1, Neighbor *unused2)
{
    // see RFC 2328 8.2

    // packet version must be OSPF version 2
    if (packet->getVersion() == 2) {
        IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo *>(packet->getControlInfo());
        int interfaceId = controlInfo->getInterfaceId();
        AreaID areaID = packet->getAreaID();
        Area *area = router->getAreaByID(areaID);

        if (area != NULL) {
            // packet Area ID must either match the Area ID of the receiving interface or...
            Interface *intf = area->getInterface(interfaceId);

            if (intf == NULL) {
                // it must be the backbone area and...
                if (areaID == BACKBONE_AREAID) {
                    if (router->getAreaCount() > 1) {
                        // it must be a virtual link and the source router's router ID must be the endpoint of this virtual link and...
                        intf = area->findVirtualLink(packet->getRouterID());

                        if (intf != NULL) {
                            Area *virtualLinkTransitArea = router->getAreaByID(intf->getTransitAreaID());

                            if (virtualLinkTransitArea != NULL) {
                                // the receiving interface must attach to the virtual link's configured transit area
                                Interface *virtualLinkInterface = virtualLinkTransitArea->getInterface(interfaceId);

                                if (virtualLinkInterface == NULL) {
                                    intf = NULL;
                                }
                            }
                            else {
                                intf = NULL;
                            }
                        }
                    }
                }
            }
            if (intf != NULL) {
                IPv4Address destinationAddress = controlInfo->getDestAddr();
                IPv4Address allDRouters = IPv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST;
                Interface::InterfaceStateType interfaceState = intf->getState();

                // if destination address is ALL_D_ROUTERS the receiving interface must be in DesignatedRouter or Backup state
                if (
                    ((destinationAddress == allDRouters) &&
                     (
                         (interfaceState == Interface::DESIGNATED_ROUTER_STATE) ||
                         (interfaceState == Interface::BACKUP_STATE)
                     )
                    ) ||
                    (destinationAddress != allDRouters)
                    )
                {
                    // packet authentication
                    if (authenticatePacket(packet)) {
                        OSPFPacketType packetType = static_cast<OSPFPacketType>(packet->getType());
                        Neighbor *neighbor = NULL;

                        // all packets except HelloPackets are sent only along adjacencies, so a Neighbor must exist
                        if (packetType != HELLO_PACKET) {
                            switch (intf->getType()) {
                                case Interface::BROADCAST:
                                case Interface::NBMA:
                                case Interface::POINTTOMULTIPOINT:
                                    neighbor = intf->getNeighborByAddress(controlInfo->getSrcAddr());
                                    break;

                                case Interface::POINTTOPOINT:
                                case Interface::VIRTUAL:
                                    neighbor = intf->getNeighborByID(packet->getRouterID());
                                    break;

                                default:
                                    break;
                            }
                        }
                        switch (packetType) {
                            case HELLO_PACKET:
                                helloHandler.processPacket(packet, intf);
                                break;

                            case DATABASE_DESCRIPTION_PACKET:
                                if (neighbor != NULL) {
                                    ddHandler.processPacket(packet, intf, neighbor);
                                }
                                break;

                            case LINKSTATE_REQUEST_PACKET:
                                if (neighbor != NULL) {
                                    lsRequestHandler.processPacket(packet, intf, neighbor);
                                }
                                break;

                            case LINKSTATE_UPDATE_PACKET:
                                if (neighbor != NULL) {
                                    lsUpdateHandler.processPacket(packet, intf, neighbor);
                                }
                                break;

                            case LINKSTATE_ACKNOWLEDGEMENT_PACKET:
                                if (neighbor != NULL) {
                                    lsAckHandler.processPacket(packet, intf, neighbor);
                                }
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
        }
    }
    delete packet;
}

void MessageHandler::sendPacket(OSPFPacket *packet, IPv4Address destination, int outputIfIndex, short ttl)
{
    IPv4ControlInfo *ipControlInfo = new IPv4ControlInfo();
    ipControlInfo->setProtocol(IP_PROT_OSPF);
    ipControlInfo->setDestAddr(destination);
    ipControlInfo->setTimeToLive(ttl);
    ipControlInfo->setInterfaceId(outputIfIndex);

    packet->setControlInfo(ipControlInfo);
    switch (packet->getType()) {
        case HELLO_PACKET: {
            packet->setKind(HELLO_PACKET);
            packet->setName("OSPF_HelloPacket");

            OSPFHelloPacket *helloPacket = check_and_cast<OSPFHelloPacket *>(packet);
            printHelloPacket(helloPacket, destination, outputIfIndex);
        }
        break;

        case DATABASE_DESCRIPTION_PACKET: {
            packet->setKind(DATABASE_DESCRIPTION_PACKET);
            packet->setName("OSPF_DDPacket");

            OSPFDatabaseDescriptionPacket *ddPacket = check_and_cast<OSPFDatabaseDescriptionPacket *>(packet);
            printDatabaseDescriptionPacket(ddPacket, destination, outputIfIndex);
        }
        break;

        case LINKSTATE_REQUEST_PACKET: {
            packet->setKind(LINKSTATE_REQUEST_PACKET);
            packet->setName("OSPF_LSReqPacket");

            OSPFLinkStateRequestPacket *requestPacket = check_and_cast<OSPFLinkStateRequestPacket *>(packet);
            printLinkStateRequestPacket(requestPacket, destination, outputIfIndex);
        }
        break;

        case LINKSTATE_UPDATE_PACKET: {
            packet->setKind(LINKSTATE_UPDATE_PACKET);
            packet->setName("OSPF_LSUpdPacket");

            OSPFLinkStateUpdatePacket *updatePacket = check_and_cast<OSPFLinkStateUpdatePacket *>(packet);
            printLinkStateUpdatePacket(updatePacket, destination, outputIfIndex);
        }
        break;

        case LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            packet->setKind(LINKSTATE_ACKNOWLEDGEMENT_PACKET);
            packet->setName("OSPF_LSAckPacket");

            OSPFLinkStateAcknowledgementPacket *ackPacket = check_and_cast<OSPFLinkStateAcknowledgementPacket *>(packet);
            printLinkStateAcknowledgementPacket(ackPacket, destination, outputIfIndex);
        }
        break;

        default:
            break;
    }

    ospfModule->send(packet, "ipOut");
}

void MessageHandler::clearTimer(cMessage *timer)
{
    ospfModule->cancelEvent(timer);
}

void MessageHandler::startTimer(cMessage *timer, simtime_t delay)
{
    ospfModule->scheduleAt(simTime() + delay, timer);
}

void MessageHandler::printEvent(const char *eventString, const Interface *onInterface, const Neighbor *forNeighbor    /*= NULL*/) const
{
    EV_DETAIL << eventString;
    if ((onInterface != NULL) || (forNeighbor != NULL)) {
        EV_DETAIL << ": ";
    }
    if (forNeighbor != NULL) {
        EV_DETAIL << "neighbor["
                  << forNeighbor->getNeighborID()
                  << "] (state: "
                  << forNeighbor->getStateString(forNeighbor->getState())
                  << "); ";
    }
    if (onInterface != NULL) {
        EV_DETAIL << "interface["
                  << static_cast<short>(onInterface->getIfIndex())
                  << "] ";
        switch (onInterface->getType()) {
            case Interface::POINTTOPOINT:
                EV_DETAIL << "(PointToPoint)";
                break;

            case Interface::BROADCAST:
                EV_DETAIL << "(Broadcast)";
                break;

            case Interface::NBMA:
                EV_DETAIL << "(NBMA).\n";
                break;

            case Interface::POINTTOMULTIPOINT:
                EV_DETAIL << "(PointToMultiPoint)";
                break;

            case Interface::VIRTUAL:
                EV_DETAIL << "(Virtual)";
                break;

            default:
                EV_DETAIL << "(Unknown)";
                break;
        }
        EV_DETAIL << " (state: "
                  << onInterface->getStateString(onInterface->getState())
                  << ")";
    }
    EV_DETAIL << ".\n";
}

void MessageHandler::printHelloPacket(const OSPFHelloPacket *helloPacket, IPv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Hello packet to " << destination << " on interface[" << outputIfIndex << "] with contents:\n";
    EV_INFO << "  netMask=" << helloPacket->getNetworkMask() << "\n";
    EV_INFO << "  DR=" << helloPacket->getDesignatedRouter() << "\n";
    EV_INFO << "  BDR=" << helloPacket->getBackupDesignatedRouter() << "\n";

    EV_DETAIL << "  neighbors:\n";

    unsigned int neighborCount = helloPacket->getNeighborArraySize();
    for (unsigned int i = 0; i < neighborCount; i++) {
        EV_DETAIL << "    " << helloPacket->getNeighbor(i) << "\n";
    }
}

void MessageHandler::printDatabaseDescriptionPacket(const OSPFDatabaseDescriptionPacket *ddPacket, IPv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Database Description packet to " << destination << " on interface[" << outputIfIndex << "] with contents:\n";

    const OSPFDDOptions& ddOptions = ddPacket->getDdOptions();
    EV_INFO << "  ddOptions="
            << ((ddOptions.I_Init) ? "I " : "_ ")
            << ((ddOptions.M_More) ? "M " : "_ ")
            << ((ddOptions.MS_MasterSlave) ? "MS" : "__")
            << "\n";
    EV_INFO << "  seqNumber=" << ddPacket->getDdSequenceNumber() << "\n";
    EV_DETAIL << "  LSA headers:\n";

    unsigned int lsaCount = ddPacket->getLsaHeadersArraySize();
    for (unsigned int i = 0; i < lsaCount; i++) {
        EV_DETAIL << "    " << ddPacket->getLsaHeaders(i) << "\n";
    }
}

void MessageHandler::printLinkStateRequestPacket(const OSPFLinkStateRequestPacket *requestPacket, IPv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Link State Request packet to " << destination << " on interface[" << outputIfIndex << "] with requests:\n";

    unsigned int requestCount = requestPacket->getRequestsArraySize();
    for (unsigned int i = 0; i < requestCount; i++) {
        const LSARequest& request = requestPacket->getRequests(i);
        EV_DETAIL << "  type=" << request.lsType
                  << ", LSID=" << request.linkStateID
                  << ", advertisingRouter=" << request.advertisingRouter << "\n";
    }
}

void MessageHandler::printLinkStateUpdatePacket(const OSPFLinkStateUpdatePacket *updatePacket, IPv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Link State Update packet to " << destination << " on interface[" << outputIfIndex << "] with updates:\n";

    unsigned int i = 0;
    unsigned int updateCount = updatePacket->getRouterLSAsArraySize();

    for (i = 0; i < updateCount; i++) {
        const OSPFRouterLSA& lsa = updatePacket->getRouterLSAs(i);
        EV_DETAIL << "  " << lsa.getHeader() << "\n";

        EV_DETAIL << "  bits="
                  << ((lsa.getV_VirtualLinkEndpoint()) ? "V " : "_ ")
                  << ((lsa.getE_ASBoundaryRouter()) ? "E " : "_ ")
                  << ((lsa.getB_AreaBorderRouter()) ? "B" : "_")
                  << "\n";
        EV_DETAIL << "  links:\n";

        unsigned int linkCount = lsa.getLinksArraySize();
        for (unsigned int j = 0; j < linkCount; j++) {
            const Link& link = lsa.getLinks(j);
            EV_DETAIL << "    ID=" << link.getLinkID();
            EV_DETAIL << ", data="
                      << link.getLinkData() << " (" << IPv4Address(link.getLinkData()) << ")"
                      << ", type=";
            switch (link.getType()) {
                case POINTTOPOINT_LINK:
                    EV_INFO << "PointToPoint";
                    break;

                case TRANSIT_LINK:
                    EV_INFO << "Transit";
                    break;

                case STUB_LINK:
                    EV_INFO << "Stub";
                    break;

                case VIRTUAL_LINK:
                    EV_INFO << "Virtual";
                    break;

                default:
                    EV_INFO << "Unknown";
                    break;
            }
            EV_DETAIL << ", cost=" << link.getLinkCost() << "\n";
        }
    }

    updateCount = updatePacket->getNetworkLSAsArraySize();
    for (i = 0; i < updateCount; i++) {
        const OSPFNetworkLSA& lsa = updatePacket->getNetworkLSAs(i);
        EV_DETAIL << "  " << lsa.getHeader() << "\n";
        EV_DETAIL << "  netMask=" << lsa.getNetworkMask() << "\n";
        EV_DETAIL << "  attachedRouters:\n";

        unsigned int routerCount = lsa.getAttachedRoutersArraySize();
        for (unsigned int j = 0; j < routerCount; j++) {
            EV_DETAIL << "    " << lsa.getAttachedRouters(j) << "\n";
        }
    }

    updateCount = updatePacket->getSummaryLSAsArraySize();
    for (i = 0; i < updateCount; i++) {
        const OSPFSummaryLSA& lsa = updatePacket->getSummaryLSAs(i);
        EV_DETAIL << "  " << lsa.getHeader() << "\n";
        EV_DETAIL << "  netMask=" << lsa.getNetworkMask() << "\n";
        EV_DETAIL << "  cost=" << lsa.getRouteCost() << "\n";
    }

    updateCount = updatePacket->getAsExternalLSAsArraySize();
    for (i = 0; i < updateCount; i++) {
        const OSPFASExternalLSA& lsa = updatePacket->getAsExternalLSAs(i);
        EV_DETAIL << "  " << lsa.getHeader() << "\n";

        const OSPFASExternalLSAContents& contents = lsa.getContents();
        EV_DETAIL << "  netMask=" << contents.getNetworkMask() << "\n";
        EV_DETAIL << "  bits=" << ((contents.getE_ExternalMetricType()) ? "E\n" : "_\n");
        EV_DETAIL << "  cost=" << contents.getRouteCost() << "\n";
        EV_DETAIL << "  forward=" << contents.getForwardingAddress() << "\n";
    }
}

void MessageHandler::printLinkStateAcknowledgementPacket(const OSPFLinkStateAcknowledgementPacket *ackPacket, IPv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Link State Acknowledgement packet to " << destination << " on interface[" << outputIfIndex << "] with acknowledgements:\n";

    unsigned int lsaCount = ackPacket->getLsaHeadersArraySize();
    for (unsigned int i = 0; i < lsaCount; i++) {
        EV_DETAIL << "    " << ackPacket->getLsaHeaders(i) << "\n";
    }
}

} // namespace ospf

} // namespace inet

