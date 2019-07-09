#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateBackup.h"

#include "inet/routing/ospfv3/interface/OSPFv3Interface.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDown.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateLoopback.h"
#include "inet/routing/ospfv3/OSPFv3Timers.h"

namespace inet{
void OSPFv3InterfaceStateBackup::processEvent(OSPFv3Interface* interface, OSPFv3Interface::OSPFv3InterfaceEvent event)
{
    /*
     * HELLO_TIMER - watch for changes in DR, BDR or priority!!
     * NEIGHBOR_CHANGE
     *  -bidirectional comm established
     *  -no longer bidirectional comm
     *  -one of the neighbors declared itself DR or BDR
     *  -one of the neighbors is no londer declaring itself DR or BDR
     *  -advertised priority of neighbot has changed
     * INTERFACE_DOWN or LOOPBACK_IND - DOWN or LOOPBACK
     */
    if (event == OSPFv3Interface::NEIGHBOR_CHANGE_EVENT) {
        calculateDesignatedRouter(interface);
    }
    if (event == OSPFv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new OSPFv3InterfaceStateDown, this);
    }
    if (event == OSPFv3Interface::LOOP_IND_EVENT) {
        interface->reset();
        changeState(interface, new OSPFv3InterfaceStateLoopback, this);
    }
    if (event == OSPFv3Interface::HELLO_TIMER_EVENT) {
        if (interface->getType() == OSPFv3Interface::BROADCAST_TYPE) {
            EV_DEBUG << "Sending Hello to all in " << this->getInterfaceStateString() << "\n";
            Packet* hello = interface->prepareHello();
            interface->getArea()->getInstance()->getProcess()->sendPacket(hello, Ipv6Address::ALL_OSPF_ROUTERS_MCAST, interface->getIntName().c_str());
        }
        else {    // Interface::NBMA
            EV_DEBUG << "Sending Hello to all NBMA neighbors in " << this->getInterfaceStateString() << "\n";
            unsigned long neighborCount = interface->getNeighborCount();
            int hopLimit = (interface->getType() == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;
            for (unsigned long i = 0; i < neighborCount; i++) {
                Packet* hello = interface->prepareHello();
                Ipv6Address dest = interface->getNeighbor(i)->getNeighborIP();
                interface->getArea()->getInstance()->getProcess()->sendPacket(hello, dest, interface->getIntName().c_str(), hopLimit);
            }
        }
        interface->getArea()->getInstance()->getProcess()->setTimer(interface->getHelloTimer(), interface->getHelloInterval());
    }
    if (event == OSPFv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT) {
        interface->sendDelayedAcknowledgements();
    }
    if (event == OSPFv3Interface::NEIGHBOR_REVIVED_EVENT) {
        changeState(interface, new OSPFv3InterfaceStateWaiting, this);
        interface->getArea()->getInstance()->getProcess()->setTimer(interface->getWaitTimer(), interface->getDeadInterval());
    }
}//processEvent


}//namespace inet
