#ifndef __INET_OSPFV3INTERFACESTATEPOINTTOPOINT_H_
#define __INET_OSPFV3INTERFACESTATEPOINTTOPOINT_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/OSPFv3InterfaceState.h"
#include "inet/common/INETDefs.h"

namespace inet{

/*
 * Interface is operational and it is connected to either a physical point-to-point interface
 * or it is a virtual link. When this state is entered, the interface is trying to form an adjacency
 * with the neighbor. Hello is sent normally every Hello interval.
 */

class INET_API OSPFv3InterfaceStatePointToPoint : public OSPFv3InterfaceState
{
  public:
    ~OSPFv3InterfaceStatePointToPoint() {};
    virtual void processEvent(OSPFv3Interface* intf, OSPFv3Interface::OSPFv3InterfaceEvent event) override;
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const override { return OSPFv3Interface::INTERFACE_STATE_POINTTOPOINT; }
    std::string getInterfaceStateString() const {return std::string("OSPFv3InterfaceStatePointToPoint");};
};

}//namespace inet

#endif
