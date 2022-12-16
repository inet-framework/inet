//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/neighbor/Ospfv2Neighbor.h"

#include <memory.h>

#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/routing/ospfv2/Ospfv2Crc.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborState.h"
#include "inet/routing/ospfv2/neighbor/Ospfv2NeighborStateDown.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {
namespace ospfv2 {

using namespace ospf;

Neighbor::Neighbor(RouterId neighbor) :
    neighborID(neighbor),
    neighborIPAddress(NULL_IPV4ADDRESS),
    neighborsDesignatedRouter(NULL_DESIGNATEDROUTERID),
    neighborsBackupDesignatedRouter(NULL_DESIGNATEDROUTERID),
    neighborsRouterDeadInterval(40)
{
    Neighbor::DdPacketId emptyDD;
    lastReceivedDDPacket = emptyDD;
    // setting only I and M bits is invalid -> good initializer
    lastReceivedDDPacket.ddOptions.I_Init = true;
    lastReceivedDDPacket.ddOptions.M_More = true;
    inactivityTimer = new cMessage("Neighbor::NeighborInactivityTimer", NEIGHBOR_INACTIVITY_TIMER);
    inactivityTimer->setContextPointer(this);
    pollTimer = new cMessage("Neighbor::NeighborPollTimer", NEIGHBOR_POLL_TIMER);
    pollTimer->setContextPointer(this);
    ddRetransmissionTimer = new cMessage("Neighbor::NeighborDDRetransmissionTimer", NEIGHBOR_DD_RETRANSMISSION_TIMER);
    ddRetransmissionTimer->setContextPointer(this);
    updateRetransmissionTimer = new cMessage("Neighbor::Neighbor::NeighborUpdateRetransmissionTimer", NEIGHBOR_UPDATE_RETRANSMISSION_TIMER);
    updateRetransmissionTimer->setContextPointer(this);
    requestRetransmissionTimer = new cMessage("Neighbor::NeighborRequestRetransmissionTimer", NEIGHBOR_REQUEST_RETRANSMISSION_TIMER);
    requestRetransmissionTimer->setContextPointer(this);
    state = new NeighborStateDown;
    previousState = nullptr;
}

Neighbor::~Neighbor()
{
    reset();
    if (parentInterface && parentInterface->getArea()) {
        MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(inactivityTimer);
        messageHandler->clearTimer(pollTimer);
    }
    delete inactivityTimer;
    delete pollTimer;
    delete ddRetransmissionTimer;
    delete updateRetransmissionTimer;
    delete requestRetransmissionTimer;
    if (previousState)
        delete previousState;
    delete state;
}

void Neighbor::changeState(NeighborState *newState, NeighborState *currentState)
{
    EV_INFO << "Changing neighborhood state of " << this->getNeighborID().str(false)
            << " from '" << getStateString(currentState->getState())
            << "' to '" << getStateString(newState->getState()) << "'" << std::endl;

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
    for (auto& elem : linkStateRetransmissionList)
        delete elem;
    linkStateRetransmissionList.clear();

    for (auto& elem : databaseSummaryList)
        delete elem;
    databaseSummaryList.clear();

    for (auto& elem : linkStateRequestList)
        delete elem;
    linkStateRequestList.clear();

    if (parentInterface && parentInterface->getArea())
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
    const auto& ddPacket = makeShared<Ospfv2DatabaseDescriptionPacket>();
    auto crcMode = parentInterface->getCrcMode();

    ddPacket->setType(DATABASE_DESCRIPTION_PACKET);
    ddPacket->setRouterID(Ipv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    ddPacket->setAreaID(Ipv4Address(parentInterface->getArea()->getAreaID()));
    ddPacket->setAuthenticationType(parentInterface->getAuthenticationType());

    if (parentInterface->getType() != Ospfv2Interface::VIRTUAL) {
        ddPacket->setInterfaceMTU(parentInterface->getMtu());
    }
    else {
        ddPacket->setInterfaceMTU(0);
    }

    Ospfv2Options options;
    options.E_ExternalRoutingCapability = parentInterface->getArea()->getExternalRoutingCapability();
    ddPacket->setOptions(options);

    ddPacket->setDdSequenceNumber(ddSequenceNumber);

    B maxPacketSize = (((IPv4_MAX_HEADER_LENGTH + OSPFv2_HEADER_LENGTH + OSPFv2_DD_HEADER_LENGTH + OSPFv2_LSA_HEADER_LENGTH) > B(parentInterface->getMtu())) ?
                       IPV4_DATAGRAM_LENGTH :
                       B(parentInterface->getMtu())) - IPv4_MAX_HEADER_LENGTH;
    B packetSize = OSPFv2_HEADER_LENGTH + OSPFv2_DD_HEADER_LENGTH;

    if (init || databaseSummaryList.empty()) {
        ddPacket->setLsaHeadersArraySize(0);
    }
    else {
        // delete included LSAs from summary list
        // (they are still in lastTransmittedDDPacket)
        while ((!databaseSummaryList.empty()) && (packetSize <= (maxPacketSize - OSPFv2_LSA_HEADER_LENGTH))) {
            Ospfv2LsaHeader *lsaHeader = *(databaseSummaryList.begin());
            setLsaHeaderCrc(*lsaHeader, crcMode);
            ddPacket->appendLsaHeaders(*lsaHeader);
            delete lsaHeader;
            databaseSummaryList.pop_front();
            packetSize += OSPFv2_LSA_HEADER_LENGTH;
        }
    }

    Ospfv2DdOptions ddOptions;
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

    ddPacket->setPacketLengthField(B(packetSize).get());
    ddPacket->setChunkLength(packetSize);

    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        ddPacket->setAuthentication(i, authKey.bytes[i]);
    }

    setOspfCrc(ddPacket, crcMode);

    Packet *pk = new Packet();
    pk->insertAtBack(ddPacket);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

    if (lastTransmittedDDPacket != nullptr)
        delete lastTransmittedDDPacket;
    lastTransmittedDDPacket = pk->dup();

    if (parentInterface->getType() == Ospfv2Interface::POINTTOPOINT) {
        messageHandler->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface, ttl);
    }
    else {
        messageHandler->sendPacket(pk, neighborIPAddress, parentInterface, ttl);
    }
}

