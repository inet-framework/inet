/*
 * ProbabilisticBroadcast.cc
 *
 *  Created on: Nov 26, 2008
 *      Author: Damien Piguet, Dimitris Kotsakos, Jérôme Rousselot
 */

#include "inet/networklayer/probabilistic/AdaptiveProbabilisticBroadcast.h"

namespace inet {

using std::map;
using std::pair;
using std::make_pair;
using std::endl;

Define_Module(AdaptiveProbabilisticBroadcast);

void AdaptiveProbabilisticBroadcast::initialize(int stage)
{
    ProbabilisticBroadcast::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        beta = 1.0;

        bvec.setName("Beta Vector");

        timeInNeighboursTable = par("timeInNeighboursTable");
    }
}

void AdaptiveProbabilisticBroadcast::handleLowerPacket(Packet *packet)
{
    const auto& macHeader = packet->peekAtFront<ProbabilisticBroadcastHeader>();
    // Update neighbors table before calling the method of the super class
    // because it may delete the message.
    updateNeighMap(macHeader.get());
    ProbabilisticBroadcast::handleLowerPacket(packet);
}

void AdaptiveProbabilisticBroadcast::updateNeighMap(const ProbabilisticBroadcastHeader *m)
{
    //find the network address of the node who sent the msg
    NeighborMap::key_type nodeAddress = m->getSrcAddr();
    //EV << "updateNeighMap(): neighAddress: " << nodeAddress << endl;

    //search for it in the "already-neighbors" map
    auto it = neighMap.find(nodeAddress);

    //if the node is a "new" neighbor
    if (it == neighMap.end()) {
        EV << "updateNeighMap(): The message came from a new neighbor! " << endl;

        // insert key value pair <node address, event> in neighborhood map.
        cMessage *removeEvent = new cMessage("removeEvent", NEIGHBOR_TIMER);

        // schedule the event to remove the entry after initT seconds
        scheduleAfter(timeInNeighboursTable, removeEvent);

        NeighborMap::value_type pairToInsert = make_pair(nodeAddress, removeEvent);
        pair<NeighborMap::iterator, bool> ret = neighMap.insert(pairToInsert);

        // set the context pointer to point to the integer that resembles to the address of
        // the node to be removed when the corresponding event occurs
        (ret.first)->second->setContextPointer((void *)(&(ret.first)->first));
    }
    //if the node is NOT a "new" neighbor update its timer
    else {
        EV << "updateNeighMap(): The message came from an already known neighbor! " << endl;
        //cancel the event that was scheduled to remove the entry for this neighbor
        cancelEvent(it->second);
        // Define a new event in order to remove the entry after initT seconds
        // Set the context pointer to point to the integer that resembles to the address of
        // the node to be removed when the corresponding event occurs
        it->second->setContextPointer((void *)(&it->first));

        scheduleAfter(timeInNeighboursTable, it->second);
    }
    updateBeta();
}

void AdaptiveProbabilisticBroadcast::handleSelfMessage(cMessage *msg)
{
    if (msg->getKind() == NEIGHBOR_TIMER) {
        const NeighborMap::key_type& node = *static_cast<NeighborMap::key_type *>(msg->getContextPointer());
        EV << "handleSelfMsg(): Remove node " << node << " from NeighMap!" << endl;
        auto it = neighMap.find(node);
        cancelAndDelete(neighMap.find(it->first)->second);
        neighMap.erase(it);
        updateBeta();
    }
    else {
        ProbabilisticBroadcast::handleSelfMessage(msg);
    }
}

void AdaptiveProbabilisticBroadcast::updateBeta()
{
    int k = neighMap.size();

    // those values are derived from the simulations
    // with the non-adaptive protocol.
    if (k < 4)
        beta = 1.0;
    else if (k < 6)
        beta = 0.9;
    else if (k < 8)
        beta = 0.8;
    else if (k < 10)
        beta = 0.7;
    else if (k < 12)
        beta = 0.6;
    else if (k < 14)
        beta = 0.5;
    else if (k < 16)
        beta = 0.4;
    else if (k < 18)
        beta = 0.3;
    else if (k < 20)
        beta = 0.2;
    else
        beta = 0.1;
    bvec.record(beta);
}

} // namespace inet

