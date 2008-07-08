#ifndef __INET_OSPFINTERFACESTATELOOPBACK_H
#define __INET_OSPFINTERFACESTATELOOPBACK_H

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateLoopback : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::LoopbackState; }
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACESTATELOOPBACK_H

