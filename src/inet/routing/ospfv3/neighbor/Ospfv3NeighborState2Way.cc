/*
 * ADJACENCY_OK? - 2WAY if adjacency is not to be established
 *               - EXSTART otherwise and perform the same as in INIT for 2WAYRECEIVED action
 * KILL_NBR - new state DOWN
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 * 1WAY_RECEIVED - new state INIT
 * 2WAY_RECEIVED - no change
 */
#include "inet/routing/ospfv3/neighbor/Ospfv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborState2Way.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateExStart.h"
#include "inet/routing/ospfv3/neighbor/Ospfv3NeighborStateInit.h"

namespace inet {
namespace ospfv3 {

void Ospfv3NeighborState2Way::processEvent(Ospfv3Neighbor *neighbor, Ospfv3Neighbor::Ospfv3NeighborEventType event)
{
    if ((event == Ospfv3Neighbor::KILL_NEIGHBOR) || (event == Ospfv3Neighbor::LINK_DOWN)) {
        neighbor->reset();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        changeState(neighbor, new Ospfv3NeighborStateDown, this);
    }
    if (event == Ospfv3Neighbor::INACTIVITY_TIMER) {
        neighbor->reset();
        if (neighbor->getInterface()->getType() == Ospfv3Interface::NBMA_TYPE)
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());

        changeState(neighbor, new Ospfv3NeighborStateDown, this);
    }
    if (event == Ospfv3Neighbor::ONEWAY_RECEIVED) {
        neighbor->reset();
        changeState(neighbor, new Ospfv3NeighborStateInit, this);
    }
    if (event == Ospfv3Neighbor::HELLO_RECEIVED) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getInterface()->getDeadInterval());
    }
    if (event == Ospfv3Neighbor::IS_ADJACENCY_OK) {
        EV_DEBUG << "IS_ADJACENCY_OK in Neighbor state 2way\n";
        if (neighbor->needAdjacency()) {
            if (!(neighbor->isFirstAdjacencyInited())) {
                neighbor->initFirstAdjacency();
            }
            else {
                neighbor->incrementDDSequenceNumber();
            }
            neighbor->sendDDPacket(true);
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
            changeState(neighbor, new Ospfv3NeighborStateExStart, this);
        }
    }
}

} // namespace ospfv3
}//namespace inet

