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

#include <memory.h>

#include "inet/routing/ospfv2/neighbor/OspfNeighbor.h"

#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OspfArea.h"
#include "inet/routing/ospfv2/neighbor/OspfNeighborState.h"
#include "inet/routing/ospfv2/neighbor/OspfNeighborStateDown.h"
#include "inet/routing/ospfv2/router/OspfRouter.h"

namespace inet {

namespace ospf {

// FIXME!!! Should come from a global unique number generator module.
unsigned long Neighbor::ddSequenceNumberInitSeed = 0;

Neighbor::Neighbor(RouterId neighbor) :
    neighborID(neighbor),
    neighborIPAddress(NULL_IPV4ADDRESS),
    neighborsDesignatedRouter(NULL_DESIGNATEDROUTERID),
    neighborsBackupDesignatedRouter(NULL_DESIGNATEDROUTERID),
    neighborsRouterDeadInterval(40)
{
    memset(&lastReceivedDDPacket, 0, sizeof(Neighbor::DdPacketId));
    // setting only I and M bits is invalid -> good initializer
    lastReceivedDDPacket.ddOptions.I_Init = true;
    lastReceivedDDPacket.ddOptions.M_More = true;
    inactivityTimer = new cMessage();
    inactivityTimer->setKind(NEIGHBOR_INACTIVITY_TIMER);
    inactivityTimer->setContextPointer(this);
    inactivityTimer->setName("Neighbor::NeighborInactivityTimer");
    pollTimer = new cMessage();
    pollTimer->setKind(NEIGHBOR_POLL_TIMER);
    pollTimer->setContextPointer(this);
    pollTimer->setName("Neighbor::NeighborPollTimer");
    ddRetransmissionTimer = new cMessage();
    ddRetransmissionTimer->setKind(NEIGHBOR_DD_RETRANSMISSION_TIMER);
    ddRetransmissionTimer->setContextPointer(this);
    ddRetransmissionTimer->setName("Neighbor::NeighborDDRetransmissionTimer");
    updateRetransmissionTimer = new cMessage();
    updateRetransmissionTimer->setKind(NEIGHBOR_UPDATE_RETRANSMISSION_TIMER);
    updateRetransmissionTimer->setContextPointer(this);
    updateRetransmissionTimer->setName("Neighbor::Neighbor::NeighborUpdateRetransmissionTimer");
    requestRetransmissionTimer = new cMessage();
    requestRetransmissionTimer->setKind(NEIGHBOR_REQUEST_RETRANSMISSION_TIMER);
    requestRetransmissionTimer->setContextPointer(this);
    requestRetransmissionTimer->setName("Neighbor::NeighborRequestRetransmissionTimer");
    state = new NeighborStateDown;
    previousState = nullptr;
}

Neighbor::~Neighbor()
{
    reset();
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->clearTimer(inactivityTimer);
    messageHandler->clearTimer(pollTimer);
    delete inactivityTimer;
    delete pollTimer;
    delete ddRetransmissionTimer;
    delete updateRetransmissionTimer;
    delete requestRetransmissionTimer;
    if (previousState != nullptr) {
        delete previousState;
    }
    delete state;
}

void Neighbor::changeState(NeighborState *newState, NeighborState *currentState)
{
    if (previousState != nullptr) {
        delete previousState;
    }
    state = newState;
    previousState = currentState;
}

void Neighbor::processEvent(Neighbor::NeighborEventType event)
{
    state->processEvent(this, event);
}

void Neighbor::reset()
{
    for (auto & elem : linkStateRetransmissionList)
    {
        delete (elem);
    }
    linkStateRetransmissionList.clear();

    for (auto & elem : databaseSummaryList) {
        delete (elem);
    }
    databaseSummaryList.clear();
    for (auto & elem : linkStateRequestList) {
        delete (elem);
    }
    linkStateRequestList.clear();

    parentInterface->getArea()->getRouter()->getMessageHandler()->clearTimer(ddRetransmissionTimer);
    clearUpdateRetransmissionTimer();
    clearRequestRetransmissionTimer();

    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}

void Neighbor::initFirstAdjacency()
{
    ddSequenceNumber = getUniqueULong();
    firstAdjacencyInited = true;
}

unsigned long Neighbor::getUniqueULong()
{
    // FIXME!!! Should come from a global unique number generator module.
    return ddSequenceNumberInitSeed++;
}

Neighbor::NeighborStateType Neighbor::getState() const
{
    return state->getState();
}

const char *Neighbor::getStateString(Neighbor::NeighborStateType stateType)
{
    switch (stateType) {
        case DOWN_STATE:
            return "Down";

        case ATTEMPT_STATE:
            return "Attempt";

        case INIT_STATE:
            return "Init";

        case TWOWAY_STATE:
            return "TwoWay";

        case EXCHANGE_START_STATE:
            return "ExchangeStart";

        case EXCHANGE_STATE:
            return "Exchange";

        case LOADING_STATE:
            return "Loading";

        case FULL_STATE:
            return "Full";

        default:
            ASSERT(false);
            break;
    }
    return "";
}

void Neighbor::sendDatabaseDescriptionPacket(bool init)
{
    const auto& ddPacket = makeShared<OspfDatabaseDescriptionPacket>();

    ddPacket->setType(DATABASE_DESCRIPTION_PACKET);
    ddPacket->setRouterID(Ipv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    ddPacket->setAreaID(Ipv4Address(parentInterface->getArea()->getAreaID()));
    ddPacket->setAuthenticationType(parentInterface->getAuthenticationType());
    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        ddPacket->setAuthentication(i, authKey.bytes[i]);
    }

    if (parentInterface->getType() != Interface::VIRTUAL) {
        ddPacket->setInterfaceMTU(parentInterface->getMtu());
    }
    else {
        ddPacket->setInterfaceMTU(0);
    }

    OspfOptions options;
    memset(&options, 0, sizeof(OspfOptions));
    options.E_ExternalRoutingCapability = parentInterface->getArea()->getExternalRoutingCapability();
    ddPacket->setOptions(options);

    ddPacket->setDdSequenceNumber(ddSequenceNumber);

    B maxPacketSize = (((IPv4_MAX_HEADER_LENGTH + OSPF_HEADER_LENGTH + OSPF_DD_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH) > B(parentInterface->getMtu())) ?
                          IPV4_DATAGRAM_LENGTH :
                          B(parentInterface->getMtu())) - IPv4_MAX_HEADER_LENGTH;
    B packetSize = OSPF_HEADER_LENGTH + OSPF_DD_HEADER_LENGTH;

    if (init || databaseSummaryList.empty()) {
        ddPacket->setLsaHeadersArraySize(0);
    }
    else {
        // delete included LSAs from summary list
        // (they are still in lastTransmittedDDPacket)
        while ((!databaseSummaryList.empty()) && (packetSize <= (maxPacketSize - OSPF_LSA_HEADER_LENGTH))) {
            unsigned long headerCount = ddPacket->getLsaHeadersArraySize();
            OspfLsaHeader *lsaHeader = *(databaseSummaryList.begin());
            ddPacket->setLsaHeadersArraySize(headerCount + 1);
            ddPacket->setLsaHeaders(headerCount, *lsaHeader);
            delete lsaHeader;
            databaseSummaryList.pop_front();
            packetSize += OSPF_LSA_HEADER_LENGTH;
        }
    }

    OspfDdOptions ddOptions;
    memset(&ddOptions, 0, sizeof(OspfDdOptions));
    if (init) {
        ddOptions.I_Init = true;
        ddOptions.M_More = true;
        ddOptions.MS_MasterSlave = true;
    }
    else {
        ddOptions.I_Init = false;
        ddOptions.M_More = (databaseSummaryList.empty()) ? false : true;
        ddOptions.MS_MasterSlave = (databaseExchangeRelationship == Neighbor::MASTER) ? true : false;
    }
    ddPacket->setDdOptions(ddOptions);

    ddPacket->setChunkLength(packetSize);
    Packet *pk = new Packet();
    pk->insertAtBack(ddPacket);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

    if (lastTransmittedDDPacket != nullptr)
        delete lastTransmittedDDPacket;
    lastTransmittedDDPacket = pk->dup();

    if (parentInterface->getType() == Interface::POINTTOPOINT) {
        messageHandler->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface->getIfIndex(), ttl);
    }
    else {
        messageHandler->sendPacket(pk, neighborIPAddress, parentInterface->getIfIndex(), ttl);
    }
}

bool Neighbor::retransmitDatabaseDescriptionPacket()
{
    if (lastTransmittedDDPacket != nullptr) {
        Packet *ddPacket = new Packet(*lastTransmittedDDPacket);
        MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
        int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        if (parentInterface->getType() == Interface::POINTTOPOINT) {
            messageHandler->sendPacket(ddPacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface->getIfIndex(), ttl);
        }
        else {
            messageHandler->sendPacket(ddPacket, neighborIPAddress, parentInterface->getIfIndex(), ttl);
        }

        return true;
    }
    else {
        return false;
    }
}

void Neighbor::createDatabaseSummary()
{
    Area *area = parentInterface->getArea();
    unsigned long routerLSACount = area->getRouterLSACount();

    /* Note: OSPF specification says:
     * "LSAs whose age is equal to MaxAge are instead added to the neighbor's
     *  Link state retransmission list."
     * But this task has been already done during the aging of the database. (???)
     * So we'll skip this.
     */
    for (unsigned long i = 0; i < routerLSACount; i++) {
        if (area->getRouterLSA(i)->getHeader().getLsAge() < MAX_AGE) {
            OspfLsaHeader *routerLSA = new OspfLsaHeader(area->getRouterLSA(i)->getHeader());
            databaseSummaryList.push_back(routerLSA);
        }
    }

    unsigned long networkLSACount = area->getNetworkLSACount();
    for (unsigned long j = 0; j < networkLSACount; j++) {
        if (area->getNetworkLSA(j)->getHeader().getLsAge() < MAX_AGE) {
            OspfLsaHeader *networkLSA = new OspfLsaHeader(area->getNetworkLSA(j)->getHeader());
            databaseSummaryList.push_back(networkLSA);
        }
    }

    unsigned long summaryLSACount = area->getSummaryLSACount();
    for (unsigned long k = 0; k < summaryLSACount; k++) {
        if (area->getSummaryLSA(k)->getHeader().getLsAge() < MAX_AGE) {
            OspfLsaHeader *summaryLSA = new OspfLsaHeader(area->getSummaryLSA(k)->getHeader());
            databaseSummaryList.push_back(summaryLSA);
        }
    }

    if ((parentInterface->getType() != Interface::VIRTUAL) &&
        (area->getExternalRoutingCapability()))
    {
        Router *router = area->getRouter();
        unsigned long asExternalLSACount = router->getASExternalLSACount();

        for (unsigned long m = 0; m < asExternalLSACount; m++) {
            if (router->getASExternalLSA(m)->getHeader().getLsAge() < MAX_AGE) {
                OspfLsaHeader *asExternalLSA = new OspfLsaHeader(router->getASExternalLSA(m)->getHeader());
                databaseSummaryList.push_back(asExternalLSA);
            }
        }
    }
}

void Neighbor::sendLinkStateRequestPacket()
{
    const auto& requestPacket = makeShared<OspfLinkStateRequestPacket>();

    requestPacket->setType(LINKSTATE_REQUEST_PACKET);
    requestPacket->setRouterID(Ipv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    requestPacket->setAreaID(Ipv4Address(parentInterface->getArea()->getAreaID()));
    requestPacket->setAuthenticationType(parentInterface->getAuthenticationType());
    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        requestPacket->setAuthentication(i, authKey.bytes[i]);
    }

    B maxPacketSize = ((IPv4_MAX_HEADER_LENGTH + OSPF_HEADER_LENGTH + OSPF_REQUEST_LENGTH) > B(parentInterface->getMtu())) ?
                          IPV4_DATAGRAM_LENGTH :
                          B(parentInterface->getMtu()) - IPv4_MAX_HEADER_LENGTH;
    B packetSize = OSPF_HEADER_LENGTH;

    if (linkStateRequestList.empty()) {
        requestPacket->setRequestsArraySize(0);
    }
    else {
        auto it = linkStateRequestList.begin();

        while ((it != linkStateRequestList.end()) && (packetSize <= (maxPacketSize - OSPF_REQUEST_LENGTH))) {
            unsigned long requestCount = requestPacket->getRequestsArraySize();
            OspfLsaHeader *requestHeader = (*it);
            LsaRequest request;

            request.lsType = requestHeader->getLsType();
            request.linkStateID = requestHeader->getLinkStateID();
            request.advertisingRouter = requestHeader->getAdvertisingRouter();

            requestPacket->setRequestsArraySize(requestCount + 1);
            requestPacket->setRequests(requestCount, request);

            packetSize += OSPF_REQUEST_LENGTH;
            it++;
        }
    }

    requestPacket->setChunkLength(packetSize);
    Packet *pk = new Packet();
    pk->insertAtBack(requestPacket);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
    if (parentInterface->getType() == Interface::POINTTOPOINT) {
        messageHandler->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface->getIfIndex(), ttl);
    }
    else {
        messageHandler->sendPacket(pk, neighborIPAddress, parentInterface->getIfIndex(), ttl);
    }
}

