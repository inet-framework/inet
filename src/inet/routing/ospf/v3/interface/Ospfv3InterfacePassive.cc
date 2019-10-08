
#include "inet/routing/ospf/v3/interface/Ospfv3Interface.h"
#include "inet/routing/ospf/v3/interface/Ospfv3InterfacePassive.h"
#include "inet/routing/ospf/v3/interface/Ospfv3InterfaceStateDown.h"

namespace inet {
namespace ospf {
namespace v3 {

void Ospfv3InterfacePassive::processEvent(Ospfv3Interface* interface, Ospfv3Interface::Ospfv3InterfaceEvent event)
{
    if (event == Ospfv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new Ospfv3InterfaceStateDown, this);
    }
}//processEvent

} // namespace v3
} // namespace ospf
}//namespace inet

