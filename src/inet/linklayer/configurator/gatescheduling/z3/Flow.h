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

#ifndef __INET_Z3_FLOW_H
#define __INET_Z3_FLOW_H

#include <deque>
#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/PathNode.h"
#include "inet/linklayer/configurator/gatescheduling/z3/PathTree.h"

namespace inet {

using namespace z3;

/**
 * [Class]: Flow
 * [Usage]: This class specifies a flow (or a stream, in other
 * words) of packets from one source to one or multiple destinations.
 * It contains references for all the data related to this flow,
 * including path, timing, packet properties and so on and so forth.
 * The flows can be unicast type or publish subscribe type flows.
 *
 */
class INET_API Flow {
  public:
    // TODO: CHECK FUNCTIONS FOR UNICAST FLOWS
    enum Type {
        UNICAST,
        PUBLISH_SUBSCRIBE
    };

    bool isModifiedOrCreated = false;

    static int instanceCounter;
    int instance = 0;

    std::string name;
    int type = 0;
    int totalNumOfPackets = 0;

    bool fixedPriority = false;
    int priorityValue = -1;

    std::vector<Switch *> path;
    std::vector<FlowFragment *> flowFragments;
    PathTree *pathTree;

    int numberOfPackets = -1;
    int pathTreeCount = 0;

    std::shared_ptr<expr> flowPriority; // In the future, priority might be fixed
    Device *startDevice;
    Device *endDevice;

    double flowFirstSendingTime;
    double flowSendingPeriodicity;

    std::shared_ptr<expr> flowFirstSendingTimeZ3;
    std::shared_ptr<expr> flowSendingPeriodicityZ3;

    int numOfPacketsSentInFragment = 0;

    bool useCustomValues = false;

    /**
     * [Method]: Flow
     * [Usage]: Default constructor method for flow objects.
     * Must be explicit due to call on child class.
     */
    Flow() {
    }

    /**
     * [Method]: Flow
     * [Usage]: Overloaded constructor method of a flow.
     * Specifies the type of the flow.
     *
     * @param type      Value specifying the type of the flow (0 - Unicast; 1 - Publish subscribe)
     */
    Flow(int type) {
        instanceCounter++;

        this->instance = instanceCounter;
        this->name = std::string("flow") + std::to_string(instanceCounter);

        if (type == UNICAST) {
            //Its not a unicast flow
            this->type = 0;
            path.clear();
            flowFragments.clear();
        } else if (type == PUBLISH_SUBSCRIBE) {
            //Its a publish subscribe flow
            this->type = 1;
            pathTree = new PathTree();
        } else {
            instanceCounter--;
            //[TODO]: Throw error
        }
    }

    /**
     * [Method]: Flow
     * [Usage]: Overloaded constructor method of a flow.
     * Specifies the type of the flow.
     *
     * @param type      Value specifying the type of the flow (0 - Unicast; 1 - Publish subscribe)
     */
    Flow(int type, double flowFirstSendingTime, double flowSendingPeriodicity) {
        instanceCounter++;
        this->instance = instanceCounter;
        this->name = std::string("flow") + std::to_string(instanceCounter);

        if (type == UNICAST) {
            //Its not a unicast flow
            this->type = 0;
            path.clear();
            flowFragments.clear();
        } else if (type == PUBLISH_SUBSCRIBE) {
            //Its a publish subscribe flow
            this->type = 1;
            pathTree = new PathTree();
        } else {
            instanceCounter--;
            //[TODO]: Throw error
        }

        this->flowFirstSendingTime = flowFirstSendingTime;
        this->flowSendingPeriodicity = flowSendingPeriodicity;

        this->useCustomValues = true;
    }

    /**
     * [Method]: addToPath
     * [Usage]: Adds a switch to the path of switches of the flow
     *
     * @param swt   Switch to be added to the list
     */
    void addToPath(Switch *swt) {
        path.push_back(swt);
    }

