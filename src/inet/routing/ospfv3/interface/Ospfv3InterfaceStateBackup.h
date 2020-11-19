#ifndef __INET_OSPFV3INTERFACESTATEBACKUP_H
#define __INET_OSPFV3INTERFACESTATEBACKUP_H

#include <string>

#include "inet/routing/ospfv3/interface/Ospfv3InterfaceState.h"

namespace inet {
namespace ospfv3 {

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
    virtual void processEvent(Ospfv3Interface *intf, Ospfv3Interface::Ospfv3InterfaceEvent event) override;
    virtual Ospfv3Interface::Ospfv3InterfaceFaState getState() const override { return Ospfv3Interface::INTERFACE_STATE_BACKUP; }
    virtual std::string getInterfaceStateString() const override { return std::string("Ospfv3InterfaceStateBackup"); }
};

} // namespace ospfv3
} // namespace inet

#endif

