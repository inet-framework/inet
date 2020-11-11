#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateLoopback.h"

#include "inet/routing/ospfv3/Ospfv3Timers.h"
#include "inet/routing/ospfv3/interface/Ospfv3Interface.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateDown.h"

namespace inet {
namespace ospfv3 {

void Ospfv3InterfaceStateLoopback::processEvent(Ospfv3Interface *interface, Ospfv3Interface::Ospfv3InterfaceEvent event)
{
    /*
     * UNLOOP_IND - change to DOWN and then follow the procedure
     */
    if (event == Ospfv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new Ospfv3InterfaceStateDown, this);
    }
    if (event == Ospfv3Interface::UNLOOP_IND_EVENT) {
        changeState(interface, new Ospfv3InterfaceStateDown, this);
    }
} // processEvent

} // namespace ospfv3
} // namespace inet

