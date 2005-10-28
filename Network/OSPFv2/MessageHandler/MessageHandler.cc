#include "MessageHandler.h"
#include "OSPFRouter.h"

OSPF::MessageHandler::MessageHandler (OSPF::Router* containingRouter, cSimpleModule* containingModule) :
    OSPF::IMessageHandler (containingRouter),
    helloHandler (containingRouter),
    ddHandler (containingRouter),
    lsRequestHandler (containingRouter),
    lsUpdateHandler (containingRouter),
    lsAckHandler (containingRouter),
    ospfModule (containingModule)
{
}

void OSPF::MessageHandler::MessageReceived (cMessage* message)
{
    if (message->isSelfMessage ()) {
        HandleTimer (check_and_cast<OSPFTimer*> (message));
    } else {
        OSPFPacket* packet = check_and_cast<OSPFPacket*> (message);
        if (packet->getRouterID () == router->GetRouterID ()) {
            ev << "Discarding self-message.\n";
            delete message;
        } else {
            ev << "Received packet: (" << packet->className () << ")" << packet->name() << "\n";
            ProcessPacket (packet);
        }
    }
}

void OSPF::MessageHandler::HandleTimer (OSPFTimer* timer)
{
    switch (timer->getTimerKind ()) {
        case InterfaceHelloTimer:
            {
                OSPF::Interface* intf;
                if (! (intf = reinterpret_cast <OSPF::Interface*> (timer->contextPointer ()))) {
                    // should not reach this point
                    ev << "Discarding invalid InterfaceHelloTimer.\n";
                    delete timer;
                } else {
                    PrintEvent ("Hello Timer expired", intf);
                    intf->ProcessEvent (OSPF::Interface::HelloTimer);
                }
            }
            break;
        case InterfaceWaitTimer:
            {
                OSPF::Interface* intf;
                if (! (intf = reinterpret_cast <OSPF::Interface*> (timer->contextPointer ()))) {
                    // should not reach this point
                    ev << "Discarding invalid InterfaceWaitTimer.\n";
                    delete timer;
                } else {
                    PrintEvent ("Wait Timer expired", intf);
                    intf->ProcessEvent (OSPF::Interface::WaitTimer);
                }
            }
            break;
        case InterfaceAcknowledgementTimer:
            {
                OSPF::Interface* intf;
                if (! (intf = reinterpret_cast <OSPF::Interface*> (timer->contextPointer ()))) {
                    // should not reach this point
                    ev << "Discarding invalid InterfaceAcknowledgementTimer.\n";
                    delete timer;
                } else {
                    PrintEvent ("Acknowledgement Timer expired", intf);
                    intf->ProcessEvent (OSPF::Interface::AcknowledgementTimer);
                }
            }
            break;
        case NeighborInactivityTimer:
            {
                OSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <OSPF::Neighbor*> (timer->contextPointer ()))) {
                    // should not reach this point
                    ev << "Discarding invalid NeighborInactivityTimer.\n";
                    delete timer;
                } else {
                    PrintEvent ("Inactivity Timer expired", neighbor->GetInterface (), neighbor);
                    neighbor->ProcessEvent (OSPF::Neighbor::InactivityTimer);
                }
            }
            break;
        case NeighborPollTimer:
            {
                OSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <OSPF::Neighbor*> (timer->contextPointer ()))) {
                    // should not reach this point
                    ev << "Discarding invalid NeighborInactivityTimer.\n";
                    delete timer;
                } else {
                    PrintEvent ("Poll Timer expired", neighbor->GetInterface (), neighbor);
                    neighbor->ProcessEvent (OSPF::Neighbor::PollTimer);
                }
            }
            break;
        case NeighborDDRetransmissionTimer:
            {
                OSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <OSPF::Neighbor*> (timer->contextPointer ()))) {
                    // should not reach this point
                    ev << "Discarding invalid NeighborDDRetransmissionTimer.\n";
                    delete timer;
                } else {
                    PrintEvent ("Database Description Retransmission Timer expired", neighbor->GetInterface (), neighbor);
                    neighbor->ProcessEvent (OSPF::Neighbor::DDRetransmissionTimer);
                }
            }
            break;
        case NeighborUpdateRetransmissionTimer:
            {
                OSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <OSPF::Neighbor*> (timer->contextPointer ()))) {
                    // should not reach this point
                    ev << "Discarding invalid NeighborUpdateRetransmissionTimer.\n";
                    delete timer;
                } else {
                    PrintEvent ("Update Retransmission Timer expired", neighbor->GetInterface (), neighbor);
                    neighbor->ProcessEvent (OSPF::Neighbor::UpdateRetransmissionTimer);
                }
            }
            break;
        case NeighborRequestRetransmissionTimer:
            {
                OSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <OSPF::Neighbor*> (timer->contextPointer ()))) {
                    // should not reach this point
                    ev << "Discarding invalid NeighborRequestRetransmissionTimer.\n";
                    delete timer;
                } else {
                    PrintEvent ("Request Retransmission Timer expired", neighbor->GetInterface (), neighbor);
                    neighbor->ProcessEvent (OSPF::Neighbor::RequestRetransmissionTimer);
                }
            }
            break;
        case DatabaseAgeTimer:
            {
                PrintEvent ("Ageing the database");
                router->AgeDatabase ();
            }
            break;
        default: break;
    }
}

