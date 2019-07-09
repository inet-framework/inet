#ifndef __INET_OSPFV3AREA_H_
#define __INET_OSPFV3AREA_H_

#include <omnetpp.h>

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/OSPFv3Packet_m.h"
#include "inet/routing/ospfv3/process/OSPFv3LSA.h"
#include "inet/routing/ospfv3/process/OSPFv3Process.h"
#include "inet/routing/ospfv3/process/OSPFv3RoutingTableEntry.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"


namespace inet{

class OSPFv3Instance;
class OSPFv3Interface;
class OSPFv3Process;

enum OSPFv3AreaType {
    NORMAL = 0,
    STUB,
    TOTALLY_STUBBY,
    NSSA,
    NSSA_TOTALLY_STUB
};

class INET_API OSPFv3Area : public cObject
{
  public:
    OSPFv3Area(Ipv4Address areaID, OSPFv3Instance* containingInstance, OSPFv3AreaType type);
    virtual ~OSPFv3Area();
    Ipv4Address getAreaID() const {return this->areaID;}
    OSPFv3AreaType getAreaType() const {return this->areaType;}
    bool hasInterface(std::string);
    void addInterface(OSPFv3Interface*);
    void init();
    void debugDump();
    void ageDatabase();
    int getInstanceType(){return this->instanceType;};
    void setiInstanceType(int type){this->instanceType = type;};
    OSPFv3Instance* getInstance() const {return this->containingInstance;};
    void setExternalRoutingCapability(bool capable){this->externalRoutingCapability=capable;}
    void setStubDefaultCost(int newCost){this->stubDefaultCost=newCost;}
    void setTransitCapability(bool capable){this->transitCapability=capable;}
    OSPFv3Interface* getInterfaceById(int id);
    OSPFv3Interface* getNetworkLSAInterface(Ipv4Address id);
    OSPFv3Interface* getInterfaceByIndex(int id);
    bool getExternalRoutingCapability(){return this->externalRoutingCapability;}
    int getStubDefaultCost(){return this->stubDefaultCost;}
    bool getTransitCapability(){return this->transitCapability;}
    OSPFv3Interface* findVirtualLink(Ipv4Address routerID);
    OSPFv3Interface* getInterface(int i) const {return this->interfaceList.at(i);}
    OSPFv3Interface* getInterfaceByIndex (Ipv4Address address);
    int getInterfaceCount() const {return this->interfaceList.size();}
    OSPFv3LSA* getLSAbyKey(LSAKeyType lsaKey);

    void addAddressRange(Ipv6AddressRange addressRange, bool advertise);
    bool hasAddressRange(Ipv6AddressRange addressRange) const;
    void addAddressRange(Ipv4AddressRange addressRange, bool advertise);
    bool hasAddressRange(Ipv4AddressRange addressRange) const;


    /* ROUTER LSA */
    RouterLSA* originateRouterLSA();//this originates one router LSA for one area
    int getRouterLSACount(){return this->routerLSAList.size();}
    RouterLSA* getRouterLSA(int i){return this->routerLSAList.at(i);}
    RouterLSA* getRouterLSAbyKey(LSAKeyType lsaKey);
    bool installRouterLSA(const OSPFv3RouterLSA *lsaC);
    bool updateRouterLSA(RouterLSA* currentLsa, OSPFv3RouterLSA* newLsa);
    bool routerLSADiffersFrom(OSPFv3RouterLSA* currentLsa, OSPFv3RouterLSA* newLsa);
    Ipv4Address getNewRouterLinkStateID();
    Ipv4Address getRouterLinkStateID(){return this->routerLsID;}
    uint32_t getCurrentRouterSequence(){return this->routerLSASequenceNumber;}
    void incrementRouterSequence(){this->routerLSASequenceNumber++;}
    RouterLSA* findRouterLSAByID(Ipv4Address linkStateID);
    RouterLSA* findRouterLSA(Ipv4Address routerID);
    void deleteRouterLSA(int index);
    void addRouterLSA(RouterLSA* newLSA){this->routerLSAList.push_back(newLSA);}
    RouterLSA* routerLSAAlreadyExists(RouterLSA* newLsa);

    /*NETWORK LSA */
    void addNetworkLSA(NetworkLSA* newLSA){this->networkLSAList.push_back(newLSA);}
    NetworkLSA* originateNetworkLSA(OSPFv3Interface* interface);//this originates one router LSA for one area
    int getNetworkLSACount(){return this->networkLSAList.size();}
    NetworkLSA* getNetworkLSA(int i){return this->networkLSAList.at(i);}
    bool installNetworkLSA(const OSPFv3NetworkLSA *lsaC);
    bool updateNetworkLSA(NetworkLSA* currentLsa, OSPFv3NetworkLSA* newLsa);
    bool networkLSADiffersFrom(OSPFv3NetworkLSA* currentLsa, OSPFv3NetworkLSA* newLsa);
    Ipv4Address getNewNetworkLinkStateID();
    Ipv4Address getNetworkLinkStateID(){return this->networkLsID;}
    uint32_t getCurrentNetworkSequence(){return this->networkLSASequenceNumber;}
    void incrementNetworkSequence(){this->networkLSASequenceNumber++;}
    NetworkLSA* findNetworkLSAByLSID(Ipv4Address linkStateID);
    NetworkLSA* getNetworkLSAbyKey(LSAKeyType LSAKey);
    NetworkLSA* findNetworkLSA(uint32_t intID, Ipv4Address routerID);

