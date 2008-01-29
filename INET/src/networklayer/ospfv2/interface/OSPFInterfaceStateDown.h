#ifndef __OSPFINTERFACESTATEDOWN_HPP__
#define __OSPFINTERFACESTATEDOWN_HPP__

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateDown : public InterfaceState
{
public:
    virtual void ProcessEvent (Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState (void) const { return Interface::DownState; }
};

} // namespace OSPF

#endif // __OSPFINTERFACESTATEDOWN_HPP__

