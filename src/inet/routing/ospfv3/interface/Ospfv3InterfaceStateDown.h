#ifndef __INET_OSPFV3INTERFACESTATEDOWN_H_
#define __INET_OSPFV3INTERFACESTATEDOWN_H_

#include <string>

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateWaiting.h"

namespace inet {
namespace ospfv3 {

class INET_API Ospfv3InterfaceStateDown : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateDown() {};
    void processEvent(Ospfv3Interface* intf,Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_DOWN; }
    std::string getInterfaceStateString() const override { return std::string("Ospfv3InterfaceStateDown"); };
};

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3INTERFACESTATEDOWN_H_

