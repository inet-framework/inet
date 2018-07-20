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

#ifndef __INET_OSPFNEIGHBOR_H
#define __INET_OSPFNEIGHBOR_H

#include <list>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/routing/ospfv2/router/Lsa.h"
#include "inet/routing/ospfv2/router/OspfCommon.h"
#include "inet/routing/ospfv2/OspfPacket_m.h"
#include "inet/routing/ospfv2/OspfTimer.h"

namespace inet {

namespace ospf {

class NeighborState;
class Interface;

class INET_API Neighbor
{
    friend class NeighborState;

  public:
    enum NeighborEventType {
        HELLO_RECEIVED = 0,
        START = 1,
        TWOWAY_RECEIVED = 2,
        NEGOTIATION_DONE = 3,
        EXCHANGE_DONE = 4,
        BAD_LINK_STATE_REQUEST = 5,
        LOADING_DONE = 6,
        IS_ADJACENCY_OK = 7,
        SEQUENCE_NUMBER_MISMATCH = 8,
        ONEWAY_RECEIVED = 9,
        KILL_NEIGHBOR = 10,
        INACTIVITY_TIMER = 11,
        POLL_TIMER = 12,
        LINK_DOWN = 13,
        DD_RETRANSMISSION_TIMER = 14,
        UPDATE_RETRANSMISSION_TIMER = 15,
        REQUEST_RETRANSMISSION_TIMER = 16
    };

    enum NeighborStateType {
        DOWN_STATE = 0,
        ATTEMPT_STATE = 1,
        INIT_STATE = 2,
        TWOWAY_STATE = 4,
        EXCHANGE_START_STATE = 8,
        EXCHANGE_STATE = 16,
        LOADING_STATE = 32,
        FULL_STATE = 64
    };

    enum DatabaseExchangeRelationshipType {
        MASTER = 0,
        SLAVE = 1
    };

    struct DdPacketId
    {
        OspfDdOptions ddOptions;
        OspfOptions options;
        unsigned long sequenceNumber;
    };

  private:
    struct TransmittedLsa
    {
        LsaKeyType lsaKey;
        unsigned short age;
    };

  private:
    NeighborState *state = nullptr;
    NeighborState *previousState = nullptr;
    cMessage *inactivityTimer = nullptr;
    cMessage *pollTimer = nullptr;
    cMessage *ddRetransmissionTimer = nullptr;
    cMessage *updateRetransmissionTimer = nullptr;
    bool updateRetransmissionTimerActive = false;
    cMessage *requestRetransmissionTimer = nullptr;
    bool requestRetransmissionTimerActive = false;
    DatabaseExchangeRelationshipType databaseExchangeRelationship = static_cast<DatabaseExchangeRelationshipType>(-1);
    bool firstAdjacencyInited = false;
    unsigned long ddSequenceNumber = 0;
    DdPacketId lastReceivedDDPacket;
    RouterId neighborID;
    unsigned char neighborPriority = 0;
    Ipv4Address neighborIPAddress;
    OspfOptions neighborOptions;
    DesignatedRouterId neighborsDesignatedRouter;
    DesignatedRouterId neighborsBackupDesignatedRouter;
    bool designatedRoutersSetUp = false;
    short neighborsRouterDeadInterval = 0;
    std::list<OspfLsa *> linkStateRetransmissionList;
    std::list<OspfLsaHeader *> databaseSummaryList;
    std::list<OspfLsaHeader *> linkStateRequestList;
    std::list<TransmittedLsa> transmittedLSAs;
    Packet *lastTransmittedDDPacket = nullptr;

    Interface *parentInterface = nullptr;

    // TODO: Should come from a global unique number generator module.
    static unsigned long ddSequenceNumberInitSeed;

  private:
    void changeState(NeighborState *newState, NeighborState *currentState);

  public:
    Neighbor(RouterId neighbor = NULL_ROUTERID);
    virtual ~Neighbor();

    void processEvent(NeighborEventType event);
    void reset();
    void initFirstAdjacency();
    NeighborStateType getState() const;
    static const char *getStateString(NeighborStateType stateType);
    void sendDatabaseDescriptionPacket(bool init = false);
    bool retransmitDatabaseDescriptionPacket();
    void createDatabaseSummary();
    void sendLinkStateRequestPacket();
    void retransmitUpdatePacket();
    bool needAdjacency();
    void addToRetransmissionList(const OspfLsa *lsa);
    void removeFromRetransmissionList(LsaKeyType lsaKey);
    bool isLinkStateRequestListEmpty(LsaKeyType lsaKey) const;
    OspfLsa *findOnRetransmissionList(LsaKeyType lsaKey);
    void startUpdateRetransmissionTimer();
    void clearUpdateRetransmissionTimer();
    void addToRequestList(const OspfLsaHeader *lsaHeader);
    void removeFromRequestList(LsaKeyType lsaKey);
    bool isLSAOnRequestList(LsaKeyType lsaKey) const;
    OspfLsaHeader *findOnRequestList(LsaKeyType lsaKey);
    void startRequestRetransmissionTimer();
    void clearRequestRetransmissionTimer();
    void addToTransmittedLSAList(LsaKeyType lsaKey);
    bool isOnTransmittedLSAList(LsaKeyType lsaKey) const;
    void ageTransmittedLSAList();
    unsigned long getUniqueULong();
    void deleteLastSentDDPacket();

