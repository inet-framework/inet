#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDown.h"

#include "inet/routing/ospfv3/interface/OSPFv3InterfacePassive.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateDROther.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStateLoopback.h"
#include "inet/routing/ospfv3/interface/OSPFv3InterfaceStatePointToPoint.h"
//#include "INET/routing/ospfv3/OSPFv3Timers.h"
//#include <cmodule.h>


namespace inet{

void OSPFv3InterfaceStateDown::processEvent(OSPFv3Interface* interface, OSPFv3Interface::OSPFv3InterfaceEvent event)
{
    /*
     * Two different actions for P2P and for Broadcast (or NBMA)
     */
    if(event == OSPFv3Interface::INTERFACE_UP_EVENT)
    {
        EV_DEBUG <<"Interface " << interface->getIntName() << " is in up state\n";
        if(!interface->isInterfacePassive()){
            LinkLSA *lsa = interface->originateLinkLSA();
            interface->installLinkLSA(lsa);
            interface->getArea()->getInstance()->getProcess()->setTimer(interface->getHelloTimer(), 0);
            interface->getArea()->getInstance()->getProcess()->setTimer(interface->getAcknowledgementTimer(), interface->getAckDelay());

            switch (interface->getType()) {
            case OSPFv3Interface::POINTTOPOINT_TYPE:
            case OSPFv3Interface::POINTTOMULTIPOINT_TYPE:
            case OSPFv3Interface::VIRTUAL_TYPE:
                changeState(interface, new OSPFv3InterfaceStatePointToPoint, this);
                break;

            case OSPFv3Interface::NBMA_TYPE:
                if (interface->getRouterPriority() == 0) {
                    changeState(interface, new OSPFv3InterfaceStateDROther, this);
                }
                else {
                    changeState(interface, new OSPFv3InterfaceStateWaiting, this);
                    interface->getArea()->getInstance()->getProcess()->setTimer(interface->getWaitTimer(), interface->getDeadInterval());

                    long neighborCount = interface->getNeighborCount();
                    for (long i = 0; i < neighborCount; i++) {
                        OSPFv3Neighbor *neighbor = interface->getNeighbor(i);
                        if (neighbor->getNeighborPriority() > 0) {
                            neighbor->processEvent(OSPFv3Neighbor::START);
                        }
                    }
                }
                break;

            case OSPFv3Interface::BROADCAST_TYPE:
                if (interface->getRouterPriority() == 0) {
                    changeState(interface, new OSPFv3InterfaceStateDROther, this);
                }
                else {
                    changeState(interface, new OSPFv3InterfaceStateWaiting, this);
                    interface->getArea()->getInstance()->getProcess()->setTimer(interface->getWaitTimer(), interface->getDeadInterval());
                }
                break;

            default:
                break;
            }
        }
        else if(interface->isInterfacePassive()) {
            LinkLSA *lsa = interface->originateLinkLSA();
            interface->installLinkLSA(lsa);
            IntraAreaPrefixLSA *prefLsa = interface->getArea()->originateIntraAreaPrefixLSA();
            if (prefLsa != nullptr)
                if (!interface->getArea()->installIntraAreaPrefixLSA(prefLsa))
                    EV_DEBUG << "Intra Area Prefix LSA for network beyond interface " << interface->getIntName() << " was not created!\n";

            changeState(interface, new OSPFv3InterfacePassive, this);
        }
        if (event == OSPFv3Interface::LOOP_IND_EVENT) {
            interface->reset();
              changeState(interface, new OSPFv3InterfaceStateLoopback, this);
        }
    }
}//processEvent


}//namespace inet
