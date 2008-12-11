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

#include "OSPFInterface.h"
#include "OSPFInterfaceStateDown.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "MessageHandler.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"
#include <vector>
#include <memory.h>

OSPF::Interface::Interface(OSPF::Interface::OSPFInterfaceType ifType) :
    interfaceType(ifType),
    ifIndex(0),
    mtu(0),
    interfaceAddressRange(OSPF::NullIPv4AddressRange),
    areaID(OSPF::BackboneAreaID),
    transitAreaID(OSPF::BackboneAreaID),
    helloInterval(10),
    pollInterval(120),
    routerDeadInterval(40),
    interfaceTransmissionDelay(1),
    routerPriority(0),
    designatedRouter(OSPF::NullDesignatedRouterID),
    backupDesignatedRouter(OSPF::NullDesignatedRouterID),
    interfaceOutputCost(1),
    retransmissionInterval(5),
    acknowledgementDelay(1),
    authenticationType(OSPF::NullType),
    parentArea(NULL)
{
    state = new OSPF::InterfaceStateDown;
    previousState = NULL;
    helloTimer = new OSPFTimer;
    helloTimer->setTimerKind(InterfaceHelloTimer);
    helloTimer->setContextPointer(this);
    helloTimer->setName("OSPF::Interface::InterfaceHelloTimer");
    waitTimer = new OSPFTimer;
    waitTimer->setTimerKind(InterfaceWaitTimer);
    waitTimer->setContextPointer(this);
    waitTimer->setName("OSPF::Interface::InterfaceWaitTimer");
    acknowledgementTimer = new OSPFTimer;
    acknowledgementTimer->setTimerKind(InterfaceAcknowledgementTimer);
    acknowledgementTimer->setContextPointer(this);
    acknowledgementTimer->setName("OSPF::Interface::InterfaceAcknowledgementTimer");
    memset(authenticationKey.bytes, 0, 8 * sizeof(char));
}

OSPF::Interface::~Interface(void)
{
    MessageHandler* messageHandler = parentArea->GetRouter()->GetMessageHandler();
    messageHandler->ClearTimer(helloTimer);
    delete helloTimer;
    messageHandler->ClearTimer(waitTimer);
    delete waitTimer;
    messageHandler->ClearTimer(acknowledgementTimer);
    delete acknowledgementTimer;
    if (previousState != NULL) {
        delete previousState;
    }
    delete state;
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        delete neighboringRouters[i];
    }
}

void OSPF::Interface::SetIfIndex(unsigned char index)
{
    ifIndex = index;
    if (interfaceType == OSPF::Interface::UnknownType) {
        InterfaceEntry* routingInterface = InterfaceTableAccess().get()->getInterfaceById(ifIndex);
        interfaceAddressRange.address = IPv4AddressFromAddressString(routingInterface->ipv4Data()->getIPAddress().str().c_str());
        interfaceAddressRange.mask = IPv4AddressFromAddressString(routingInterface->ipv4Data()->getNetmask().str().c_str());
        mtu = routingInterface->getMTU();
    }
}

void OSPF::Interface::ChangeState(OSPF::InterfaceState* newState, OSPF::InterfaceState* currentState)
{
    if (previousState != NULL) {
        delete previousState;
    }
    state = newState;
    previousState = currentState;
}

void OSPF::Interface::ProcessEvent(OSPF::Interface::InterfaceEventType event)
{
    state->ProcessEvent(this, event);
}

void OSPF::Interface::Reset(void)
{
    MessageHandler* messageHandler = parentArea->GetRouter()->GetMessageHandler();
    messageHandler->ClearTimer(helloTimer);
    messageHandler->ClearTimer(waitTimer);
    messageHandler->ClearTimer(acknowledgementTimer);
    designatedRouter = NullDesignatedRouterID;
    backupDesignatedRouter = NullDesignatedRouterID;
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        neighboringRouters[i]->ProcessEvent(OSPF::Neighbor::KillNeighbor);
    }
}

