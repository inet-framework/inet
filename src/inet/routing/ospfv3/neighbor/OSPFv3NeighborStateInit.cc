/*
 * HELLO_RECEIVED - no change
 * 2-WAY_RECEIVED
 *  -if adjacency is not to be established - 2-WAY
 *  -if an adjacency is to be established - EXSTART
 *      Upon entering this state, the router increments the DD sequence number in the neighbor data structure.  If
        this is the first time that an adjacency has been
        attempted, the DD sequence number should be assigned
        some unique value (like the time of day clock).  It
        then declares itself master (sets the master/slave
        bit to master), and starts sending Database
        Description Packets, with the initialize (I), more
        (M) and master (MS) bits set.  This Database
        Description Packet should be otherwise empty.  This
        Database Description Packet should be retransmitted
        at intervals of RxmtInterval until the next state is entered
* KILL_NBR - new state DOWN
* LL_DOWN - new state DOWN
* INACTIVITY_TIMER - new state DOWN
* 1WAY_RECEIVED - no change
 *
 */
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateInit.h"

#include "inet/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborState2Way.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateDown.h"
#include "inet/routing/ospfv3/neighbor/OSPFv3NeighborStateExStart.h"


namespace inet{
void OSPFv3NeighborStateInit::processEvent(OSPFv3Neighbor *neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event)
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
    if (event == OSPFv3Neighbor::HELLO_RECEIVED) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getInactivityTimer());
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getInterface()->getDeadInterval());
    }
    if (event == OSPFv3Neighbor::TWOWAY_RECEIVED) {
        EV_DEBUG << "OSPFv3Neighbor::TWOWAY_RECEIVED caught in StateInit\n";
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
        else {
            changeState(neighbor, new OSPFv3NeighborState2Way, this);
        }
    }
}
}//namespace inet
