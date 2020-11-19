#ifndef __INET_OSPFV3INTERFACESTATEPOINTTOPOINT_H
#define __INET_OSPFV3INTERFACESTATEPOINTTOPOINT_H

#include <omnetpp.h>

#include <string>

#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"

namespace inet {
namespace ospfv3 {

/*
 * Interface is operational and it is connected to either a physical point-to-point interface
 * or it is a virtual link. When this state is entered, the interface is trying to form an adjacency
 * with the neighbor. Hello is sent normally every Hello interval.
 */

class INET_API Ospfv3InterfaceStatePointToPoint : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStatePointToPoint() {}
    virtual void processEvent(Ospfv3Interface *intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    virtual Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_POINTTOPOINT; }
    virtual std::string getInterfaceStateString() const override { return std::string("Ospfv3InterfaceStatePointToPoint"); }
};

} // namespace ospfv3
} // namespace inet

#endif

