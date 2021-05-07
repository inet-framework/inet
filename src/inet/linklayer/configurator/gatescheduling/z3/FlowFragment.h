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

#ifndef __INET_Z3_FLOWFRAGMENT_H
#define __INET_Z3_FLOWFRAGMENT_H

#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Flow.h"
#include "inet/linklayer/configurator/gatescheduling/z3/PathNode.h"
#include "inet/linklayer/configurator/gatescheduling/z3/Port.h"

namespace inet {

using namespace z3;

/**
 * This class is used to represent a fragment of a flow.
 * Simply put, a flow fragment is represents the flow it belongs to
 * regarding a specific switch in the path. With this approach,
 * a flow, regardless of its type, can be broken into flow fragments
 * and distributed to the switches in the network. It holds the time
 * values of the departure time, arrival time and scheduled time of
 * packets from this flow on the switch it belongs to.
 *
 */
class INET_API FlowFragment : public Flow {
  public:
    bool isModifiedOrCreated = false;
    Flow *parent;
    std::shared_ptr<expr> packetSize;
    std::shared_ptr<expr> packetPeriodicityZ3;
    std::vector<std::shared_ptr<expr>> departureTimeZ3;
    std::vector<std::shared_ptr<expr>> scheduledTimeZ3;
    std::shared_ptr<expr> fragmentPriorityZ3;

    Port *port;

    PathNode *referenceToNode;

    int fragmentPriority;
    std::string nodeName;
    std::string nextHopName;
    int numOfPacketsSent = -1;

    //TODO: CREATE REFERENCES TO PREVIOUS AND NEXT FRAGMENTS
    FlowFragment *previousFragment;
    std::vector<FlowFragment *> nextFragments;

    std::vector<double> departureTime;
    std::vector<double> arrivalTime;
    std::vector<double> scheduledTime;

    /**
     * Overloaded constructor method of this class. Receives a
     * flow as a parameter so it can retrieve properties of the flows
     * that it belongs to.
     *
     * @param parent    Flow object to whom this fragment belongs to
     */
    FlowFragment(Flow *parent) {
        this->setParent(parent);

        /*
         * Every time this constructor is called, the parent is also called
         * making instanceCounter++, even though it counts only the number
         * of flows.
         */
        // Flow.instanceCounter--;

        this->nextFragments.clear();

        /*
         * Since the pathing methods for unicast and publish subscribe flows
         * are different and the size of the path makes a difference on the name,
         * there must be a type check before assigning the names
         */

        if (parent->getType() == UNICAST) {
            name = parent->getName() + std::string("Fragment") + std::to_string(parent->getFlowFragments().size() + 1);
        } else if (parent->getType() == PUBLISH_SUBSCRIBE) {
            name = parent->getName() + std::string("Fragment") + std::to_string(parent->pathTreeCount + 1);
            parent->pathTreeCount++;
        } else {
            // Throw error
        }
    }

    /*
     * GETTERS AND SETTERS
     */

    std::string getName() {
        return name;
    }

    std::shared_ptr<expr> getDepartureTimeZ3(int index) {
        return departureTimeZ3.at(index);
    }

    void setDepartureTimeZ3(z3::expr dTimeZ3, int index) {
        this->departureTimeZ3[index] = std::make_shared<expr>(dTimeZ3);
    }

    void addDepartureTimeZ3(z3::expr dTimeZ3) {
        this->departureTimeZ3.push_back(std::make_shared<expr>(dTimeZ3));
    }

    void createNewDepartureTimeZ3List() {
        this->departureTimeZ3.clear();
    }

    std::shared_ptr<expr> getPacketPeriodicityZ3() {
        return packetPeriodicityZ3;
    }

    void setPacketPeriodicityZ3(z3::expr packetPeriodicity) {
        this->packetPeriodicityZ3 = std::make_shared<expr>(packetPeriodicity);
    }

    std::shared_ptr<expr> getPacketSizeZ3() {
        return packetSize;
    }

    void setPacketSizeZ3(z3::expr packetSize) {
        this->packetSize = std::make_shared<expr>(packetSize);
    }

    std::shared_ptr<expr> getFragmentPriorityZ3() {
        return fragmentPriorityZ3;
    }

    void setFragmentPriorityZ3(z3::expr flowPriority) {
        this->fragmentPriorityZ3 = std::make_shared<expr>(flowPriority);
    }

    void addDepartureTime(double val) {
        departureTime.push_back(val);
    }

    double getDepartureTime(int index) {
        return departureTime.at(index);
    }

    std::vector<double> getDepartureTimeList() {
        return departureTime;
    }

    void addArrivalTime(double val) {
        arrivalTime.push_back(val);
    }

    double getArrivalTime(int index) {
        return arrivalTime.at(index);
    }

    std::vector<double> getArrivalTimeList() {
        return arrivalTime;
    }

    void addScheduledTime(double val) {
        scheduledTime.push_back(val);
    }

    double getScheduledTime(int index) {
        return scheduledTime.at(index);
    }

    std::vector<double> getScheduledTimeList() {
        return scheduledTime;
    }

    std::string getNextHop() {
        return nextHopName;
    }

    void setNextHop(std::string nextHop) {
        this->nextHopName = nextHop;
    }

    std::string getNodeName() {
        return nodeName;
    }

    void setNodeName(std::string nodeName) {
        this->nodeName = nodeName;
    }

    int getNumOfPacketsSent() {
        return numOfPacketsSent;
    }

    void setNumOfPacketsSent(int numOfPacketsSent) {
        this->numOfPacketsSent = numOfPacketsSent;
    }

    Flow *getParent() {
        return parent;
    }

    void setParent(Flow *parent) {
        this->parent = parent;
    }

    int getFragmentPriority() {
        return fragmentPriority;
    }

    void setFragmentPriority(int fragmentPriority) {
        this->fragmentPriority = fragmentPriority;
    }

    FlowFragment *getPreviousFragment() {
        return previousFragment;
    }

    void setPreviousFragment(FlowFragment *previousFragment) {
        this->previousFragment = previousFragment;
    }

    std::vector<FlowFragment *> getNextFragments() {
        return nextFragments;
    }

    void setNextFragments(std::vector<FlowFragment *> nextFragments) {
        this->nextFragments = nextFragments;
    }

    void addToNextFragments(FlowFragment *frag) {
        this->nextFragments.push_back(frag);
    }

    PathNode *getReferenceToNode() {
        return referenceToNode;
    }

    void setReferenceToNode(PathNode *referenceToNode) {
        this->referenceToNode = referenceToNode;
    }

    bool getIsModifiedOrCreated() {
        return isModifiedOrCreated;
    }

    void setIsModifiedOrCreated(bool isModifiedOrCreated) {
        this->isModifiedOrCreated = isModifiedOrCreated;
    }

    Port *getPort() {
        return port;
    }

    std::shared_ptr<expr> getScheduledTimeZ3(int index) {
        return scheduledTimeZ3.at(index);
    }

    void setScheduledTimeZ3(z3::expr sTimeZ3, int index) {
        this->scheduledTimeZ3[index] = std::make_shared<expr>(sTimeZ3);
    }

    void addScheduledTimeZ3(z3::expr sTimeZ3) {
        this->scheduledTimeZ3.push_back(std::make_shared<expr>(sTimeZ3));
    }

    void createNewScheduledTimeZ3List() {
        this->scheduledTimeZ3.clear();
    }

    void setPort(Port *port) {
        this->port = port;
    }
};

}

#endif

