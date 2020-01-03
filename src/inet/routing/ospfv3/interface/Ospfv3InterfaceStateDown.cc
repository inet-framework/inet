
#include "inet/routing/ospfv3/interface/Ospfv3InterfacePassive.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateDown.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateDrOther.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStateLoopback.h"
#include "inet/routing/ospfv3/interface/Ospfv3InterfaceStatePointToPoint.h"

namespace inet {
namespace ospfv3 {

void Ospfv3InterfaceStateDown::processEvent(Ospfv3Interface* interface, Ospfv3Interface::Ospfv3InterfaceEvent event)
{
    /*
     * Two different actions for P2P and for Broadcast (or NBMA)
     */
    if (event == Ospfv3Interface::INTERFACE_UP_EVENT) {
        EV_DEBUG <<"Interface " << interface->getIntName() << " is in up state\n";
        if (!interface->isInterfacePassive()) {
            LinkLSA *lsa = interface->originateLinkLSA();
            interface->installLinkLSA(lsa);
            delete lsa;
            interface->getArea()->getInstance()->getProcess()->setTimer(interface->getHelloTimer(), 0);
            interface->getArea()->getInstance()->getProcess()->setTimer(interface->getAcknowledgementTimer(), interface->getAckDelay());

            switch (interface->getType()) {
            case Ospfv3Interface::POINTTOPOINT_TYPE:
            case Ospfv3Interface::POINTTOMULTIPOINT_TYPE:
            case Ospfv3Interface::VIRTUAL_TYPE:
                changeState(interface, new Ospfv3InterfaceStatePointToPoint, this);
                break;

            case Ospfv3Interface::NBMA_TYPE:
                if (interface->getRouterPriority() == 0) {
                    changeState(interface, new Ospfv3InterfaceStateDrOther, this);
                }
                else {
                    changeState(interface, new Ospfv3InterfaceStateWaiting, this);
                    interface->getArea()->getInstance()->getProcess()->setTimer(interface->getWaitTimer(), interface->getDeadInterval());

                    long neighborCount = interface->getNeighborCount();
                    for (long i = 0; i < neighborCount; i++) {
                        Ospfv3Neighbor *neighbor = interface->getNeighbor(i);
                        if (neighbor->getNeighborPriority() > 0) {
                            neighbor->processEvent(Ospfv3Neighbor::START);
                        }
                    }
                }
                break;

            case Ospfv3Interface::BROADCAST_TYPE:
                if (interface->getRouterPriority() == 0) {
                    changeState(interface, new Ospfv3InterfaceStateDrOther, this);
                }
                else {
                    changeState(interface, new Ospfv3InterfaceStateWaiting, this);
                    interface->getArea()->getInstance()->getProcess()->setTimer(interface->getWaitTimer(), interface->getDeadInterval());
                }
                break;

            default:
                break;
            }
        }
        else if (interface->isInterfacePassive()) {
            LinkLSA *lsa = interface->originateLinkLSA();
            interface->installLinkLSA(lsa);
            delete lsa;
            IntraAreaPrefixLSA *prefLsa = interface->getArea()->originateIntraAreaPrefixLSA();
            if (prefLsa != nullptr) {
                if (!interface->getArea()->installIntraAreaPrefixLSA(prefLsa))
                    EV_DEBUG << "Intra Area Prefix LSA for network beyond interface " << interface->getIntName() << " was not created!\n";
                delete prefLsa;
            }
            changeState(interface, new Ospfv3InterfacePassive, this);
        }
        if (event == Ospfv3Interface::LOOP_IND_EVENT) {
            interface->reset();
              changeState(interface, new Ospfv3InterfaceStateLoopback, this);
        }
    }
}//processEvent

} // namespace ospfv3
}//namespace inet

