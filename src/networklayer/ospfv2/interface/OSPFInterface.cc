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
    interfaceAddressRange(OSPF::NULL_IPV4ADDRESSRANGE),
    areaID(OSPF::BACKBONE_AREAID),
    transitAreaID(OSPF::BACKBONE_AREAID),
    helloInterval(10),
    pollInterval(120),
    routerDeadInterval(40),
    interfaceTransmissionDelay(1),
    routerPriority(0),
    designatedRouter(OSPF::NULL_DESIGNATEDROUTERID),
    backupDesignatedRouter(OSPF::NULL_DESIGNATEDROUTERID),
    interfaceOutputCost(1),
    retransmissionInterval(5),
    acknowledgementDelay(1),
    authenticationType(OSPF::NULL_TYPE),
    parentArea(NULL)
{
    state = new OSPF::InterfaceStateDown;
    previousState = NULL;
    helloTimer = new OSPFTimer();
    helloTimer->setTimerKind(INTERFACE_HELLO_TIMER);
    helloTimer->setContextPointer(this);
    helloTimer->setName("OSPF::Interface::InterfaceHelloTimer");
    waitTimer = new OSPFTimer();
    waitTimer->setTimerKind(INTERFACE_WAIT_TIMER);
    waitTimer->setContextPointer(this);
    waitTimer->setName("OSPF::Interface::InterfaceWaitTimer");
    acknowledgementTimer = new OSPFTimer();
    acknowledgementTimer->setTimerKind(INTERFACE_ACKNOWLEDGEMENT_TIMER);
    acknowledgementTimer->setContextPointer(this);
    acknowledgementTimer->setName("OSPF::Interface::InterfaceAcknowledgementTimer");
    memset(authenticationKey.bytes, 0, 8 * sizeof(char));
}

OSPF::Interface::~Interface()
{
    MessageHandler* messageHandler = parentArea->getRouter()->getMessageHandler();
    messageHandler->clearTimer(helloTimer);
    delete helloTimer;
    messageHandler->clearTimer(waitTimer);
    delete waitTimer;
    messageHandler->clearTimer(acknowledgementTimer);
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

void OSPF::Interface::setIfIndex(unsigned char index)
{
    ifIndex = index;
    if (interfaceType == OSPF::Interface::UNKNOWN_TYPE) {
        InterfaceEntry* routingInterface = InterfaceTableAccess().get()->getInterfaceById(ifIndex);
        interfaceAddressRange.address = ipv4AddressFromAddressString(routingInterface->ipv4Data()->getIPAddress().str().c_str());
        interfaceAddressRange.mask = ipv4AddressFromAddressString(routingInterface->ipv4Data()->getNetmask().str().c_str());
        mtu = routingInterface->getMTU();
    }
}

void OSPF::Interface::changeState(OSPF::InterfaceState* newState, OSPF::InterfaceState* currentState)
{
    if (previousState != NULL) {
        delete previousState;
    }
    state = newState;
    previousState = currentState;
}

void OSPF::Interface::processEvent(OSPF::Interface::InterfaceEventType event)
{
    state->processEvent(this, event);
}

void OSPF::Interface::reset()
{
    MessageHandler* messageHandler = parentArea->getRouter()->getMessageHandler();
    messageHandler->clearTimer(helloTimer);
    messageHandler->clearTimer(waitTimer);
    messageHandler->clearTimer(acknowledgementTimer);
    designatedRouter = NULL_DESIGNATEDROUTERID;
    backupDesignatedRouter = NULL_DESIGNATEDROUTERID;
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        neighboringRouters[i]->processEvent(OSPF::Neighbor::KILL_NEIGHBOR);
    }
}

void OSPF::Interface::sendHelloPacket(IPv4Address destination, short ttl)
{
    OSPFOptions options;
    OSPFHelloPacket* helloPacket = new OSPFHelloPacket();
    std::vector<IPv4Address> neighbors;

    helloPacket->setRouterID(parentArea->getRouter()->getRouterID());
    helloPacket->setAreaID(parentArea->getAreaID());
    helloPacket->setAuthenticationType(authenticationType);
    for (int i = 0; i < 8; i++) {
        helloPacket->setAuthentication(i, authenticationKey.bytes[i]);
    }

    if (((interfaceType == POINTTOPOINT) &&
         (interfaceAddressRange.address == OSPF::NULL_IPV4ADDRESS)) ||
        (interfaceType == VIRTUAL))
    {
        helloPacket->setNetworkMask(ulongFromIPv4Address(OSPF::NULL_IPV4ADDRESS));
    } else {
        helloPacket->setNetworkMask(ulongFromIPv4Address(interfaceAddressRange.mask));
    }
    memset(&options, 0, sizeof(OSPFOptions));
    options.E_ExternalRoutingCapability = parentArea->getExternalRoutingCapability();
    helloPacket->setOptions(options);
    helloPacket->setHelloInterval(helloInterval);
    helloPacket->setRouterPriority(routerPriority);
    helloPacket->setRouterDeadInterval(routerDeadInterval);
    helloPacket->setDesignatedRouter(ulongFromIPv4Address(designatedRouter.ipInterfaceAddress));
    helloPacket->setBackupDesignatedRouter(ulongFromIPv4Address(backupDesignatedRouter.ipInterfaceAddress));
    long neighborCount = neighboringRouters.size();
    for (long j = 0; j < neighborCount; j++) {
        if (neighboringRouters[j]->getState() >= OSPF::Neighbor::INIT_STATE) {
            neighbors.push_back(neighboringRouters[j]->getAddress());
        }
    }
    unsigned int initedNeighborCount = neighbors.size();
    helloPacket->setNeighborArraySize(initedNeighborCount);
    for (unsigned int k = 0; k < initedNeighborCount; k++) {
        helloPacket->setNeighbor(k, ulongFromIPv4Address(neighbors[k]));
    }

    helloPacket->setPacketLength(0); // TODO: Calculate correct length
    helloPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

    parentArea->getRouter()->getMessageHandler()->sendPacket(helloPacket, destination, ifIndex, ttl);
}

void OSPF::Interface::sendLSAcknowledgement(OSPFLSAHeader* lsaHeader, IPv4Address destination)
{
    OSPFOptions options;
    OSPFLinkStateAcknowledgementPacket* lsAckPacket = new OSPFLinkStateAcknowledgementPacket();

    lsAckPacket->setType(LINKSTATE_ACKNOWLEDGEMENT_PACKET);
    lsAckPacket->setRouterID(parentArea->getRouter()->getRouterID());
    lsAckPacket->setAreaID(parentArea->getAreaID());
    lsAckPacket->setAuthenticationType(authenticationType);
    for (int i = 0; i < 8; i++) {
        lsAckPacket->setAuthentication(i, authenticationKey.bytes[i]);
    }

    lsAckPacket->setLsaHeadersArraySize(1);
    lsAckPacket->setLsaHeaders(0, *lsaHeader);

    lsAckPacket->setPacketLength(0); // TODO: Calculate correct length
    lsAckPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

    int ttl = (interfaceType == OSPF::Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
    parentArea->getRouter()->getMessageHandler()->sendPacket(lsAckPacket, destination, ifIndex, ttl);
}


OSPF::Neighbor* OSPF::Interface::getNeighborByID(OSPF::RouterID neighborID)
{
    std::map<OSPF::RouterID, OSPF::Neighbor*>::iterator neighborIt = neighboringRoutersByID.find(neighborID);
    if (neighborIt != neighboringRoutersByID.end()) {
        return (neighborIt->second);
    }
    else {
        return NULL;
    }
}

OSPF::Neighbor* OSPF::Interface::getNeighborByAddress(IPv4Address address)
{
    std::map<IPv4Address, OSPF::Neighbor*, OSPF::IPv4Address_Less>::iterator neighborIt =
            neighboringRoutersByAddress.find(address);

    if (neighborIt != neighboringRoutersByAddress.end()) {
        return (neighborIt->second);
    }
    else {
        return NULL;
    }
}

void OSPF::Interface::addNeighbor(OSPF::Neighbor* neighbor)
{
    neighboringRoutersByID[neighbor->getNeighborID()] = neighbor;
    neighboringRoutersByAddress[neighbor->getAddress()] = neighbor;
    neighbor->setInterface(this);
    neighboringRouters.push_back(neighbor);
}

OSPF::Interface::InterfaceStateType OSPF::Interface::getState() const
{
    return state->getState();
}

const char* OSPF::Interface::getStateString(OSPF::Interface::InterfaceStateType stateType)
{
    switch (stateType) {
        case DOWN_STATE:                 return "Down";
        case LOOPBACK_STATE:             return "Loopback";
        case WAITING_STATE:              return "Waiting";
        case POINTTOPOINT_STATE:         return "PointToPoint";
        case NOT_DESIGNATED_ROUTER_STATE:  return "NotDesignatedRouter";
        case BACKUP_STATE:               return "Backup";
        case DESIGNATED_ROUTER_STATE:     return "DesignatedRouter";
        default:                        ASSERT(false);
    }
    return "";
}

bool OSPF::Interface::hasAnyNeighborInStates(int states) const
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        OSPF::Neighbor::NeighborStateType neighborState = neighboringRouters[i]->getState();
        if (neighborState & states) {
            return true;
        }
    }
    return false;
}

