/*
 * EXCHANGE_DONE - new state FULL if the neighbor LSR is empty.
 *               - new state LOADING if the LSR is not empty
 *
 * ADJACENCY_OK? - of yes, stay, if not, it should be destroyed and the new state should be 2WAY
 * SEQUENCE_NUMBER_MISMATCH - new state EXSTART
 * BAD_LS_REQ - new state EXSTART
 * KILL_NBR - new state DOWN
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 * 1WAY_RECEIVED - new state INIT
 * 2WAY_RECEIVED - no change
 */
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateExchange.h"

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState2Way.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateExStart.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateFull.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateInit.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateLoading.h"


namespace inet{
void OSPFv3NeighborStateExchange::processEvent(OSPFv3Neighbor *neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event)
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
        if (!neighbor->needAdjacency()) {
            neighbor->reset();
            changeState(neighbor, new OSPFv3NeighborState2Way, this);
        }
    }
    if ((event == OSPFv3Neighbor::SEQUENCE_NUMBER_MISMATCH) || (event == OSPFv3Neighbor::BAD_LINK_STATE_REQUEST)) {
        neighbor->reset();
        neighbor->incrementDDSequenceNumber();
        neighbor->sendDDPacket(true);
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
        changeState(neighbor, new OSPFv3NeighborStateExStart, this);
    }
    if (event == OSPFv3Neighbor::EXCHANGE_DONE) {
        EV_DEBUG << "OSPFv3Neighbor::EXCHANGE_DONE caught in ExchangeState\n";
        if (!neighbor->isLinkStateRequestListEmpty()) {
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getDeadInterval());
            neighbor->clearRequestRetransmissionTimer();
            changeState(neighbor, new OSPFv3NeighborStateFull, this);
        }
        else {
            neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getDDRetransmissionTimer(), neighbor->getInterface()->getRetransmissionInterval());
            changeState(neighbor, new OSPFv3NeighborStateLoading, this);
        }
    }
    if (event == OSPFv3Neighbor::UPDATE_RETRANSMISSION_TIMER) {
        EV_DEBUG << "OSPFv3Neighbor::UPDATE_RETRANSMISSION_TIMER caught in ExchangeState\n";
        neighbor->retransmitUpdatePacket();                             //  ZAKOMENTOVANE PRED MIGRACIOU .  PRECO ?
        neighbor->startUpdateRetransmissionTimer();
    }
    if (event == OSPFv3Neighbor::REQUEST_RETRANSMISSION_TIMER) {
        EV_DEBUG << "OSPFv3Neighbor::REQUEST_RETRANSMISSION_TIMER caught in ExchangeState\n";
        neighbor->sendLinkStateRequestPacket();                             //  ZAKOMENTOVANE PRED MIGRACIOU .  PRECO ?
        neighbor->startRequestRetransmissionTimer();
    }
}
}//namespace inet