    /**
     * [Method]: convertUnicastFlows
     * [Usage]: Most of the unicast functionalities are not supported anymore.
     * At the beginning of the scheduling process, convert the to a multicast
     * structure without a branching path.
     *
     */
    void convertUnicastFlow() {
        // AVOID USING THE ARRAY LIST
        // TODO: REMOVE OPTION TO DISTINGUISH BETWEEN UNICAST AND MULTICAST LATER
        if (this->type == UNICAST) {
            std::deque<PathNode *> nodeList;

            PathTree *pathTree = new PathTree();
            PathNode *pathNode;
            pathNode = pathTree->addRoot(this->startDevice);
            pathNode = pathNode->addChild(path.at(0));
            nodeList.push_back(pathNode);
            for (int i = 1;  i < path.size(); i++) {
                nodeList.push_back(nodeList.front()->addChild(path.at(i)));
                nodeList.pop_front();
            }
            nodeList.front()->addChild(this->endDevice);
            nodeList.pop_front();
            this->setPathTree(pathTree);

            this->type = PUBLISH_SUBSCRIBE;
        }
    }

    /**
     * [Method]: toZ3
     * [Usage]: After setting all the numeric input values of the class,
     * generates the z3 equivalent of these values and creates any extra
     * variable needed.
     *
     * @param ctx      context variable containing the z3 environment used
     */
    void toZ3(context& ctx) {
        if (this->type == UNICAST) { // If flow is unicast
            // Convert start device to z3
            startDevice->toZ3(ctx);

            /*
             * Iterate over the switches in the path. For each switch,
             * a flow fragment will be created.
             */

            int currentSwitchIndex = 0;
            for (Switch *swt : this->path) {
                this->pathToZ3(ctx, swt, currentSwitchIndex);
                currentSwitchIndex++;
            }

        } else if (this->type == PUBLISH_SUBSCRIBE) { // If flow is publish subscribe
            /*
             * Converts the properties of the root to z3 and traverse the tree
             * doing the same and creating flow fragments for every stream
             * going out of a switch.
             */

            this->startDevice = (Device *) this->pathTree->getRoot()->getNode();
            this->startDevice->toZ3(ctx);

            if (this->priorityValue < 0 || this->priorityValue > 7) {
                this->flowPriority = std::make_shared<expr>(ctx.int_const((this->name + std::string("Priority")).c_str()));
            } else {
                this->flowPriority = std::make_shared<expr>(ctx.int_val(this->priorityValue));
            }

            if (this->useCustomValues) {
                this->flowSendingPeriodicityZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(this->flowSendingPeriodicity).c_str()));

                if (this->flowFirstSendingTime >= 0){
                    this->flowFirstSendingTimeZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(this->flowFirstSendingTime).c_str()));
                } else {
                    this->flowFirstSendingTimeZ3 = std::make_shared<expr>(ctx.real_const((std::string("flow") + std::to_string(this->instance) + std::string("FirstSendingTime")).c_str()));
                }
            } else {
                this->flowFirstSendingTimeZ3 = this->startDevice->getFirstT1TimeZ3();
                this->flowSendingPeriodicityZ3 = this->startDevice->getPacketPeriodicityZ3();
            }

            this->nodeToZ3(ctx, this->pathTree->getRoot(), nullptr);
        }
    }

    /**
     * [Method]: nodeToZ3
     * [Usage]: Given a node of a tree, the method iterate over its children.
     * For each grand-child, a flow fragment is created. This represents the
     * departure time from the current node, the arrival time in the child and
     * the scheduled time (departure time for grand-child).
     *
     * @param ctx       context variable containing the z3 environment used
     * @param node      A node of the pathTree
     */
    FlowFragment *nodeToZ3(context& ctx, PathNode *node, FlowFragment *frag);

    /**
     * [Method]: pathToZ3
     * [Usage]: On a unicast flow, the path is a simple std::vector.
     * Each switch in the path will be given as a parameter for this function
     * so a flow fragment for each hop on the path can be created.
     *
     * @param ctx                   context variable containing the z3 environment used
     * @param swt                   Switch of the current flow fragment
     * @param currentSwitchIndex    Index of the current switch in the path on the iteration
     */
    void pathToZ3(context& ctx, Switch *swt, int currentSwitchIndex);

    void bindToNextFragment(solver& solver, context& ctx, FlowFragment *frag);

    void bindAllFragments(solver& solver, context& ctx){
        for (PathNode node : this->pathTree->getRoot()->getChildren()){
            for (FlowFragment *frag : node.getFlowFragments()){
                this->bindToNextFragment(solver, ctx, frag);
            }
        }
    }

    /**
     * [Method]: getFlowFromRootToNode
     * [Usage]: Given an end device of a publish subscriber flow, or in other
     * words, a leaf in the pathTree, returns the flow fragments used to go from
     * the root to the leaf.
     *
     * @param endDevice     End device (leaf) of the desired path
     * @return              std::vector of flow fragments containing every flow fragment from source to destination
     */
    std::vector<FlowFragment *> *getFlowFromRootToNode(Device *endDevice){
        std::vector<FlowFragment *> *flowFragments = new std::vector<FlowFragment *>();
        std::vector<Device *> flowEndDevices;
        PathNode *auxNode = nullptr;

        // Iterate over leaves, get reference to the leaf of end device
        for (PathNode *node : *this->pathTree->getLeaves()) {
            flowEndDevices.push_back((Device *) node->getNode());

            if (dynamic_cast<Device *>((node->getNode())) &&
                    ((Device *) node->getNode())->getName() == endDevice->getName()) {
                auxNode = node;
            }
        }

        // If no leaf contains the desired end device, throw error returns nullptr
        if (std::find(flowEndDevices.begin(), flowEndDevices.end(), endDevice) == flowEndDevices.end()) {
            // TODO [Priority: Low]: Throw error
            return nullptr;
        }

        // Goes from parent to parent adding flowFragments to the list
        while(auxNode->getParent()->getParent() != nullptr) {
            auto children = auxNode->getParent()->getChildren();
            flowFragments->push_back(
                    auxNode->getParent()->getFlowFragments().at(
                            std::find(children.begin(), children.end(), auxNode) - children.begin()));

            auxNode = auxNode->getParent();
        }

        /*
         * Since the fragments were added from end device to start device,
         * reverse array list.
         */

        std::reverse(flowFragments->begin(), flowFragments->end());

        return flowFragments;
    }

    /**
     * [Method]: getNodesFromRootToNode
     * [Usage]: Given an end device of a publish subscriber flow, or in other
     * words, a leaf in the pathTree, returns the nodes of the path used to go from
     * the root to the leaf.
     *
     * @param endDevice     End device (leaf) of the desired path
     * @return              std::vector of nodes containing every node from source to destination
     */
    std::vector<PathNode *> *getNodesFromRootToNode(Device *endDevice){
        std::vector<PathNode *> *pathNodes = new std::vector<PathNode *>();
        std::vector<Device *> flowEndDevices;
        PathNode *auxNode = nullptr;

        // Iterate over leaves, get reference to the leaf of end device
        for (PathNode *node : *this->pathTree->getLeaves()) {
            flowEndDevices.push_back((Device *) node->getNode());
            if (dynamic_cast<Device *>((node->getNode())) &&
                    ((Device *) node->getNode())->getName() == endDevice->getName()) {
                auxNode = node;
            }
        }

        // If no leaf contains the desired end device, throw error returns nullptr
        if (std::find(flowEndDevices.begin(), flowEndDevices.end(), endDevice) == flowEndDevices.end()) {
            // TODO [Priority: Low]: Throw error
            return nullptr;
        }

        // Goes from parent to parent adding nodes to the list
        while(auxNode != nullptr) {
            pathNodes->push_back(auxNode);

            auxNode = auxNode->getParent();
        }

        /*
         * Since the nodes were added from end device to start device,
         * reverse array list.
         */

        std::reverse(pathNodes->begin(), pathNodes->end());

        return pathNodes;
    }

    /**
     * [Method]: getDepartureTime
     * [Usage]: On a unicast flow, returns the departure time
     * of a certain packet in a certain hop specified by the
     * parameters.
     *
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Departure time of the specific packet
     */
    double getDepartureTime(int hop, int packetNum);

    /**
     * [Method]: setUpPeriods
     * [Usage]: Iterate over the switches in the path of the flow.
     * Adds its periodicity to the periodicity list in the switch.
     * This will be used for the automated application cycles.
     */
    void setUpPeriods(PathNode *node) {
        if (node->getChildren().empty()) {
            return;
        } else if (dynamic_cast<Device *>(node->getNode())) {
            for (PathNode *child : node->getChildren()) {
                this->setUpPeriods(child);
            }
        } else {
            Switch *swt = (Switch *) node->getNode(); //no good. Need the port
            Port *port = nullptr;

            for (PathNode *child : node->getChildren()) {
                if (dynamic_cast<Device *>(child->getNode())) {
                    port = swt->getPortOf(((Device *) child->getNode())->getName());
                    this->setUpPeriods(child);
                } else if (dynamic_cast<Switch *>(child->getNode())) {
                    port = swt->getPortOf(((Switch *) child->getNode())->getName());
                    this->setUpPeriods(child);
                } else {
                    std::cout << "Unrecognized node\n";
                    return;
                }

                auto periods = port->getListOfPeriods();
                if (std::find(periods.begin(), periods.end(), this->flowSendingPeriodicity) == periods.end()) {
                    port->addToListOfPeriods(this->flowSendingPeriodicity);
                }
            }

        }
    }

    /**
     * [Method]: getDepartureTime
     * [Usage]: On a publish subscribe flow, returns the departure time
     * of a certain packet in a certain hop that reaches a certain device.
     * The specifications of the packet and destination are given as
     * parameters.
     *
     * @param deviceName    Name of the desired target device
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Departure time of the specific packet
     */
    double getDepartureTime(std::string deviceName, int hop, int packetNum);

    /**
     * [Method]: getDepartureTime
     * [Usage]: On a publish subscribe flow, returns the departure time
     * of a certain packet in a certain hop that reaches a certain device.
     * The specifications of the packet and destination are given as
     * parameters.
     *
     * @param targetDevice  Object containing the desired end device
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Departure time of the specific packet
     */
    double getDepartureTime(Device *targetDevice, int hop, int packetNum);

    /**
     * [Method]: getArrivalTime
     * [Usage]: On a unicast flow, returns the arrival time
     * of a certain packet in a certain hop specified by the
     * parameters.
     *
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Arrival time of the specific packet
     */
    double getArrivalTime(int hop, int packetNum);

    /**
     * [Method]: getArrivalTime
     * [Usage]: On a publish subscribe flow, returns the arrival time
     * of a certain packet in a certain hop that reaches a certain
     * device. The specifications of the packet and destination are
     * given as parameters.
     *
     * @param deviceName    Name of the desired target device
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Arrival time of the specific packet
     */
    double getArrivalTime(std::string deviceName, int hop, int packetNum);

    /**
     * [Method]: getArrivalTime
     * [Usage]: On a publish subscribe flow, returns the arrival time
     * of a certain packet in a certain hop that reaches a certain device.
     * The specifications of the packet and destination are given as
     * parameters.
     *
     * @param targetDevice  Object containing the desired end device
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Arrival time of the specific packet
     */
    double getArrivalTime(Device *targetDevice, int hop, int packetNum);

    /**
     * [Method]: getScheduledTime
     * [Usage]: On a unicast flow, returns the scheduled time
     * of a certain packet in a certain hop specified by the
     * parameters.
     *
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Scheduled time of the specific packet
     */
    double getScheduledTime(int hop, int packetNum);

    /**
     * [Method]: getScheduledTime
     * [Usage]: On a publish subscribe flow, returns the scheduled time
     * of a certain packet in a certain hop that reaches a certain
     * device. The specifications of the packet and destination are
     * given as parameters.
     *
     * @param deviceName    Name of the desired target device
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Scheduled time of the specific packet
     */
    double getScheduledTime(std::string deviceName, int hop, int packetNum);

    /**
     * [Method]: getScheduledTime
     * [Usage]: On a publish subscribe flow, returns the scheduled time
     * of a certain packet in a certain hop that reaches a certain device.
     * The specifications of the packet and destination are given as
     * parameters.
     *
     * @param targetDevice  Object containing the desired end device
     * @param hop           Number of the hop of the packet from the flow
     * @param packetNum     Number of the packet sent by the flow
     * @return              Scheduled time of the specific packet
     */
    double getScheduledTime(Device *targetDevice, int hop, int packetNum);

    /**
     * [Method]: getAverageLatency
     * [Usage]: Returns the average latency from this flow.
     * On a unicast flow, gets the last scheduled time of
     * every packet and subtracts by the first departure
     * time of same packet, then divides by the quantity
     * of packets. A similar process is done with the pub-
     * sub flows, the difference is that the flow is broken
     * into multiple unicast flows to repeat the previously
     * mentioned process.
     *
     * @return          Average latency of the flow
     */
    double getAverageLatency();

    double getAverageLatencyToDevice(Device *dev);

    /**
     * [Method]: getAverageJitter
     * [Usage]: Returns the average jitter of this flow.
     * Each absolute value resulting of the difference between
     * the last scheduled time, the first departure time and the
     * average latency of the flow is added up to a variable.
     * The process is repeated to every packet sent by the starting
     * device. This sum is then divided by how many packets where
     * sent.
     *
     * @return      Average jitter of the flow
     */
    double getAverageJitter() {
        double averageJitter = 0;
        double auxAverageJitter = 0;
        double averageLatency = this->getAverageLatency();
        int timeListSize = 0;

        if (type == UNICAST) {
            timeListSize = this->getTimeListSize();
            for (int i = 0; i < timeListSize; i++) {
                averageJitter +=
                        fabs(
                                this->getScheduledTime(this->flowFragments.size() - 1, i) -
                                        this->getDepartureTime(0, i) -
                                        averageLatency);
            }

            averageJitter = averageJitter / (timeListSize);
        } else if (type == PUBLISH_SUBSCRIBE) {

            for (PathNode *node : *this->pathTree->getLeaves()) {

                auxAverageJitter = this->getAverageJitterToDevice(((Device *) node->getNode()));
                averageJitter += auxAverageJitter;

            }

            averageJitter = averageJitter / this->pathTree->getLeaves()->size();
        } else {
            // TODO: Throw error
            ;
        }

        return averageJitter;
    }

    /**
     * [Method]: getAverageJitterToDevice
     * [Usage]: From the path tree, retrieve the average jitter of
     * the stream aimed at a specific device.
     *
     * @param dev         Specific end-device of the flow to retrieve the jitter
     * @return            double value of the variation of the latency
     */
    double getAverageJitterToDevice(Device *dev);

    /**
     * [Method]: getLatency
     * [Usage]: Gets the Z3 variable containing the latency
     * of the flow for a certain packet specified by the index.
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Index of the desired packet
     * @return          Z3 variable containing the latency of the packet
     */
    std::shared_ptr<expr> getLatencyZ3(solver& solver, context &ctx, int index);

    /**
     * [Method]: getLatencyZ3
     * [Usage]: Gets the Z3 variable containing the latency
     * of the flow for a certain packet specified by the index
     * for a certain device.
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param dev       End device of the packet
     * @param ctx       Z3 variable and function environment
     * @param index     Index of the desired packet
     * @return          Z3 variable containing the latency of the packet
     */
    std::shared_ptr<expr> getLatencyZ3(solver& solver, Device *dev, context &ctx, int index);

    /**
     * [Method]: getSumOfLatencyZ3
     * [Usage]: Recursively creates values to sum the z3 latencies
     * of the flow from 0 up to a certain packet.
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Index of the current packet in the sum
     * @return          Z3 variable containing sum of latency up to index packet
     */
    std::shared_ptr<expr> getSumOfLatencyZ3(solver& solver, context& ctx, int index) {
        if (index == 0) {
            return getLatencyZ3(solver, ctx, 0);
        }

        return std::make_shared<expr>(mkAdd(getLatencyZ3(solver, ctx, index), getSumOfLatencyZ3(solver, ctx, index - 1)));
    }

    /**
     * [Method]: getSumOfLatencyZ3
     * [Usage]: Recursively creates values to sum the z3 latencies
     * of the flow from 0 up to a certain packet for a certain device.
     *
     * @param dev       Destination of the packet
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Index of the current packet in the sum
     * @return          Z3 variable containing sum of latency up to index packet
     */
    std::shared_ptr<expr> getSumOfLatencyZ3(Device *dev, solver& solver, context& ctx, int index) {
        if (index == 0) {
            return getLatencyZ3(solver, dev, ctx, 0);
        }

        return std::make_shared<expr>(mkAdd(getLatencyZ3(solver, dev, ctx, index), getSumOfLatencyZ3(dev, solver, ctx, index - 1)));
    }

    /**
     * [Method]: getSumOfAllDevLatencyZ3
     * [Usage]: Returns the sum of all latency for all destinations
     * of the flow for the [index] number of packets sent.
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Number of packet sent (as index)
     * @return          Z3 variable containing the sum of all latencies of the flow
     */
    std::shared_ptr<expr> getSumOfAllDevLatencyZ3(solver& solver, context& ctx, int index) {
        expr sumValue = ctx.real_val(0);
        Device *currentDev = nullptr;

        for (PathNode *node : *this->pathTree->getLeaves()) {
            currentDev = (Device *) node->getNode();
            sumValue = (z3::expr) mkAdd(this->getSumOfLatencyZ3(currentDev, solver, ctx, index), sumValue);
        }

        return std::make_shared<expr>(sumValue);
    }

    /**
     * [Method]: getSumOfAllDevLatencyZ3
     * [Usage]: Returns the sum of all latency for all destinations
     * of the flow for the [index] number of packets sent.
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @return          Z3 variable containing the average latency of the flow
     */
    std::shared_ptr<expr> getAvgLatency(solver& solver, context& ctx) {
        if (this->type == UNICAST) {
            return std::make_shared<expr>(mkDiv(
                    getSumOfLatencyZ3(solver, ctx, this->numOfPacketsSentInFragment - 1),
                    ctx.real_val(this->numOfPacketsSentInFragment)));
        } else if (this->type == PUBLISH_SUBSCRIBE) {
            return std::make_shared<expr>(mkDiv(
                    getSumOfAllDevLatencyZ3(solver, ctx, this->numOfPacketsSentInFragment - 1),
                    ctx.real_val((this->numOfPacketsSentInFragment) * this->pathTree->getLeaves()->size())));
        } else {
            // TODO: THROW ERROR
        }

        return nullptr;
    }

    /**
     * [Method]: getAvgLatency
     * [Usage]: Retrieves the average latency for one of the subscribers
     * of the flow.
     *
     * @param dev         Subscriber to which the average latency will be calculated
     * @param solver    solver object
     * @param ctx        context object for the solver
     * @return            z3 variable with the average latency for the device
     */
    std::shared_ptr<expr> getAvgLatency(Device *dev, solver& solver, context& ctx) {

        return std::make_shared<expr>(mkDiv(
                this->getSumOfLatencyZ3(dev, solver, ctx, this->numOfPacketsSentInFragment - 1),
                ctx.real_val(this->numOfPacketsSentInFragment)));

    }

    /**
     * [Method]: getJitterZ3
     * [Usage]: Returns the z3 variable containing the jitter of that
     * packet.
     *
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Number of packet sent (as index)
     * @return          Z3 variable for the jitter of packet [index]
     */
    std::shared_ptr<expr> getJitterZ3(solver& solver, context& ctx, int index) {
        std::shared_ptr<expr> avgLatency = this->getAvgLatency(solver, ctx);
        std::shared_ptr<expr> latency = this->getLatencyZ3(solver, ctx, index);

        return std::make_shared<expr>(mkITE(
                mkGe(
                        latency,
                        avgLatency),
                mkSub(latency , avgLatency),
                mkMul(
                        mkSub(latency , avgLatency),
                        ctx.real_val(-1))));
    }

    /**
     * [Method]: getJitterZ3
     * [Usage]: Returns the z3 variable containing the jitter of that
     * packet.
     *
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Number of packet sent (as index)
     * @return          Z3 variable for the jitter of packet [index]
     */
    std::shared_ptr<expr> getJitterZ3(Device *dev, solver& solver, context &ctx, int index);

    /**
     * [Method]: getSumOfJitterZ3
     * [Usage]: Returns the sum of all jitter from packet 0
     * to packet of the given index as a Z3 variable.
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Number of packet sent (as index)
     * @return          Z3 variable containing the sum of all jitter
     */
    std::shared_ptr<expr> getSumOfJitterZ3(solver& solver, context& ctx, int index) {
        if (index == 0) {
            return getJitterZ3(solver, ctx, 0);
        }

        return std::make_shared<expr>(mkAdd(getJitterZ3(solver, ctx, index), getSumOfJitterZ3(solver, ctx, index - 1)));
    }

    /**
     * [Method]: getSumOfJitterZ3
     * [Usage]: Returns the sum of all jitter from packet 0
     * to packet of the given index to a specific destination
     * on a pub sub flow as a Z3 variable.
     *
     * @param dev       Destination of the packet
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Number of packet sent (as index)
     * @return          Z3 variable containing the sum of all jitter
     */
    std::shared_ptr<expr> getSumOfJitterZ3(Device *dev, solver& solver, context& ctx, int index) {
        if (index == 0) {
            return getJitterZ3(dev, solver, ctx, 0);
        }

        return std::make_shared<expr>(mkAdd(getJitterZ3(dev, solver, ctx, index), getSumOfJitterZ3(dev, solver, ctx, index - 1)));
    }

    /**
     * [Method]: getSumOfAllDevJitterZ3
     * [Usage]: Returns the sum of all jitter for all destinations
     * of the flow from 0 to the [index] packet.
     *
     * @param solver    solver in which the rules of the problem will be added
     * @param ctx       Z3 variable and function environment
     * @param index     Number of packet sent (as index)
     * @return          Z3 variable containing the sum of all jitter of the flow
     */
    std::shared_ptr<expr> getSumOfAllDevJitterZ3(solver& solver, context& ctx, int index) {
        expr sumValue = ctx.real_val(0);
        Device *currentDev = nullptr;

        for (PathNode *node : *this->pathTree->getLeaves()) {
            currentDev = (Device *) node->getNode();
            sumValue = (z3::expr) mkAdd(this->getSumOfJitterZ3(currentDev, solver, ctx, index), sumValue);
        }

        return std::make_shared<expr>(sumValue);
    }

    /**
     * [Method]: setNumberOfPacketsSent
     * [Usage]: Search through the flow fragments in order to find the highest
     * number of packets scheduled in a fragment. This is useful to set the hard
     * constraint for all packets scheduled within the flow.
     */

    void setNumberOfPacketsSent(PathNode *node);

    void modifyIfUsingCustomVal(){
        if (!this->useCustomValues) {
            this->flowSendingPeriodicity = startDevice->getPacketPeriodicity();
            this->flowFirstSendingTime = startDevice->getFirstT1Time();
        }
    }

    /*
     * GETTERS AND SETTERS:
     */

    Device *getStartDevice() {
        return startDevice;
    }

    void setStartDevice(Device *startDevice) {
        this->startDevice = startDevice;

        if (!this->useCustomValues) {
            this->flowSendingPeriodicity = startDevice->getPacketPeriodicity();
            this->flowFirstSendingTime = startDevice->getFirstT1Time();
        }
    }

    Device *getEndDevice() {
        return endDevice;
    }

    void setEndDevice(Device *endDevice) {
        this->endDevice = endDevice;
    }

    std::vector<Switch *> getPath() {
        return path;
    }

    void setPath(std::vector<Switch *> path) {
        this->path = path;
    }

    std::shared_ptr<expr> getFragmentPriorityZ3() {
        return flowPriority;
    }

    void setFlowPriority(z3::expr priority) {
        this->flowPriority = std::make_shared<expr>(priority);
    }

    std::string getName() {
        return name;
    }

    void setName(std::string name) {
        this->name = name;
    }

    std::vector<FlowFragment *> getFlowFragments() {
        return flowFragments;
    }

    void setFlowFragments(std::vector<FlowFragment *> flowFragments) {
        this->flowFragments = flowFragments;
    }

    int getTimeListSize();

    PathTree *getPathTree() {
        return pathTree;
    }

    void setPathTree(PathTree *pathTree) {
        this->startDevice = (Device *) pathTree->getRoot()->getNode();
        this->pathTree = pathTree;
    }

    int getType() {
        return type;
    }

    void setType(int type) {
        this->type = type;
    }

    int getNumOfPacketsSent() {
        return numOfPacketsSentInFragment;
    }

    void setNumOfPacketsSent(int numOfPacketsSent) {
        this->numOfPacketsSentInFragment = numOfPacketsSent;
    }

    int getTotalNumOfPackets() {
        return totalNumOfPackets;
    }

    void setTotalNumOfPackets(int totalNumOfPackets) {
        this->totalNumOfPackets = totalNumOfPackets;
    }

    void addToTotalNumOfPackets(int num) {
        this->totalNumOfPackets = this->totalNumOfPackets + num;
    }

    int getInstance() {
        return instance;
    }

    void setInstance(int instance) {
        this->instance = instance;
    }

    double getPacketSize() {
        return this->startDevice->getPacketSize();
    }

    std::shared_ptr<expr> getPacketSizeZ3() {
        return this->startDevice->getPacketSizeZ3();
    }

    double getFlowFirstSendingTime() {
        return flowFirstSendingTime;
    }

    void setFlowFirstSendingTime(double flowFirstSendingTime) {
        this->flowFirstSendingTime = flowFirstSendingTime;
    }

    double getFlowSendingPeriodicity() {
        return flowSendingPeriodicity;
    }

    void setFlowSendingPeriodicity(double flowSendingPeriodicity) {
        this->flowSendingPeriodicity = flowSendingPeriodicity;
    }

    std::shared_ptr<expr> getFlowFirstSendingTimeZ3() {
        return flowFirstSendingTimeZ3;
    }

    void setFlowFirstSendingTimeZ3(z3::expr flowFirstSendingTimeZ3) {
        this->flowFirstSendingTimeZ3 = std::make_shared<expr>(flowFirstSendingTimeZ3);
    }

    std::shared_ptr<expr> getFlowSendingPeriodicityZ3() {
        return flowSendingPeriodicityZ3;
    }

    void setFlowSendingPeriodicityZ3(z3::expr flowSendingPeriodicityZ3) {
        this->flowSendingPeriodicityZ3 = std::make_shared<expr>(flowSendingPeriodicityZ3);
    }


    bool isFixedPriority() {
        return fixedPriority;
    }

    void setFixedPriority(bool fixedPriority) {
        this->fixedPriority = fixedPriority;
    }

    int getPriorityValue() {
        return priorityValue;
    }

    void setPriorityValue(int priorityValue) {
        this->priorityValue = priorityValue;
    }

    static int getInstanceCounter() {
        return instanceCounter;
    }

    static void setInstanceCounter(int instanceCounter) {
        Flow::instanceCounter = instanceCounter;
    }

    bool getIsModifiedOrCreated() {
        return isModifiedOrCreated;
    }

    void setIsModifiedOrCreated(bool isModifiedOrCreated) {
        this->isModifiedOrCreated = isModifiedOrCreated;
    }
};

}

#endif

