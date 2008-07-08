#ifndef __INET_OSPFINTERFACESTATENOTDESIGNATEDROUTER_H
#define __INET_OSPFINTERFACESTATENOTDESIGNATEDROUTER_H

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateNotDesignatedRouter : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::NotDesignatedRouterState; }
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACESTATENOTDESIGNATEDROUTER_H

