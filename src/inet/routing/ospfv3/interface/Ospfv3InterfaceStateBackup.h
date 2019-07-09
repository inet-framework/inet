#ifndef __INET_OSPFV3INTERFACESTATEBACKUP_H_
#define __INET_OSPFV3INTERFACESTATEBACKUP_H_

#include <omnetpp.h>
#include <string>

#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"
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

class INET_API Ospfv3InterfaceStateBackup : public Ospfv3InterfaceState
{
  public:
    ~Ospfv3InterfaceStateBackup() {};
    void processEvent(Ospfv3Interface* intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_BACKUP;};
    std::string getInterfaceStateString() const {return std::string("Ospfv3InterfaceStateBackup");};
};

}//namespace inet

#endif