bool Neighbor::retransmitDatabaseDescriptionPacket()
{
    if (lastTransmittedDDPacket != nullptr) {
        Packet *ddPacket = new Packet(*lastTransmittedDDPacket);
        MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
        int ttl = (parentInterface->getType() == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        if (parentInterface->getType() == Ospfv2Interface::POINTTOPOINT) {
            messageHandler->sendPacket(ddPacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface, ttl);
        }
        else {
            messageHandler->sendPacket(ddPacket, neighborIPAddress, parentInterface, ttl);
        }

        return true;
    }
    else {
        return false;
    }
}

void Neighbor::createDatabaseSummary()
{
    Ospfv2Area *area = parentInterface->getArea();
    unsigned long routerLSACount = area->getRouterLSACount();

    /* Note: OSPF specification says:
     * "LSAs whose age is equal to MaxAge are instead added to the neighbor's
     *  Link state retransmission list."
     * But this task has been already done during the aging of the database. (???)
     * So we'll skip this.
     */
    for (unsigned long i = 0; i < routerLSACount; i++) {
        if (area->getRouterLSA(i)->getHeader().getLsAge() < MAX_AGE) {
            Ospfv2LsaHeader *routerLSA = new Ospfv2LsaHeader(area->getRouterLSA(i)->getHeader());
            databaseSummaryList.push_back(routerLSA);
        }
    }

    unsigned long networkLSACount = area->getNetworkLSACount();
    for (unsigned long j = 0; j < networkLSACount; j++) {
        if (area->getNetworkLSA(j)->getHeader().getLsAge() < MAX_AGE) {
            Ospfv2LsaHeader *networkLSA = new Ospfv2LsaHeader(area->getNetworkLSA(j)->getHeader());
            databaseSummaryList.push_back(networkLSA);
        }
    }

    unsigned long summaryLSACount = area->getSummaryLSACount();
    for (unsigned long k = 0; k < summaryLSACount; k++) {
        if (area->getSummaryLSA(k)->getHeader().getLsAge() < MAX_AGE) {
            Ospfv2LsaHeader *summaryLSA = new Ospfv2LsaHeader(area->getSummaryLSA(k)->getHeader());
            databaseSummaryList.push_back(summaryLSA);
        }
    }

    if ((parentInterface->getType() != Ospfv2Interface::VIRTUAL) &&
        (area->getExternalRoutingCapability()))
    {
        Router *router = area->getRouter();
        unsigned long asExternalLSACount = router->getASExternalLSACount();

        for (unsigned long m = 0; m < asExternalLSACount; m++) {
            if (router->getASExternalLSA(m)->getHeader().getLsAge() < MAX_AGE) {
                Ospfv2LsaHeader *asExternalLSA = new Ospfv2LsaHeader(router->getASExternalLSA(m)->getHeader());
                databaseSummaryList.push_back(asExternalLSA);
            }
        }
    }
}

void Neighbor::sendLinkStateRequestPacket()
{
    const auto& requestPacket = makeShared<Ospfv2LinkStateRequestPacket>();

    requestPacket->setType(LINKSTATE_REQUEST_PACKET);
    requestPacket->setRouterID(Ipv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    requestPacket->setAreaID(Ipv4Address(parentInterface->getArea()->getAreaID()));
    requestPacket->setAuthenticationType(parentInterface->getAuthenticationType());

    B maxPacketSize = ((IPv4_MAX_HEADER_LENGTH + OSPFv2_HEADER_LENGTH + OSPFv2_REQUEST_LENGTH) > B(parentInterface->getMtu())) ?
                          IPV4_DATAGRAM_LENGTH :
                          B(parentInterface->getMtu()) - IPv4_MAX_HEADER_LENGTH;
    B packetSize = OSPFv2_HEADER_LENGTH;

    if (linkStateRequestList.empty()) {
        requestPacket->setRequestsArraySize(0);
    }
    else {
        auto it = linkStateRequestList.begin();

        while ((it != linkStateRequestList.end()) && (packetSize <= (maxPacketSize - OSPFv2_REQUEST_LENGTH))) {
            unsigned long requestCount = requestPacket->getRequestsArraySize();
            Ospfv2LsaHeader *requestHeader = (*it);
            Ospfv2LsaRequest request;

            request.lsType = requestHeader->getLsType();
            request.linkStateID = requestHeader->getLinkStateID();
            request.advertisingRouter = requestHeader->getAdvertisingRouter();

            requestPacket->setRequestsArraySize(requestCount + 1);
            requestPacket->setRequests(requestCount, request);

            packetSize += OSPFv2_REQUEST_LENGTH;
            it++;
        }
    }

    requestPacket->setPacketLengthField(B(packetSize).get());
    requestPacket->setChunkLength(packetSize);

    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        requestPacket->setAuthentication(i, authKey.bytes[i]);
    }

    setOspfCrc(requestPacket, parentInterface->getCrcMode());

    Packet *pk = new Packet();
    pk->insertAtBack(requestPacket);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
    if (parentInterface->getType() == Ospfv2Interface::POINTTOPOINT) {
        messageHandler->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface, ttl);
    }
    else {
        messageHandler->sendPacket(pk, neighborIPAddress, parentInterface, ttl);
    }
}

