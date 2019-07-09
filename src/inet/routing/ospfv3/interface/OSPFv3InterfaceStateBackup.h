#ifndef __INET_OSPFV3INTERFACESTATEBACKUP_H_
#define __INET_OSPFV3INTERFACESTATEBACKUP_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/OSPFv3InterfaceState.h"
#include "inet/common/INETDefs.h"

namespace inet{

/*
 * The router is BDR, it will be promoted to DR in case of failure. It forms adjacencies with
 * all DROther routers attached to the network. It performs slightly different function
 * during the flooding procedure.
 *
 * Special behaviour:
 *  -It does not flood the network-LSAs
 *  -
 */

class INET_API OSPFv3InterfaceStateBackup : public OSPFv3InterfaceState
{
  public:
    ~OSPFv3InterfaceStateBackup() {};
    void processEvent(OSPFv3Interface* intf, OSPFv3Interface::OSPFv3InterfaceEvent event) override;
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const override { return OSPFv3Interface::INTERFACE_STATE_BACKUP;};
    std::string getInterfaceStateString() const {return std::string("OSPFv3InterfaceStateBackup");};
};

}//namespace inet

#endif