    void setNeighborID(RouterId id) { neighborID = id; }
    RouterId getNeighborID() const { return neighborID; }
    void setPriority(unsigned char priority) { neighborPriority = priority; }
    unsigned char getPriority() const { return neighborPriority; }
    void setAddress(Ipv4Address address) { neighborIPAddress = address; }
    Ipv4Address getAddress() const { return neighborIPAddress; }
    void setDesignatedRouter(DesignatedRouterId routerID) { neighborsDesignatedRouter = routerID; }
    DesignatedRouterId getDesignatedRouter() const { return neighborsDesignatedRouter; }
    void setBackupDesignatedRouter(DesignatedRouterId routerID) { neighborsBackupDesignatedRouter = routerID; }
    DesignatedRouterId getBackupDesignatedRouter() const { return neighborsBackupDesignatedRouter; }
    void setRouterDeadInterval(short interval) { neighborsRouterDeadInterval = interval; }
    short getRouterDeadInterval() const { return neighborsRouterDeadInterval; }
    void setDDSequenceNumber(unsigned long sequenceNumber) { ddSequenceNumber = sequenceNumber; }
    unsigned long getDDSequenceNumber() const { return ddSequenceNumber; }
    void setOptions(OspfOptions options) { neighborOptions = options; }
    OspfOptions getOptions() const { return neighborOptions; }
    void setLastReceivedDDPacket(DdPacketId packetID) { lastReceivedDDPacket = packetID; }
    DdPacketId getLastReceivedDDPacket() const { return lastReceivedDDPacket; }

    void setDatabaseExchangeRelationship(DatabaseExchangeRelationshipType relation) { databaseExchangeRelationship = relation; }
    DatabaseExchangeRelationshipType getDatabaseExchangeRelationship() const { return databaseExchangeRelationship; }

    void setInterface(Interface *intf) { parentInterface = intf; }
    Interface *getInterface() { return parentInterface; }
    const Interface *getInterface() const { return parentInterface; }

    cMessage *getInactivityTimer() { return inactivityTimer; }
    cMessage *getPollTimer() { return pollTimer; }
    cMessage *getDDRetransmissionTimer() { return ddRetransmissionTimer; }
    cMessage *getUpdateRetransmissionTimer() { return updateRetransmissionTimer; }
    bool isUpdateRetransmissionTimerActive() const { return updateRetransmissionTimerActive; }
    bool isRequestRetransmissionTimerActive() const { return requestRetransmissionTimerActive; }
    bool isFirstAdjacencyInited() const { return firstAdjacencyInited; }
    bool designatedRoutersAreSetUp() const { return designatedRoutersSetUp; }
    void setupDesignatedRouters(bool setUp) { designatedRoutersSetUp = setUp; }
    unsigned long getDatabaseSummaryListCount() const { return databaseSummaryList.size(); }

    void incrementDDSequenceNumber() { ddSequenceNumber++; }
    bool isLinkStateRequestListEmpty() const { return linkStateRequestList.empty(); }
    bool isLinkStateRetransmissionListEmpty() const { return linkStateRetransmissionList.empty(); }
    void popFirstLinkStateRequest() { linkStateRequestList.pop_front(); }
};

inline bool operator==(Neighbor::DdPacketId leftID, Neighbor::DdPacketId rightID)
{
    return (leftID.ddOptions.I_Init == rightID.ddOptions.I_Init) &&
           (leftID.ddOptions.M_More == rightID.ddOptions.M_More) &&
           (leftID.ddOptions.MS_MasterSlave == rightID.ddOptions.MS_MasterSlave) &&
           (leftID.options == rightID.options) &&
           (leftID.sequenceNumber == rightID.sequenceNumber);
}

inline bool operator!=(Neighbor::DdPacketId leftID, Neighbor::DdPacketId rightID)
{
    return !(leftID == rightID);
}

} // namespace ospf

} // namespace inet

#endif // ifndef __INET_OSPFNEIGHBOR_H

