#ifndef __INET_OSPFV3INTERFACEPASSIVE_H_
#define __INET_OSPFV3INTERFACEPASSIVE_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/OSPFv3InterfaceState.h"
#include "inet/common/INETDefs.h"

namespace inet{

/*
 * Adjacencies are established with every router on the network.
 * It originates network LSAs, which contain information about every router in the network.
 */

class INET_API OSPFv3InterfacePassive : public OSPFv3InterfaceState
{
  public:
    ~OSPFv3InterfacePassive(){};
    void processEvent(OSPFv3Interface* intf, OSPFv3Interface::OSPFv3InterfaceEvent event) override;
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const override { return OSPFv3Interface::INTERFACE_PASSIVE; }
    std::string getInterfaceStateString() const {return std::string("OSPFv3InterfacePassive");};
};

}//namespace inet

#endif