bool Neighbor::needAdjacency()
{
    Interface::OspfInterfaceType interfaceType = parentInterface->getType();
    RouterId routerID = parentInterface->getArea()->getRouter()->getRouterID();
    DesignatedRouterId dRouter = parentInterface->getDesignatedRouter();
    DesignatedRouterId backupDRouter = parentInterface->getBackupDesignatedRouter();

    if ((interfaceType == Interface::POINTTOPOINT) ||
        (interfaceType == Interface::POINTTOMULTIPOINT) ||
        (interfaceType == Interface::VIRTUAL) ||
        (dRouter.routerID == routerID) ||
        (backupDRouter.routerID == routerID) ||
        ((neighborsDesignatedRouter.routerID == dRouter.routerID) ||
         (!designatedRoutersSetUp &&
          (neighborsDesignatedRouter.ipInterfaceAddress == dRouter.ipInterfaceAddress))) ||
        ((neighborsBackupDesignatedRouter.routerID == backupDRouter.routerID) ||
         (!designatedRoutersSetUp &&
          (neighborsBackupDesignatedRouter.ipInterfaceAddress == backupDRouter.ipInterfaceAddress))))
    {
        return true;
    }
    else {
        return false;
    }
}

/**
 * If the LSA is already on the retransmission list then it is replaced, else
 * a copy of the LSA is added to the end of the retransmission list.
 * @param lsa [in] The LSA to be added.
 */
