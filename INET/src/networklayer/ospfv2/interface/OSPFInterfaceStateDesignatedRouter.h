#ifndef __INET_OSPFINTERFACESTATEDESIGNATEDROUTER_H
#define __INET_OSPFINTERFACESTATEDESIGNATEDROUTER_H

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateDesignatedRouter : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::DesignatedRouterState; }
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACESTATEDESIGNATEDROUTER_H

