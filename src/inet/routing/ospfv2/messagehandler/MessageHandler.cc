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

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"

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
    else {
        Packet *pk = check_and_cast<Packet *>(message);
        auto protocol = pk->getTag<PacketProtocolTag>()->getProtocol();
        if (protocol == &Protocol::icmpv4) {
            EV_ERROR << "ICMP error received -- discarding\n";
            delete message;
        }
        else if (protocol == &Protocol::ospf) {
            processPacket(pk);
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

void MessageHandler::processPacket(Packet *pk, Interface *unused1, Neighbor *unused2)
{
    const auto& packet = pk->peekAtFront<OspfPacket>();
    EV_INFO << "Received packet: (" << packet.get()->getClassName() << ")" << pk->getName() << "\n";
    if (packet->getRouterID() == Ipv4Address(router->getRouterID())) {
        EV_INFO << "This packet is from ourselves, discarding.\n";
        delete pk;
        return;
    }
    // see RFC 2328 8.2

    // packet version must be OSPF version 2
    if (packet->getVersion() == 2) {
        int interfaceId = pk->getTag<InterfaceInd>()->getInterfaceId();
        AreaId areaID = packet->getAreaID();
        Area *area = router->getAreaByID(areaID);

        if (area != nullptr) {
            // packet Area ID must either match the Area ID of the receiving interface or...
            Interface *intf = area->getInterface(interfaceId);

            if (intf == nullptr) {
                // it must be the backbone area and...
                if (areaID == BACKBONE_AREAID) {
                    if (router->getAreaCount() > 1) {
                        // it must be a virtual link and the source router's router ID must be the endpoint of this virtual link and...
                        intf = area->findVirtualLink(packet->getRouterID());

                        if (intf != nullptr) {
                            Area *virtualLinkTransitArea = router->getAreaByID(intf->getTransitAreaId());

                            if (virtualLinkTransitArea != nullptr) {
                                // the receiving interface must attach to the virtual link's configured transit area
                                Interface *virtualLinkInterface = virtualLinkTransitArea->getInterface(interfaceId);

                                if (virtualLinkInterface == nullptr) {
                                    intf = nullptr;
                                }
                            }
                            else {
                                intf = nullptr;
                            }
                        }
                    }
                }
            }
            if (intf != nullptr) {
                Ipv4Address sourceAddress = pk->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
                Ipv4Address destinationAddress = pk->getTag<L3AddressInd>()->getDestAddress().toIpv4();
                Ipv4Address allDRouters = Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST;
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
                    if (authenticatePacket(packet.get())) {
                        OspfPacketType packetType = static_cast<OspfPacketType>(packet->getType());
                        Neighbor *neighbor = nullptr;

                        // all packets except HelloPackets are sent only along adjacencies, so a Neighbor must exist
                        if (packetType != HELLO_PACKET) {
                            switch (intf->getType()) {
                                case Interface::BROADCAST:
                                case Interface::NBMA:
                                case Interface::POINTTOMULTIPOINT:
                                    neighbor = intf->getNeighborByAddress(sourceAddress);
                                    break;

                                case Interface::POINTTOPOINT:
                                case Interface::VIRTUAL:
                                    neighbor = intf->getNeighborById(packet->getRouterID());
                                    break;

                                default:
                                    break;
                            }
                        }
                        switch (packetType) {
                            case HELLO_PACKET:
                                helloHandler.processPacket(pk, intf);
                                break;

                            case DATABASE_DESCRIPTION_PACKET:
                                if (neighbor != nullptr) {
                                    ddHandler.processPacket(pk, intf, neighbor);
                                }
                                break;

                            case LINKSTATE_REQUEST_PACKET:
                                if (neighbor != nullptr) {
                                    lsRequestHandler.processPacket(pk, intf, neighbor);
                                }
                                break;

                            case LINKSTATE_UPDATE_PACKET:
                                if (neighbor != nullptr) {
                                    lsUpdateHandler.processPacket(pk, intf, neighbor);
                                }
                                break;

                            case LINKSTATE_ACKNOWLEDGEMENT_PACKET:
                                if (neighbor != nullptr) {
                                    lsAckHandler.processPacket(pk, intf, neighbor);
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
    delete pk;
}

void MessageHandler::sendPacket(Packet *packet, Ipv4Address destination, int outputIfIndex, short ttl)
{
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ospf);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(outputIfIndex);
    packet->addTagIfAbsent<L3AddressReq>()->setDestAddress(destination);
    packet->addTagIfAbsent<HopLimitReq>()->setHopLimit(ttl);
    const auto& ospfPacket = packet->peekAtFront<OspfPacket>();

    switch (ospfPacket->getType()) {
        case HELLO_PACKET: {
            packet->setName("OSPF_HelloPacket");

            const auto& helloPacket = packet->peekAtFront<OspfHelloPacket>();
            printHelloPacket(helloPacket.get(), destination, outputIfIndex);
        }
        break;

        case DATABASE_DESCRIPTION_PACKET: {
            packet->setName("OSPF_DDPacket");

            const auto& ddPacket = packet->peekAtFront<OspfDatabaseDescriptionPacket>();
            printDatabaseDescriptionPacket(ddPacket.get(), destination, outputIfIndex);
        }
        break;

        case LINKSTATE_REQUEST_PACKET: {
            packet->setName("OSPF_LSReqPacket");

            const auto& requestPacket = packet->peekAtFront<OspfLinkStateRequestPacket>();
            printLinkStateRequestPacket(requestPacket.get(), destination, outputIfIndex);
        }
        break;

        case LINKSTATE_UPDATE_PACKET: {
            packet->setName("OSPF_LSUpdPacket");

            const auto& updatePacket = packet->peekAtFront<OspfLinkStateUpdatePacket>();
            printLinkStateUpdatePacket(updatePacket.get(), destination, outputIfIndex);
        }
        break;

        case LINKSTATE_ACKNOWLEDGEMENT_PACKET: {
            packet->setName("OSPF_LSAckPacket");

            const auto& ackPacket = packet->peekAtFront<OspfLinkStateAcknowledgementPacket>();
            printLinkStateAcknowledgementPacket(ackPacket.get(), destination, outputIfIndex);
        }
        break;

        default:
            break;
    }

    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
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

void MessageHandler::printEvent(const char *eventString, const Interface *onInterface, const Neighbor *forNeighbor    /*= nullptr*/) const
{
    EV_DETAIL << eventString;
    if ((onInterface != nullptr) || (forNeighbor != nullptr)) {
        EV_DETAIL << ": ";
    }
    if (forNeighbor != nullptr) {
        EV_DETAIL << "neighbor["
                  << forNeighbor->getNeighborID()
                  << "] (state: "
                  << forNeighbor->getStateString(forNeighbor->getState())
                  << "); ";
    }
    if (onInterface != nullptr) {
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

void MessageHandler::printHelloPacket(const OspfHelloPacket *helloPacket, Ipv4Address destination, int outputIfIndex) const
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

void MessageHandler::printDatabaseDescriptionPacket(const OspfDatabaseDescriptionPacket *ddPacket, Ipv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Database Description packet to " << destination << " on interface[" << outputIfIndex << "] with contents:\n";

    const OspfDdOptions& ddOptions = ddPacket->getDdOptions();
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

void MessageHandler::printLinkStateRequestPacket(const OspfLinkStateRequestPacket *requestPacket, Ipv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Link State Request packet to " << destination << " on interface[" << outputIfIndex << "] with requests:\n";

    unsigned int requestCount = requestPacket->getRequestsArraySize();
    for (unsigned int i = 0; i < requestCount; i++) {
        const LsaRequest& request = requestPacket->getRequests(i);
        EV_DETAIL << "  type=" << request.lsType
                  << ", LSID=" << request.linkStateID
                  << ", advertisingRouter=" << request.advertisingRouter << "\n";
    }
}

void MessageHandler::printLinkStateUpdatePacket(const OspfLinkStateUpdatePacket *updatePacket, Ipv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Link State Update packet to " << destination << " on interface[" << outputIfIndex << "] with updates:\n";

    unsigned int i = 0;
    unsigned int updateCount = updatePacket->getRouterLSAsArraySize();

    for (i = 0; i < updateCount; i++) {
        const OspfRouterLsa& lsa = updatePacket->getRouterLSAs(i);
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
                      << link.getLinkData() << " (" << Ipv4Address(link.getLinkData()) << ")"
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
        const OspfNetworkLsa& lsa = updatePacket->getNetworkLSAs(i);
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
        const OspfSummaryLsa& lsa = updatePacket->getSummaryLSAs(i);
        EV_DETAIL << "  " << lsa.getHeader() << "\n";
        EV_DETAIL << "  netMask=" << lsa.getNetworkMask() << "\n";
        EV_DETAIL << "  cost=" << lsa.getRouteCost() << "\n";
    }

    updateCount = updatePacket->getAsExternalLSAsArraySize();
    for (i = 0; i < updateCount; i++) {
        const OspfAsExternalLsa& lsa = updatePacket->getAsExternalLSAs(i);
        EV_DETAIL << "  " << lsa.getHeader() << "\n";

        const OspfAsExternalLsaContents& contents = lsa.getContents();
        EV_DETAIL << "  netMask=" << contents.getNetworkMask() << "\n";
        EV_DETAIL << "  bits=" << ((contents.getE_ExternalMetricType()) ? "E\n" : "_\n");
        EV_DETAIL << "  cost=" << contents.getRouteCost() << "\n";
        EV_DETAIL << "  forward=" << contents.getForwardingAddress() << "\n";
    }
}

void MessageHandler::printLinkStateAcknowledgementPacket(const OspfLinkStateAcknowledgementPacket *ackPacket, Ipv4Address destination, int outputIfIndex) const
{
    EV_INFO << "Sending Link State Acknowledgement packet to " << destination << " on interface[" << outputIfIndex << "] with acknowledgements:\n";

    unsigned int lsaCount = ackPacket->getLsaHeadersArraySize();
    for (unsigned int i = 0; i < lsaCount; i++) {
        EV_DETAIL << "    " << ackPacket->getLsaHeaders(i) << "\n";
    }
}

} // namespace ospf

} // namespace inet