void OSPF::MessageHandler::ProcessPacket (OSPFPacket* packet, OSPF::Interface* unused1, OSPF::Neighbor* unused2)
{
    // packet version must be OSPF version 2
    if (packet->getVersion () == 2) {
        IPControlInfo*  controlInfo = check_and_cast<IPControlInfo *> (packet->controlInfo ());
        int             inputPort   = controlInfo->inputPort ();
        OSPF::AreaID    areaID      = packet->getAreaID ().getInt ();
        OSPF::Area*     area        = router->GetArea (areaID);

        if (area != NULL) {
            // packet Area ID must either match the Area ID of the receiving interface or...
            OSPF::Interface* intf = area->GetInterface (inputPort);

            if (intf == NULL) {
                // it must be the backbone area and...
                if (areaID == BackboneAreaID) {
                    if (router->GetAreaCount () > 1) {
                        // it must be a virtual link and the source router's router ID must be the endpoint of this virtual link and...
                        intf = area->FindVirtualLink (packet->getRouterID ().getInt ());

                        if (intf != NULL) {
                            OSPF::Area* virtualLinkTransitArea = router->GetArea (intf->GetAreaID ());

                            if (virtualLinkTransitArea != NULL) {
                                // the receiving interface must attach to the virtual link's configured transit area
                                OSPF::Interface* virtualLinkInterface = virtualLinkTransitArea->GetInterface (inputPort);

                                if (virtualLinkInterface == NULL) {
                                    intf = NULL;
                                }
                            } else {
                                intf = NULL;
                            }
                        }
                    }
                }
            }
            if (intf != NULL) {
                unsigned long                       destinationAddress = controlInfo->destAddr ().getInt ();
                unsigned long                       allDRouters        = ULongFromIPv4Address (OSPF::AllDRouters);
                OSPF::Interface::InterfaceStateType interfaceState     = intf->GetState ();

                // if destination address is AllDRouters the receiving interface must be in DesignatedRouter or Backup state
                if (
                    ((destinationAddress == allDRouters) &&
                     (
                      (interfaceState == OSPF::Interface::DesignatedRouterState) ||
                      (interfaceState == OSPF::Interface::BackupState)
                     )
                    ) ||
                    (destinationAddress != allDRouters)
                   )
                {
                    // packet authentication
                    if (AuthenticatePacket (packet)) {
                        OSPFPacketType  packetType = static_cast<OSPFPacketType> (packet->getType ());
                        OSPF::Neighbor* neighbor   = NULL;

                        // all packets except HelloPackets are sent only along adjacencies, so a Neighbor must exist
                        if (packetType != HelloPacket) {
                            switch (intf->GetType ()) {
                                case OSPF::Interface::Broadcast:
                                case OSPF::Interface::NBMA:
                                case OSPF::Interface::PointToMultiPoint:
                                    neighbor = intf->GetNeighborByAddress (IPv4AddressFromULong (controlInfo->srcAddr ().getInt ()));
                                    break;
                                case OSPF::Interface::PointToPoint:
                                case OSPF::Interface::Virtual:
                                    neighbor = intf->GetNeighborByID (packet->getRouterID ().getInt ());
                                    break;
                                default: break;
                            }
                        }
                        switch (packetType) {
                            case HelloPacket:
                                helloHandler.ProcessPacket (packet, intf);
                                break;
                            case DatabaseDescriptionPacket:
                                if (neighbor != NULL) {
                                    ddHandler.ProcessPacket (packet, intf, neighbor);
                                }
                                break;
                            case LinkStateRequestPacket:
                                if (neighbor != NULL) {
                                    lsRequestHandler.ProcessPacket (packet, intf, neighbor);
                                }
                                break;
                            case LinkStateUpdatePacket:
                                if (neighbor != NULL) {
                                    lsUpdateHandler.ProcessPacket (packet, intf, neighbor);
                                }
                                break;
                            case LinkStateAcknowledgementPacket:
                                if (neighbor != NULL) {
                                    lsAckHandler.ProcessPacket (packet, intf, neighbor);
                                }
                                break;
                            default: break;
                        }
                    }
                }
            }
        }
    }
    delete packet;
}

