#ifndef __INET_OSPFV3INTERFACESTATEDR_H_
#define __INET_OSPFV3INTERFACESTATEDR_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospf/v3/interface/Ospfv3InterfaceState.h"
#include "inet/common/INETDefs.h"

namespace inet{

/*
 * Adjacencies are established with every router on the network.
 * It originates network LSAs, which contain information about every router in the network.
 */

class INET_API Ospfv3InterfaceStateDr : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateDr(){};
    virtual void processEvent(Ospfv3Interface* intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    virtual Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_DESIGNATED; }
    virtual std::string getInterfaceStateString() const override { return std::string("Ospfv3InterfaceStateDr"); }
};

}//namespace inet

#endif