    /* INTER AREA PREFIX LSA */
    void addInterAreaPrefixLSA(InterAreaPrefixLSA* newLSA){this->interAreaPrefixLSAList.push_back(newLSA);};
    int getInterAreaPrefixLSACount(){return this->interAreaPrefixLSAList.size();}
    InterAreaPrefixLSA* getInterAreaPrefixLSA(int i){return this->interAreaPrefixLSAList.at(i);}
    void originateDefaultInterAreaPrefixLSA(OSPFv3Area* toArea);
    void originateInterAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA* lsa, OSPFv3Area* fromArea, bool checkDuplicate);
    void originateInterAreaPrefixLSA(const OSPFv3LSA* prefLsa, OSPFv3Area* fromArea);
    bool installInterAreaPrefixLSA(const OSPFv3InterAreaPrefixLSA* lsaC);
    bool updateInterAreaPrefixLSA(InterAreaPrefixLSA* currentLsa, OSPFv3InterAreaPrefixLSA* newLsa);
    bool interAreaPrefixLSADiffersFrom(OSPFv3InterAreaPrefixLSA* currentLsa, OSPFv3InterAreaPrefixLSA* newLsa);
    Ipv4Address getNewInterAreaPrefixLinkStateID();
    uint32_t getCurrentInterAreaPrefixSequence(){return this->interAreaPrefixLSASequenceNumber;}
    void incrementInterAreaPrefixSequence(){this->interAreaPrefixLSASequenceNumber++;}
    InterAreaPrefixLSA* InterAreaPrefixLSAAlreadyExists(OSPFv3InterAreaPrefixLSA *newLsa);
    InterAreaPrefixLSA* findInterAreaPrefixLSAbyAddress(const L3Address address, int prefixLen);


    //* INTRA AREA PREFIX LSA */
    IntraAreaPrefixLSA* originateIntraAreaPrefixLSA();//this is for non-BROADCAST links
    IntraAreaPrefixLSA* originateNetIntraAreaPrefixLSA(NetworkLSA* networkLSA, OSPFv3Interface* interface, bool checkDuplicate);//this originates one router LSA for one area
    void addIntraAreaPrefixLSA(IntraAreaPrefixLSA* newLSA){this->intraAreaPrefixLSAList.push_back(newLSA);}
    int getIntraAreaPrefixLSACount(){return this->intraAreaPrefixLSAList.size();}
    IntraAreaPrefixLSA* getIntraAreaPrefixLSA(int i){return this->intraAreaPrefixLSAList.at(i);}
    IntraAreaPrefixLSA* getNetIntraAreaPrefixLSA(L3Address prefix, int prefLen);
    bool installIntraAreaPrefixLSA(const OSPFv3IntraAreaPrefixLSA *lsaC);
    bool updateIntraAreaPrefixLSA(IntraAreaPrefixLSA* currentLsa, OSPFv3IntraAreaPrefixLSA* newLsa);
    bool intraAreaPrefixLSADiffersFrom(OSPFv3IntraAreaPrefixLSA* currentLsa, OSPFv3IntraAreaPrefixLSA* newLsa);
    Ipv4Address getNewIntraAreaPrefixLinkStateID();
    void subtractIntraAreaPrefixLinkStateID();
    uint32_t getCurrentIntraAreaPrefixSequence(){return this->intraAreaPrefixLSASequenceNumber;}
    void incrementIntraAreaPrefixSequence(){this->intraAreaPrefixLSASequenceNumber++;}
    IntraAreaPrefixLSA* findIntraAreaPrefixByAddress(L3Address address, int prefix);
    IntraAreaPrefixLSA* findIntraAreaPrefixLSAByReference(LSAKeyType lsaKey);
    IntraAreaPrefixLSA* IntraAreaPrefixLSAAlreadyExists(OSPFv3IntraAreaPrefixLSA *newLsa);

    const OSPFv3LSAHeader* findLSA(LSAKeyType lsaKey);
    bool floodLSA(const OSPFv3LSA* lsa, OSPFv3Interface* interface=nullptr, OSPFv3Neighbor* neighbor=nullptr);

    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);
    bool isOnAnyRetransmissionList(LSAKeyType lsaKey) const;
    bool hasAnyNeighborInStates(int state) const;
    bool hasAnyPassiveInterface() const;


    void calculateShortestPathTree(std::vector<OSPFv3RoutingTableEntry* >& newTableIPv6, std::vector<OSPFv3IPv4RoutingTableEntry* >& newTableIPv4);
    void calculateInterAreaRoutes(std::vector<OSPFv3RoutingTableEntry* >& newTable, std::vector<OSPFv3IPv4RoutingTableEntry* >& newTableIPv4);

