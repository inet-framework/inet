#ifndef __INET_OSPFV3INSTANCE_H_
#define __INET_OSPFV3INSTANCE_H_

#include <string>

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/routing/ospfv3/Ospfv3Common.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"
#include "inet/routing/ospfv3/interface/Ospfv3Interface.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/process/Ospfv3Area.h"
#include "inet/routing/ospfv3/process/Ospfv3Process.h"

namespace inet {
namespace ospfv3 {

class Ospfv3Process;
class Ospfv3Area;
//class Ospfv3Neighbor;

class INET_API Ospfv3Instance : public cObject
{
  public:
    Ospfv3Instance(int instanceId, Ospfv3Process* parentProcess, int addressFamily);
    virtual ~Ospfv3Instance();
    int getInstanceID() { return this->instanceID; }
    int getAddressFamily() { return this->addressFamily; }
    Ospfv3Process* getProcess() const { return this->containingProcess; }
    void init();
    bool hasArea(Ipv4Address);
    void addArea(Ospfv3Area*);
    Ospfv3Area* getAreaById(Ipv4Address areaId);
    Ospfv3Area* getArea(int i) { return this->areas.at(i); }
    void debugDump();
    Ipv4Address getNewInterAreaPrefixLinkStateID();
    void subtractInterAreaPrefixLinkStateID();
    void processPacket(Packet* pk);
    int getUniqueId() { return ospfv3IfIndex++; }
    int getAreaCount() { return this->areas.size(); }
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);
    std::string detailedInfo() const OMNETPP5_CODE(override);

  public:
    IInterfaceTable* ift = nullptr;

  private:
    int addressFamily;
    int instanceID;
    Ospfv3Process* containingProcess;
    std::vector<Ospfv3Area* > areas; //list of areas in this instance
    cModule* containingModule=nullptr;
    std::map<Ipv4Address , Ospfv3Area*> areasById; //mapping the area id to area
    int ospfv3IfIndex = 0; //unique number for interfaces
    Ipv4Address interAreaPrefixLsID = Ipv4Address::UNSPECIFIED_ADDRESS;
};

inline std::ostream& operator<<(std::ostream& ostr, const Ospfv3Instance& instance)
{
    ostr << instance.detailedInfo();
    return ostr;
}

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3INSTANCE_H_