bool Neighbor::needAdjacency()
{
    Ospfv2Interface::Ospfv2InterfaceType interfaceType = parentInterface->getType();
    RouterId routerID = parentInterface->getArea()->getRouter()->getRouterID();
    DesignatedRouterId dRouter = parentInterface->getDesignatedRouter();
    DesignatedRouterId backupDRouter = parentInterface->getBackupDesignatedRouter();

    if ((interfaceType == Ospfv2Interface::POINTTOPOINT) ||
        (interfaceType == Ospfv2Interface::POINTTOMULTIPOINT) ||
        (interfaceType == Ospfv2Interface::VIRTUAL) ||
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
void Neighbor::addToRetransmissionList(const Ospfv2Lsa *lsa)
{
    auto it = linkStateRetransmissionList.begin();
    for (; it != linkStateRetransmissionList.end(); it++) {
        if (((*it)->getHeader().getLinkStateID() == lsa->getHeader().getLinkStateID()) &&
            ((*it)->getHeader().getAdvertisingRouter().getInt() == lsa->getHeader().getAdvertisingRouter().getInt()))
        {
            break;
        }
    }

    Ospfv2Lsa *lsaCopy = nullptr;
    switch (lsa->getHeader().getLsType()) {
        case ROUTERLSA_TYPE:
            lsaCopy = new Ospfv2RouterLsa(*(check_and_cast<const Ospfv2RouterLsa *>(lsa)));
            break;

        case NETWORKLSA_TYPE:
            lsaCopy = new Ospfv2NetworkLsa(*(check_and_cast<const Ospfv2NetworkLsa *>(lsa)));
            break;

        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
            lsaCopy = new Ospfv2SummaryLsa(*(check_and_cast<const Ospfv2SummaryLsa *>(lsa)));
            break;

        case AS_EXTERNAL_LSA_TYPE:
            lsaCopy = new Ospfv2AsExternalLsa(*(check_and_cast<const Ospfv2AsExternalLsa *>(lsa)));
            break;

        default:
            ASSERT(false); // error
            break;
    }

    if (it != linkStateRetransmissionList.end()) {
        delete *it;
        *it = static_cast<Ospfv2Lsa *>(lsaCopy);
    }
    else {
        linkStateRetransmissionList.push_back(static_cast<Ospfv2Lsa *>(lsaCopy));
    }
}

void Neighbor::removeFromRetransmissionList(LsaKeyType lsaKey)
{
    auto it = linkStateRetransmissionList.begin();
    while (it != linkStateRetransmissionList.end()) {
        if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete *it;
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

Ospfv2Lsa *Neighbor::findOnRetransmissionList(LsaKeyType lsaKey)
{
    for (auto& elem : linkStateRetransmissionList) {
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
    if (parentInterface && parentInterface->getArea()) {
        MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(updateRetransmissionTimer);
    }
    updateRetransmissionTimerActive = false;
}

void Neighbor::addToRequestList(const Ospfv2LsaHeader *lsaHeader)
{
    linkStateRequestList.push_back(new Ospfv2LsaHeader(*lsaHeader));
}

void Neighbor::removeFromRequestList(LsaKeyType lsaKey)
{
    auto it = linkStateRequestList.begin();
    while (it != linkStateRequestList.end()) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete *it;
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

Ospfv2LsaHeader *Neighbor::findOnRequestList(LsaKeyType lsaKey)
{
    for (auto& elem : linkStateRequestList) {
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
    if (parentInterface && parentInterface->getArea()) {
        MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
        messageHandler->clearTimer(requestRetransmissionTimer);
    }
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
    for (const auto& elem : transmittedLSAs) {
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
    const auto& updatePacket = makeShared<Ospfv2LinkStateUpdatePacket>();

    updatePacket->setType(LINKSTATE_UPDATE_PACKET);
    updatePacket->setRouterID(Ipv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    updatePacket->setAreaID(Ipv4Address(parentInterface->getArea()->getAreaID()));
    updatePacket->setAuthenticationType(parentInterface->getAuthenticationType());

    bool packetFull = false;
    unsigned short lsaCount = 0;
    B packetLength = IPv4_MAX_HEADER_LENGTH + OSPFv2_HEADER_LENGTH + B(4);
    auto it = linkStateRetransmissionList.begin();

    while (!packetFull && (it != linkStateRetransmissionList.end())) {
        Ospfv2Lsa *ospfLsa = *it;
        B lsaSize = B(0);
        bool includeLSA = false;

        if (ospfLsa != nullptr) {
            lsaSize = calculateLSASize(ospfLsa);
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
            unsigned int ospfLSACount = updatePacket->getOspfLSAsArraySize();
            setLsaCrc(*ospfLsa, parentInterface->getCrcMode());

            updatePacket->setOspfLSAsArraySize(ospfLSACount + 1);
            updatePacket->setOspfLSAs(ospfLSACount, ospfLsa->dup());

            unsigned short lsAge = updatePacket->getOspfLSAs(ospfLSACount)->getHeader().getLsAge();
            if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                lsAge += parentInterface->getTransmissionDelay();
            }
            else {
                lsAge = MAX_AGE;
            }
            updatePacket->getOspfLSAsForUpdate(ospfLSACount)->getHeaderForUpdate().setLsAge(lsAge);
        }
        it++;
    }

    updatePacket->setPacketLengthField(B(packetLength - IPv4_MAX_HEADER_LENGTH).get());
    updatePacket->setChunkLength(B(updatePacket->getPacketLengthField()));

    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        updatePacket->setAuthentication(i, authKey.bytes[i]);
    }

    setOspfCrc(updatePacket, parentInterface->getCrcMode());

    Packet *pk = new Packet();
    pk->insertAtBack(updatePacket);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
    messageHandler->sendPacket(pk, neighborIPAddress, parentInterface, ttl);
}

void Neighbor::deleteLastSentDDPacket()
{
    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}

} // namespace ospfv2
} // namespace inet

