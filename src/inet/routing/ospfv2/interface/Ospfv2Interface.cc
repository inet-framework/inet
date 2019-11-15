//
// Copyright (C) 2006 Andras Babos and Andras Varga
// Copyright (C) 2019 OpenSim Ltd.
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
#include <vector>

#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/ospfv2/Ospfv2Crc.h"
#include "inet/routing/ospfv2/Ospfv2PacketSerializer.h"
#include "inet/routing/ospfv2/interface/Ospfv2Interface.h"
#include "inet/routing/ospfv2/interface/Ospfv2InterfaceStateDown.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/Ospfv2Area.h"
#include "inet/routing/ospfv2/router/Ospfv2Router.h"

namespace inet {

namespace ospfv2 {

using namespace ospf;

Ospfv2Interface::Ospfv2Interface(Ospfv2Interface::Ospfv2InterfaceType ifType) :
    interfaceType(ifType),
    interfaceMode(ACTIVE),
    crcMode(CRC_MODE_UNDEFINED),
    interfaceName(""),
    ifIndex(0),
    mtu(0),
    interfaceAddressRange(NULL_IPV4ADDRESSRANGE),
    areaID(BACKBONE_AREAID),
    transitAreaID(BACKBONE_AREAID),
    helloInterval(10),
    pollInterval(120),
    routerDeadInterval(40),
    interfaceTransmissionDelay(1),
    routerPriority(0),
    designatedRouter(NULL_DESIGNATEDROUTERID),
    backupDesignatedRouter(NULL_DESIGNATEDROUTERID),
    interfaceOutputCost(1),
    retransmissionInterval(5),
    acknowledgementDelay(1),
    authenticationType(NULL_TYPE),
    parentArea(nullptr)
{
    state = new InterfaceStateDown;
    previousState = nullptr;
    helloTimer = new cMessage("Interface::InterfaceHelloTimer", INTERFACE_HELLO_TIMER);
    helloTimer->setContextPointer(this);
    waitTimer = new cMessage("Interface::InterfaceWaitTimer", INTERFACE_WAIT_TIMER);
    waitTimer->setContextPointer(this);
    acknowledgementTimer = new cMessage("Interface::InterfaceAcknowledgementTimer", INTERFACE_ACKNOWLEDGEMENT_TIMER);
    acknowledgementTimer->setContextPointer(this);
    memset(authenticationKey.bytes, 0, 8 * sizeof(char));
}

Ospfv2Interface::~Ospfv2Interface()
{
    if(parentArea) {
        MessageHandler *messageHandler = parentArea->getRouter()->getMessageHandler();
        messageHandler->clearTimer(helloTimer);
        messageHandler->clearTimer(waitTimer);
        messageHandler->clearTimer(acknowledgementTimer);
    }
    delete helloTimer;
    delete waitTimer;
    delete acknowledgementTimer;
    if (previousState)
        delete previousState;
    delete state;
    for (uint32_t i = 0; i < neighboringRouters.size(); i++)
        delete neighboringRouters[i];
}

const char *Ospfv2Interface::getTypeString(Ospfv2InterfaceType intfType)
{
    switch (intfType) {
        case UNKNOWN_TYPE:
            return "Unknown";

        case POINTTOPOINT:
            return "PointToPoint";

        case BROADCAST:
            return "Broadcast";

        case NBMA:
            return "NBMA";

        case POINTTOMULTIPOINT:
            return "PointToMultiPoint";

        case VIRTUAL:
            return "Virtual";

        default:
            ASSERT(false);
            break;
    }
    return "";
}

const char *Ospfv2Interface::getModeString(Ospfv2InterfaceMode intfMode)
{
    switch (intfMode) {
        case ACTIVE:
            return "Active";

        case PASSIVE:
            return "Passive";

        case NO_OSPF:
            return "NoOSPF";

        default:
            ASSERT(false);
            break;
    }
    return "";
}

void Ospfv2Interface::setIfIndex(IInterfaceTable *ift, int index)
{
    ifIndex = index;
    if (interfaceType == Ospfv2Interface::UNKNOWN_TYPE) {
        InterfaceEntry *routingInterface = ift->getInterfaceById(ifIndex);
        interfaceAddressRange.address = routingInterface->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
        interfaceAddressRange.mask = routingInterface->getProtocolData<Ipv4InterfaceData>()->getNetmask();
        mtu = routingInterface->getMtu();
    }
}

void Ospfv2Interface::changeState(Ospfv2InterfaceState *newState, Ospfv2InterfaceState *currentState)
{
    EV_INFO << "Changing the state of interface " << this->getIfIndex() <<
            " from '" << getStateString(currentState->getState()) <<
            "' to '" << getStateString(newState->getState()) << "'" << std::endl;

    if (previousState != nullptr) {
        delete previousState;
    }
    state = newState;
    previousState = currentState;
}

void Ospfv2Interface::processEvent(Ospfv2Interface::Ospfv2InterfaceEventType event)
{
    state->processEvent(this, event);
}

void Ospfv2Interface::reset()
{
    MessageHandler *messageHandler = parentArea->getRouter()->getMessageHandler();
    messageHandler->clearTimer(helloTimer);
    messageHandler->clearTimer(waitTimer);
    messageHandler->clearTimer(acknowledgementTimer);
    designatedRouter = NULL_DESIGNATEDROUTERID;
    backupDesignatedRouter = NULL_DESIGNATEDROUTERID;
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        neighboringRouters[i]->processEvent(Neighbor::KILL_NEIGHBOR);
    }
}

void Ospfv2Interface::sendHelloPacket(Ipv4Address destination, short ttl)
{
    Ospfv2Options options;
    const auto& helloPacket = makeShared<Ospfv2HelloPacket>();
    std::vector<Ipv4Address> neighbors;

    helloPacket->setRouterID(Ipv4Address(parentArea->getRouter()->getRouterID()));
    helloPacket->setAreaID(Ipv4Address(parentArea->getAreaID()));
    helloPacket->setAuthenticationType(authenticationType);

    if (((interfaceType == POINTTOPOINT) &&
         (interfaceAddressRange.address == NULL_IPV4ADDRESS)) ||
        (interfaceType == VIRTUAL))
    {
        helloPacket->setNetworkMask(NULL_IPV4ADDRESS);
    }
    else {
        helloPacket->setNetworkMask(interfaceAddressRange.mask);
    }
    memset(&options, 0, sizeof(Ospfv2Options));
    options.E_ExternalRoutingCapability = parentArea->getExternalRoutingCapability();
    helloPacket->setOptions(options);
    helloPacket->setHelloInterval(helloInterval);
    helloPacket->setRouterPriority(routerPriority);
    helloPacket->setRouterDeadInterval(routerDeadInterval);
    helloPacket->setDesignatedRouter(designatedRouter.ipInterfaceAddress);
    helloPacket->setBackupDesignatedRouter(backupDesignatedRouter.ipInterfaceAddress);
    long neighborCount = neighboringRouters.size();
    for (long j = 0; j < neighborCount; j++) {
        if (neighboringRouters[j]->getState() >= Neighbor::INIT_STATE) {
            neighbors.push_back(neighboringRouters[j]->getAddress());
        }
    }
    unsigned int initedNeighborCount = neighbors.size();
    helloPacket->setNeighborArraySize(initedNeighborCount);
    for (unsigned int k = 0; k < initedNeighborCount; k++) {
        helloPacket->setNeighbor(k, neighbors[k]);
    }

    helloPacket->setPacketLengthField(B(OSPFv2_HEADER_LENGTH + OSPFv2_HELLO_HEADER_LENGTH + B(initedNeighborCount * 4)).get());
    helloPacket->setChunkLength(B(helloPacket->getPacketLengthField()));

    for (int i = 0; i < 8; i++) {
        helloPacket->setAuthentication(i, authenticationKey.bytes[i]);
    }

    setOspfCrc(helloPacket, crcMode);

    Packet *pk = new Packet();
    pk->insertAtBack(helloPacket);

    parentArea->getRouter()->getMessageHandler()->sendPacket(pk, destination, this, ttl);
}

void Ospfv2Interface::sendLsAcknowledgement(const Ospfv2LsaHeader *lsaHeader, Ipv4Address destination)
{
    Ospfv2Options options;
    const auto& lsAckPacket = makeShared<Ospfv2LinkStateAcknowledgementPacket>();

    lsAckPacket->setType(LINKSTATE_ACKNOWLEDGEMENT_PACKET);
    lsAckPacket->setRouterID(Ipv4Address(parentArea->getRouter()->getRouterID()));
    lsAckPacket->setAreaID(Ipv4Address(parentArea->getAreaID()));
    lsAckPacket->setAuthenticationType(authenticationType);

    lsAckPacket->setLsaHeadersArraySize(1);
    lsAckPacket->setLsaHeaders(0, *lsaHeader);

    lsAckPacket->setPacketLengthField(B(OSPFv2_HEADER_LENGTH + OSPFv2_LSA_HEADER_LENGTH).get());
    lsAckPacket->setChunkLength(B(lsAckPacket->getPacketLengthField()));

    for (int i = 0; i < 8; i++) {
        lsAckPacket->setAuthentication(i, authenticationKey.bytes[i]);
    }

    setOspfCrc(lsAckPacket, crcMode);

    Packet *pk = new Packet();
    pk->insertAtBack(lsAckPacket);

    int ttl = (interfaceType == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
    parentArea->getRouter()->getMessageHandler()->sendPacket(pk, destination, this, ttl);
}

Neighbor *Ospfv2Interface::getNeighborById(RouterId neighborID)
{
    auto neighborIt = neighboringRoutersByID.find(neighborID);
    if (neighborIt != neighboringRoutersByID.end())
        return neighborIt->second;
    else
        return nullptr;
}

Neighbor *Ospfv2Interface::getNeighborByAddress(Ipv4Address address)
{
    auto neighborIt = neighboringRoutersByAddress.find(address);
    if (neighborIt != neighboringRoutersByAddress.end())
        return neighborIt->second;
    else
        return nullptr;
}

void Ospfv2Interface::addNeighbor(Neighbor *neighbor)
{
    neighboringRoutersByID[neighbor->getNeighborID()] = neighbor;
    neighboringRoutersByAddress[neighbor->getAddress()] = neighbor;
    neighbor->setInterface(this);
    neighboringRouters.push_back(neighbor);
}

Ospfv2Interface::Ospfv2InterfaceStateType Ospfv2Interface::getState() const
{
    return state->getState();
}

const char *Ospfv2Interface::getStateString(Ospfv2Interface::Ospfv2InterfaceStateType stateType)
{
    switch (stateType) {
        case DOWN_STATE:
            return "Down";

        case LOOPBACK_STATE:
            return "Loopback";

        case WAITING_STATE:
            return "Waiting";

        case POINTTOPOINT_STATE:
            return "PointToPoint";

        case NOT_DESIGNATED_ROUTER_STATE:
            return "NotDesignatedRouter";

        case BACKUP_STATE:
            return "Backup";

        case DESIGNATED_ROUTER_STATE:
            return "DesignatedRouter";

        default:
            ASSERT(false);
            break;
    }
    return "";
}

bool Ospfv2Interface::hasAnyNeighborInStates(int states) const
{
    for (uint32_t i = 0; i < neighboringRouters.size(); i++) {
        Neighbor::NeighborStateType neighborState = neighboringRouters[i]->getState();
        if (neighborState & states)
            return true;
    }
    return false;
}

void Ospfv2Interface::removeFromAllRetransmissionLists(LsaKeyType lsaKey)
{
    for (uint32_t i = 0; i < neighboringRouters.size(); i++)
        neighboringRouters[i]->removeFromRetransmissionList(lsaKey);
}

bool Ospfv2Interface::isOnAnyRetransmissionList(LsaKeyType lsaKey) const
{
    for (uint32_t i = 0; i < neighboringRouters.size(); i++) {
        if (neighboringRouters[i]->isLinkStateRequestListEmpty(lsaKey))
            return true;
    }
    return false;
}

/**
 * @see RFC2328 Section 13.3.
 */
bool Ospfv2Interface::floodLsa(const Ospfv2Lsa *lsa, Ospfv2Interface *intf, Neighbor *neighbor)
{
    bool floodedBackOut = false;

    if (
        (
            (lsa->getHeader().getLsType() == AS_EXTERNAL_LSA_TYPE) &&
            (interfaceType != Ospfv2Interface::VIRTUAL) &&
            (parentArea->getExternalRoutingCapability())
        ) ||
        (
            (lsa->getHeader().getLsType() != AS_EXTERNAL_LSA_TYPE) &&
            (
                (
                    (areaID != BACKBONE_AREAID) &&
                    (interfaceType != Ospfv2Interface::VIRTUAL)
                ) ||
                (areaID == BACKBONE_AREAID)
            )
        )
        )
    {
        long neighborCount = neighboringRouters.size();
        bool lsaAddedToRetransmissionList = false;
        LinkStateId linkStateID = lsa->getHeader().getLinkStateID();
        LsaKeyType lsaKey;

        lsaKey.linkStateID = linkStateID;
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        for (long i = 0; i < neighborCount; i++) {    // (1)
            if (neighboringRouters[i]->getState() < Neighbor::EXCHANGE_STATE) {    // (1) (a)
                continue;
            }
            if (neighboringRouters[i]->getState() < Neighbor::FULL_STATE) {    // (1) (b)
                Ospfv2LsaHeader *requestLSAHeader = neighboringRouters[i]->findOnRequestList(lsaKey);
                if (requestLSAHeader != nullptr) {
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
            if (neighbor == neighboringRouters[i]) {    // (1) (c)
                continue;
            }
            neighboringRouters[i]->addToRetransmissionList(lsa);    // (1) (d)
            lsaAddedToRetransmissionList = true;
        }
        if (lsaAddedToRetransmissionList) {    // (2)
            if ((intf != this) ||
                ((neighbor != nullptr) &&
                 (neighbor->getNeighborID() != designatedRouter.routerID) &&
                 (neighbor->getNeighborID() != backupDesignatedRouter.routerID)))    // (3)
            {
                if ((intf != this) || (getState() != Ospfv2Interface::BACKUP_STATE)) {    // (4)
                    Packet *updatePacket = createUpdatePacket(lsa);    // (5)

                    if (updatePacket != nullptr) {
                        int ttl = (interfaceType == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
                        MessageHandler *messageHandler = parentArea->getRouter()->getMessageHandler();

                        if (interfaceType == Ospfv2Interface::BROADCAST) {
                            if ((getState() == Ospfv2Interface::DESIGNATED_ROUTER_STATE) ||
                                (getState() == Ospfv2Interface::BACKUP_STATE) ||
                                (designatedRouter == NULL_DESIGNATEDROUTERID))
                            {
                                messageHandler->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, this, ttl);
                                for (long k = 0; k < neighborCount; k++) {
                                    neighboringRouters[k]->addToTransmittedLSAList(lsaKey);
                                    if (!neighboringRouters[k]->isUpdateRetransmissionTimerActive()) {
                                        neighboringRouters[k]->startUpdateRetransmissionTimer();
                                    }
                                }
                            }
                            else {
                                messageHandler->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, this, ttl);
                                Neighbor *dRouter = getNeighborById(designatedRouter.routerID);
                                Neighbor *backupDRouter = getNeighborById(backupDesignatedRouter.routerID);
                                if (dRouter != nullptr) {
                                    dRouter->addToTransmittedLSAList(lsaKey);
                                    if (!dRouter->isUpdateRetransmissionTimerActive()) {
                                        dRouter->startUpdateRetransmissionTimer();
                                    }
                                }
                                if (backupDRouter != nullptr) {
                                    backupDRouter->addToTransmittedLSAList(lsaKey);
                                    if (!backupDRouter->isUpdateRetransmissionTimerActive()) {
                                        backupDRouter->startUpdateRetransmissionTimer();
                                    }
                                }
                            }
                        }
                        else {
                            if (interfaceType == Ospfv2Interface::POINTTOPOINT) {
                                messageHandler->sendPacket(updatePacket, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, this, ttl);
                                if (neighborCount > 0) {
                                    neighboringRouters[0]->addToTransmittedLSAList(lsaKey);
                                    if (!neighboringRouters[0]->isUpdateRetransmissionTimerActive()) {
                                        neighboringRouters[0]->startUpdateRetransmissionTimer();
                                    }
                                }
                            }
                            else {
                                for (long m = 0; m < neighborCount; m++) {
                                    if (neighboringRouters[m]->getState() >= Neighbor::EXCHANGE_STATE) {
                                        messageHandler->sendPacket(updatePacket, neighboringRouters[m]->getAddress(), this, ttl);
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

Packet *Ospfv2Interface::createUpdatePacket(const Ospfv2Lsa *lsa)
{
    Ospfv2LsaType lsaType = lsa->getHeader().getLsType();
    const auto& updatePacket = makeShared<Ospfv2LinkStateUpdatePacket>();
    B packetLength = OSPFv2_HEADER_LENGTH + B(sizeof(uint32_t));    // OSPF header + place for number of advertisements

    updatePacket->setType(LINKSTATE_UPDATE_PACKET);
    updatePacket->setRouterID(Ipv4Address(parentArea->getRouter()->getRouterID()));
    updatePacket->setAreaID(Ipv4Address(areaID));
    updatePacket->setAuthenticationType(authenticationType);

    switch (lsaType) {
        case ROUTERLSA_TYPE:
        case NETWORKLSA_TYPE:
        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
        case AS_EXTERNAL_LSA_TYPE: {
            updatePacket->setOspfLSAsArraySize(1);
            updatePacket->setOspfLSAs(0, lsa->dup());
            auto lsa = updatePacket->getOspfLSAsForUpdate(0);
            auto& lsaHeader = lsa->getHeaderForUpdate();
            unsigned short lsAge = lsaHeader.getLsAge();
            if (lsAge < MAX_AGE - interfaceTransmissionDelay) {
                lsAge += interfaceTransmissionDelay;
            }
            else {
                lsAge = MAX_AGE;
            }
            lsaHeader.setLsAge(lsAge);
            auto lsaSize = calculateLSASize(lsa);
            ASSERT(lsaSize == B(lsaHeader.getLsaLength()));
            setLsaCrc(*lsa, crcMode);
            packetLength += lsaSize;
        }
        break;

        default:
            throw cRuntimeError("Invalid LSA type: %d", lsaType);
    }

    updatePacket->setPacketLengthField(B(packetLength).get());
    updatePacket->setChunkLength(packetLength);

    for (int j = 0; j < 8; j++) {
        updatePacket->setAuthentication(j, authenticationKey.bytes[j]);
    }

    setOspfCrc(updatePacket, crcMode);

    Packet *pk = new Packet();
    pk->insertAtBack(updatePacket);

    return pk;
}

void Ospfv2Interface::addDelayedAcknowledgement(const Ospfv2LsaHeader& lsaHeader)
{
    if (interfaceType == Ospfv2Interface::BROADCAST) {
        if ((getState() == Ospfv2Interface::DESIGNATED_ROUTER_STATE) ||
            (getState() == Ospfv2Interface::BACKUP_STATE) ||
            (designatedRouter == NULL_DESIGNATEDROUTERID))
        {
            delayedAcknowledgements[Ipv4Address::ALL_OSPF_ROUTERS_MCAST].push_back(lsaHeader);
        }
        else {
            delayedAcknowledgements[Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST].push_back(lsaHeader);
        }
    }
    else {
        long neighborCount = neighboringRouters.size();
        for (long i = 0; i < neighborCount; i++) {
            if (neighboringRouters[i]->getState() >= Neighbor::EXCHANGE_STATE) {
                delayedAcknowledgements[neighboringRouters[i]->getAddress()].push_back(lsaHeader);
            }
        }
    }
}

void Ospfv2Interface::sendDelayedAcknowledgements()
{
    MessageHandler *messageHandler = parentArea->getRouter()->getMessageHandler();
    B maxPacketSize = ((IPv4_MAX_HEADER_LENGTH + OSPFv2_HEADER_LENGTH + OSPFv2_LSA_HEADER_LENGTH) > B(mtu)) ? IPV4_DATAGRAM_LENGTH : B(mtu);

    for (auto & elem : delayedAcknowledgements)
    {
        int ackCount = elem.second.size();
        if (ackCount > 0) {
            while (!(elem.second.empty())) {
                const auto& ackPacket = makeShared<Ospfv2LinkStateAcknowledgementPacket>();
                B packetSize = IPv4_MAX_HEADER_LENGTH + OSPFv2_HEADER_LENGTH;

                ackPacket->setType(LINKSTATE_ACKNOWLEDGEMENT_PACKET);
                ackPacket->setRouterID(Ipv4Address(parentArea->getRouter()->getRouterID()));
                ackPacket->setAreaID(Ipv4Address(areaID));
                ackPacket->setAuthenticationType(authenticationType);

                while ((!(elem.second.empty())) && (packetSize <= (maxPacketSize - OSPFv2_LSA_HEADER_LENGTH))) {
                    unsigned long headerCount = ackPacket->getLsaHeadersArraySize();
                    ackPacket->setLsaHeadersArraySize(headerCount + 1);
                    ackPacket->setLsaHeaders(headerCount, *(elem.second.begin()));
                    elem.second.pop_front();
                    packetSize += OSPFv2_LSA_HEADER_LENGTH;
                }

                ackPacket->setPacketLengthField(B(packetSize - IPv4_MAX_HEADER_LENGTH).get());
                ackPacket->setChunkLength(B(ackPacket->getPacketLengthField()));

                for (int i = 0; i < 8; i++) {
                    ackPacket->setAuthentication(i, authenticationKey.bytes[i]);
                }

                setOspfCrc(ackPacket, crcMode);

                Packet *pk = new Packet();
                pk->insertAtBack(ackPacket);

                int ttl = (interfaceType == Ospfv2Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

                if (interfaceType == Ospfv2Interface::BROADCAST) {
                    if ((getState() == Ospfv2Interface::DESIGNATED_ROUTER_STATE) ||
                        (getState() == Ospfv2Interface::BACKUP_STATE) ||
                        (designatedRouter == NULL_DESIGNATEDROUTERID))
                    {
                        messageHandler->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, this, ttl);
                    }
                    else {
                        messageHandler->sendPacket(pk, Ipv4Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, this, ttl);
                    }
                }
                else {
                    if (interfaceType == Ospfv2Interface::POINTTOPOINT) {
                        messageHandler->sendPacket(pk, Ipv4Address::ALL_OSPF_ROUTERS_MCAST, this, ttl);
                    }
                    else {
                        messageHandler->sendPacket(pk, elem.first, this, ttl);
                    }
                }
            }
        }
    }
    messageHandler->startTimer(acknowledgementTimer, acknowledgementDelay);
}

void Ospfv2Interface::ageTransmittedLsaLists()
{
    long neighborCount = neighboringRouters.size();
    for (long i = 0; i < neighborCount; i++) {
        neighboringRouters[i]->ageTransmittedLSAList();
    }
}

std::ostream& operator<<(std::ostream& stream, const Ospfv2Interface& intf)
{
    std::string neighbors = "";
    for(auto &neighbor : intf.neighboringRoutersByID) {
        std::string neighborState = Neighbor::getStateString((neighbor.second)->getState());
        neighbors = neighbors + (neighbor.first).str() + "(" + neighborState + ") ";
    }

    return stream << "name: " << intf.getInterfaceName() << " "
            << "index: " << intf.ifIndex << " "
            << "type: '" << intf.getTypeString(intf.interfaceType) << "' "
            << "MTU: " << intf.mtu << " "
            << "state: '" << intf.getStateString(intf.state->getState()) << "' "
            << "mode: '" << intf.getModeString(intf.interfaceMode) << "' "
            << "cost: " << intf.interfaceOutputCost << " "

            << "area: " << intf.areaID.str(false) << " "
            << "transitArea: " << intf.transitAreaID.str(false) << " "

            << "helloInterval: " << intf.helloInterval << " "
            << "pollInterval: " << intf.pollInterval << " "
            << "routerDeadInterval: " << intf.routerDeadInterval << " "
            << "retransmissionInterval: " << intf.retransmissionInterval << " "

            << "acknowledgementDelay: " << intf.acknowledgementDelay << " "
            << "interfaceTransmissionDelay: " << intf.interfaceTransmissionDelay << " "

            << "neighboringRouters: " << ((neighbors == "") ? "<none>(down)" : neighbors) << " "

            << "routerPriority: " << (int)(intf.routerPriority) << " "
            << "designatedRouterID: " << intf.designatedRouter.routerID << " "
            << "designatedRouterInterface: " << intf.designatedRouter.ipInterfaceAddress << " "
            << "backupDesignatedRouterID: " << intf.backupDesignatedRouter.routerID << " "
            << "backupDesignatedRouterInterface: " << intf.backupDesignatedRouter.ipInterfaceAddress;
}

} // namespace ospfv2

} // namespace inet

