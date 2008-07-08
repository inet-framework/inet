#ifndef __INET_OSPFINTERFACESTATEWAITING_H
#define __INET_OSPFINTERFACESTATEWAITING_H

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateWaiting : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::WaitingState; }
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACESTATEWAITING_H

