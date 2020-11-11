#ifndef __INET_OSPFV3AREA_H
#define __INET_OSPFV3AREA_H

#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"
#include "inet/routing/ospfv3/interface/Ospfv3Interface.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/process/Ospfv3Lsa.h"
#include "inet/routing/ospfv3/process/Ospfv3Process.h"
#include "inet/routing/ospfv3/process/Ospfv3RoutingTableEntry.h"

namespace inet {
namespace ospfv3 {

class Ospfv3Instance;
class Ospfv3Interface;
class Ospfv3Process;

enum Ospfv3AreaType {
    NORMAL = 0,
    STUB,
    TOTALLY_STUBBY,
    NSSA,
    NSSA_TOTALLY_STUB
};

class INET_API Ospfv3Area : public cObject
{
  public:
    Ospfv3Area(Ipv4Address areaID, Ospfv3Instance *containingInstance, Ospfv3AreaType type);
    virtual ~Ospfv3Area();
    Ipv4Address getAreaID() const { return this->areaID; }
    Ospfv3AreaType getAreaType() const { return this->areaType; }
    bool hasInterface(std::string);
    void addInterface(Ospfv3Interface *);
    void init();
    void debugDump();
    void ageDatabase();
    int getInstanceType() { return this->instanceType; }
    void setiInstanceType(int type) { this->instanceType = type; }
    Ospfv3Instance *getInstance() const { return this->containingInstance; }
    void setExternalRoutingCapability(bool capable) { this->externalRoutingCapability = capable; }
    void setStubDefaultCost(int newCost) { this->stubDefaultCost = newCost; }
    void setTransitCapability(bool capable) { this->transitCapability = capable; }
    Ospfv3Interface *getInterfaceById(int id);
    Ospfv3Interface *getNetworkLSAInterface(Ipv4Address id);
    Ospfv3Interface *getInterfaceByIndex(int id);
    bool getExternalRoutingCapability() { return this->externalRoutingCapability; }
    int getStubDefaultCost() { return this->stubDefaultCost; }
    bool getTransitCapability() { return this->transitCapability; }
    Ospfv3Interface *findVirtualLink(Ipv4Address routerID);
    Ospfv3Interface *getInterface(int i) const { return this->interfaceList.at(i); }
    Ospfv3Interface *getInterfaceByIndex(Ipv4Address address);
    int getInterfaceCount() const { return this->interfaceList.size(); }
    Ospfv3Lsa *getLSAbyKey(LSAKeyType lsaKey);

    void addAddressRange(Ipv6AddressRange addressRange, bool advertise);
    bool hasAddressRange(Ipv6AddressRange addressRange) const;
    void addAddressRange(Ipv4AddressRange addressRange, bool advertise);
    bool hasAddressRange(Ipv4AddressRange addressRange) const;

    /* ROUTER LSA */
    RouterLSA *originateRouterLSA(); // this originates one router LSA for one area
    int getRouterLSACount() { return this->routerLSAList.size(); }
    RouterLSA *getRouterLSA(int i) { return this->routerLSAList.at(i); }
    RouterLSA *getRouterLSAbyKey(LSAKeyType lsaKey);
    bool installRouterLSA(const Ospfv3RouterLsa *lsaC);
    bool updateRouterLSA(RouterLSA *currentLsa, const Ospfv3RouterLsa *newLsa);
    bool routerLSADiffersFrom(Ospfv3RouterLsa *currentLsa, const Ospfv3RouterLsa *newLsa);
    Ipv4Address getNewRouterLinkStateID();
    Ipv4Address getRouterLinkStateID() { return this->routerLsID; }
    uint32_t getCurrentRouterSequence() { return this->routerLSASequenceNumber; }
    void incrementRouterSequence() { this->routerLSASequenceNumber++; }
    RouterLSA *findRouterLSAByID(Ipv4Address linkStateID);
    RouterLSA *findRouterLSA(Ipv4Address routerID);
    void deleteRouterLSA(int index);
    void addRouterLSA(RouterLSA *newLSA) { this->routerLSAList.push_back(newLSA); }
    RouterLSA *routerLSAAlreadyExists(RouterLSA *newLsa);

    /*NETWORK LSA */
    void addNetworkLSA(NetworkLSA *newLSA) { this->networkLSAList.push_back(newLSA); }
    NetworkLSA *originateNetworkLSA(Ospfv3Interface *interface); // this originates one router LSA for one area
    int getNetworkLSACount() { return this->networkLSAList.size(); }
    NetworkLSA *getNetworkLSA(int i) { return this->networkLSAList.at(i); }
    bool installNetworkLSA(const Ospfv3NetworkLsa *lsaC);
    bool updateNetworkLSA(NetworkLSA *currentLsa, const Ospfv3NetworkLsa *newLsa);
    bool networkLSADiffersFrom(Ospfv3NetworkLsa *currentLsa, const Ospfv3NetworkLsa *newLsa);
    Ipv4Address getNewNetworkLinkStateID();
    Ipv4Address getNetworkLinkStateID() { return this->networkLsID; }
    uint32_t getCurrentNetworkSequence() { return this->networkLSASequenceNumber; }
    void incrementNetworkSequence() { this->networkLSASequenceNumber++; }
    NetworkLSA *findNetworkLSAByLSID(Ipv4Address linkStateID);
    NetworkLSA *getNetworkLSAbyKey(LSAKeyType LSAKey);
    NetworkLSA *findNetworkLSA(uint32_t intID, Ipv4Address routerID);