void OSPF::Interface::SendHelloPacket(OSPF::IPv4Address destination, short ttl)
{
    OSPFOptions options;
    OSPFHelloPacket* helloPacket = new OSPFHelloPacket;
    std::vector<OSPF::IPv4Address> neighbors;

    helloPacket->setRouterID(parentArea->GetRouter()->GetRouterID());
    helloPacket->setAreaID(parentArea->GetAreaID());
    helloPacket->setAuthenticationType(authenticationType);
    for (int i = 0; i < 8; i++) {
        helloPacket->setAuthentication(i, authenticationKey.bytes[i]);
    }

    if (((interfaceType == PointToPoint) &&
         (interfaceAddressRange.address == OSPF::NullIPv4Address)) ||
        (interfaceType == Virtual))
    {
        helloPacket->setNetworkMask(ULongFromIPv4Address(OSPF::NullIPv4Address));
    } else {
        helloPacket->setNetworkMask(ULongFromIPv4Address(interfaceAddressRange.mask));
    }
    memset(&options, 0, sizeof(OSPFOptions));
    options.E_ExternalRoutingCapability = parentArea->GetExternalRoutingCapability();
    helloPacket->setOptions(options);
    helloPacket->setHelloInterval(helloInterval);
    helloPacket->setRouterPriority(routerPriority);
    helloPacket->setRouterDeadInterval(routerDeadInterval);
    helloPacket->setDesignatedRouter(ULongFromIPv4Address(designatedRouter.ipInterfaceAddress));
    helloPacket->setBackupDesignatedRouter(ULongFromIPv4Address(backupDesignatedRouter.ipInterfaceAddress));
    long neighborCount = neighboringRouters.size();
    for (long j = 0; j < neighborCount; j++) {
        if (neighboringRouters[j]->GetState() >= OSPF::Neighbor::InitState) {
            neighbors.push_back(neighboringRouters[j]->GetAddress());
        }
    }
    unsigned int initedNeighborCount = neighbors.size();
    helloPacket->setNeighborArraySize(initedNeighborCount);
    for (unsigned int k = 0; k < initedNeighborCount; k++) {
        helloPacket->setNeighbor(k, ULongFromIPv4Address(neighbors[k]));
    }

    helloPacket->setPacketLength(0); // TODO: Calculate correct length
    helloPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

    parentArea->GetRouter()->GetMessageHandler()->SendPacket(helloPacket, destination, ifIndex, ttl);
}

void OSPF::Interface::SendLSAcknowledgement(OSPFLSAHeader* lsaHeader, IPv4Address destination)
{
    OSPFOptions                         options;
    OSPFLinkStateAcknowledgementPacket* lsAckPacket = new OSPFLinkStateAcknowledgementPacket;

    lsAckPacket->setType(LinkStateAcknowledgementPacket);
    lsAckPacket->setRouterID(parentArea->GetRouter()->GetRouterID());
    lsAckPacket->setAreaID(parentArea->GetAreaID());
    lsAckPacket->setAuthenticationType(authenticationType);
    for (int i = 0; i < 8; i++) {
        lsAckPacket->setAuthentication(i, authenticationKey.bytes[i]);
    }

    lsAckPacket->setLsaHeadersArraySize(1);
    lsAckPacket->setLsaHeaders(0, *lsaHeader);

    lsAckPacket->setPacketLength(0); // TODO: Calculate correct length
    lsAckPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

    int ttl = (interfaceType == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
    parentArea->GetRouter()->GetMessageHandler()->SendPacket(lsAckPacket, destination, ifIndex, ttl);
}


OSPF::Neighbor* OSPF::Interface::GetNeighborByID(OSPF::RouterID neighborID)
{
    std::map<OSPF::RouterID, OSPF::Neighbor*>::iterator neighborIt = neighboringRoutersByID.find(neighborID);
    if (neighborIt != neighboringRoutersByID.end()) {
        return (neighborIt->second);
    }
    else {
        return NULL;
    }
}

OSPF::Neighbor* OSPF::Interface::GetNeighborByAddress(OSPF::IPv4Address address)
{
    std::map<OSPF::IPv4Address, OSPF::Neighbor*, OSPF::IPv4Address_Less>::iterator neighborIt = neighboringRoutersByAddress.find(address);
    if (neighborIt != neighboringRoutersByAddress.end()) {
        return (neighborIt->second);
    }
    else {
        return NULL;
    }
}

void OSPF::Interface::AddNeighbor(OSPF::Neighbor* neighbor)
{
    neighboringRoutersByID[neighbor->GetNeighborID()] = neighbor;
    neighboringRoutersByAddress[neighbor->GetAddress()] = neighbor;
    neighbor->SetInterface(this);
    neighboringRouters.push_back(neighbor);
}

OSPF::Interface::InterfaceStateType OSPF::Interface::GetState(void) const
{
    return state->GetState();
}

const char* OSPF::Interface::GetStateString(OSPF::Interface::InterfaceStateType stateType)
{
    switch (stateType) {
        case DownState:                 return "Down";
        case LoopbackState:             return "Loopback";
        case WaitingState:              return "Waiting";
        case PointToPointState:         return "PointToPoint";
        case NotDesignatedRouterState:  return "NotDesignatedRouter";
        case BackupState:               return "Backup";
        case DesignatedRouterState:     return "DesignatedRouter";
        default:                        ASSERT(false);
    }
    return "";
}

bool OSPF::Interface::HasAnyNeighborInStates(int states) const
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        OSPF::Neighbor::NeighborStateType neighborState = neighboringRouters[i]->GetState();
        if (neighborState & states) {
            return true;
        }
    }
    return false;
}

