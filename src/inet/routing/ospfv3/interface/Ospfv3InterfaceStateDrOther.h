#ifndef __INET_OSPFV3INTERFACESTATEDROTHER_H_
#define __INET_OSPFV3INTERFACESTATEDROTHER_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"
#include "inet/common/INETDefs.h"

namespace inet{

/*
 * This router is neither DR nor BDR. The interface is on NBMA Network. It forms adjacencies
 * with both DR and BDR.
 */

class INET_API Ospfv3InterfaceStateDrOther : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateDrOther() {};
    virtual void processEvent(Ospfv3Interface* intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_DROTHER; }
    std::string getInterfaceStateString() const override{return std::string("Ospfv3InterfaceStateDrOther");};
};

}//namespace inet

#endif
