/*
 * NEGOTIATION_DONE - new state EXCHANGE
 * ADJACENCY_OK? - of yes, stay, if not, it should be destroyed and the new state should be 2WAY
 * KILL_NBR - new state DOWN
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 * 1WAY_RECEIVED - new state INIT
 * 2WAY_RECEIVED - no change
 */
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateExStart.h"

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState2Way.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateExchange.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateInit.h"


namespace inet{
void OSPFv3NeighborStateExStart::processEvent(OSPFv3Neighbor *neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event)
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
        EV_DEBUG << "OSPFv3Neighbor::IS_ADJACENCY_OK caught in ExStartState for neighbor "<<neighbor->getNeighborID() << "\n";
        if (!neighbor->needAdjacency()) {
            EV_DEBUG << "The adjacency is needed for neighbor "<<neighbor->getNeighborID() << "\n";
            neighbor->reset();
            changeState(neighbor, new OSPFv3NeighborState2Way, this);
        }
    }
    if (event == OSPFv3Neighbor::DD_RETRANSMISSION_TIMER) {
        EV_DEBUG << "OSPFv3Neighbor::DD_RETRANSMISSION_TIMER caught in ExStartState\n";
        neighbor->retransmitDatabaseDescriptionPacket();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
    }
    if (event == OSPFv3Neighbor::NEGOTIATION_DONE) {
        EV_DEBUG << "OSPFv3Neighbor::NEGOTIATION_DONE caught in ExStartState\n";
        neighbor->createDatabaseSummary();
        EV_DEBUG << "SummaryListCount " << neighbor->getDatabaseSummaryListCount() << endl;
        neighbor->sendDDPacket();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getDDRetransmissionTimer());
        changeState(neighbor, new OSPFv3NeighborStateExchange, this);
    }
}
}//namespace inet