void OSPF::Interface::removeFromAllRetransmissionLists(OSPF::LSAKeyType lsaKey)
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        neighboringRouters[i]->removeFromRetransmissionList(lsaKey);
    }
}

bool OSPF::Interface::isOnAnyRetransmissionList(OSPF::LSAKeyType lsaKey) const
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        if (neighboringRouters[i]->isLinkStateRequestListEmpty(lsaKey)) {
            return true;
        }
    }
    return false;
}

/**
 * @see RFC2328 Section 13.3.
 */
bool OSPF::Interface::floodLSA(OSPFLSA* lsa, OSPF::Interface* intf, OSPF::Neighbor* neighbor)
{
    bool floodedBackOut = false;

    if (
        (
         (lsa->getHeader().getLsType() == AS_EXTERNAL_LSA_TYPE) &&
         (interfaceType != OSPF::Interface::VIRTUAL) &&
         (parentArea->getExternalRoutingCapability())
        ) ||
        (
         (lsa->getHeader().getLsType() != AS_EXTERNAL_LSA_TYPE) &&
         (
          (
           (areaID != OSPF::BACKBONE_AREAID) &&
           (interfaceType != OSPF::Interface::VIRTUAL)
          ) ||
          (areaID == OSPF::BACKBONE_AREAID)
         )
        )
       )
    {
        long neighborCount = neighboringRouters.size();
        bool lsaAddedToRetransmissionList = false;
        OSPF::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
        OSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = linkStateID;
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

        for (long i = 0; i < neighborCount; i++) {  // (1)
            if (neighboringRouters[i]->getState() < OSPF::Neighbor::EXCHANGE_STATE) {   // (1) (a)
                continue;
            }
            if (neighboringRouters[i]->getState() < OSPF::Neighbor::FULL_STATE) {   // (1) (b)
                OSPFLSAHeader* requestLSAHeader = neighboringRouters[i]->findOnRequestList(lsaKey);
                if (requestLSAHeader != NULL) {
                    // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
                    if (lsa->getHeader() < (*requestLSAHeader)) {
                        continue;
                    }
                    if (operator==(lsa->getHeader(), (*requestLSAHeader))) {
                        neighboringRouters[i]->removeFromRequestList(lsaKey);
                        continue;
                    }
                    neighboringRouters[i]->removeFromRequestList(lsaKey);
                }
            }
            if (neighbor == neighboringRouters[i]) {     // (1) (c)
                continue;
            }
            neighboringRouters[i]->addToRetransmissionList(lsa);   // (1) (d)
            lsaAddedToRetransmissionList = true;
        }
        if (lsaAddedToRetransmissionList) {     // (2)
            if ((intf != this) ||
                ((neighbor != NULL) &&
                 (neighbor->getNeighborID() != designatedRouter.routerID) &&
                 (neighbor->getNeighborID() != backupDesignatedRouter.routerID)))  // (3)
            {
                if ((intf != this) || (getState() != OSPF::Interface::BACKUP_STATE)) {  // (4)
                    OSPFLinkStateUpdatePacket* updatePacket = createUpdatePacket(lsa);    // (5)

                    if (updatePacket != NULL) {
                        int ttl = (interfaceType == OSPF::Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
                        OSPF::MessageHandler* messageHandler = parentArea->getRouter()->getMessageHandler();

                        if (interfaceType == OSPF::Interface::BROADCAST) {
                            if ((getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
                                (getState() == OSPF::Interface::BACKUP_STATE) ||
                                (designatedRouter == OSPF::NULL_DESIGNATEDROUTERID))
                            {
                                messageHandler->sendPacket(updatePacket, OSPF::ALL_SPF_ROUTERS, ifIndex, ttl);
                                for (long k = 0; k < neighborCount; k++) {
                                    neighboringRouters[k]->addToTransmittedLSAList(lsaKey);
                                    if (!neighboringRouters[k]->isUpdateRetransmissionTimerActive()) {
                                        neighboringRouters[k]->startUpdateRetransmissionTimer();
                                    }
                                }
                            } else {
                                messageHandler->sendPacket(updatePacket, OSPF::ALL_D_ROUTERS, ifIndex, ttl);
                                OSPF::Neighbor* dRouter = getNeighborByID(designatedRouter.routerID);
                                OSPF::Neighbor* backupDRouter = getNeighborByID(backupDesignatedRouter.routerID);
                                if (dRouter != NULL) {
                                    dRouter->addToTransmittedLSAList(lsaKey);
                                    if (!dRouter->isUpdateRetransmissionTimerActive()) {
                                        dRouter->startUpdateRetransmissionTimer();
                                    }
                                }
                                if (backupDRouter != NULL) {
                                    backupDRouter->addToTransmittedLSAList(lsaKey);
                                    if (!backupDRouter->isUpdateRetransmissionTimerActive()) {
                                        backupDRouter->startUpdateRetransmissionTimer();
                                    }
                                }
                            }
                        } else {
                            if (interfaceType == OSPF::Interface::POINTTOPOINT) {
                                messageHandler->sendPacket(updatePacket, OSPF::ALL_SPF_ROUTERS, ifIndex, ttl);
                                if (neighborCount > 0) {
                                    neighboringRouters[0]->addToTransmittedLSAList(lsaKey);
                                    if (!neighboringRouters[0]->isUpdateRetransmissionTimerActive()) {
                                        neighboringRouters[0]->startUpdateRetransmissionTimer();
                                    }
                                }
                            } else {
                                for (long m = 0; m < neighborCount; m++) {
                                    if (neighboringRouters[m]->getState() >= OSPF::Neighbor::EXCHANGE_STATE) {
                                        messageHandler->sendPacket(updatePacket, neighboringRouters[m]->getAddress(), ifIndex, ttl);
                                        neighboringRouters[m]->addToTransmittedLSAList(lsaKey);
                                        if (!neighboringRouters[m]->isUpdateRetransmissionTimerActive()) {
                                            neighboringRouters[m]->startUpdateRetransmissionTimer();
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

OSPFLinkStateUpdatePacket* OSPF::Interface::createUpdatePacket(OSPFLSA* lsa)
{
    LSAType lsaType = static_cast<LSAType> (lsa->getHeader().getLsType());
    OSPFRouterLSA* routerLSA = (lsaType == ROUTERLSA_TYPE) ? dynamic_cast<OSPFRouterLSA*> (lsa) : NULL;
    OSPFNetworkLSA* networkLSA = (lsaType == NETWORKLSA_TYPE) ? dynamic_cast<OSPFNetworkLSA*> (lsa) : NULL;
    OSPFSummaryLSA* summaryLSA = ((lsaType == SUMMARYLSA_NETWORKS_TYPE) ||
                                        (lsaType == SUMMARYLSA_ASBOUNDARYROUTERS_TYPE)) ? dynamic_cast<OSPFSummaryLSA*> (lsa) : NULL;
    OSPFASExternalLSA* asExternalLSA = (lsaType == AS_EXTERNAL_LSA_TYPE) ? dynamic_cast<OSPFASExternalLSA*> (lsa) : NULL;

    if (((lsaType == ROUTERLSA_TYPE) && (routerLSA != NULL)) ||
        ((lsaType == NETWORKLSA_TYPE) && (networkLSA != NULL)) ||
        (((lsaType == SUMMARYLSA_NETWORKS_TYPE) || (lsaType == SUMMARYLSA_ASBOUNDARYROUTERS_TYPE)) && (summaryLSA != NULL)) ||
        ((lsaType == AS_EXTERNAL_LSA_TYPE) && (asExternalLSA != NULL)))
    {
        OSPFLinkStateUpdatePacket* updatePacket = new OSPFLinkStateUpdatePacket();

        updatePacket->setType(LINKSTATE_UPDATE_PACKET);
        updatePacket->setRouterID(parentArea->getRouter()->getRouterID());
        updatePacket->setAreaID(areaID);
        updatePacket->setAuthenticationType(authenticationType);
        for (int j = 0; j < 8; j++) {
            updatePacket->setAuthentication(j, authenticationKey.bytes[j]);
        }

        updatePacket->setNumberOfLSAs(1);

        switch (lsaType) {
            case ROUTERLSA_TYPE:
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
            case NETWORKLSA_TYPE:
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
            case SUMMARYLSA_NETWORKS_TYPE:
            case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
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
            case AS_EXTERNAL_LSA_TYPE:
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

void OSPF::Interface::addDelayedAcknowledgement(OSPFLSAHeader& lsaHeader)
{
    if (interfaceType == OSPF::Interface::BROADCAST) {
        if ((getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
            (getState() == OSPF::Interface::BACKUP_STATE) ||
            (designatedRouter == OSPF::NULL_DESIGNATEDROUTERID))
        {
            delayedAcknowledgements[OSPF::ALL_SPF_ROUTERS].push_back(lsaHeader);
        } else {
            delayedAcknowledgements[OSPF::ALL_D_ROUTERS].push_back(lsaHeader);
        }
    } else {
        long neighborCount = neighboringRouters.size();
        for (long i = 0; i < neighborCount; i++) {
            if (neighboringRouters[i]->getState() >= OSPF::Neighbor::EXCHANGE_STATE) {
                delayedAcknowledgements[neighboringRouters[i]->getAddress()].push_back(lsaHeader);
            }
        }
    }
}

void OSPF::Interface::sendDelayedAcknowledgements()
{
    OSPF::MessageHandler* messageHandler = parentArea->getRouter()->getMessageHandler();
    long maxPacketSize = ((IPV4_HEADER_LENGTH + OSPF_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH) > mtu) ? IPV4_DATAGRAM_LENGTH : mtu;

    for (std::map<IPv4Address, std::list<OSPFLSAHeader>, OSPF::IPv4Address_Less>::iterator delayIt = delayedAcknowledgements.begin();
         delayIt != delayedAcknowledgements.end();
         delayIt++)
    {
        int ackCount = delayIt->second.size();
        if (ackCount > 0) {
            while (!(delayIt->second.empty())) {
                OSPFLinkStateAcknowledgementPacket* ackPacket = new OSPFLinkStateAcknowledgementPacket();
                long packetSize = IPV4_HEADER_LENGTH + OSPF_HEADER_LENGTH;

                ackPacket->setType(LINKSTATE_ACKNOWLEDGEMENT_PACKET);
                ackPacket->setRouterID(parentArea->getRouter()->getRouterID());
                ackPacket->setAreaID(areaID);
                ackPacket->setAuthenticationType(authenticationType);
                for (int i = 0; i < 8; i++) {
                    ackPacket->setAuthentication(i, authenticationKey.bytes[i]);
                }

                while ((!(delayIt->second.empty())) && (packetSize <= (maxPacketSize - OSPF_LSA_HEADER_LENGTH))) {
                    unsigned long headerCount = ackPacket->getLsaHeadersArraySize();
                    ackPacket->setLsaHeadersArraySize(headerCount + 1);
                    ackPacket->setLsaHeaders(headerCount, *(delayIt->second.begin()));
                    delayIt->second.pop_front();
                    packetSize += OSPF_LSA_HEADER_LENGTH;
                }

                ackPacket->setPacketLength(0); // TODO: Calculate correct length
                ackPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

                int ttl = (interfaceType == OSPF::Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

                if (interfaceType == OSPF::Interface::BROADCAST) {
                    if ((getState() == OSPF::Interface::DESIGNATED_ROUTER_STATE) ||
                        (getState() == OSPF::Interface::BACKUP_STATE) ||
                        (designatedRouter == OSPF::NULL_DESIGNATEDROUTERID))
                    {
                        messageHandler->sendPacket(ackPacket, OSPF::ALL_SPF_ROUTERS, ifIndex, ttl);
                    } else {
                        messageHandler->sendPacket(ackPacket, OSPF::ALL_D_ROUTERS, ifIndex, ttl);
                    }
                } else {
                    if (interfaceType == OSPF::Interface::POINTTOPOINT) {
                        messageHandler->sendPacket(ackPacket, OSPF::ALL_SPF_ROUTERS, ifIndex, ttl);
                    } else {
                        messageHandler->sendPacket(ackPacket, delayIt->first, ifIndex, ttl);
                    }
                }
            }
        }
    }
    messageHandler->startTimer(acknowledgementTimer, acknowledgementDelay);
}

void OSPF::Interface::ageTransmittedLSALists()
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        neighboringRouters[i]->ageTransmittedLSAList();
    }
}
