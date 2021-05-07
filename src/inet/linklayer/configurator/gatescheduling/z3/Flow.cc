//
// Copyright (C) 2021 by original authors
//
// This file is copied from the following project with the explicit permission
// from the authors: https://github.com/ACassimiro/TSNsched
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/linklayer/configurator/gatescheduling/z3/Flow.h"
#include "inet/linklayer/configurator/gatescheduling/z3/FlowFragment.h"

namespace inet {

FlowFragment *Flow::nodeToZ3(context& ctx, PathNode *node, FlowFragment *frag)
{
    FlowFragment *flowFrag = nullptr;

    // If, by chance, the given node has no child, then its a leaf
    if (node->getChildren().size() == 0) {
        //System.out.println(std::string("On flow " + name + std::string(" leaving on node ")) + ((Device *) node.getNode())->getName());
        return flowFrag;
    }

    // Iterate over node's children
    for (PathNode *auxN : node->getChildren()) {

        // For each grand children of the current child node
        for (PathNode *n : auxN->getChildren()) {

            // Create a new flow fragment
            flowFrag = new FlowFragment(this);

            // Setting next hop
            if (dynamic_cast<Switch *>(n->getNode())) {
                flowFrag->setNextHop(
                        ((Switch *) n->getNode())->getName());
            } else {
                flowFrag->setNextHop(
                        ((Device *) n->getNode())->getName());
            }

            if (((Switch *)auxN->getNode())->getPortOf(flowFrag->getNextHop())->checkIfAutomatedApplicationPeriod()) {
                numberOfPackets = (int) (((Switch *)auxN->getNode())->getPortOf(flowFrag->getNextHop())->getDefinedHyperCycleSize()/flowSendingPeriodicity);
            }
            flowFrag->setNumOfPacketsSent(numberOfPackets);

            if (auxN->getParent()->getParent() == nullptr) { //First flow fragment, fragment first departure = device's first departure
                flowFrag->setNodeName(((Switch *)auxN->getNode())->getName());

                for (int i = 0; i < numberOfPackets; i++) {

                    flowFrag->addDepartureTimeZ3(
                                    flowFirstSendingTimeZ3 +
                                    ctx.real_val(std::to_string(flowSendingPeriodicity * i).c_str()));
                }
            } else { // Fragment first departure = last fragment scheduled time

                for (int i = 0; i < numberOfPackets; i++) {

                    flowFrag->addDepartureTimeZ3( // Flow fragment link constraint
                            *((Switch *) auxN->getParent()->getNode())
                                    ->scheduledTime(
                                            ctx,
                                            i,
                                            auxN->getParent()->getFlowFragments().at(auxN->getParent()->getChildIndex(auxN))));
                }

                flowFrag->setNodeName(((Switch *) auxN->getNode())->getName());
            }

            // Setting z3 properties of the flow fragment
            flowFrag->setFragmentPriorityZ3(*flowPriority);

            auto connectsTo = ((Switch *) auxN->getNode())->getConnectsTo();
            int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
            flowFrag->setPort(
                    ((Switch *) auxN->getNode())
                            ->getPorts().at(portIndex));

            for (int i = 0; i<flowFrag->getNumOfPacketsSent(); i++){
                flowFrag->addScheduledTimeZ3(
                    *flowFrag->getPort()->scheduledTime(ctx, i, flowFrag));
            }

            flowFrag->setPacketPeriodicityZ3(*flowSendingPeriodicityZ3);
            flowFrag->setPacketSizeZ3(*startDevice->getPacketSizeZ3());
            flowFrag->setStartDevice(startDevice);
            flowFrag->setReferenceToNode(auxN);

            //Adding fragment to the fragment list and to the switch's fragment list
            auxN->addFlowFragment(flowFrag);
            ((Switch *)auxN->getNode())->addToFragmentList(flowFrag);
            // System.out.println(std::string("Adding fragment to switch " + ((Switch *)auxN->getNode())->getName() + std::string(" has ") + auxN->getChildren().size() + " children"));

        }

        if (flowFrag == nullptr){
            continue;
        }

        if (frag != nullptr && flowFrag->getPreviousFragment() == nullptr) {
            flowFrag->setPreviousFragment(frag);
        }

        // Recursively repeats process to children
        FlowFragment *nextFragment = nodeToZ3(ctx, auxN, flowFrag);


        if (nextFragment != nullptr) {
            flowFrag->addToNextFragments(nextFragment);
        }
    }

    return flowFrag;
}

void Flow::pathToZ3(context& ctx, Switch *swt, int currentSwitchIndex)
{
    // Flow fragment is created
    FlowFragment *flowFrag = new FlowFragment(this);

    /*
     * If this flow fragment is the same on the fragment list, then
     * this fragment departure time = source device departure time. Else,
     * this fragment departure time = last fragment scheduled time.
     */
    if (flowFragments.size() == 0) {
        // If no flowFragment has been added to the path, flowPriority is nullptr, so initiate it
        //flowFrag->setNodeName(startDevice->getName());
        for (int i = 0; i < numberOfPackets; i++) {
            flowFrag->addDepartureTimeZ3( // Packet departure constraint
                            flowFirstSendingTimeZ3 +
                            ctx.real_val(std::to_string(flowSendingPeriodicity * i).c_str()));
        }
    } else {
        for (int i = 0; i < numberOfPackets; i++) {
            flowFrag->addDepartureTimeZ3(
                    *((Switch *) path.at(currentSwitchIndex - 1))->scheduledTime(ctx, i, flowFragments.at(flowFragments.size() - 1)));
        }
    }
    flowFrag->setNodeName(((Switch *) path.at(currentSwitchIndex))->getName());

    // Setting extra flow properties
    flowFrag->setFragmentPriorityZ3(ctx.int_val((flowFrag->getName() + std::string("Priority")).c_str()));
    flowFrag->setPacketPeriodicityZ3(*flowSendingPeriodicityZ3);
    flowFrag->setPacketSizeZ3(*startDevice->getPacketSizeZ3());

    /*
     * If index of current switch = last switch in the path, then
     * next hop will be to the end device, else, next hop will be to
     * the next switch in the path.
     */

    if ((path.size() - 1) == currentSwitchIndex) {
        flowFrag->setNextHop(endDevice->getName());
    } else {
        flowFrag->setNextHop(
                path.at(currentSwitchIndex + 1)->getName());
    }

    /*
     * The newly created fragment is added to both the switch
     * (on the list of fragments that go through it) and to
     * the flow fragment list of this flow.
     */

    ((Switch *)swt)->addToFragmentList(flowFrag);
    flowFragments.push_back(flowFrag);
}

void Flow::bindToNextFragment(solver& solver, context& ctx, FlowFragment *frag)
{
    if (frag->getNextFragments().size() > 0){

        for (FlowFragment *childFrag : frag->getNextFragments()){

            for (int i = 0; i < numOfPacketsSentInFragment; i++){
//                    System.out.println(std::string("On fragment " + frag->getName() + std::string(" making " + frag->getPort()->scheduledTime(ctx, i, frag) + " = " + childFrag->getPort()->departureTime(ctx, i, childFrag) + " that leads to ")) + childFrag->getPort()->scheduledTime(ctx, i, childFrag)
//                            + std::string(" on cycle of port ") + frag->getPort()->getCycle().getFirstCycleStartZ3());
                addAssert(solver, frag->getPort()->scheduledTime(ctx, i, frag) ==
                                  childFrag->getPort()->departureTime(ctx, i, childFrag));
            }

            bindToNextFragment(solver, ctx, childFrag);

        }

    }
}

double Flow::getDepartureTime(int hop, int packetNum) {
    double time;
    time = getFlowFragments().at(hop)->getDepartureTime(
            packetNum);
    return time;
}

double Flow::getDepartureTime(std::string deviceName, int hop,
        int packetNum) {
    double time;
    Device *targetDevice = nullptr;
    std::vector<FlowFragment*> *auxFlowFragments;
    for (cObject *node : *pathTree->getLeaves()) {
        if (dynamic_cast<Device*>(node)) {
            if (((Device*) ((node)))->getName() == deviceName) {
                targetDevice = (Device*) ((node));
            }
        }
    }
    if (targetDevice == nullptr) {
        //TODO: Throw error
    }
    auxFlowFragments = getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getDepartureTime(packetNum);
    return time;
}

double Flow::getDepartureTime(Device *targetDevice, int hop,
        int packetNum) {
    double time;
    std::vector<FlowFragment*> *auxFlowFragments;
//    auto leaves = pathTree->getLeaves();
//    if (std::find(leaves->begin(), leaves->end(), targetDevice) == leaves->end()) {
//        //TODO: Throw error
//    }
    auxFlowFragments = getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getDepartureTime(packetNum);
    return time;
}

double Flow::getArrivalTime(Device *targetDevice, int hop,
        int packetNum) {
    double time;
    std::vector<FlowFragment*> *auxFlowFragments;
//    auto leaves = pathTree->getLeaves();
//    if (std::find(leaves->begin(), leaves->end(), targetDevice) == leaves->end()) {
//        //TODO: Throw error
//    }
    auxFlowFragments = getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getArrivalTime(packetNum);
    return time;
}

double Flow::getArrivalTime(int hop, int packetNum)
{
    double time;
    time = getFlowFragments().at(hop)->getArrivalTime(packetNum);
    return time;
}

double Flow::getArrivalTime(std::string deviceName, int hop, int packetNum)
{
    double time;
    Device *targetDevice = nullptr;
    std::vector<FlowFragment*> *auxFlowFragments;
    for (cObject *node : *pathTree->getLeaves()) {
        if (dynamic_cast<Device*>(node)) {
            if (((Device*) (node))->getName() == deviceName) {
                targetDevice = (Device*) (node);
            }
        }
    }
    if (targetDevice == nullptr) {
        //TODO: Throw error
    }
    auxFlowFragments = getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getArrivalTime(packetNum);
    return time;
}

double Flow::getScheduledTime(Device *targetDevice, int hop, int packetNum)
{
    double time;
    std::vector<FlowFragment*> *auxFlowFragments;
//    auto leaves = pathTree->getLeaves();
//    if (std::find(leaves->begin(), leaves->end(), targetDevice) == leaves->end()) {
//        //TODO: Throw error
//    }
    auxFlowFragments = getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getScheduledTime(packetNum);
    return time;
}

double Flow::getScheduledTime(std::string deviceName, int hop, int packetNum)
{
    double time;
    Device *targetDevice = nullptr;
    std::vector<FlowFragment*> *auxFlowFragments;
    for (cObject *node : *pathTree->getLeaves()) {
        if (dynamic_cast<Device*>(node)) {
            if (((Device*) (node))->getName() == deviceName) {
                targetDevice = (Device*) (node);
            }
        }
    }
    if (targetDevice == nullptr) {
        //TODO: Throw error
    }
    auxFlowFragments = getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getScheduledTime(packetNum);
    return time;
}

double Flow::getScheduledTime(int hop, int packetNum)
{
    double time;
    time = getFlowFragments().at(hop)->getScheduledTime(packetNum);
    return time;
}

double Flow::getAverageLatency()
{
    double averageLatency = 0;
    double auxAverageLatency = 0;
    int timeListSize = 0;
    Device *endDevice = nullptr;
    if (type == UNICAST) {
        timeListSize = getTimeListSize();
        for (int i = 0; i < timeListSize; i++) {
            averageLatency += getScheduledTime(
                    flowFragments.size() - 1, i)
                    - getDepartureTime(0, i);
        }

        averageLatency = averageLatency / (timeListSize);

    } else if (type == PUBLISH_SUBSCRIBE) {

        for (PathNode *node : *pathTree->getLeaves()) {
            timeListSize =
                    pathTree->getRoot()->getChildren().at(0)->getFlowFragments().at(
                            0)->getArrivalTimeList().size();
            ;
            endDevice = (Device*) node->getNode();
            auxAverageLatency = 0;

            for (int i = 0; i < timeListSize; i++) {
                auxAverageLatency += getScheduledTime(endDevice,
                        getFlowFromRootToNode(endDevice)->size() - 1, i)
                        - getDepartureTime(endDevice, 0, i);
            }

            auxAverageLatency = auxAverageLatency / timeListSize;

            averageLatency += auxAverageLatency;

        }

        averageLatency = averageLatency / pathTree->getLeaves()->size();

    } else {
        // TODO: Throw error
        ;
    }

    return averageLatency;
}

double Flow::getAverageLatencyToDevice(Device *dev)
{
    double averageLatency = 0;
//    double auxAverageLatency = 0;
//    Device *endDevice = nullptr;
    std::vector<FlowFragment*> *fragments = getFlowFromRootToNode(dev);
    for (int i = 0; i < fragments->at(0)->getParent()->getNumOfPacketsSent();
            i++) {
        averageLatency += getScheduledTime(dev, fragments->size() - 1, i)
                - getDepartureTime(dev, 0, i);
    }
    averageLatency = averageLatency
            / (fragments->at(0)->getParent()->getNumOfPacketsSent());
    return averageLatency;
}

double Flow::getAverageJitterToDevice(Device *dev)
{
    double averageJitter = 0;
    double averageLatency = getAverageLatencyToDevice(dev);
    std::vector<FlowFragment*> *fragments = getFlowFromRootToNode(dev);
    for (int i = 0; i < fragments->at(0)->getNumOfPacketsSent(); i++) {
        averageJitter += fabs(
                getScheduledTime(dev,
                        getFlowFromRootToNode(dev)->size() - 1, i)
                        - getDepartureTime(dev, 0, i) - averageLatency);
    }
    averageJitter = averageJitter / fragments->at(0)->getNumOfPacketsSent();
    return averageJitter;
}

std::shared_ptr<expr> Flow::getLatencyZ3(solver& solver, context &ctx, int index)
{
    //index += 1;
    std::shared_ptr<expr> latency = std::make_shared<expr>(
            ctx.real_const(
                    (name + std::string("latencyOfPacket")
                            + std::to_string(index)).c_str()));
    Switch *lastSwitchInPath =
            ((Switch*) (path.at(path.size() - 1)));
    FlowFragment *lastFragmentInList = flowFragments.at(
            flowFragments.size() - 1);
    Switch *firstSwitchInPath = ((Switch*) (path.at(0)));
    FlowFragment *firstFragmentInList = flowFragments.at(0);
    addAssert(solver, latency == (lastSwitchInPath->getPortOf(
                                      lastFragmentInList->getNextHop())->scheduledTime(
                                      ctx, index, lastFragmentInList) -
                                  firstSwitchInPath->getPortOf(
                                      firstFragmentInList->getNextHop())->departureTime(
                                      ctx, index, firstFragmentInList)));
    return latency;
}

std::shared_ptr<expr> Flow::getLatencyZ3(solver& solver, Device *dev, context &ctx, int index)
{
    //index += 1;
    std::shared_ptr<expr> latency = std::make_shared<expr>(
            ctx.real_const(
                    (name
                            + std::string(
                                    "latencyOfPacket" + std::to_string(index)
                                            + std::string("For"))
                            + dev->getName()).c_str()));
    std::vector<PathNode*> *nodes = getNodesFromRootToNode(dev);
    std::vector<FlowFragment*> *flowFrags = getFlowFromRootToNode(dev);
    Switch *lastSwitchInPath =
            ((Switch*) (nodes->at(nodes->size() - 2)->getNode())); // - 1 for indexing, - 1 for last node being the end device
    FlowFragment *lastFragmentInList = flowFrags->at(flowFrags->size() - 1);
    Switch *firstSwitchInPath = ((Switch*) (nodes->at(1)->getNode())); // 1 since the first node is the publisher
    FlowFragment *firstFragmentInList = flowFrags->at(0);
    addAssert(solver, latency == (lastSwitchInPath->getPortOf(
                                      lastFragmentInList->getNextHop())->scheduledTime(
                                      ctx, index, lastFragmentInList) -
                                  firstSwitchInPath->getPortOf(
                                      firstFragmentInList->getNextHop())->departureTime(
                                      ctx, index, firstFragmentInList)));
    return latency;
}

std::shared_ptr<expr> Flow::getJitterZ3(Device *dev, solver& solver, context &ctx, int index)
{
    //index += 1;
    std::shared_ptr<expr> jitter = std::make_shared<expr>(ctx.real_const(
            (name
                    + std::string("JitterOfPacket" + std::to_string(index) + std::string("For"))
                    + dev->getName()).c_str()));
    std::vector<PathNode*> *nodes = getNodesFromRootToNode(dev);
    Switch *lastSwitchInPath =
            ((Switch*) (nodes->at(nodes->size() - 2)->getNode())); // - 1 for indexing, - 1 for last node being the end device
    FlowFragment *lastFragmentInList =
            nodes->at(nodes->size() - 2)->getFlowFragments().at(
                    nodes->at(nodes->size() - 2)->getChildIndex(
                            nodes->at(nodes->size() - 1)));
    Switch *firstSwitchInPath = ((Switch*) (nodes->at(1)->getNode())); // 1 since the first node is the publisher
    FlowFragment *firstFragmentInList = nodes->at(1)->getFlowFragments().at(0);
    // z3::expr avgLatency = (z3::expr) mkDiv(getSumOfLatencyZ3(solver, dev, ctx, index), ctx.int_val(PACKETUPPERBOUNDRANGE - 1));
    std::shared_ptr<expr> avgLatency = getAvgLatency(dev, solver, ctx);
    expr latency = (
                    lastSwitchInPath->getPortOf(
                            lastFragmentInList->getNextHop())->scheduledTime(
                            ctx, index, lastFragmentInList) -
                    firstSwitchInPath->getPortOf(
                            firstFragmentInList->getNextHop())->departureTime(
                            ctx, index, firstFragmentInList));
    addAssert(solver, jitter == (latency >= avgLatency ? latency - avgLatency : avgLatency - latency));
    return jitter;
}

void Flow::setNumberOfPacketsSent(PathNode *node)
{
    if (dynamic_cast<Device*>(node->getNode())
            && (node->getChildren().size() == 0)) {
        return;
    } else if (dynamic_cast<Device*>(node->getNode())) {
        for (PathNode *child : node->getChildren()) {
            setNumberOfPacketsSent(child);
        }
    } else {
        int index = 0;
        for (FlowFragment *frag : node->getFlowFragments()) {
            if (numOfPacketsSentInFragment
                    < frag->getNumOfPacketsSent()) {
                numOfPacketsSentInFragment = frag->getNumOfPacketsSent();
            }
            // System.out.println(std::string("On node ") + ((Switch *)node->getNode())->getName() + std::string(" trying to reach children"));
            // System.out.println(std::string("Node has: ") + node.getFlowFragments().size() + std::string(" frags"));
            // System.out.println(std::string("Node has: ") + node->getChildren().size() + std::string(" children"));
            // for (PathNode n : node->getChildren()) {
            //         System.out.println(std::string("Child is a: ") + (dynamic_cast<Device *>(n->getNode()) ? "Device" : "Switch"));
            // }
            setNumberOfPacketsSent(node->getChildren().at(index));
            index = index + 1;
        }
    }
}

int Flow::getTimeListSize()
{
    return getFlowFragments().at(0)->getArrivalTimeList().size();
}

}
