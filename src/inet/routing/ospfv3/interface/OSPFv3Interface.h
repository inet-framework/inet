#ifndef __INET_OSPFV3INTERFACE_H_
#define __INET_OSPFV3INTERFACE_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/OSPFv3Common.h"
#include "inet/routing/ospfv3/OSPFv3Packet_m.h"
#include "inet/routing/ospfv3/process/OSPFv3Area.h"
#include "inet/routing/ospfv3/process/OSPFv3LSA.h"
#include "inet/routing/ospfv3/process/OSPFv3Process.h"
//#include <cmodule.h>
#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/packet/Packet.h"


namespace inet{

class OSPFv3Area;
class OSPFv3InterfaceState;
class OSPFv3Process;


class INET_API OSPFv3Interface : public cObject
{
  public:
    enum  OSPFv3InterfaceFAState{
        INTERFACE_STATE_DOWN = 0,
        INTERFACE_STATE_LOOPBACK = 1,
        INTERFACE_STATE_WAITING = 2,
        INTERFACE_STATE_POINTTOPOINT = 3,
        INTERFACE_STATE_DROTHER = 4,
        INTERFACE_STATE_BACKUP = 5,
        INTERFACE_STATE_DESIGNATED = 6,
        INTERFACE_PASSIVE = 7
    };

    enum OSPFv3InterfaceEvent{
        INTERFACE_UP_EVENT = 0,
        UNLOOP_IND_EVENT = 1,
        WAIT_TIMER_EVENT = 2,
        BACKUP_SEEN_EVENT = 3,
        NEIGHBOR_CHANGE_EVENT = 4,
        INTERFACE_DOWN_EVENT = 5,
        LOOP_IND_EVENT = 6,
        HELLO_TIMER_EVENT = 7,
        ACKNOWLEDGEMENT_TIMER_EVENT = 8,
        NEIGHBOR_REVIVED_EVENT = 9
    };

    enum OSPFv3InterfaceType {
        UNKNOWN_TYPE = 0,
        POINTTOPOINT_TYPE = 1,
        BROADCAST_TYPE = 2,
        NBMA_TYPE = 3,
        POINTTOMULTIPOINT_TYPE = 4,
        VIRTUAL_TYPE = 5
    };

    const char* OSPFv3IntStateOutput[7] = {
            "Down",
            "Loopback",
            "Waiting",
            "Point-to-Point",
            "DROther",
            "Backup",
            "DR"
    };

    const char* OSPFv3IntTypeOutput[6] = {
            "UNKNOWN",
            "POINT-TO-POINT",
            "BROADCAST",
            "NBMA",
            "POINT-TO-MULTIPOINT",
            "VIRTUAL"
    };

    struct AcknowledgementFlags
    {
        bool floodedBackOut;
        bool lsaIsNewer;
        bool lsaIsDuplicate;
        bool impliedAcknowledgement;
        bool lsaReachedMaxAge;
        bool noLSAInstanceInDatabase;
        bool anyNeighborInExchangeOrLoadingState;
    };

    cModule* containingModule=nullptr;
    OSPFv3Process* containingProcess=nullptr;
    IInterfaceTable* ift = nullptr;

