#ifndef __INET_OSPFV3INTERFACESTATEWAITING_H
#define __INET_OSPFV3INTERFACESTATEWAITING_H

#include <string>

#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"

namespace inet {
namespace ospfv3 {

/*
 * In this state the interface is waiting for the DR and BDR election. It does do by monitoring hello packets
 * at least for the time of wait timer. It cannot move out of this state until the wait time - this prevents
 * unnecessary changes of the DR or BDR.
 */

class INET_API Ospfv3InterfaceStateWaiting : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateWaiting() {};
    void processEvent(Ospfv3Interface *intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_WAITING; }
    std::string getInterfaceStateString() const override { return std::string("Ospfv3InterfaceStateWaiting"); }
};

} // namespace ospfv3
} // namespace inet

#endif

