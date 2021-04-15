#include "inet/linklayer/configurator/z3/Flow.h"
#include "inet/linklayer/configurator/z3/FlowFragment.h"

namespace inet {

FlowFragment *Flow::nodeToZ3(context& ctx, PathNode *node, FlowFragment *frag)
{
    FlowFragment *flowFrag = nullptr;
    int numberOfPackets = PACKETUPPERBOUNDRANGE;

    // If, by chance, the given node has no child, then its a leaf
    if(node->getChildren().size() == 0) {
        //System.out.println(std::string("On flow " + this->name + std::string(" leaving on node ")) + ((Device *) node.getNode())->getName());
        return flowFrag;
    }

    // Iterate over node's children
    for(PathNode *auxN : node->getChildren()) {

        // For each grand children of the current child node
        for(PathNode *n : auxN->getChildren()) {

            // Create a new flow fragment
            flowFrag = new FlowFragment(this);

            // Setting next hop
            if(dynamic_cast<TSNSwitch *>(n->getNode())) {
                flowFrag->setNextHop(
                        ((TSNSwitch *) n->getNode())->getName()
                );
            } else {
                flowFrag->setNextHop(
                        ((Device *) n->getNode())->getName()
                );
            }

            if(((TSNSwitch *)auxN->getNode())->getPortOf(flowFrag->getNextHop())->checkIfAutomatedApplicationPeriod()) {
                numberOfPackets = (int) (((TSNSwitch *)auxN->getNode())->getPortOf(flowFrag->getNextHop())->getDefinedHyperCycleSize()/this->flowSendingPeriodicity);
                flowFrag->setNumOfPacketsSent(numberOfPackets);
            }

            if(auxN->getParent()->getParent() == nullptr) { //First flow fragment, fragment first departure = device's first departure
                flowFrag->setNodeName(((Switch *)auxN->getNode())->getName());

                for (int i = 0; i < numberOfPackets; i++) {

                    flowFrag->addDepartureTimeZ3(
                            (z3::expr) mkAdd(
                                    this->flowFirstSendingTimeZ3,
                                    ctx.real_val(std::to_string(this->flowSendingPeriodicity * i).c_str())
                            )
                    );

                }


            } else { // Fragment first departure = last fragment scheduled time

                for (int i = 0; i < numberOfPackets; i++) {

                    flowFrag->addDepartureTimeZ3( // Flow fragment link constraint
                            *((TSNSwitch *) auxN->getParent()->getNode())
                                    ->scheduledTime(
                                            ctx,
                                            i,
                                            auxN->getParent()->getFlowFragments().at(auxN->getParent()->getChildIndex(auxN))
                                    )
                    );

                }

                flowFrag->setNodeName(((TSNSwitch *) auxN->getNode())->getName());

            }


            // Setting z3 properties of the flow fragment
            if(this->fixedPriority) {
                flowFrag->setFragmentPriorityZ3(*this->flowPriority); // FIXED PRIORITY (Fixed priority per flow constraint)
            } else {
                flowFrag->setFragmentPriorityZ3(ctx.int_const((flowFrag->getName() + std::string("Priority")).c_str()));
            }

            auto connectsTo = ((TSNSwitch *) auxN->getNode())->getConnectsTo();
            int portIndex = std::find(connectsTo.begin(), connectsTo.end(), flowFrag->getNextHop()) - connectsTo.begin();
            flowFrag->setPort(
                    ((TSNSwitch *) auxN->getNode())
                            ->getPorts().at(portIndex)
            );

            for (int i = 0; i<flowFrag->getNumOfPacketsSent(); i++){
                flowFrag->addScheduledTimeZ3(
                    *flowFrag->getPort()->scheduledTime(ctx, i, flowFrag)
                );
            }

            flowFrag->setPacketPeriodicityZ3(*this->flowSendingPeriodicityZ3);
            flowFrag->setPacketSizeZ3(*startDevice->getPacketSizeZ3());
            flowFrag->setStartDevice(this->startDevice);
            flowFrag->setReferenceToNode(auxN);

            //Adding fragment to the fragment list and to the switch's fragment list
            auxN->addFlowFragment(flowFrag);
            ((TSNSwitch *)auxN->getNode())->addToFragmentList(flowFrag);
            // System.out.println(std::string("Adding fragment to switch " + ((TSNSwitch *)auxN->getNode())->getName() + std::string(" has ") + auxN->getChildren().size() + " children"));

        }

        if(flowFrag == nullptr){
            continue;
        }

        if(frag != nullptr && flowFrag->getPreviousFragment() == nullptr) {
            flowFrag->setPreviousFragment(frag);
        }

        // Recursively repeats process to children
        FlowFragment *nextFragment = this->nodeToZ3(ctx, auxN, flowFrag);


        if(nextFragment != nullptr) {
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
    if(flowFragments.size() == 0) {
        // If no flowFragment has been added to the path, flowPriority is nullptr, so initiate it
        //flowFrag->setNodeName(this->startDevice->getName());
        for (int i = 0; i < PACKETUPPERBOUNDRANGE; i++) {
            flowFrag->addDepartureTimeZ3( // Packet departure constraint
                    (z3::expr) mkAdd(
                            this->flowFirstSendingTimeZ3,
                            ctx.real_val(std::to_string(this->flowSendingPeriodicity * i).c_str())
                    )
            );
        }
    } else {
        for (int i = 0; i < PACKETUPPERBOUNDRANGE; i++) {
            flowFrag->addDepartureTimeZ3(
                    *((TSNSwitch *) path.at(currentSwitchIndex - 1))->scheduledTime(ctx, i, flowFragments.at(flowFragments.size() - 1))
            );
        }
    }
    flowFrag->setNodeName(((TSNSwitch *) path.at(currentSwitchIndex))->getName());

    // Setting extra flow properties
    flowFrag->setFragmentPriorityZ3(ctx.int_val((flowFrag->getName() + std::string("Priority")).c_str()));
    flowFrag->setPacketPeriodicityZ3(*this->flowSendingPeriodicityZ3);
    flowFrag->setPacketSizeZ3(*startDevice->getPacketSizeZ3());

    /*
     * If index of current switch = last switch in the path, then
     * next hop will be to the end device, else, next hop will be to
     * the next switch in the path.
     */

    if((path.size() - 1) == currentSwitchIndex) {
        flowFrag->setNextHop(this->endDevice->getName());
    } else {
        flowFrag->setNextHop(
                path.at(currentSwitchIndex + 1)->getName()
        );
    }

    /*
     * The newly created fragment is added to both the switch
     * (on the list of fragments that go through it) and to
     * the flow fragment list of this flow.
     */

    ((TSNSwitch *)swt)->addToFragmentList(flowFrag);
    flowFragments.push_back(flowFrag);
}

void Flow::bindToNextFragment(solver& solver, context& ctx, FlowFragment *frag)
{
    if(frag->getNextFragments().size() > 0){

        for(FlowFragment *childFrag : frag->getNextFragments()){

            for (int i = 0; i < this->numOfPacketsSentInFragment; i++){
//                    System.out.println(std::string("On fragment " + frag->getName() + std::string(" making " + frag->getPort()->scheduledTime(ctx, i, frag) + " = " + childFrag->getPort()->departureTime(ctx, i, childFrag) + " that leads to ")) + childFrag->getPort()->scheduledTime(ctx, i, childFrag)
//                            + std::string(" on cycle of port ") + frag->getPort()->getCycle().getFirstCycleStartZ3());
                addAssert(solver,
                    mkEq(
                        frag->getPort()->scheduledTime(ctx, i, frag),
                        childFrag->getPort()->departureTime(ctx, i, childFrag)
                    )
                );
            }

            this->bindToNextFragment(solver, ctx, childFrag);

        }

    }
}

float Flow::getDepartureTime(int hop, int packetNum) {
    float time;
    time = this->getFlowFragments().at(hop)->getDepartureTime(
            packetNum);
    return time;
}

float Flow::getDepartureTime(std::string deviceName, int hop,
        int packetNum) {
    float time;
    Device *targetDevice = nullptr;
    std::vector<FlowFragment*> *auxFlowFragments;
    for (cObject *node : *this->pathTree->getLeaves()) {
        if (dynamic_cast<Device*>(node)) {
            if (((Device*) ((node)))->getName() == deviceName) {
                targetDevice = (Device*) ((node));
            }
        }
    }
    if (targetDevice == nullptr) {
        //TODO: Throw error
    }
    auxFlowFragments = this->getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getDepartureTime(packetNum);
    return time;
}

float Flow::getDepartureTime(Device *targetDevice, int hop,
        int packetNum) {
    float time;
    std::vector<FlowFragment*> *auxFlowFragments;
//    auto leaves = pathTree->getLeaves();
//    if (std::find(leaves->begin(), leaves->end(), targetDevice) == leaves->end()) {
//        //TODO: Throw error
//    }
    auxFlowFragments = this->getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getDepartureTime(packetNum);
    return time;
}

float Flow::getArrivalTime(Device *targetDevice, int hop,
        int packetNum) {
    float time;
    std::vector<FlowFragment*> *auxFlowFragments;
//    auto leaves = pathTree->getLeaves();
//    if (std::find(leaves->begin(), leaves->end(), targetDevice) == leaves->end()) {
//        //TODO: Throw error
//    }
    auxFlowFragments = this->getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getArrivalTime(packetNum);
    return time;
}

float Flow::getArrivalTime(int hop, int packetNum)
{
    float time;
    time = this->getFlowFragments().at(hop)->getArrivalTime(packetNum);
    return time;
}

float Flow::getArrivalTime(std::string deviceName, int hop, int packetNum)
{
    float time;
    Device *targetDevice = nullptr;
    std::vector<FlowFragment*> *auxFlowFragments;
    for (cObject *node : *this->pathTree->getLeaves()) {
        if (dynamic_cast<Device*>(node)) {
            if (((Device*) (node))->getName() == deviceName) {
                targetDevice = (Device*) (node);
            }
        }
    }
    if (targetDevice == nullptr) {
        //TODO: Throw error
    }
    auxFlowFragments = this->getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getArrivalTime(packetNum);
    return time;
}

float Flow::getScheduledTime(Device *targetDevice, int hop, int packetNum)
{
    float time;
    std::vector<FlowFragment*> *auxFlowFragments;
//    auto leaves = pathTree->getLeaves();
//    if (std::find(leaves->begin(), leaves->end(), targetDevice) == leaves->end()) {
//        //TODO: Throw error
//    }
    auxFlowFragments = this->getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getScheduledTime(packetNum);
    return time;
}

float Flow::getScheduledTime(std::string deviceName, int hop, int packetNum)
{
    float time;
    Device *targetDevice = nullptr;
    std::vector<FlowFragment*> *auxFlowFragments;
    for (cObject *node : *this->pathTree->getLeaves()) {
        if (dynamic_cast<Device*>(node)) {
            if (((Device*) (node))->getName() == deviceName) {
                targetDevice = (Device*) (node);
            }
        }
    }
    if (targetDevice == nullptr) {
        //TODO: Throw error
    }
    auxFlowFragments = this->getFlowFromRootToNode(targetDevice);
    time = auxFlowFragments->at(hop)->getScheduledTime(packetNum);
    return time;
}

float Flow::getScheduledTime(int hop, int packetNum)
{
    float time;
    time = this->getFlowFragments().at(hop)->getScheduledTime(packetNum);
    return time;
}

float Flow::getAverageLatency()
{
    float averageLatency = 0;
    float auxAverageLatency = 0;
    int timeListSize = 0;
    Device *endDevice = nullptr;
    if (type == UNICAST) {
        timeListSize = this->getTimeListSize();
        for (int i = 0; i < timeListSize; i++) {
            averageLatency += this->getScheduledTime(
                    this->flowFragments.size() - 1, i)
                    - this->getDepartureTime(0, i);
        }

        averageLatency = averageLatency / (timeListSize);

    } else if (type == PUBLISH_SUBSCRIBE) {

        for (PathNode *node : *this->pathTree->getLeaves()) {
            timeListSize =
                    this->pathTree->getRoot()->getChildren().at(0)->getFlowFragments().at(
                            0)->getArrivalTimeList().size();
            ;
            endDevice = (Device*) node->getNode();
            auxAverageLatency = 0;

            for (int i = 0; i < timeListSize; i++) {
                auxAverageLatency += this->getScheduledTime(endDevice,
                        this->getFlowFromRootToNode(endDevice)->size() - 1, i)
                        - this->getDepartureTime(endDevice, 0, i);
            }

            auxAverageLatency = auxAverageLatency / timeListSize;

            averageLatency += auxAverageLatency;

        }

        averageLatency = averageLatency / this->pathTree->getLeaves()->size();

    } else {
        // TODO: Throw error
        ;
    }

    return averageLatency;
}

float Flow::getAverageLatencyToDevice(Device *dev)
{
    float averageLatency = 0;
//    float auxAverageLatency = 0;
//    Device *endDevice = nullptr;
    std::vector<FlowFragment*> *fragments = this->getFlowFromRootToNode(dev);
    for (int i = 0; i < fragments->at(0)->getParent()->getNumOfPacketsSent();
            i++) {
        averageLatency += this->getScheduledTime(dev, fragments->size() - 1, i)
                - this->getDepartureTime(dev, 0, i);
    }
    averageLatency = averageLatency
            / (fragments->at(0)->getParent()->getNumOfPacketsSent());
    return averageLatency;
}

float Flow::getAverageJitterToDevice(Device *dev)
{
    float averageJitter = 0;
    float averageLatency = this->getAverageLatencyToDevice(dev);
    std::vector<FlowFragment*> *fragments = this->getFlowFromRootToNode(dev);
    for (int i = 0; i < fragments->at(0)->getNumOfPacketsSent(); i++) {
        averageJitter += fabs(
                this->getScheduledTime(dev,
                        this->getFlowFromRootToNode(dev)->size() - 1, i)
                        - this->getDepartureTime(dev, 0, i) - averageLatency);
    }
    averageJitter = averageJitter / fragments->at(0)->getNumOfPacketsSent();
    return averageJitter;
}

std::shared_ptr<expr> Flow::getLatencyZ3(solver& solver, context &ctx, int index)
{
    //index += 1;
    std::shared_ptr<expr> latency = std::make_shared<expr>(
            ctx.real_const(
                    (this->name + std::string("latencyOfPacket")
                            + std::to_string(index)).c_str()));
    TSNSwitch *lastSwitchInPath =
            ((TSNSwitch*) (this->path.at(path.size() - 1)));
    FlowFragment *lastFragmentInList = this->flowFragments.at(
            flowFragments.size() - 1);
    TSNSwitch *firstSwitchInPath = ((TSNSwitch*) (this->path.at(0)));
    FlowFragment *firstFragmentInList = this->flowFragments.at(0);
    addAssert(solver,
            mkEq(latency,
                    mkSub(
                            lastSwitchInPath->getPortOf(
                                    lastFragmentInList->getNextHop())->scheduledTime(
                                    ctx, index, lastFragmentInList),
                            firstSwitchInPath->getPortOf(
                                    firstFragmentInList->getNextHop())->departureTime(
                                    ctx, index, firstFragmentInList))));
    return latency;
}

std::shared_ptr<expr> Flow::getLatencyZ3(solver& solver, Device *dev, context &ctx, int index)
{
    //index += 1;
    std::shared_ptr<expr> latency = std::make_shared<expr>(
            ctx.real_const(
                    (this->name
                            + std::string(
                                    "latencyOfPacket" + std::to_string(index)
                                            + std::string("For"))
                            + dev->getName()).c_str()));
    std::vector<PathNode*> *nodes = this->getNodesFromRootToNode(dev);
    std::vector<FlowFragment*> *flowFrags = this->getFlowFromRootToNode(dev);
    TSNSwitch *lastSwitchInPath =
            ((TSNSwitch*) (nodes->at(nodes->size() - 2)->getNode())); // - 1 for indexing, - 1 for last node being the end device
    FlowFragment *lastFragmentInList = flowFrags->at(flowFrags->size() - 1);
    TSNSwitch *firstSwitchInPath = ((TSNSwitch*) (nodes->at(1)->getNode())); // 1 since the first node is the publisher
    FlowFragment *firstFragmentInList = flowFrags->at(0);
    addAssert(solver,
            mkEq(latency,
                    mkSub(
                            lastSwitchInPath->getPortOf(
                                    lastFragmentInList->getNextHop())->scheduledTime(
                                    ctx, index, lastFragmentInList),
                            firstSwitchInPath->getPortOf(
                                    firstFragmentInList->getNextHop())->departureTime(
                                    ctx, index, firstFragmentInList))));
    return latency;
}

std::shared_ptr<expr> Flow::getJitterZ3(Device *dev, solver& solver, context &ctx, int index)
{
    //index += 1;
    std::shared_ptr<expr> jitter = std::make_shared<expr>(ctx.real_const(
            (this->name
                    + std::string("JitterOfPacket" + std::to_string(index) + std::string("For"))
                    + dev->getName()).c_str()));
    std::vector<PathNode*> *nodes = this->getNodesFromRootToNode(dev);
    TSNSwitch *lastSwitchInPath =
            ((TSNSwitch*) (nodes->at(nodes->size() - 2)->getNode())); // - 1 for indexing, - 1 for last node being the end device
    FlowFragment *lastFragmentInList =
            nodes->at(nodes->size() - 2)->getFlowFragments().at(
                    nodes->at(nodes->size() - 2)->getChildIndex(
                            nodes->at(nodes->size() - 1)));
    TSNSwitch *firstSwitchInPath = ((TSNSwitch*) (nodes->at(1)->getNode())); // 1 since the first node is the publisher
    FlowFragment *firstFragmentInList = nodes->at(1)->getFlowFragments().at(0);
    // z3::expr avgLatency = (z3::expr) mkDiv(getSumOfLatencyZ3(solver, dev, ctx, index), ctx.int_val(PACKETUPPERBOUNDRANGE - 1));
    std::shared_ptr<expr> avgLatency = this->getAvgLatency(dev, solver, ctx);
    expr latency =
            mkSub(
                    lastSwitchInPath->getPortOf(
                            lastFragmentInList->getNextHop())->scheduledTime(
                            ctx, index, lastFragmentInList),
                    firstSwitchInPath->getPortOf(
                            firstFragmentInList->getNextHop())->departureTime(
                            ctx, index, firstFragmentInList));
    addAssert(solver,
            mkEq(jitter,
                    mkITE(mkGe(latency, avgLatency), mkSub(latency, avgLatency),
                            mkSub(avgLatency, latency))));
    return jitter;
}

void Flow::setNumberOfPacketsSent(PathNode *node)
{
    if (dynamic_cast<Device*>(node->getNode())
            && (node->getChildren().size() == 0)) {
        return;
    } else if (dynamic_cast<Device*>(node->getNode())) {
        for (PathNode *child : node->getChildren()) {
            this->setNumberOfPacketsSent(child);
        }
    } else {
        int index = 0;
        for (FlowFragment *frag : node->getFlowFragments()) {
            if (this->numOfPacketsSentInFragment
                    < frag->getNumOfPacketsSent()) {
                this->numOfPacketsSentInFragment = frag->getNumOfPacketsSent();
            }
            // System.out.println(std::string("On node ") + ((TSNSwitch *)node->getNode())->getName() + std::string(" trying to reach children"));
            // System.out.println(std::string("Node has: ") + node.getFlowFragments().size() + std::string(" frags"));
            // System.out.println(std::string("Node has: ") + node->getChildren().size() + std::string(" children"));
            // for(PathNode n : node->getChildren()) {
            //         System.out.println(std::string("Child is a: ") + (dynamic_cast<Device *>(n->getNode()) ? "Device" : "Switch"));
            // }
            this->setNumberOfPacketsSent(node->getChildren().at(index));
            index = index + 1;
        }
    }
}

int Flow::getTimeListSize()
{
    return this->getFlowFragments().at(0)->getArrivalTimeList().size();
}

}
