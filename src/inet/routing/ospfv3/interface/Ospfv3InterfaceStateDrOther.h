#ifndef __INET_OSPFV3INTERFACESTATEDROTHER_H_
#define __INET_OSPFV3INTERFACESTATEDROTHER_H_

#include <string>

#include "inet/common/INETDefs.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"

namespace inet {
namespace ospfv3 {

/*
 * This router is neither DR nor BDR. The interface is on NBMA Network. It forms adjacencies
 * with both DR and BDR.
 */

class INET_API Ospfv3InterfaceStateDrOther : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateDrOther() {}
    virtual void processEvent(Ospfv3Interface* intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_DROTHER; }
    std::string getInterfaceStateString() const override { return std::string("Ospfv3InterfaceStateDrOther"); }
};

} // namespace ospfv3
}//namespace inet

#endif // __INET_OSPFV3INTERFACESTATEDROTHER_H_