void OSPF::MessageHandler::SendPacket (OSPFPacket* packet, IPv4Address destination, int outputIfIndex, short ttl)
{
    IPControlInfo *ipControlInfo = new IPControlInfo ();
    ipControlInfo->setProtocol (IP_PROT_OSPF);
    ipControlInfo->setDestAddr (ULongFromIPv4Address (destination));
    ipControlInfo->setTimeToLive (ttl);
    ipControlInfo->setOutputPort (outputIfIndex);

    packet->setControlInfo (ipControlInfo);
    switch (packet->getType ()) {
        case HelloPacket:
            {
                packet->setKind (HelloPacket);
                packet->setName ("OSPF_HelloPacket");

                OSPFHelloPacket* helloPacket = check_and_cast<OSPFHelloPacket*> (packet);
                PrintHelloPacket (helloPacket, destination, outputIfIndex);
            }
            break;
        case DatabaseDescriptionPacket:
            {
                packet->setKind (DatabaseDescriptionPacket);
                packet->setName ("OSPF_DDPacket");

                OSPFDatabaseDescriptionPacket* ddPacket = check_and_cast<OSPFDatabaseDescriptionPacket*> (packet);
                PrintDatabaseDescriptionPacket (ddPacket, destination, outputIfIndex);
            }
            break;
        case LinkStateRequestPacket:
            {
                packet->setKind (LinkStateRequestPacket);
                packet->setName ("OSPF_LSReqPacket");

                OSPFLinkStateRequestPacket* requestPacket = check_and_cast<OSPFLinkStateRequestPacket*> (packet);
                PrintLinkStateRequestPacket (requestPacket, destination, outputIfIndex);
            }
            break;
        case LinkStateUpdatePacket:
            {
                packet->setKind (LinkStateUpdatePacket);
                packet->setName ("OSPF_LSUpdPacket");

                OSPFLinkStateUpdatePacket* updatePacket = check_and_cast<OSPFLinkStateUpdatePacket*> (packet);
                PrintLinkStateUpdatePacket (updatePacket, destination, outputIfIndex);
            }
            break;
        case LinkStateAcknowledgementPacket:
            {
                packet->setKind (LinkStateAcknowledgementPacket);
                packet->setName ("OSPF_LSAckPacket");

                OSPFLinkStateAcknowledgementPacket* ackPacket = check_and_cast<OSPFLinkStateAcknowledgementPacket*> (packet);
                PrintLinkStateAcknowledgementPacket (ackPacket, destination, outputIfIndex);
            }
            break;
        default: break;
    }

    ospfModule->send (packet,"to_ip");
}

void OSPF::MessageHandler::ClearTimer (OSPFTimer* timer)
{
    ospfModule->cancelEvent (timer);
}

void OSPF::MessageHandler::StartTimer (OSPFTimer* timer, simtime_t delay)
{
    ospfModule->scheduleAt (ospfModule->simTime () + delay, timer);
}

