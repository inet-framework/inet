#ifndef __OSPFINTERFACESTATEDESIGNATEDROUTER_HPP__
#define __OSPFINTERFACESTATEDESIGNATEDROUTER_HPP__

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateDesignatedRouter : public InterfaceState
{
public:
    virtual void ProcessEvent (Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState (void) const { return Interface::DesignatedRouterState; }
};

} // namespace OSPF

#endif // __OSPFINTERFACESTATEDESIGNATEDROUTER_HPP__

