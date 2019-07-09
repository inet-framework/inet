#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateLoopback.h"

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDown.h"
#include "inet/routing/ospfv3/OSPFv3Timers.h"

namespace inet{
void OSPFv3InterfaceStateLoopback::processEvent(OSPFv3Interface* interface, OSPFv3Interface::OSPFv3InterfaceEvent event)
{
    /*
     * UNLOOP_IND - change to DOWN and then follow the procedure
     */
    if (event == OSPFv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new OSPFv3InterfaceStateDown, this);
    }
    if (event == OSPFv3Interface::UNLOOP_IND_EVENT) {
        changeState(interface, new OSPFv3InterfaceStateDown, this);
    }
}//processEvent
}//namespace inet