void OSPF::Interface::RemoveFromAllRetransmissionLists(OSPF::LSAKeyType lsaKey)
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        neighboringRouters[i]->RemoveFromRetransmissionList(lsaKey);
    }
}

bool OSPF::Interface::IsOnAnyRetransmissionList(OSPF::LSAKeyType lsaKey) const
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        if (neighboringRouters[i]->IsLSAOnRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}

/**
 * @see RFC2328 Section 13.3.
 */
bool OSPF::Interface::FloodLSA(OSPFLSA* lsa, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    bool floodedBackOut = false;

    if (
        (
         (lsa->getHeader().getLsType() == ASExternalLSAType) &&
         (interfaceType != OSPF::Interface::Virtual) &&
         (parentArea->GetExternalRoutingCapability())
        ) ||
        (
         (lsa->getHeader().getLsType() != ASExternalLSAType) &&
         (
          (
           (areaID != OSPF::BackboneAreaID) &&
           (interfaceType != OSPF::Interface::Virtual)
          ) ||
          (areaID == OSPF::BackboneAreaID)
         )
        )
       )
    {
        long              neighborCount                = neighboringRouters.size();
        bool              lsaAddedToRetransmissionList = false;
        OSPF::LinkStateID linkStateID                  = lsa->getHeader().getLinkStateID();
        OSPF::LSAKeyType  lsaKey;

        lsaKey.linkStateID = linkStateID;
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

        for (long i = 0; i < neighborCount; i++) {  // (1)
            if (neighboringRouters[i]->GetState() < OSPF::Neighbor::ExchangeState) {   // (1) (a)
                continue;
            }
            if (neighboringRouters[i]->GetState() < OSPF::Neighbor::FullState) {   // (1) (b)
                OSPFLSAHeader* requestLSAHeader = neighboringRouters[i]->FindOnRequestList(lsaKey);
                if (requestLSAHeader != NULL) {
                    // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
                    if (lsa->getHeader() < (*requestLSAHeader)) {
                        continue;
                    }
                    if (operator== (lsa->getHeader(), (*requestLSAHeader))) {
                        neighboringRouters[i]->RemoveFromRequestList(lsaKey);
                        continue;
                    }
                    neighboringRouters[i]->RemoveFromRequestList(lsaKey);
                }
            }
            if (neighbor == neighboringRouters[i]) {     // (1) (c)
                continue;
            }
            neighboringRouters[i]->AddToRetransmissionList(lsa);   // (1) (d)
            lsaAddedToRetransmissionList = true;
        }
        if (lsaAddedToRetransmissionList) {     // (2)
            if ((intf != this) ||
                ((neighbor != NULL) &&
                 (neighbor->GetNeighborID() != designatedRouter.routerID) &&
                 (neighbor->GetNeighborID() != backupDesignatedRouter.routerID)))  // (3)
            {
                if ((intf != this) || (GetState() != OSPF::Interface::BackupState)) {  // (4)
                    OSPFLinkStateUpdatePacket* updatePacket = CreateUpdatePacket(lsa);    // (5)

                    if (updatePacket != NULL) {
                        int                   ttl            = (interfaceType == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
                        OSPF::MessageHandler* messageHandler = parentArea->GetRouter()->GetMessageHandler();

                        if (interfaceType == OSPF::Interface::Broadcast) {
                            if ((GetState() == OSPF::Interface::DesignatedRouterState) ||
                                (GetState() == OSPF::Interface::BackupState) ||
                                (designatedRouter == OSPF::NullDesignatedRouterID))
                            {
                                messageHandler->SendPacket(updatePacket, OSPF::AllSPFRouters, ifIndex, ttl);
                                for (long k = 0; k < neighborCount; k++) {
                                    neighboringRouters[k]->AddToTransmittedLSAList(lsaKey);
                                    if (!neighboringRouters[k]->IsUpdateRetransmissionTimerActive()) {
                                        neighboringRouters[k]->StartUpdateRetransmissionTimer();
                                    }
                                }
                            } else {
                                messageHandler->SendPacket(updatePacket, OSPF::AllDRouters, ifIndex, ttl);
                                OSPF::Neighbor* dRouter = GetNeighborByID(designatedRouter.routerID);
                                OSPF::Neighbor* backupDRouter = GetNeighborByID(backupDesignatedRouter.routerID);
                                if (dRouter != NULL) {
                                    dRouter->AddToTransmittedLSAList(lsaKey);
                                    if (!dRouter->IsUpdateRetransmissionTimerActive()) {
                                        dRouter->StartUpdateRetransmissionTimer();
                                    }
                                }
                                if (backupDRouter != NULL) {
                                    backupDRouter->AddToTransmittedLSAList(lsaKey);
                                    if (!backupDRouter->IsUpdateRetransmissionTimerActive()) {
                                        backupDRouter->StartUpdateRetransmissionTimer();
                                    }
                                }
                            }
                        } else {
                            if (interfaceType == OSPF::Interface::PointToPoint) {
                                messageHandler->SendPacket(updatePacket, OSPF::AllSPFRouters, ifIndex, ttl);
                                if (neighborCount > 0) {
                                    neighboringRouters[0]->AddToTransmittedLSAList(lsaKey);
                                    if (!neighboringRouters[0]->IsUpdateRetransmissionTimerActive()) {
                                        neighboringRouters[0]->StartUpdateRetransmissionTimer();
                                    }
                                }
                            } else {
                                for (long m = 0; m < neighborCount; m++) {
                                    if (neighboringRouters[m]->GetState() >= OSPF::Neighbor::ExchangeState) {
                                        messageHandler->SendPacket(updatePacket, neighboringRouters[m]->GetAddress(), ifIndex, ttl);
                                        neighboringRouters[m]->AddToTransmittedLSAList(lsaKey);
                                        if (!neighboringRouters[m]->IsUpdateRetransmissionTimerActive()) {
                                            neighboringRouters[m]->StartUpdateRetransmissionTimer();
                                        }
                                    }
                                }
                            }
                        }

                        if (intf == this) {
                            floodedBackOut = true;
                        }
                    }
                }
            }
        }
    }

    return floodedBackOut;
}

OSPFLinkStateUpdatePacket* OSPF::Interface::CreateUpdatePacket(OSPFLSA* lsa)
{
    LSAType lsaType                  = static_cast<LSAType> (lsa->getHeader().getLsType());
    OSPFRouterLSA* routerLSA         = (lsaType == RouterLSAType) ? dynamic_cast<OSPFRouterLSA*> (lsa) : NULL;
    OSPFNetworkLSA* networkLSA       = (lsaType == NetworkLSAType) ? dynamic_cast<OSPFNetworkLSA*> (lsa) : NULL;
    OSPFSummaryLSA* summaryLSA       = ((lsaType == SummaryLSA_NetworksType) ||
                                        (lsaType == SummaryLSA_ASBoundaryRoutersType)) ? dynamic_cast<OSPFSummaryLSA*> (lsa) : NULL;
    OSPFASExternalLSA* asExternalLSA = (lsaType == ASExternalLSAType) ? dynamic_cast<OSPFASExternalLSA*> (lsa) : NULL;

    if (((lsaType == RouterLSAType) && (routerLSA != NULL)) ||
        ((lsaType == NetworkLSAType) && (networkLSA != NULL)) ||
        (((lsaType == SummaryLSA_NetworksType) || (lsaType == SummaryLSA_ASBoundaryRoutersType)) && (summaryLSA != NULL)) ||
        ((lsaType == ASExternalLSAType) && (asExternalLSA != NULL)))
    {
        OSPFLinkStateUpdatePacket* updatePacket = new OSPFLinkStateUpdatePacket;

        updatePacket->setType(LinkStateUpdatePacket);
        updatePacket->setRouterID(parentArea->GetRouter()->GetRouterID());
        updatePacket->setAreaID(areaID);
        updatePacket->setAuthenticationType(authenticationType);
        for (int j = 0; j < 8; j++) {
            updatePacket->setAuthentication(j, authenticationKey.bytes[j]);
        }

        updatePacket->setNumberOfLSAs(1);

        switch (lsaType) {
            case RouterLSAType:
                {
                    updatePacket->setRouterLSAsArraySize(1);
                    updatePacket->setRouterLSAs(0, *routerLSA);
                    unsigned short lsAge = updatePacket->getRouterLSAs(0).getHeader().getLsAge();
                    if (lsAge < MAX_AGE - interfaceTransmissionDelay) {
                        updatePacket->getRouterLSAs(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
                    } else {
                        updatePacket->getRouterLSAs(0).getHeader().setLsAge(MAX_AGE);
                    }
                }
                break;
            case NetworkLSAType:
                {
                    updatePacket->setNetworkLSAsArraySize(1);
                    updatePacket->setNetworkLSAs(0, *networkLSA);
                    unsigned short lsAge = updatePacket->getNetworkLSAs(0).getHeader().getLsAge();
                    if (lsAge < MAX_AGE - interfaceTransmissionDelay) {
                        updatePacket->getNetworkLSAs(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
                    } else {
                        updatePacket->getNetworkLSAs(0).getHeader().setLsAge(MAX_AGE);
                    }
                }
                break;
            case SummaryLSA_NetworksType:
            case SummaryLSA_ASBoundaryRoutersType:
                {
                    updatePacket->setSummaryLSAsArraySize(1);
                    updatePacket->setSummaryLSAs(0, *summaryLSA);
                    unsigned short lsAge = updatePacket->getSummaryLSAs(0).getHeader().getLsAge();
                    if (lsAge < MAX_AGE - interfaceTransmissionDelay) {
                        updatePacket->getSummaryLSAs(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
                    } else {
                        updatePacket->getSummaryLSAs(0).getHeader().setLsAge(MAX_AGE);
                    }
                }
                break;
            case ASExternalLSAType:
                {
                    updatePacket->setAsExternalLSAsArraySize(1);
                    updatePacket->setAsExternalLSAs(0, *asExternalLSA);
                    unsigned short lsAge = updatePacket->getAsExternalLSAs(0).getHeader().getLsAge();
                    if (lsAge < MAX_AGE - interfaceTransmissionDelay) {
                        updatePacket->getAsExternalLSAs(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
                    } else {
                        updatePacket->getAsExternalLSAs(0).getHeader().setLsAge(MAX_AGE);
                    }
                }
                break;
            default: break;
        }

        updatePacket->setPacketLength(0); // TODO: Calculate correct length
        updatePacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

        return updatePacket;
    }
    return NULL;
}

void OSPF::Interface::AddDelayedAcknowledgement(OSPFLSAHeader& lsaHeader)
{
    if (interfaceType == OSPF::Interface::Broadcast) {
        if ((GetState() == OSPF::Interface::DesignatedRouterState) ||
            (GetState() == OSPF::Interface::BackupState) ||
            (designatedRouter == OSPF::NullDesignatedRouterID))
        {
            delayedAcknowledgements[OSPF::AllSPFRouters].push_back(lsaHeader);
        } else {
            delayedAcknowledgements[OSPF::AllDRouters].push_back(lsaHeader);
        }
    } else {
        long neighborCount = neighboringRouters.size();
        for (long i = 0; i < neighborCount; i++) {
            if (neighboringRouters[i]->GetState() >= OSPF::Neighbor::ExchangeState) {
                delayedAcknowledgements[neighboringRouters[i]->GetAddress()].push_back(lsaHeader);
            }
        }
    }
}

void OSPF::Interface::SendDelayedAcknowledgements(void)
{
    OSPF::MessageHandler* messageHandler = parentArea->GetRouter()->GetMessageHandler();
    long                  maxPacketSize  = ((IPV4_HEADER_LENGTH + OSPF_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH) > mtu) ? IPV4_DATAGRAM_LENGTH : mtu;

    for (std::map<IPv4Address, std::list<OSPFLSAHeader>, OSPF::IPv4Address_Less>::iterator delayIt = delayedAcknowledgements.begin();
         delayIt != delayedAcknowledgements.end();
         delayIt++)
    {
        int ackCount = delayIt->second.size();
        if (ackCount > 0) {
            while (!(delayIt->second.empty())) {
                OSPFLinkStateAcknowledgementPacket* ackPacket  = new OSPFLinkStateAcknowledgementPacket;
                long                                packetSize = IPV4_HEADER_LENGTH + OSPF_HEADER_LENGTH;

                ackPacket->setType(LinkStateAcknowledgementPacket);
                ackPacket->setRouterID(parentArea->GetRouter()->GetRouterID());
                ackPacket->setAreaID(areaID);
                ackPacket->setAuthenticationType(authenticationType);
                for (int i = 0; i < 8; i++) {
                    ackPacket->setAuthentication(i, authenticationKey.bytes[i]);
                }

                while ((!(delayIt->second.empty())) && (packetSize <= (maxPacketSize - OSPF_LSA_HEADER_LENGTH))) {
                    unsigned long   headerCount = ackPacket->getLsaHeadersArraySize();
                    ackPacket->setLsaHeadersArraySize(headerCount + 1);
                    ackPacket->setLsaHeaders(headerCount, *(delayIt->second.begin()));
                    delayIt->second.pop_front();
                    packetSize += OSPF_LSA_HEADER_LENGTH;
                }

                ackPacket->setPacketLength(0); // TODO: Calculate correct length
                ackPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

                int ttl = (interfaceType == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

                if (interfaceType == OSPF::Interface::Broadcast) {
                    if ((GetState() == OSPF::Interface::DesignatedRouterState) ||
                        (GetState() == OSPF::Interface::BackupState) ||
                        (designatedRouter == OSPF::NullDesignatedRouterID))
                    {
                        messageHandler->SendPacket(ackPacket, OSPF::AllSPFRouters, ifIndex, ttl);
                    } else {
                        messageHandler->SendPacket(ackPacket, OSPF::AllDRouters, ifIndex, ttl);
                    }
                } else {
                    if (interfaceType == OSPF::Interface::PointToPoint) {
                        messageHandler->SendPacket(ackPacket, OSPF::AllSPFRouters, ifIndex, ttl);
                    } else {
                        messageHandler->SendPacket(ackPacket, delayIt->first, ifIndex, ttl);
                    }
                }
            }
        }
    }
    messageHandler->StartTimer(acknowledgementTimer, acknowledgementDelay);
}

void OSPF::Interface::AgeTransmittedLSALists(void)
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        neighboringRouters[i]->AgeTransmittedLSAList();
    }
}