void OSPF::MessageHandler::PrintEvent (const char* eventString, const OSPF::Interface* onInterface, const OSPF::Neighbor* forNeighbor /*= NULL*/) const
{
    ev << eventString;
    if ((onInterface != NULL) || (forNeighbor != NULL)) {
        ev << ": ";
    }
    if (forNeighbor != NULL) {
        char addressString[16];
        ev << "neighbor["
           << AddressStringFromULong (addressString, sizeof (addressString), forNeighbor->GetNeighborID ())
           << "] (state: "
           << forNeighbor->GetStateString (forNeighbor->GetState ())
           << "); ";
    }
    if (onInterface != NULL) {
        ev << "interface["
           << static_cast <short> (onInterface->GetIfIndex ())
           << "] ";
        switch (onInterface->GetType ()) {
            case OSPF::Interface::PointToPoint:      ev << "(PointToPoint)";
                                                     break;
            case OSPF::Interface::Broadcast:         ev << "(Broadcast)";
                                                     break;
            case OSPF::Interface::NBMA:              ev << "(NBMA).\n";
                                                     break;
            case OSPF::Interface::PointToMultiPoint: ev << "(PointToMultiPoint)";
                                                     break;
            case OSPF::Interface::Virtual:           ev << "(Virtual)";
                                                     break;
            default:                                 ev << "(Unknown)";
        }
        ev << " (state: "
           << onInterface->GetStateString (onInterface->GetState ())
           << ")";
    }
    ev << ".\n";
}

