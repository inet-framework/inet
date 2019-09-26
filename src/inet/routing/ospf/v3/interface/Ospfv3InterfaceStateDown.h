#ifndef __INET_OSPFV3INTERFACESTATEDOWN_H_
#define __INET_OSPFV3INTERFACESTATEDOWN_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospf/v3/interface/Ospfv3InterfaceState.h"
#include "inet/routing/ospf/v3/interface/Ospfv3InterfaceStateWaiting.h"
#include "inet/common/INETDefs.h"


namespace inet{

class INET_API Ospfv3InterfaceStateDown : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateDown() {};
    void processEvent(Ospfv3Interface* intf,Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_DOWN; }
    std::string getInterfaceStateString() const override {return std::string("Ospfv3InterfaceStateDown");};
};

}//namespace inet

#endif
