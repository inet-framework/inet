#ifndef __INET_OSPFV3PROCESS_H_
#define __INET_OSPFV3PROCESS_H_

#include <string>

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/routing/ospfv3/Ospfv3Common.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"
#include "inet/routing/ospfv3/Ospfv3Timers.h"
#include "inet/routing/ospfv3/process/Ospfv3Area.h"
#include "inet/routing/ospfv3/process/Ospfv3Instance.h"
#include "inet/routing/ospfv3/process/Ospfv3RoutingTableEntry.h"

namespace inet {
namespace ospfv3 {

class Ospfv3Instance;

class INET_API Ospfv3Process : protected cListener, public cSimpleModule
{
  public:
    Ospfv3Process();
    virtual ~Ospfv3Process();
    int getProcessID() const { return this->processID; };
    Ipv4Address getRouterID() { return this->routerID; };
    bool isActivated() { return this->isActive; };
    void activateProcess();
    void setTimer(cMessage* msg, double delay);
    void clearTimer(cMessage* msg) { this->cancelEvent(msg); }
    Ospfv3Instance* getInstanceById(int instanceId);
    void addInstance(Ospfv3Instance* newInstance);
    void sendPacket(Packet *packet, Ipv6Address destination, const char* ifName, short hopLimit = 1);
    Ospfv3Lsa* findLSA(LSAKeyType lsaKey, Ipv4Address areaID, int instanceID);
    bool floodLSA(const Ospfv3Lsa* lsa, Ipv4Address areaID=Ipv4Address::UNSPECIFIED_ADDRESS, Ospfv3Interface* intf=nullptr, Ospfv3Neighbor* neighbor=nullptr);
    bool installLSA(const Ospfv3Lsa *lsaC, int instanceID, Ipv4Address areaID=Ipv4Address::UNSPECIFIED_ADDRESS, Ospfv3Interface* intf=nullptr);
    void rebuildRoutingTable();
    void calculateASExternalRoutes(std::vector<Ospfv3RoutingTableEntry* > newTableIPv6, std::vector<Ospfv3Ipv4RoutingTableEntry* > newTableIPv4);

    void ageDatabase();
    cMessage* getAgeTimer() { return this->ageTimer; }

    /**
     * Returns true if one of the Router's Areas the same address range configured as the
     * input address , false otherwise.
     * @param addressRange [in] The IPv6 address  to look for.
     */
    int isInRoutingTable(IIpv4RoutingTable *rtTable, Ipv4Address addr);
    int isInRoutingTable6(Ipv6RoutingTable *rtTable, Ipv6Address addr);
    int isInInterfaceTable(IInterfaceTable *ifTable, Ipv4Address addr);
    int isInInterfaceTable6(IInterfaceTable *ifTable, Ipv6Address addr);

    bool hasAddressRange(const Ipv6AddressRange& addressRange) const;
    bool hasAddressRange(const Ipv4AddressRange& addressRange) const;

  public:
    IInterfaceTable* ift = nullptr;
    Ipv6RoutingTable *rt6 = nullptr;
    IIpv4RoutingTable *rt4 = nullptr;
    cModule* containingModule=nullptr;

  private:
    void handleTimer(cMessage* msg);

  private:
    std::vector<Ospfv3Instance*> instances;
    std::map<int, Ospfv3Instance*> instancesById;
    int processID = 0;
    Ipv4Address routerID;
    bool isActive=false;
    void debugDump();
    std::vector<Ospfv3RoutingTableEntry *> routingTableIPv6;
    std::vector<Ospfv3Ipv4RoutingTableEntry *> routingTableIPv4;
    cMessage *ageTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage* msg) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void parseConfig(cXMLElement* areaConfig);

    //backbone area structure
    //virtual links
    //list of external routes
    //list of as-external routes
};

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3PROCESS_H_