void Neighbor::addToRetransmissionList(const OspfLsa *lsa)
{
    auto it = linkStateRetransmissionList.begin();
    for ( ; it != linkStateRetransmissionList.end(); it++) {
        if (((*it)->getHeader().getLinkStateID() == lsa->getHeader().getLinkStateID()) &&
            ((*it)->getHeader().getAdvertisingRouter().getInt() == lsa->getHeader().getAdvertisingRouter().getInt()))
        {
            break;
        }
    }

    OspfLsa *lsaCopy = nullptr;
    switch (lsa->getHeader().getLsType()) {
        case ROUTERLSA_TYPE:
            lsaCopy = new OspfRouterLsa(*(check_and_cast<const OspfRouterLsa *>(lsa)));
            break;

        case NETWORKLSA_TYPE:
            lsaCopy = new OspfNetworkLsa(*(check_and_cast<const OspfNetworkLsa *>(lsa)));
            break;

        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
            lsaCopy = new OspfSummaryLsa(*(check_and_cast<const OspfSummaryLsa *>(lsa)));
            break;

        case AS_EXTERNAL_LSA_TYPE:
            lsaCopy = new OspfAsExternalLsa(*(check_and_cast<const OspfAsExternalLsa *>(lsa)));
            break;

        default:
            ASSERT(false);    // error
            break;
    }

    if (it != linkStateRetransmissionList.end()) {
        delete (*it);
        *it = static_cast<OspfLsa *>(lsaCopy);
    }
    else {
        linkStateRetransmissionList.push_back(static_cast<OspfLsa *>(lsaCopy));
    }
}