//    std::vector<NextHop> *calculateNextHops(OSPFv3SPFVertex* destination, OSPFv3SPFVertex *parent) const;
    bool nextHopAlreadyExists(std::vector<NextHop> *hops,NextHop nextHop) const;
    std::vector<NextHop> *calculateNextHops(OSPFv3LSA *destination, OSPFv3LSA *parent) const;

    void addRouterEntry(RouterLSA* routerLSA, LSAKeyType lsaKey, std::vector<OSPFv3RoutingTableEntry *>& newTableIPv6, std::vector<OSPFv3IPv4RoutingTableEntry *>& newTableIPv4 );
    bool findSameOrWorseCostRoute(const std::vector<OSPFv3RoutingTableEntry *>& newTable,
            const InterAreaPrefixLSA& interAreaPrefixLSA,
            unsigned short currentCost,
            bool& destinationInRoutingTable,
            std::list<OSPFv3RoutingTableEntry *>& sameOrWorseCost) const;
    bool findSameOrWorseCostRoute(const std::vector<OSPFv3IPv4RoutingTableEntry *>& newTable,
            const InterAreaPrefixLSA& interAreaPrefixLSA,
            unsigned short currentCost,
            bool& destinationInRoutingTable,
            std::list<OSPFv3IPv4RoutingTableEntry *>& sameOrWorseCost) const;
    OSPFv3RoutingTableEntry *createRoutingTableEntryFromInterAreaPrefixLSA(const InterAreaPrefixLSA& interAreaPrefixLSA,
            unsigned short entryCost,
            const OSPFv3RoutingTableEntry& borderRouterEntry) const;
    OSPFv3IPv4RoutingTableEntry *createRoutingTableEntryFromInterAreaPrefixLSA(const InterAreaPrefixLSA& interAreaPrefixLSA,
            unsigned short entryCost,
            const OSPFv3IPv4RoutingTableEntry& borderRouterEntry) const;
    void recheckInterAreaPrefixLSAs(std::vector<OSPFv3RoutingTableEntry* >& newTableIPv6);
    void recheckInterAreaPrefixLSAs(std::vector<OSPFv3IPv4RoutingTableEntry* >& newTableIPv4);
    bool hasLink(OSPFv3LSA *fromLSA, OSPFv3LSA *toLSA) const;

    std::string detailedInfo() const override;

    void setSpfTreeRoot(RouterLSA* routerLSA){spfTreeRoot = routerLSA;};

  private:
    bool v6; // for IPv6 AF is this set to true, for IPv4 to false

    Ipv4Address areaID;
    OSPFv3AreaType areaType;
    std::vector<OSPFv3Interface*> interfaceList;//associated router interfaces

    //address ranges - networks where router within this area have a direct connection
    std::vector<Ipv6AddressRange> IPv6areaAddressRanges;
    std::map<Ipv6AddressRange, bool> IPv6advertiseAddressRanges;
    std::vector<Ipv4AddressRange> IPv4areaAddressRanges;
    std::map<Ipv4AddressRange, bool> IPv4advertiseAddressRanges;

    std::map<std::string, OSPFv3Interface*> interfaceByName;//interfaces by ids
    std::map<int, OSPFv3Interface*> interfaceById;
    std::map<int, OSPFv3Interface*> interfaceByIndex;
    int instanceType;
    OSPFv3Instance* containingInstance;
    bool externalRoutingCapability;
    int stubDefaultCost;
    bool transitCapability;

    std::map<Ipv4Address, RouterLSA *> routerLSAsByID;
    std::map<Ipv4Address, NetworkLSA *> networkLSAsByID;
    std::map<Ipv4Address, InterAreaPrefixLSA *> interAreaPrefixLSAByID;
    std::map<Ipv4Address, IntraAreaPrefixLSA *> intraAreaPrefixLSAByID;

    std::vector<RouterLSA* > routerLSAList;
    Ipv4Address routerLsID = Ipv4Address::UNSPECIFIED_ADDRESS;
    uint32_t routerLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    std::vector<NetworkLSA* > networkLSAList;
    Ipv4Address networkLsID = Ipv4Address::UNSPECIFIED_ADDRESS;
    uint32_t networkLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    std::vector<InterAreaPrefixLSA* > interAreaPrefixLSAList;
    uint32_t interAreaPrefixLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    std::vector<IntraAreaPrefixLSA*> intraAreaPrefixLSAList;
    Ipv4Address intraAreaPrefixLsID = (Ipv4Address) 1; //in simulaiton, zero print as <unspec>
    uint32_t intraAreaPrefixLSASequenceNumber = INITIAL_SEQUENCE_NUMBER;

    RouterLSA* spfTreeRoot=nullptr;

};

inline std::ostream& operator<<(std::ostream& ostr, const OSPFv3Area& area)
{
    ostr << area.detailedInfo();
    return ostr;
}

}//namespace inet

#endif
