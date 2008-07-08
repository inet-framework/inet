#ifndef __INET_OSPFINTERFACESTATEDOWN_H
#define __INET_OSPFINTERFACESTATEDOWN_H

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateDown : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::DownState; }
};

} // namespace OSPF

#endif // __INET_OSPFINTERFACESTATEDOWN_H