void Neighbor::removeFromRetransmissionList(LsaKeyType lsaKey)
{
    auto it = linkStateRetransmissionList.begin();
    while (it != linkStateRetransmissionList.end()) {
        if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete (*it);
            it = linkStateRetransmissionList.erase(it);
        }
        else {
            it++;
        }
    }
}

bool Neighbor::isLinkStateRequestListEmpty(LsaKeyType lsaKey) const
{
    for (auto lsa : linkStateRetransmissionList) {
        if ((lsa->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            (lsa->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

OspfLsa *Neighbor::findOnRetransmissionList(LsaKeyType lsaKey)
{
    for (auto & elem : linkStateRetransmissionList) {
        if (((elem)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((elem)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return elem;
        }
    }
    return nullptr;
}

void Neighbor::startUpdateRetransmissionTimer()
{
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->startTimer(updateRetransmissionTimer, parentInterface->getRetransmissionInterval());
    updateRetransmissionTimerActive = true;
}

void Neighbor::clearUpdateRetransmissionTimer()
{
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->clearTimer(updateRetransmissionTimer);
    updateRetransmissionTimerActive = false;
}

void Neighbor::addToRequestList(const OspfLsaHeader *lsaHeader)
{
    linkStateRequestList.push_back(new OspfLsaHeader(*lsaHeader));
}

void Neighbor::removeFromRequestList(LsaKeyType lsaKey)
{
    auto it = linkStateRequestList.begin();
    while (it != linkStateRequestList.end()) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete (*it);
            it = linkStateRequestList.erase(it);
        }
        else {
            it++;
        }
    }

    if ((getState() == Neighbor::LOADING_STATE) && (linkStateRequestList.empty())) {
        clearRequestRetransmissionTimer();
        processEvent(Neighbor::LOADING_DONE);
    }
}

bool Neighbor::isLSAOnRequestList(LsaKeyType lsaKey) const
{
    for (auto lsaHeader : linkStateRequestList) {
        if ((lsaHeader->getLinkStateID() == lsaKey.linkStateID) &&
            (lsaHeader->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

OspfLsaHeader *Neighbor::findOnRequestList(LsaKeyType lsaKey)
{
    for (auto & elem : linkStateRequestList) {
        if (((elem)->getLinkStateID() == lsaKey.linkStateID) &&
            ((elem)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return elem;
        }
    }
    return nullptr;
}

void Neighbor::startRequestRetransmissionTimer()
{
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->startTimer(requestRetransmissionTimer, parentInterface->getRetransmissionInterval());
    requestRetransmissionTimerActive = true;
}

void Neighbor::clearRequestRetransmissionTimer()
{
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->clearTimer(requestRetransmissionTimer);
    requestRetransmissionTimerActive = false;
}

void Neighbor::addToTransmittedLSAList(LsaKeyType lsaKey)
{
    TransmittedLsa transmit;

    transmit.lsaKey = lsaKey;
    transmit.age = 0;

    transmittedLSAs.push_back(transmit);
}

bool Neighbor::isOnTransmittedLSAList(LsaKeyType lsaKey) const
{
    for (const auto & elem : transmittedLSAs) {
        if ((elem.lsaKey.linkStateID == lsaKey.linkStateID) &&
            (elem.lsaKey.advertisingRouter == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

void Neighbor::ageTransmittedLSAList()
{
    auto it = transmittedLSAs.begin();
    while ((it != transmittedLSAs.end()) && (it->age == MIN_LS_ARRIVAL)) {
        transmittedLSAs.pop_front();
        it = transmittedLSAs.begin();
    }
    for (it = transmittedLSAs.begin(); it != transmittedLSAs.end(); it++) {
        it->age++;
    }
}

void Neighbor::retransmitUpdatePacket()
{
    const auto& updatePacket = makeShared<OspfLinkStateUpdatePacket>();

    updatePacket->setType(LINKSTATE_UPDATE_PACKET);
    updatePacket->setRouterID(Ipv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    updatePacket->setAreaID(Ipv4Address(parentInterface->getArea()->getAreaID()));
    updatePacket->setAuthenticationType(parentInterface->getAuthenticationType());
    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        updatePacket->setAuthentication(i, authKey.bytes[i]);
    }

    bool packetFull = false;
    unsigned short lsaCount = 0;
    B packetLength = IPv4_MAX_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH;
    auto it = linkStateRetransmissionList.begin();

    while (!packetFull && (it != linkStateRetransmissionList.end())) {
        LsaType lsaType = static_cast<LsaType>((*it)->getHeader().getLsType());
        OspfRouterLsa *routerLSA = (lsaType == ROUTERLSA_TYPE) ? dynamic_cast<OspfRouterLsa *>(*it) : nullptr;
        OspfNetworkLsa *networkLSA = (lsaType == NETWORKLSA_TYPE) ? dynamic_cast<OspfNetworkLsa *>(*it) : nullptr;
        OspfSummaryLsa *summaryLSA = ((lsaType == SUMMARYLSA_NETWORKS_TYPE) ||
                                      (lsaType == SUMMARYLSA_ASBOUNDARYROUTERS_TYPE)) ? dynamic_cast<OspfSummaryLsa *>(*it) : nullptr;
        OspfAsExternalLsa *asExternalLSA = (lsaType == AS_EXTERNAL_LSA_TYPE) ? dynamic_cast<OspfAsExternalLsa *>(*it) : nullptr;
        B lsaSize = B(0);
        bool includeLSA = false;

        switch (lsaType) {
            case ROUTERLSA_TYPE:
                if (routerLSA != nullptr) {
                    lsaSize = calculateLSASize(routerLSA);
                }
                break;

            case NETWORKLSA_TYPE:
                if (networkLSA != nullptr) {
                    lsaSize = calculateLSASize(networkLSA);
                }
                break;

            case SUMMARYLSA_NETWORKS_TYPE:
            case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
                if (summaryLSA != nullptr) {
                    lsaSize = calculateLSASize(summaryLSA);
                }
                break;

            case AS_EXTERNAL_LSA_TYPE:
                if (asExternalLSA != nullptr) {
                    lsaSize = calculateLSASize(asExternalLSA);
                }
                break;

            default:
                break;
        }

        if (packetLength + lsaSize < B(parentInterface->getMtu())) {
            includeLSA = true;
            lsaCount++;
        }
        else {
            if ((lsaCount == 0) && (packetLength + lsaSize < IPV4_DATAGRAM_LENGTH)) {
                includeLSA = true;
                lsaCount++;
                packetFull = true;
            }
        }

        if (includeLSA) {
            packetLength += lsaSize;
            switch (lsaType) {
                case ROUTERLSA_TYPE:
                    if (routerLSA != nullptr) {
                        unsigned int routerLSACount = updatePacket->getRouterLSAsArraySize();

                        updatePacket->setRouterLSAsArraySize(routerLSACount + 1);
                        updatePacket->setRouterLSAs(routerLSACount, *routerLSA);

                        unsigned short lsAge = updatePacket->getRouterLSAs(routerLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                            updatePacket->getRouterLSAsForUpdate(routerLSACount).getHeaderForUpdate().setLsAge(lsAge + parentInterface->getTransmissionDelay());
                        }
                        else {
                            updatePacket->getRouterLSAsForUpdate(routerLSACount).getHeaderForUpdate().setLsAge(MAX_AGE);
                        }
                    }
                    break;

                case NETWORKLSA_TYPE:
                    if (networkLSA != nullptr) {
                        unsigned int networkLSACount = updatePacket->getNetworkLSAsArraySize();

                        updatePacket->setNetworkLSAsArraySize(networkLSACount + 1);
                        updatePacket->setNetworkLSAs(networkLSACount, *networkLSA);

                        unsigned short lsAge = updatePacket->getNetworkLSAs(networkLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                            updatePacket->getNetworkLSAsForUpdate(networkLSACount).getHeaderForUpdate().setLsAge(lsAge + parentInterface->getTransmissionDelay());
                        }
                        else {
                            updatePacket->getNetworkLSAsForUpdate(networkLSACount).getHeaderForUpdate().setLsAge(MAX_AGE);
                        }
                    }
                    break;

                case SUMMARYLSA_NETWORKS_TYPE:
                case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
                    if (summaryLSA != nullptr) {
                        unsigned int summaryLSACount = updatePacket->getSummaryLSAsArraySize();

                        updatePacket->setSummaryLSAsArraySize(summaryLSACount + 1);
                        updatePacket->setSummaryLSAs(summaryLSACount, *summaryLSA);

                        unsigned short lsAge = updatePacket->getSummaryLSAs(summaryLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                            updatePacket->getSummaryLSAsForUpdate(summaryLSACount).getHeaderForUpdate().setLsAge(lsAge + parentInterface->getTransmissionDelay());
                        }
                        else {
                            updatePacket->getSummaryLSAsForUpdate(summaryLSACount).getHeaderForUpdate().setLsAge(MAX_AGE);
                        }
                    }
                    break;

                case AS_EXTERNAL_LSA_TYPE:
                    if (asExternalLSA != nullptr) {
                        unsigned int asExternalLSACount = updatePacket->getAsExternalLSAsArraySize();

                        updatePacket->setAsExternalLSAsArraySize(asExternalLSACount + 1);
                        updatePacket->setAsExternalLSAs(asExternalLSACount, *asExternalLSA);

                        unsigned short lsAge = updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                            updatePacket->getAsExternalLSAsForUpdate(asExternalLSACount).getHeaderForUpdate().setLsAge(lsAge + parentInterface->getTransmissionDelay());
                        }
                        else {
                            updatePacket->getAsExternalLSAsForUpdate(asExternalLSACount).getHeaderForUpdate().setLsAge(MAX_AGE);
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        it++;
    }

    updatePacket->setChunkLength(packetLength - IPv4_MAX_HEADER_LENGTH);
    Packet *pk = new Packet();
    pk->insertAtBack(updatePacket);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
    messageHandler->sendPacket(pk, neighborIPAddress, parentInterface->getIfIndex(), ttl);
}

void Neighbor::deleteLastSentDDPacket()
{
    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}

} // namespace ospf

} // namespace inet

