#ifndef __INET_OSPFV3INTERFACEPASSIVE_H_
#define __INET_OSPFV3INTERFACEPASSIVE_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"
#include "inet/common/INETDefs.h"

namespace inet{

/*
 * Adjacencies are established with every router on the network.
 * It originates network LSAs, which contain information about every router in the network.
 */

class INET_API Ospfv3InterfacePassive : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfacePassive(){};
    void processEvent(Ospfv3Interface* intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_PASSIVE; }
    std::string getInterfaceStateString() const {return std::string("Ospfv3InterfacePassive");};
};

}//namespace inet

#endif
