#ifndef __INET_OSPFV3NEIGHBOR_H_
#define __INET_OSPFV3NEIGHBOR_H_

#include <omnetpp.h>

#include "inet/routing/ospfv3/OSPFv3Common.h"
#include "inet/routing/ospfv3/OSPFv3Packet_m.h"
#include "inet/routing/ospfv3/OSPFv3Timers.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/common/packet/Packet.h"

namespace inet{

class OSPFv3NeighborState;
class OSPFv3Interface;
//struct OSPFv3DDPacketID;

class OSPFv3Neighbor{
  public:
    enum OSPFv3NeighborEventType {
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

    enum OSPFv3NeighborStateType {
        DOWN_STATE = 0,
        ATTEMPT_STATE = 1,
        INIT_STATE = 2,
        TWOWAY_STATE = 4,
        EXCHANGE_START_STATE = 8,
        EXCHANGE_STATE = 16,
        LOADING_STATE = 32,
        FULL_STATE = 64
    };

    enum OSPFv3DatabaseExchangeRelationshipType {
        MASTER = 0,
        SLAVE = 1
    };

    struct TransmittedLSA{
        LSAKeyType lsaKey;
        int age;
    };

    OSPFv3Neighbor(Ipv4Address newId, OSPFv3Interface* parent);
    virtual ~OSPFv3Neighbor();
    void setNeighborPriority(int newPriority){this->neighborRtrPriority=newPriority;}
    void setNeighborDeadInterval(int newInterval){this->neighborsRouterDeadInterval=newInterval;}
    void setNeighborID(Ipv4Address newId){this->neighborId=newId;}
    void setNeighborAddress(Ipv6Address newAddress){this->neighborIPAddress=newAddress;}
    void setDesignatedRouterID(Ipv4Address newId){this->neighborsDesignatedRouter=newId;}
    void setBackupDesignatedRouterID(Ipv4Address newId){this->neighborsBackupDesignatedRouter=newId;}
    void setOptions(OSPFv3Options options) { this->neighborOptions = options; }
    void setDatabaseExchangeRelationship(OSPFv3DatabaseExchangeRelationshipType newRelationship){this->databaseExchangeRelationship = newRelationship;}
    void setDDSequenceNumber(unsigned long newSequenceNmr){this->ddSequenceNumber = newSequenceNmr;}
    void setupDesignatedRouters(bool setUp) { this->designatedRoutersSetUp = setUp; }
    void setNeighborInterfaceID(uint32_t newID){this->neighborInterfaceID = newID;}
    bool designatedRoutersAreSetUp() const { return designatedRoutersSetUp; }

    OSPFv3Interface* getInterface(){return this->containingInterface;}
    Ipv4Address getNeighborID(){return this->neighborId;}
    Ipv4Address getNeighborsDR(){return this->neighborsDesignatedRouter;}
    Ipv4Address getNeighborsBackup(){return this->neighborsBackupDesignatedRouter;}
    Ipv6Address getNeighborIP(){return this->neighborIPAddress;}
    L3Address getNeighborBackupIP(){return this->neighborsBackupIP;};
    L3Address getNeighborDesignatedIP(){return this->neighborsDesignatedIP;}
    uint32_t getNeighborInterfaceID(){return this->neighborInterfaceID;}
    OSPFv3Options getOptions() const { return this->neighborOptions; }
    OSPFv3DatabaseExchangeRelationshipType getDatabaseExchangeRelationship(){return this->databaseExchangeRelationship;}
    unsigned long getDDSequenceNumber(){return this->ddSequenceNumber;}
    OSPFv3NeighborStateType getState() const;
    unsigned short getNeighborPriority(){return this->neighborRtrPriority;}
    unsigned short getNeighborDeadInterval(){return this->neighborsRouterDeadInterval;}
    void processEvent(OSPFv3Neighbor::OSPFv3NeighborEventType event);
    void changeState(OSPFv3NeighborState *newState, OSPFv3NeighborState *currentState);
    void reset();

    cMessage *getInactivityTimer() { return this->inactivityTimer; }
    cMessage *getPollTimer() { return this->pollTimer; }
    cMessage *getDDRetransmissionTimer() { return this->ddRetransmissionTimer; }
    cMessage *getUpdateRetransmissionTimer() { return this->updateRetransmissionTimer; }
    cMessage *getRequestRetransmissionTimer() { return this->requestRetransmissionTimer; }

