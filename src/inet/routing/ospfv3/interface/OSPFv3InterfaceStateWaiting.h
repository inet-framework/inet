#ifndef __INET_OSPFV3INTERFACESTATEWAITING_H_
#define __INET_OSPFV3INTERFACESTATEWAITING_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/OSPFv3InterfaceState.h"
#include "inet/common/INETDefs.h"

namespace inet{

/*
 * In this state the interface is waiting for the DR and BDR election. It does do by monitoring hello packets
 * at least for the time of wait timer. It cannot move out of this state until the wait time - this prevents
 * unnecessary changes of the DR or BDR.
 */

class INET_API OSPFv3InterfaceStateWaiting : public OSPFv3InterfaceState
{
  public:
    ~OSPFv3InterfaceStateWaiting() {};
    void processEvent(OSPFv3Interface* intf, OSPFv3Interface::OSPFv3InterfaceEvent event) override;
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const override { return OSPFv3Interface::INTERFACE_STATE_WAITING; }
    std::string getInterfaceStateString() const override{return std::string("OSPFv3InterfaceStateWaiting");};
};

}//namespace inet

#endif
