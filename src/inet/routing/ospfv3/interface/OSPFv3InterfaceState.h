#ifndef __INET_OSPFV3INTERFACESTATE_H_
#define __INET_OSPFV3INTERFACESTATE_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/common/INETDefs.h"

namespace inet{

class INET_API OSPFv3InterfaceState
{
  public:
    virtual void processEvent(OSPFv3Interface *interface, OSPFv3Interface::OSPFv3InterfaceEvent eventNum) = 0;
    virtual OSPFv3Interface::OSPFv3InterfaceFAState getState() const = 0;
    virtual ~OSPFv3InterfaceState() {};
    virtual std::string getInterfaceStateString() const = 0;

  protected:

    void changeState(OSPFv3Interface *intf, OSPFv3InterfaceState *newState, OSPFv3InterfaceState *currentState);
    void calculateDesignatedRouter(OSPFv3Interface *intf);

};

}//namespace inet

#endif
