#ifndef __OSPFINTERFACESTATEPOINTTOPOINT_HPP__
#define __OSPFINTERFACESTATEPOINTTOPOINT_HPP__

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStatePointToPoint : public InterfaceState
{
public:
    virtual void ProcessEvent (Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState (void) const { return Interface::PointToPointState; }
};

} // namespace OSPF

#endif // __OSPFINTERFACESTATEPOINTTOPOINT_HPP__

