#ifndef __INET_OSPFV3INTERFACESTATEDR_H
#define __INET_OSPFV3INTERFACESTATEDR_H

#include <string>

#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"

namespace inet {
namespace ospfv3 {

/*
 * Adjacencies are established with every router on the network.
 * It originates network LSAs, which contain information about every router in the network.
 */

class INET_API Ospfv3InterfaceStateDr : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateDr() {};
    virtual void processEvent(Ospfv3Interface *intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    virtual Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_DESIGNATED; }
    virtual std::string getInterfaceStateString() const override { return std::string("Ospfv3InterfaceStateDr"); }
};

} // namespace ospfv3
} // namespace inet

#endif

