#ifndef __INET_OSPFV3INSTANCE_H_
#define __INET_OSPFV3INSTANCE_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/OSPFv3Common.h"
#include "inet/routing/ospfv3/OSPFv3Packet_m.h"
#include "inet/routing/ospfv3/process/OSPFv3Area.h"
#include "inet/routing/ospfv3/process/OSPFv3Process.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"

#include "inet/common/packet/Packet.h"

namespace inet{

class OSPFv3Process;
class OSPFv3Area;
//class OSPFv3Neighbor;

class INET_API OSPFv3Instance : public cObject
{
  public:
    OSPFv3Instance(int instanceId, OSPFv3Process* parentProcess, int addressFamily);
    virtual ~OSPFv3Instance();
    int getInstanceID(){return this->instanceID;};
    int getAddressFamily(){return this->addressFamily;};
    OSPFv3Process* getProcess() const {return this->containingProcess;}
    void init();
    bool hasArea(Ipv4Address);
    void addArea(OSPFv3Area*);
    OSPFv3Area* getAreaById(Ipv4Address areaId);
    OSPFv3Area* getArea(int i){return this->areas.at(i);}
    void debugDump();
    Ipv4Address getNewInterAreaPrefixLinkStateID();
    void subtractInterAreaPrefixLinkStateID();
    void processPacket(Packet* pk);
    int getUniqueId(){return OSPFv3IfIndex++;}
    int getAreaCount(){return this->areas.size();}
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);
    std::string detailedInfo() const override;

  public:
    IInterfaceTable* ift = nullptr;
  private:
    int addressFamily;
    int instanceID;
    OSPFv3Process* containingProcess;
    std::vector<OSPFv3Area* > areas; //list of areas in this instance
    cModule* containingModule=nullptr;
    std::map<Ipv4Address , OSPFv3Area*> areasById; //mapping the area id to area
    int OSPFv3IfIndex = 0; //unique number for interfaces
    Ipv4Address interAreaPrefixLsID = Ipv4Address::UNSPECIFIED_ADDRESS;
};

inline std::ostream& operator<<(std::ostream& ostr, const OSPFv3Instance& instance)
{
    ostr << instance.detailedInfo();
    return ostr;
}
}//namespace inet
#endif