    /* INTER AREA PREFIX LSA */
    void addInterAreaPrefixLSA(InterAreaPrefixLSA *newLSA) { this->interAreaPrefixLSAList.push_back(newLSA); }
    int getInterAreaPrefixLSACount() { return this->interAreaPrefixLSAList.size(); }
    InterAreaPrefixLSA *getInterAreaPrefixLSA(int i) { return this->interAreaPrefixLSAList.at(i); }
    void originateDefaultInterAreaPrefixLSA(Ospfv3Area *toArea);
    void originateInterAreaPrefixLSA(Ospfv3IntraAreaPrefixLsa *lsa, Ospfv3Area *fromArea, bool checkDuplicate);
    void originateInterAreaPrefixLSA(const Ospfv3Lsa *prefLsa, Ospfv3Area *fromArea);
    bool installInterAreaPrefixLSA(const Ospfv3InterAreaPrefixLsa *lsaC);
    bool updateInterAreaPrefixLSA(InterAreaPrefixLSA *currentLsa, const Ospfv3InterAreaPrefixLsa *newLsa);
    bool interAreaPrefixLSADiffersFrom(Ospfv3InterAreaPrefixLsa *currentLsa, const Ospfv3InterAreaPrefixLsa *newLsa);
    Ipv4Address getNewInterAreaPrefixLinkStateID();
    uint32_t getCurrentInterAreaPrefixSequence() { return this->interAreaPrefixLSASequenceNumber; }
    void incrementInterAreaPrefixSequence() { this->interAreaPrefixLSASequenceNumber++; }
    InterAreaPrefixLSA *InterAreaPrefixLSAAlreadyExists(const Ospfv3InterAreaPrefixLsa *newLsa);
    InterAreaPrefixLSA *findInterAreaPrefixLSAbyAddress(const L3Address address, int prefixLen);

    // * INTRA AREA PREFIX LSA */
    IntraAreaPrefixLSA *originateIntraAreaPrefixLSA(); // this is for non-BROADCAST links
    IntraAreaPrefixLSA *originateNetIntraAreaPrefixLSA(NetworkLSA *networkLSA, Ospfv3Interface *interface, bool checkDuplicate); // this originates one router LSA for one area
    void addIntraAreaPrefixLSA(IntraAreaPrefixLSA *newLSA) { this->intraAreaPrefixLSAList.push_back(newLSA); }
    int getIntraAreaPrefixLSACount() { return this->intraAreaPrefixLSAList.size(); }
    IntraAreaPrefixLSA *getIntraAreaPrefixLSA(int i) { return this->intraAreaPrefixLSAList.at(i); }
    IntraAreaPrefixLSA *getNetIntraAreaPrefixLSA(L3Address prefix, int prefLen);
    bool installIntraAreaPrefixLSA(const Ospfv3IntraAreaPrefixLsa *lsaC);
    bool updateIntraAreaPrefixLSA(IntraAreaPrefixLSA *currentLsa, const Ospfv3IntraAreaPrefixLsa *newLsa);
    bool intraAreaPrefixLSADiffersFrom(Ospfv3IntraAreaPrefixLsa *currentLsa, const Ospfv3IntraAreaPrefixLsa *newLsa);
    Ipv4Address getNewIntraAreaPrefixLinkStateID();
    void subtractIntraAreaPrefixLinkStateID();
    uint32_t getCurrentIntraAreaPrefixSequence() { return this->intraAreaPrefixLSASequenceNumber; }
    void incrementIntraAreaPrefixSequence() { this->intraAreaPrefixLSASequenceNumber++; }
    IntraAreaPrefixLSA *findIntraAreaPrefixByAddress(L3Address address, int prefix);
    IntraAreaPrefixLSA *findIntraAreaPrefixLSAByReference(LSAKeyType lsaKey);
    IntraAreaPrefixLSA *IntraAreaPrefixLSAAlreadyExists(const Ospfv3IntraAreaPrefixLsa *newLsa);

    const Ospfv3LsaHeader *findLSA(LSAKeyType lsaKey);
    bool floodLSA(const Ospfv3Lsa *lsa, Ospfv3Interface *interface = nullptr, Ospfv3Neighbor *neighbor = nullptr);

    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);
    bool isOnAnyRetransmissionList(LSAKeyType lsaKey) const;
    bool hasAnyNeighborInStates(int state) const;
    bool hasAnyPassiveInterface() const;