  public:
    OSPFv3Interface(const char* name, cModule* routerModule, OSPFv3Process* processModule, OSPFv3InterfaceType interfaceType, bool passive);
    virtual ~OSPFv3Interface();
    std::string getIntName() const {return this->interfaceName;}
    void reset();
    void processEvent(OSPFv3Interface::OSPFv3InterfaceEvent);
    void setRouterPriority(int newPriority){this->routerPriority = newPriority;}
    void setHelloInterval(int newInterval){this->helloInterval=newInterval;}
    void setDeadInterval(int newInterval){this->deadInterval=newInterval;}
    void setInterfaceCost(int newInterval){this->interfaceCost=newInterval;}
    void setInterfaceType(OSPFv3InterfaceType newType){this->interfaceType=newType;}
    void setAckDelay(int newAckDelay){this->ackDelay = newAckDelay;}
    void setDesignatedIP(Ipv6Address newIP){this->DesignatedRouterIP = newIP;}
    void setBackupIP(Ipv6Address newIP){this->BackupRouterIP = newIP;}
    void setDesignatedID(Ipv4Address newID){this->DesignatedRouterID = newID;}
    void setDesignatedIntID(int newIntID){this->DesignatedIntID = newIntID;}
    void setBackupID(Ipv4Address newID){this->BackupRouterID = newID;}
    void sendDelayedAcknowledgements();
    void setInterfaceIndex(int newIndex){this->interfaceIndex = newIndex;}
    void setTransitAreaID(Ipv4Address areaId) { this->transitAreaID = areaId;}
    int getRouterPriority() const {return this->routerPriority;}
    short getHelloInterval() const {return this->helloInterval;}
    short getDeadInterval() const {return this->deadInterval;}
    short getPollInterval() const {return this->pollInterval;}
    short getTransDelayInterval() const {return this->transmissionDelay;}
    short getRetransmissionInterval() const {return this->retransmissionInterval;}
    short getAckDelay() const {return this->ackDelay;}
    int getInterfaceCost() const {return this->interfaceCost;}
    int getInterfaceId() const {return this->interfaceId;}
    int getInterfaceIndex() const {return this->interfaceIndex;}
    int getNeighborCount() const {return this->neighbors.size();}
    int getInterfaceMTU() const;
    int getInterfaceIndex(){return this->interfaceIndex;}
    Ipv6Address getInterfaceLLIP() const {return this->interfaceLLIP;}
    bool isInterfacePassive(){return this->passiveInterface;}
    Ipv4Address getTransitAreaID() const { return this->transitAreaID; }

    OSPFv3InterfaceType getType() const {return this->interfaceType;}
    OSPFv3Neighbor* getNeighbor(int i){return this->neighbors.at(i);}
    Ipv6Address getDesignatedIP() const {return this->DesignatedRouterIP;}
    Ipv6Address getBackupIP() const {return this->BackupRouterIP;}
    Ipv4Address getDesignatedID() const {return this->DesignatedRouterID;}
    Ipv4Address getBackupID() const {return this->BackupRouterID;}
    int getDesignatedIntID() const {return this->DesignatedIntID;}
    void removeNeighborByID(Ipv4Address neighborID);

    int calculateInterfaceCost(); //only prototype
    cMessage* getWaitTimer(){return this->waitTimer;}
    cMessage* getHelloTimer(){return this->helloTimer;}
    cMessage* getAcknowledgementTimer(){return this->acknowledgementTimer;}
    OSPFv3Area* getArea() const {return this->containingArea;};
    void setArea(OSPFv3Area* area){this->containingArea=area;};

    OSPFv3InterfaceState* getCurrentState(){return this->state;};
    OSPFv3InterfaceState* getPreviousState(){return this->previousState;};
    void changeState(OSPFv3InterfaceState* currentState, OSPFv3InterfaceState* newState);
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const;

    void processHelloPacket(Packet* packet);
    void processDDPacket(Packet* packet);
    bool preProcessDDPacket(Packet* packet, OSPFv3Neighbor* neighbor, bool inExchangeStart);
    void processLSR(Packet* packet, OSPFv3Neighbor* neighbor);
    void processLSU(Packet* packet, OSPFv3Neighbor* neighbor);
    void processLSAck(Packet* packet, OSPFv3Neighbor* neighbor);
    bool floodLSA(const OSPFv3LSA* lsa, OSPFv3Interface* interface=nullptr, OSPFv3Neighbor* neighbor=nullptr);
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);
    bool isOnAnyRetransmissionList(LSAKeyType lsaKey) const;
    bool hasAnyNeighborInState(int state) const;
    void ageTransmittedLSALists();

    Packet* prepareHello();
    Packet* prepareLSUHeader();
    Packet* prepareUpdatePacket(std::vector<OSPFv3LSA *>lsas);

