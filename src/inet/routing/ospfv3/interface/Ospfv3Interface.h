#ifndef __INET_OSPFV3INTERFACE_H
#define __INET_OSPFV3INTERFACE_H

#include <string>

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/ospfv3/Ospfv3Common.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/process/Ospfv3Area.h"
#include "inet/routing/ospfv3/process/Ospfv3Lsa.h"
#include "inet/routing/ospfv3/process/Ospfv3Process.h"

namespace inet {
namespace ospfv3 {

class Ospfv3Area;
class Ospfv3InterfaceState;
class Ospfv3Process;

class INET_API Ospfv3Interface : public cObject
{
  public:
    enum  Ospfv3InterfaceFaState {
        INTERFACE_STATE_DOWN         = 0,
        INTERFACE_STATE_LOOPBACK     = 1,
        INTERFACE_STATE_WAITING      = 2,
        INTERFACE_STATE_POINTTOPOINT = 3,
        INTERFACE_STATE_DROTHER      = 4,
        INTERFACE_STATE_BACKUP       = 5,
        INTERFACE_STATE_DESIGNATED   = 6,
        INTERFACE_PASSIVE            = 7
    };

    enum Ospfv3InterfaceEvent {
        INTERFACE_UP_EVENT          = 0,
        UNLOOP_IND_EVENT            = 1,
        WAIT_TIMER_EVENT            = 2,
        BACKUP_SEEN_EVENT           = 3,
        NEIGHBOR_CHANGE_EVENT       = 4,
        INTERFACE_DOWN_EVENT        = 5,
        LOOP_IND_EVENT              = 6,
        HELLO_TIMER_EVENT           = 7,
        ACKNOWLEDGEMENT_TIMER_EVENT = 8,
        NEIGHBOR_REVIVED_EVENT      = 9
    };

    enum Ospfv3InterfaceType {
        UNKNOWN_TYPE           = 0,
        POINTTOPOINT_TYPE      = 1,
        BROADCAST_TYPE         = 2,
        NBMA_TYPE              = 3,
        POINTTOMULTIPOINT_TYPE = 4,
        VIRTUAL_TYPE           = 5
    };

    const char *ospfv3IntStateOutput[7] = {
        "Down",
        "Loopback",
        "Waiting",
        "Point-to-Point",
        "DROther",
        "Backup",
        "DR"
    };

    const char *ospfv3IntTypeOutput[6] = {
        "UNKNOWN",
        "POINT-TO-POINT",
        "BROADCAST",
        "NBMA",
        "POINT-TO-MULTIPOINT",
        "VIRTUAL"
    };

    struct AcknowledgementFlags {
        bool floodedBackOut             = false;
        bool lsaIsNewer                 = false;
        bool lsaIsDuplicate             = false;
        bool impliedAcknowledgement     = false;
        bool lsaReachedMaxAge           = false;
        bool noLSAInstanceInDatabase    = false;
        bool anyNeighborInExchangeOrLoadingState= false;
    };

    cModule *containingModule = nullptr;
    Ospfv3Process *containingProcess = nullptr;
    IInterfaceTable *ift = nullptr;

  public:
    Ospfv3Interface(const char *name, cModule *routerModule, Ospfv3Process *processModule, Ospfv3InterfaceType interfaceType, bool passive);
    virtual ~Ospfv3Interface();
    std::string getIntName() const { return this->interfaceName; }
    void reset();
    void processEvent(Ospfv3Interface::Ospfv3InterfaceEvent);
    void setRouterPriority(int newPriority) { this->routerPriority = newPriority; }
    void setHelloInterval(int newInterval) { this->helloInterval = newInterval; }
    void setDeadInterval(int newInterval) { this->deadInterval = newInterval; }
    void setInterfaceCost(int newInterval) { this->interfaceCost = newInterval; }
    void setInterfaceType(Ospfv3InterfaceType newType) { this->interfaceType = newType; }
    void setAckDelay(int newAckDelay) { this->ackDelay = newAckDelay; }
    void setDesignatedIP(Ipv6Address newIP) { this->DesignatedRouterIP = newIP; }
    void setBackupIP(Ipv6Address newIP) { this->BackupRouterIP = newIP; }
    void setDesignatedID(Ipv4Address newID) { this->DesignatedRouterID = newID; }
    void setDesignatedIntID(int newIntID) { this->DesignatedIntID = newIntID; }
    void setBackupID(Ipv4Address newID) { this->BackupRouterID = newID; }
    void sendDelayedAcknowledgements();
    void setInterfaceIndex(int newIndex) { this->interfaceIndex = newIndex; }
    void setTransitAreaID(Ipv4Address areaId) { this->transitAreaID = areaId; }
    int getRouterPriority() const { return this->routerPriority; }
    short getHelloInterval() const { return this->helloInterval; }
    short getDeadInterval() const { return this->deadInterval; }
    short getPollInterval() const { return this->pollInterval; }
    short getTransDelayInterval() const { return this->transmissionDelay; }
    short getRetransmissionInterval() const { return this->retransmissionInterval; }
    short getAckDelay() const { return this->ackDelay; }
    int getInterfaceCost() const { return this->interfaceCost; }
    int getInterfaceId() const { return this->interfaceId; }
    int getInterfaceIndex() const { return this->interfaceIndex; }
    int getNeighborCount() const { return this->neighbors.size(); }
    int getInterfaceMTU() const;
    int getInterfaceIndex() { return this->interfaceIndex; }
    Ipv6Address getInterfaceLLIP() const { return this->interfaceLLIP; }
    bool isInterfacePassive() { return this->passiveInterface; }
    Ipv4Address getTransitAreaID() const { return this->transitAreaID; }

