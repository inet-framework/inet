#ifndef __INET_OSPFINTERFACESTATEPOINTTOPOINT_H
#define __INET_OSPFINTERFACESTATEPOINTTOPOINT_H

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStatePointToPoint : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::PointToPointState; }
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACESTATEPOINTTOPOINT_H

