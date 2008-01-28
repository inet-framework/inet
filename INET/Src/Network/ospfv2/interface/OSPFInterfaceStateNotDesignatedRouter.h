#ifndef __OSPFINTERFACESTATENOTDESIGNATEDROUTER_HPP__
#define __OSPFINTERFACESTATENOTDESIGNATEDROUTER_HPP__

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateNotDesignatedRouter : public InterfaceState
{
public:
    virtual void ProcessEvent (Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState (void) const { return Interface::NotDesignatedRouterState; }
};

} // namespace OSPF

#endif // __OSPFINTERFACESTATENOTDESIGNATEDROUTER_HPP__