    Ospfv3InterfaceType getType() const { return this->interfaceType; }
    Ospfv3Neighbor *getNeighbor(int i) { return this->neighbors.at(i); }
    Ipv6Address getDesignatedIP() const { return this->DesignatedRouterIP; }
    Ipv6Address getBackupIP() const { return this->BackupRouterIP; }
    Ipv4Address getDesignatedID() const { return this->DesignatedRouterID; }
    Ipv4Address getBackupID() const { return this->BackupRouterID; }
    int getDesignatedIntID() const { return this->DesignatedIntID; }
    void removeNeighborByID(Ipv4Address neighborID);

    int calculateInterfaceCost(); // only prototype
    cMessage *getWaitTimer() { return this->waitTimer; }
    cMessage *getHelloTimer() { return this->helloTimer; }
    cMessage *getAcknowledgementTimer() { return this->acknowledgementTimer; }
    Ospfv3Area *getArea() const { return this->containingArea; };
    void setArea(Ospfv3Area *area) { this->containingArea = area; }

    Ospfv3InterfaceState *getCurrentState() { return this->state; };
    Ospfv3InterfaceState *getPreviousState() { return this->previousState; };
    void changeState(Ospfv3InterfaceState *currentState, Ospfv3InterfaceState *newState);
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const;

    void processHelloPacket(Packet *packet);
    void processDDPacket(Packet *packet);
    bool preProcessDDPacket(Packet *packet, Ospfv3Neighbor *neighbor, bool inExchangeStart);
    void processLSR(Packet *packet, Ospfv3Neighbor *neighbor);
    void processLSU(Packet *packet, Ospfv3Neighbor *neighbor);
    void processLSAck(Packet *packet, Ospfv3Neighbor *neighbor);
    bool floodLSA(const Ospfv3Lsa *lsa, Ospfv3Interface *interface = nullptr, Ospfv3Neighbor *neighbor = nullptr);
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);
    bool isOnAnyRetransmissionList(LSAKeyType lsaKey) const;
    bool hasAnyNeighborInState(int state) const;
    void ageTransmittedLSALists();

    Packet *prepareHello();
    Packet *prepareLSUHeader();
    Packet *prepareUpdatePacket(std::vector<Ospfv3Lsa *> lsas);

    Ospfv3Neighbor *getNeighborById(Ipv4Address neighborId);
    void addNeighbor(Ospfv3Neighbor *);

    std::string str() const override;
    std::string detailedInfo() const;
    void acknowledgeLSA(const Ospfv3LsaHeader& lsaHeader, AcknowledgementFlags ackFlags, Ipv4Address routerID);

    // LINK LSA
    LinkLSA *originateLinkLSA();
    bool installLinkLSA(const Ospfv3LinkLsa *lsa);
    void addLinkLSA(LinkLSA *newLSA) { this->linkLSAList.push_back(newLSA); }
    int getLinkLSACount() { return this->linkLSAList.size(); }
    LinkLSA *getLinkLSA(int i) { return this->linkLSAList.at(i); }
    LinkLSA *getLinkLSAbyKey(LSAKeyType lsaKey);
//    void installLinkLSA(LinkLSA *lsa);
    bool updateLinkLSA(LinkLSA *currentLsa, const Ospfv3LinkLsa *newLsa);
    bool linkLSADiffersFrom(Ospfv3LinkLsa *currentLsa, const Ospfv3LinkLsa *newLsa);
    LinkLSA *findLinkLSAbyAdvRouter(Ipv4Address advRouter);

    void sendLSAcknowledgement(const Ospfv3LsaHeader *lsaHeader, Ipv6Address destination);
    void addDelayedAcknowledgement(const Ospfv3LsaHeader& lsaHeader);

    void setTransitNetInt(bool isTransit) { this->transitNetworkInterface = isTransit; }
    bool getTransitNetInt() { return this->transitNetworkInterface; }

    void addInterfaceAddress(Ipv6AddressRange address) { this->interfaceAddresses.push_back(address); }
    int getInterfaceAddressCount() { return this->interfaceAddresses.size(); }
    Ipv6AddressRange getInterfaceAddress(int i) { return this->interfaceAddresses[i]; }

    bool ageDatabase();

  private:
    friend class Ospfv3InterfaceState;

  private:
    int interfaceId; // physical id in the simulation
    int interfaceIndex; // unique value that appears in hello packet
    std::vector<LinkLSA *> linkLSAList;
    Ipv6Address interfaceLLIP; // link-local ip address
    std::string interfaceName;
    bool passiveInterface;
    Ospfv3InterfaceState *state;
    Ospfv3InterfaceState *previousState = nullptr;
    short helloInterval;
    short deadInterval;
    short pollInterval;
    short transmissionDelay;
    short retransmissionInterval;
    short ackDelay;
    int routerPriority;

    int mtu;
    Ospfv3Area *containingArea;
    Ipv4Address transitAreaID;
    Ospfv3Interface::Ospfv3InterfaceType interfaceType;

    std::vector<Ipv6AddressRange> interfaceAddresses; // List of link prefixes
    std::vector<Ospfv3Neighbor *> neighbors;
    std::map<Ipv4Address, Ospfv3Neighbor *> neighborsById;
    std::map<Ipv6Address, std::list<Ospfv3LsaHeader>> delayedAcknowledgements;
    std::map<Ipv4Address, LinkLSA *> linkLSAsByID;

    // for Intra-Area-Prefix LSA
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

    // TODO - E-bit
    // TODO - instance ID missing (?)
};

inline std::ostream& operator<<(std::ostream& ostr, const Ospfv3Interface& interface)
{
    ostr << interface.detailedInfo();
    return ostr;
}

} // namespace ospfv3
} // namespace inet

#endif

