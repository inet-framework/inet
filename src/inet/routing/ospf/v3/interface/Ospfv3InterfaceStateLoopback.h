#ifndef __INET_OSPFV3INTERFACESTATELOOPBACK_H_
#define __INET_OSPFV3INTERFACESTATELOOPBACK_H_

#include <string>

#include "inet/common/INETDefs.h"
#include "inet/routing/ospf/v3/interface/Ospfv3InterfaceState.h"

namespace inet {
namespace ospf {

/*
 * Interface os looped back, it is unsiutable for normal traffic. It is still desirable to gain information
 * about the quality of this interface (ICMP...). Advertised as router LSA with single host route, whose dest IP
 * is the interface IP.
 */

class INET_API Ospfv3InterfaceStateLoopback : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateLoopback() {}
    virtual void processEvent(Ospfv3Interface* intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_LOOPBACK; }
    std::string getInterfaceStateString() const override { return std::string("Ospfv3InterfaceStateLoopback"); }
};

} // namespace ospf
}//namespace inet

#endif // __INET_OSPFV3INTERFACESTATELOOPBACK_H_