    void calculateShortestPathTree(std::vector<Ospfv3RoutingTableEntry *>& newTableIPv6, std::vector<Ospfv3Ipv4RoutingTableEntry *>& newTableIPv4);
    void calculateInterAreaRoutes(std::vector<Ospfv3RoutingTableEntry *>& newTable, std::vector<Ospfv3Ipv4RoutingTableEntry *>& newTableIPv4);

//    std::vector<NextHop> *calculateNextHops(Ospfv3SpfVertex* destination, Ospfv3SpfVertex *parent) const;
    bool nextHopAlreadyExists(std::vector<NextHop> *hops, NextHop nextHop) const;
    std::vector<NextHop> *calculateNextHops(Ospfv3Lsa *destination, Ospfv3Lsa *parent) const;

    void addRouterEntry(RouterLSA *routerLSA, LSAKeyType lsaKey, std::vector<Ospfv3RoutingTableEntry *>& newTableIPv6, std::vector<Ospfv3Ipv4RoutingTableEntry *>& newTableIPv4);
    bool findSameOrWorseCostRoute(const std::vector<Ospfv3RoutingTableEntry *>& newTable,
            const InterAreaPrefixLSA& interAreaPrefixLSA,
            unsigned short currentCost,
            bool& destinationInRoutingTable,
            std::list<Ospfv3RoutingTableEntry *>& sameOrWorseCost) const;
    bool findSameOrWorseCostRoute(const std::vector<Ospfv3Ipv4RoutingTableEntry *>& newTable,
            const InterAreaPrefixLSA& interAreaPrefixLSA,
            unsigned short currentCost,
            bool& destinationInRoutingTable,
            std::list<Ospfv3Ipv4RoutingTableEntry *>& sameOrWorseCost) const;
    Ospfv3RoutingTableEntry *createRoutingTableEntryFromInterAreaPrefixLSA(const InterAreaPrefixLSA& interAreaPrefixLSA,
            unsigned short entryCost,
            const Ospfv3RoutingTableEntry& borderRouterEntry) const;
    Ospfv3Ipv4RoutingTableEntry *createRoutingTableEntryFromInterAreaPrefixLSA(const InterAreaPrefixLSA& interAreaPrefixLSA,
            unsigned short entryCost,
            const Ospfv3Ipv4RoutingTableEntry& borderRouterEntry) const;
    void recheckInterAreaPrefixLSAs(std::vector<Ospfv3RoutingTableEntry *>& newTableIPv6);
    void recheckInterAreaPrefixLSAs(std::vector<Ospfv3Ipv4RoutingTableEntry *>& newTableIPv4);
    bool hasLink(Ospfv3Lsa *fromLSA, Ospfv3Lsa *toLSA) const;

    std::string detailedInfo() const;

    void setSpfTreeRoot(RouterLSA *routerLSA) { spfTreeRoot = routerLSA; }

  private:
    bool v6; // for IPv6 AF is this set to true, for IPv4 to false

    Ipv4Address areaID;
    Ospfv3AreaType areaType;
    std::vector<Ospfv3Interface *> interfaceList; // associated router interfaces

    // address ranges - networks where router within this area have a direct connection
    std::vector<Ipv6AddressRange> IPv6areaAddressRanges;
    std::map<Ipv6AddressRange, bool> IPv6advertiseAddressRanges;
    std::vector<Ipv4AddressRange> IPv4areaAddressRanges;
    std::map<Ipv4AddressRange, bool> IPv4advertiseAddressRanges;

    std::map<std::string, Ospfv3Interface *> interfaceByName; // interfaces by ids
    std::map<int, Ospfv3Interface *> interfaceById;
    std::map<int, Ospfv3Interface *> interfaceByIndex;
    int instanceType;
    Ospfv3Instance *containingInstance;
    bool externalRoutingCapability = false;
    int stubDefaultCost;
    bool transitCapability = false;

    std::map<Ipv4Address, RouterLSA *> routerLSAsByID;
    std::map<Ipv4Address, NetworkLSA *> networkLSAsByID;
    std::map<Ipv4Address, InterAreaPrefixLSA *> interAreaPrefixLSAByID;
    std::map<Ipv4Address, IntraAreaPrefixLSA *> intraAreaPrefixLSAByID;

    std::vector<RouterLSA *> routerLSAList;
    Ipv4Address routerLsID = Ipv4Address::UNSPECIFIED_ADDRESS;
    uint32_t routerLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    std::vector<NetworkLSA *> networkLSAList;
    Ipv4Address networkLsID = Ipv4Address::UNSPECIFIED_ADDRESS;
    uint32_t networkLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    std::vector<InterAreaPrefixLSA *> interAreaPrefixLSAList;
    uint32_t interAreaPrefixLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    std::vector<IntraAreaPrefixLSA *> intraAreaPrefixLSAList;
    Ipv4Address intraAreaPrefixLsID = (Ipv4Address)1; // in simulaiton, zero print as <unspec>
    uint32_t intraAreaPrefixLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    RouterLSA *spfTreeRoot = nullptr;
};

inline std::ostream& operator<<(std::ostream& ostr, const Ospfv3Area& area)
{
    ostr << area.detailedInfo();
    return ostr;
}

} // namespace ospfv3
} // namespace inet

#endif

