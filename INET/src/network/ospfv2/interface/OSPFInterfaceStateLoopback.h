#ifndef __OSPFINTERFACESTATELOOPBACK_HPP__
#define __OSPFINTERFACESTATELOOPBACK_HPP__

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateLoopback : public InterfaceState
{
public:
    virtual void ProcessEvent (Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState (void) const { return Interface::LoopbackState; }
};

} // namespace OSPF

#endif // __OSPFINTERFACESTATELOOPBACK_HPP__

