/*
 * ADJACENCY_OK? - 2WAY if adjacency is not to be established
 *               - EXSTART otherwise and perform the same as in INIT for 2WAYRECEIVED action
 * KILL_NBR - new state DOWN
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 * 1WAY_RECEIVED - new state INIT
 * 2WAY_RECEIVED - no change
 */
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState2Way.h"

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateExStart.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateInit.h"


namespace inet{
void OSPFv3NeighborState2Way::processEvent(OSPFv3Neighbor *neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event)
{
    if ((event == OSPFv3Neighbor::KILL_NEIGHBOR) || (event == OSPFv3Neighbor::LINK_DOWN)) {
        neighbor->reset();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        changeState(neighbor, new OSPFv3NeighborStateDown, this);
    }
    if (event == OSPFv3Neighbor::INACTIVITY_TIMER) {
        neighbor->reset();
        if (neighbor->getInterface()->getType() == OSPFv3Interface::NBMA_TYPE)
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getPollTimer(), neighbor->getInterface()->getPollInterval());

        changeState(neighbor, new OSPFv3NeighborStateDown, this);
    }
    if (event == OSPFv3Neighbor::ONEWAY_RECEIVED) {
        neighbor->reset();
        changeState(neighbor, new OSPFv3NeighborStateInit, this);
    }
    if (event == OSPFv3Neighbor::HELLO_RECEIVED) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getInterface()->getDeadInterval());
    }
    if (event == OSPFv3Neighbor::IS_ADJACENCY_OK) {
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
            changeState(neighbor, new OSPFv3NeighborStateExStart, this);
        }
    }
}
}//namespace inet