void OSPF::MessageHandler::PrintHelloPacket (const OSPFHelloPacket* helloPacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    ev << "Sending Hello packet to "
       << AddressStringFromIPv4Address (addressString, sizeof (addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with contents:\n";
    ev << "  netMask="
       << AddressStringFromULong (addressString, sizeof (addressString), helloPacket->getNetworkMask ().getInt ())
       << "\n";
    ev << "  DR="
       << AddressStringFromULong (addressString, sizeof (addressString), helloPacket->getDesignatedRouter ().getInt ())
       << "\n";
    ev << "  BDR="
       << AddressStringFromULong (addressString, sizeof (addressString), helloPacket->getBackupDesignatedRouter ().getInt ())
       << "\n";
    ev << "  neighbors:\n";

    unsigned int neighborCount = helloPacket->getNeighborArraySize ();
    for (unsigned int i = 0; i < neighborCount; i++) {
        ev << "    "
           << AddressStringFromULong (addressString, sizeof (addressString), helloPacket->getNeighbor (i).getInt ())
           << "\n";
    }
}

void OSPF::MessageHandler::PrintDatabaseDescriptionPacket (const OSPFDatabaseDescriptionPacket* ddPacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    ev << "Sending Database Description packet to "
       << AddressStringFromIPv4Address (addressString, sizeof (addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with contents:\n";

    const OSPFDDOptions& ddOptions = ddPacket->getDdOptions ();
    ev << "  ddOptions="
       << ((ddOptions.I_Init) ? "I " : "_ ")
       << ((ddOptions.M_More) ? "M " : "_ ")
       << ((ddOptions.MS_MasterSlave) ? "MS" : "__")
       << "\n";
    ev << "  seqNumber="
       << ddPacket->getDdSequenceNumber ()
       << "\n";
    ev << "  LSA headers:\n";

    unsigned int lsaCount = ddPacket->getLsaHeadersArraySize ();
    for (unsigned int i = 0; i < lsaCount; i++) {
        ev << "    ";
        PrintLSAHeader (ddPacket->getLsaHeaders (i));
        ev << "\n";
    }
}

void OSPF::MessageHandler::PrintLinkStateRequestPacket (const OSPFLinkStateRequestPacket* requestPacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    ev << "Sending Link State Request packet to "
       << AddressStringFromIPv4Address (addressString, sizeof (addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with requests:\n";

    unsigned int requestCount = requestPacket->getRequestsArraySize ();
    for (unsigned int i = 0; i < requestCount; i++) {
        const LSARequest& request = requestPacket->getRequests(i);
        ev << "  type="
           << request.lsType
           << ", LSID="
           << AddressStringFromULong (addressString, sizeof (addressString), request.linkStateID);
        ev << ", advertisingRouter="
           << AddressStringFromULong (addressString, sizeof (addressString), request.advertisingRouter.getInt ())
           << "\n";
    }
}

void OSPF::MessageHandler::PrintLinkStateUpdatePacket (const OSPFLinkStateUpdatePacket* updatePacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    ev << "Sending Link State Update packet to "
       << AddressStringFromIPv4Address (addressString, sizeof (addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with updates:\n";

    unsigned int i           = 0;
    unsigned int updateCount = updatePacket->getRouterLSAsArraySize ();

    for (i = 0; i < updateCount; i++) {
        const OSPFRouterLSA& lsa = updatePacket->getRouterLSAs(i);
        ev << "  ";
        PrintLSAHeader (lsa.getHeader ());
        ev << "\n";

        ev << "  bits="
           << ((lsa.getV_VirtualLinkEndpoint ()) ? "V " : "_ ")
           << ((lsa.getE_ASBoundaryRouter ()) ? "E " : "_ ")
           << ((lsa.getB_AreaBorderRouter ()) ? "B" : "_")
           << "\n";
        ev << "  links:\n";

        unsigned int linkCount = lsa.getLinksArraySize ();
        for (unsigned int j = 0; j < linkCount; j++) {
            const Link& link = lsa.getLinks (j);
            ev << "    ID="
               << AddressStringFromULong (addressString, sizeof (addressString), link.getLinkID ().getInt ())
               << ",";
            ev << " data="
               << AddressStringFromULong (addressString, sizeof (addressString), link.getLinkData ())
               << ", type=";
            switch (link.getType ()) {
                case PointToPointLink:  ev << "PointToPoint";   break;
                case TransitLink:       ev << "Transit";        break;
                case StubLink:          ev << "Stub";           break;
                case VirtualLink:       ev << "Virtual";        break;
                default:                ev << "Unknown";        break;
            }
            ev << ", cost="
               << link.getLinkCost ()
               << "\n";
        }
    }

    updateCount = updatePacket->getNetworkLSAsArraySize ();
    for (i = 0; i < updateCount; i++) {
        const OSPFNetworkLSA& lsa = updatePacket->getNetworkLSAs(i);
        ev << "  ";
        PrintLSAHeader (lsa.getHeader ());
        ev << "\n";

        ev << "  netMask="
           << AddressStringFromULong (addressString, sizeof (addressString), lsa.getNetworkMask ().getInt ())
           << "\n";
        ev << "  attachedRouters:\n";

        unsigned int routerCount = lsa.getAttachedRoutersArraySize ();
        for (unsigned int j = 0; j < routerCount; j++) {
            ev << "    "
               << AddressStringFromULong (addressString, sizeof (addressString), lsa.getAttachedRouters (j).getInt ())
               << "\n";
        }
    }

    updateCount = updatePacket->getSummaryLSAsArraySize ();
    for (i = 0; i < updateCount; i++) {
        const OSPFSummaryLSA& lsa = updatePacket->getSummaryLSAs(i);
        ev << "  ";
        PrintLSAHeader (lsa.getHeader ());
        ev << "\n";

        ev << "  netMask="
           << AddressStringFromULong (addressString, sizeof (addressString), lsa.getNetworkMask ().getInt ())
           << "\n";
        ev << "  cost="
           << lsa.getRouteCost ()
           << "\n";
    }

    updateCount = updatePacket->getAsExternalLSAsArraySize ();
    for (i = 0; i < updateCount; i++) {
        const OSPFASExternalLSA& lsa = updatePacket->getAsExternalLSAs(i);
        ev << "  ";
        PrintLSAHeader (lsa.getHeader ());
        ev << "\n";

        const OSPFASExternalLSAContents& contents = lsa.getContents ();
        ev << "  netMask="
           << AddressStringFromULong (addressString, sizeof (addressString), contents.getNetworkMask ().getInt ())
           << "\n";
        ev << "  bits="
           << ((contents.getE_ExternalMetricType ()) ? "E\n" : "_\n");
        ev << "  cost="
           << contents.getRouteCost ()
           << "\n";
        ev << "  forward="
           << AddressStringFromULong (addressString, sizeof (addressString), contents.getForwardingAddress ().getInt ())
           << "\n";
    }
}

void OSPF::MessageHandler::PrintLinkStateAcknowledgementPacket (const OSPFLinkStateAcknowledgementPacket* ackPacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    ev << "Sending Link State Acknowledgement packet to "
       << AddressStringFromIPv4Address (addressString, sizeof (addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with acknowledgements:\n";

    unsigned int lsaCount = ackPacket->getLsaHeadersArraySize ();
    for (unsigned int i = 0; i < lsaCount; i++) {
        ev << "    ";
        PrintLSAHeader (ackPacket->getLsaHeaders (i));
        ev << "\n";
    }
}

