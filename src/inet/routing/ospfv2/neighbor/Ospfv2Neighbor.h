//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_OSPFV2NEIGHBOR_H
#define __INET_OSPFV2NEIGHBOR_H

#include <list>

#include "inet/common/packet/Packet.h"
#include "inet/routing/ospfv2/Ospfv2Packet_m.h"
#include "inet/routing/ospfv2/router/Lsa.h"
#include "inet/routing/ospfv2/router/Ospfv2Common.h"

namespace inet {

namespace ospfv2 {

class NeighborState;
class Ospfv2Interface;

class INET_API Neighbor
{
    friend class NeighborState;

  public:
    enum NeighborEventType {
        HELLO_RECEIVED               = 0,
        START                        = 1,
        TWOWAY_RECEIVED              = 2,
        NEGOTIATION_DONE             = 3,
        EXCHANGE_DONE                = 4,
        BAD_LINK_STATE_REQUEST       = 5,
        LOADING_DONE                 = 6,
        IS_ADJACENCY_OK              = 7,
        SEQUENCE_NUMBER_MISMATCH     = 8,
        ONEWAY_RECEIVED              = 9,
        KILL_NEIGHBOR                = 10,
        INACTIVITY_TIMER             = 11,
        POLL_TIMER                   = 12,
        LINK_DOWN                    = 13,
        DD_RETRANSMISSION_TIMER      = 14,
        UPDATE_RETRANSMISSION_TIMER  = 15,
        REQUEST_RETRANSMISSION_TIMER = 16
    };

    enum NeighborStateType {
        DOWN_STATE           = 0,
        ATTEMPT_STATE        = 1,
        INIT_STATE           = 2,
        TWOWAY_STATE         = 4,
        EXCHANGE_START_STATE = 8,
        EXCHANGE_STATE       = 16,
        LOADING_STATE        = 32,
        FULL_STATE           = 64
    };

    enum DatabaseExchangeRelationshipType {
        MASTER = 0,
        SLAVE  = 1
    };

    struct DdPacketId {
        Ospfv2DdOptions ddOptions;
        Ospfv2Options options;
        unsigned long sequenceNumber = 0;
    };

  private:
    struct TransmittedLsa {
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
    Ospfv2Options neighborOptions;
    DesignatedRouterId neighborsDesignatedRouter;
    DesignatedRouterId neighborsBackupDesignatedRouter;
    bool designatedRoutersSetUp = false;
    short neighborsRouterDeadInterval = 0;
    std::list<Ospfv2Lsa *> linkStateRetransmissionList;
    std::list<Ospfv2LsaHeader *> databaseSummaryList;
    std::list<Ospfv2LsaHeader *> linkStateRequestList;
    std::list<TransmittedLsa> transmittedLSAs;
    Packet *lastTransmittedDDPacket = nullptr;

    Ospfv2Interface *parentInterface = nullptr;

    // TODO Should come from a global unique number generator module.
    uint64_t& ddSequenceNumberInitSeed = SIMULATION_SHARED_COUNTER(ddSequenceNumberInitSeed);

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
    void addToRetransmissionList(const Ospfv2Lsa *lsa);
    void removeFromRetransmissionList(LsaKeyType lsaKey);
    bool isLinkStateRequestListEmpty(LsaKeyType lsaKey) const;
    Ospfv2Lsa *findOnRetransmissionList(LsaKeyType lsaKey);
    void startUpdateRetransmissionTimer();
    void clearUpdateRetransmissionTimer();
    void addToRequestList(const Ospfv2LsaHeader *lsaHeader);
    void removeFromRequestList(LsaKeyType lsaKey);
    bool isLSAOnRequestList(LsaKeyType lsaKey) const;
    Ospfv2LsaHeader *findOnRequestList(LsaKeyType lsaKey);
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
    void setOptions(Ospfv2Options options) { neighborOptions = options; }
    Ospfv2Options getOptions() const { return neighborOptions; }
    void setLastReceivedDDPacket(DdPacketId packetID) { lastReceivedDDPacket = packetID; }
    DdPacketId getLastReceivedDDPacket() const { return lastReceivedDDPacket; }

    void setDatabaseExchangeRelationship(DatabaseExchangeRelationshipType relation) { databaseExchangeRelationship = relation; }
    DatabaseExchangeRelationshipType getDatabaseExchangeRelationship() const { return databaseExchangeRelationship; }

    void setInterface(Ospfv2Interface *intf) { parentInterface = intf; }
    Ospfv2Interface *getInterface() { return parentInterface; }
    const Ospfv2Interface *getInterface() const { return parentInterface; }

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

} // namespace ospfv2

} // namespace inet

#endif