    OSPFv3Neighbor* getNeighborById(Ipv4Address neighborId);
    void addNeighbor(OSPFv3Neighbor*);

    std::string info() const override;
    std::string detailedInfo() const override;
    void acknowledgeLSA(const OSPFv3LSAHeader& lsaHeader, AcknowledgementFlags ackFlags, Ipv4Address routerID);


    // LINK LSA
    LinkLSA* originateLinkLSA();
    bool installLinkLSA(const OSPFv3LinkLSA *lsa);
    void addLinkLSA(LinkLSA* newLSA){this->linkLSAList.push_back(newLSA);}
    int getLinkLSACount(){return this->linkLSAList.size();}
    LinkLSA* getLinkLSA(int i){return this->linkLSAList.at(i);}
    LinkLSA* getLinkLSAbyKey(LSAKeyType lsaKey);
//    void installLinkLSA(LinkLSA *lsa);
    bool updateLinkLSA(LinkLSA* currentLsa, OSPFv3LinkLSA* newLsa);
    bool linkLSADiffersFrom(OSPFv3LinkLSA* currentLsa, OSPFv3LinkLSA* newLsa);
    LinkLSA* findLinkLSAbyAdvRouter (Ipv4Address advRouter);

    void sendLSAcknowledgement(const OSPFv3LSAHeader *lsaHeader, Ipv6Address destination);
    void addDelayedAcknowledgement(const OSPFv3LSAHeader& lsaHeader);

    void setTransitNetInt(bool isTransit){this->transitNetworkInterface=isTransit;}
    bool getTransitNetInt(){return this->transitNetworkInterface;}

    void addInterfaceAddress(Ipv6AddressRange address) {this->interfaceAddresses.push_back(address);}
    int getInterfaceAddressCount(){return this->interfaceAddresses.size();}
    Ipv6AddressRange getInterfaceAddress(int i) {return this->interfaceAddresses[i];}



    bool ageDatabase();
  private:
    friend class OSPFv3InterfaceState;

  private:
    int interfaceId;//physical id in the simulation
    int interfaceIndex;//unique value that appears in hello packet
    std::vector<LinkLSA*> linkLSAList;
    Ipv6Address interfaceLLIP; // link-local ip address
    std::string interfaceName;
    bool passiveInterface;
    OSPFv3InterfaceState* state;
    OSPFv3InterfaceState* previousState=nullptr;
    short helloInterval;
    short deadInterval;
    short pollInterval;
    short transmissionDelay;
    short retransmissionInterval;
    short ackDelay;
    int routerPriority;

    int mtu;
    OSPFv3Area* containingArea;
    Ipv4Address transitAreaID;
    OSPFv3Interface::OSPFv3InterfaceType interfaceType;

    std::vector<Ipv6AddressRange> interfaceAddresses; // List of link prefixes
    std::vector<OSPFv3Neighbor*> neighbors;
    std::map<Ipv4Address, OSPFv3Neighbor*> neighborsById;
    std::map<Ipv6Address, std::list<OSPFv3LSAHeader> > delayedAcknowledgements;
    std::map<Ipv4Address, LinkLSA *> linkLSAsByID;

    //for Intra-Area-Prefix LSA
    bool transitNetworkInterface;

    uint32_t linkLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    Ipv6Address DesignatedRouterIP;
    Ipv6Address BackupRouterIP;
    Ipv4Address DesignatedRouterID;
    Ipv4Address BackupRouterID;
    int DesignatedIntID;
    int interfaceCost;

    cMessage *helloTimer;
    cMessage *waitTimer;
    cMessage *acknowledgementTimer;

    //TODO - E-bit
    // TODO - instance ID missing (?)
};

inline std::ostream& operator<<(std::ostream& ostr, const OSPFv3Interface& interface)
{
    ostr << interface.detailedInfo();
    return ostr;
}

}//namespace inet
#endif
