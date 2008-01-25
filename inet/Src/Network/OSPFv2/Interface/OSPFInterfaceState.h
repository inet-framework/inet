#ifndef __OSPFINTERFACESTATE_HPP__
#define __OSPFINTERFACESTATE_HPP__

#include "OSPFInterface.h"

namespace OSPF {

class InterfaceState {
protected:
    void ChangeState (Interface* intf, InterfaceState* newState, InterfaceState* currentState);
    void CalculateDesignatedRouter (Interface* intf);

public:
    virtual ~InterfaceState () {}

    virtual void ProcessEvent (Interface* intf, Interface::InterfaceEventType event) = 0;
    virtual Interface::InterfaceStateType GetState (void) const = 0;
};

} // namespace OSPF

#endif // __OSPFINTERFACESTATE_HPP__