    bool needAdjacency();
    bool isFirstAdjacencyInited() const { return this->firstAdjacencyInited; }
    void initFirstAdjacency();
    unsigned long getUniqueULong();
    void incrementDDSequenceNumber(){this->ddSequenceNumber++;}
    void startUpdateRetransmissionTimer();
    void clearUpdateRetransmissionTimer();
    void startRequestRetransmissionTimer();
    void clearRequestRetransmissionTimer();

    void sendDDPacket(bool init = false);
    void sendLinkStateRequestPacket();

    void setLastReceivedDDPacket(OSPFv3DDPacketID packetID) { lastReceivedDDPacket = packetID; }
    OSPFv3DDPacketID getLastReceivedDDPacket(){return this->lastReceivedDDPacket;}
    bool isLinkStateRequestListEmpty(){return this->linkStateRequestList.empty();}
    bool isRequestRetransmissionTimerActive(){return this->requestRetransmissionTimerActive;}
    bool isUpdateRetransmissionTimerActive() const { return this->updateRetransmissionTimerActive; }
    unsigned long getDatabaseSummaryListCount() const { return databaseSummaryList.size();}
    void createDatabaseSummary();
    void deleteLastSentDDPacket();
    void retransmitUpdatePacket();

    void addToRetransmissionList(const OSPFv3LSA *lsaC);
    void removeFromRetransmissionList(LSAKeyType lsaKey);
    OSPFv3LSA* findOnRetransmissionList(LSAKeyType lsaKey);
    bool isRetransmissionListEmpty() const { return linkStateRetransmissionList.empty();}
    void ageTransmittedLSAList();

    bool isLinkStateRequestListEmpty(LSAKeyType lsaKey) const;

    void addToRequestList(const OSPFv3LSAHeader *lsaHeader);
    bool retransmitDatabaseDescriptionPacket();
    bool isLSAOnRequestList(LSAKeyType lsaKey);
    OSPFv3LSAHeader *findOnRequestList(LSAKeyType lsaKey);
    void removeFromRequestList(LSAKeyType lsaKey);
    void addToTransmittedLSAList(LSAKeyType lsaKey);
    bool isOnTransmittedLSAList(LSAKeyType lsaKey) const;

    void setLastHelloTime(int time){this->last_hello_received=time;}
    int getLastHelloTime(){return this->last_hello_received;}


  private:
    OSPFv3NeighborState* state=nullptr;
    OSPFv3NeighborState* previousState=nullptr;
    cMessage *inactivityTimer = nullptr;
    cMessage *pollTimer = nullptr;
    cMessage *ddRetransmissionTimer = nullptr;
    cMessage *updateRetransmissionTimer = nullptr;
    bool updateRetransmissionTimerActive = false;
    cMessage *requestRetransmissionTimer = nullptr;
    bool requestRetransmissionTimerActive = false;
    OSPFv3DatabaseExchangeRelationshipType databaseExchangeRelationship = (OSPFv3DatabaseExchangeRelationshipType)-1;
    bool firstAdjacencyInited = false;
    unsigned long ddSequenceNumber;//TODO - what is the initial number?
    OSPFv3DDPacketID lastReceivedDDPacket;
    unsigned short neighborRtrPriority;//neighbor priority
    Ipv4Address neighborId;

    Ipv6Address neighborIPAddress;
    OSPFv3Options neighborOptions;//options supported by the neighbor
    Ipv4Address neighborsDesignatedRouter;//DR advertised by the neighbor
    Ipv4Address neighborsBackupDesignatedRouter;///Backup advertised by the router
    L3Address neighborsBackupIP;
    L3Address neighborsDesignatedIP;
    uint32_t neighborInterfaceID;
    bool designatedRoutersSetUp = false;
    short neighborsRouterDeadInterval = 0;
    std::list<OSPFv3LSA*> linkStateRetransmissionList;
    std::list<OSPFv3LSAHeader *> databaseSummaryList;//database summary list - complete list of LSAs that make up the area link-state database - the neighbor goes into Database Exchange state
    std::list<OSPFv3LSAHeader *> linkStateRequestList;//link state request list - list of LSAs that need to be received from the neighbor
    std::list<TransmittedLSA > transmittedLSAs;//link state retransmission list - LSA were flooded but not acknowledged, these are retransmitted until acknowledged or until the adjacency ends
    Packet *lastTransmittedDDPacket = nullptr;

    OSPFv3Interface* containingInterface;
    static unsigned long ddSequenceNumberInitSeed;

    int last_hello_received = 0;


};

}//namespace inet
#endif
