#ifndef __INET_OSPFINTERFACESTATE_H
#define __INET_OSPFINTERFACESTATE_H

#include "OSPFInterface.h"

namespace OSPF {

class InterfaceState {
protected:
    void ChangeState(Interface* intf, InterfaceState* newState, InterfaceState* currentState);
    void CalculateDesignatedRouter(Interface* intf);

public:
    virtual ~InterfaceState() {}

    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event) = 0;
    virtual Interface::InterfaceStateType GetState(void) const = 0;
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACESTATE_H

