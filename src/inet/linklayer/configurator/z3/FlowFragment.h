#ifndef __INET_Z3_FLOWFRAGMENT_H
#define __INET_Z3_FLOWFRAGMENT_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

using namespace z3;

/**
 * [Class]: FlowFragment
 * [Usage]: This class is used to represent a fragment of a flow.
 * Simply put, a flow fragment is represents the flow it belongs to
 * regarding a specific switch in the path. With this approach,
 * a flow, regardless of its type, can be broken into flow fragments
 * and distributed to the switches in the network. It holds the time
 * values of the departure time, arrival time and scheduled time of
 * packets from this flow on the switch it belongs to.
 *
 */
class INET_API FlowFragment extends Flow {
    Boolean isModifiedOrCreated = false;
    static final long serialVersionUID = 1L;
    Flow parent;
    z3::expr packetSize;
    z3::expr packetPeriodicityZ3;
    std::vector<z3::expr> departureTimeZ3;
    std::vector<z3::expr> scheduledTimeZ3;
    z3::expr fragmentPriorityZ3;

    Port port;

    PathNode referenceToNode;

    int fragmentPriority;
    std::string nodeName;
    std::string nextHopName;
    int numOfPacketsSent = Network.PACKETUPPERBOUNDRANGE;

    //TODO: CREATE REFERENCES TO PREVIOUS AND NEXT FRAGMENTS
    FlowFragment previousFragment;
    List<FlowFragment> nextFragments;

    std::vector<float> departureTime;
    std::vector<float> arrivalTime;
    std::vector<float> scheduledTime;

    /**
     * [Method]: FlowFragment
     * [Usage]: Overloaded constructor method of this class. Receives a
     * flow as a parameter so it can retrieve properties of the flows
     * that it belongs to.
     *
     * @param parent    Flow object to whom this fragment belongs to
     */
    FlowFragment(Flow parent) {
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

        if(parent.getType() == Flow.UNICAST) {
            this->name = parent.getName() + "Fragment" + (parent.getFlowFragments().size() + 1);
        } else if (parent.getType() == Flow.PUBLISH_SUBSCRIBE) {
            this->name = parent.getName() + "Fragment" + (parent.pathTreeCount + 1);
            parent.pathTreeCount++;
        } else {
            // Throw error
        }

    }

    /*
     * GETTERS AND SETTERS
     */

    std::string getName() {
        return this->name;
    }

    z3::expr getDepartureTimeZ3(int index) {
        return departureTimeZ3.get(index);
    }

    void setDepartureTimeZ3(z3::expr dTimeZ3, int index) {
        this->departureTimeZ3.set(index, dTimeZ3);
    }

    void addDepartureTimeZ3(z3::expr dTimeZ3) {
        this->departureTimeZ3.add(dTimeZ3);
    }

    void createNewDepartureTimeZ3List() {
        this->departureTimeZ3.clear();
    }

    z3::expr getPacketPeriodicityZ3() {
        return packetPeriodicityZ3;
    }

    void setPacketPeriodicityZ3(z3::expr packetPeriodicity) {
        this->packetPeriodicityZ3 = packetPeriodicity;
    }


    z3::expr getPacketSizeZ3() {
        return this->packetSize;
    }

    void setPacketSizeZ3(z3::expr packetSize) {
        this->packetSize = packetSize;
    }

    z3::expr getFragmentPriorityZ3() {
        return fragmentPriorityZ3;
    }

    void setFragmentPriorityZ3(z3::expr flowPriority) {
        this->fragmentPriorityZ3 = flowPriority;
    }


    void addDepartureTime(float val) {
        departureTime.add(val);
    }

    float getDepartureTime(int index) {
        return departureTime.get(index);
    }

    std::vector<float> getDepartureTimeList() {
        return departureTime;
    }

    void addArrivalTime(float val) {
        arrivalTime.add(val);
    }

    float getArrivalTime(int index) {
        return arrivalTime.get(index);
    }

    std::vector<float> getArrivalTimeList() {
        return arrivalTime;
    }

    void addScheduledTime(float val) {
        scheduledTime.add(val);
    }

    float getScheduledTime(int index) {
        return scheduledTime.get(index);
    }

    std::vector<float> getScheduledTimeList() {
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

    @Override
    int getNumOfPacketsSent() {
        return numOfPacketsSent;
    }

    @Override
    void setNumOfPacketsSent(int numOfPacketsSent) {
        this->numOfPacketsSent = numOfPacketsSent;
    }

    Flow getParent() {
        return parent;
    }

    void setParent(Flow parent) {
        this->parent = parent;
    }

    int getFragmentPriority() {
        return fragmentPriority;
    }

    void setFragmentPriority(int fragmentPriority) {
        this->fragmentPriority = fragmentPriority;
    }

    FlowFragment getPreviousFragment() {
        return previousFragment;
    }

    void setPreviousFragment(FlowFragment previousFragment) {
        this->previousFragment = previousFragment;
    }

    List<FlowFragment> getNextFragments() {
        return nextFragments;
    }

    void setNextFragments(List<FlowFragment> nextFragments) {
        this->nextFragments = nextFragments;
    }

    void addToNextFragments(FlowFragment frag) {
        this->nextFragments.add(frag);
    }

    PathNode getReferenceToNode() {
        return referenceToNode;
    }

    void setReferenceToNode(PathNode referenceToNode) {
        this->referenceToNode = referenceToNode;
    }

    Boolean getIsModifiedOrCreated() {
        return isModifiedOrCreated;
    }

    void setIsModifiedOrCreated(Boolean isModifiedOrCreated) {
        this->isModifiedOrCreated = isModifiedOrCreated;
    }

    Port getPort() {
        return this->port;
    }

    z3::expr getScheduledTimeZ3(int index) {
        return scheduledTimeZ3.get(index);
    }

    void setScheduledTimeZ3(z3::expr sTimeZ3, int index) {
        this->scheduledTimeZ3.set(index, sTimeZ3);
    }

    void addScheduledTimeZ3(z3::expr sTimeZ3) {
        this->scheduledTimeZ3.add(sTimeZ3);
    }

    void createNewScheduledTimeZ3List() {
        this->scheduledTimeZ3.clear();
    }

    void setPort(Port port) {
        this->port = port;
    }

};

}

#endif

