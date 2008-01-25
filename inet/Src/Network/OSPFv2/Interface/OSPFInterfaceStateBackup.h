#ifndef __OSPFINTERFACESTATEBACKUP_HPP__
#define __OSPFINTERFACESTATEBACKUP_HPP__

#include "OSPFInterfaceState.h"

namespace OSPF {

class InterfaceStateBackup : public InterfaceState
{
public:
    virtual void ProcessEvent (Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState (void) const { return Interface::BackupState; }
};

} // namespace OSPF

#endif // __OSPFINTERFACESTATEBACKUP_HPP__

