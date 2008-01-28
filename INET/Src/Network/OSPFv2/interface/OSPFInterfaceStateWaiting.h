#ifndef __OSPFINTERFACESTATEWAITING_HPP__
#define __OSPFINTERFACESTATEWAITING_HPP__

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateWaiting : public InterfaceState
{
public:
    virtual void ProcessEvent (Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState (void) const { return Interface::WaitingState; }
};

} // namespace OSPF

#endif // __OSPFINTERFACESTATEWAITING_HPP__

