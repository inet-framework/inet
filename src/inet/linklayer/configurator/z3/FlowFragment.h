#ifndef __INET_Z3_FLOWFRAGMENT_H
#define __INET_Z3_FLOWFRAGMENT_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

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
    RealExpr packetSize;
    RealExpr packetPeriodicityZ3;
    ArrayList<RealExpr> departureTimeZ3 = new ArrayList<RealExpr>();
    ArrayList<RealExpr> scheduledTimeZ3 = new ArrayList<RealExpr>();
    IntExpr fragmentPriorityZ3;

    Port port;

    PathNode referenceToNode;

    int fragmentPriority;
    String nodeName;
    String nextHopName;
    int numOfPacketsSent = Network.PACKETUPPERBOUNDRANGE;

    //TODO: CREATE REFERENCES TO PREVIOUS AND NEXT FRAGMENTS
    FlowFragment previousFragment;
    List<FlowFragment> nextFragments;

    ArrayList<Float> departureTime = new ArrayList<Float>();
    ArrayList<Float> arrivalTime = new ArrayList<Float>();
    ArrayList<Float> scheduledTime = new ArrayList<Float>();

    /**
     * [Method]: FlowFragment
     * [Usage]: Overloaded constructor method of this class. Receives a
     * flow as a parameter so it can retrieve properties of the flows
     * that it belongs to.
     *
     * @param parent    Flow object to whom this fragment belongs to
     */
    FlowFragment(Flow parent) {
        this.setParent(parent);

        /*
         * Every time this constructor is called, the parent is also called
         * making instanceCounter++, even though it counts only the number
         * of flows.
         */
        // Flow.instanceCounter--;

        this.nextFragments = new ArrayList<FlowFragment>();

        /*
         * Since the pathing methods for unicast and publish subscribe flows
         * are different and the size of the path makes a difference on the name,
         * there must be a type check before assigning the names
         */

        if(parent.getType() == Flow.UNICAST) {
            this.name = parent.getName() + "Fragment" + (parent.getFlowFragments().size() + 1);
        } else if (parent.getType() == Flow.PUBLISH_SUBSCRIBE) {
            this.name = parent.getName() + "Fragment" + (parent.pathTreeCount + 1);
            parent.pathTreeCount++;
        } else {
            // Throw error
        }

    }

    /*
     * GETTERS AND SETTERS
     */

    String getName() {
        return this.name;
    }

    RealExpr getDepartureTimeZ3(int index) {
        return departureTimeZ3.get(index);
    }

    void setDepartureTimeZ3(RealExpr dTimeZ3, int index) {
        this.departureTimeZ3.set(index, dTimeZ3);
    }

    void addDepartureTimeZ3(RealExpr dTimeZ3) {
        this.departureTimeZ3.add(dTimeZ3);
    }

    void createNewDepartureTimeZ3List() {
        this.departureTimeZ3 = new ArrayList<RealExpr>();
    }

    RealExpr getPacketPeriodicityZ3() {
        return packetPeriodicityZ3;
    }

    void setPacketPeriodicityZ3(RealExpr packetPeriodicity) {
        this.packetPeriodicityZ3 = packetPeriodicity;
    }


    RealExpr getPacketSizeZ3() {
        return this.packetSize;
    }

    void setPacketSizeZ3(RealExpr packetSize) {
        this.packetSize = packetSize;
    }

    IntExpr getFragmentPriorityZ3() {
        return fragmentPriorityZ3;
    }

    void setFragmentPriorityZ3(IntExpr flowPriority) {
        this.fragmentPriorityZ3 = flowPriority;
    }


    void addDepartureTime(float val) {
        departureTime.add(val);
    }

    float getDepartureTime(int index) {
        return departureTime.get(index);
    }

    ArrayList<Float> getDepartureTimeList() {
        return departureTime;
    }

    void addArrivalTime(float val) {
        arrivalTime.add(val);
    }

    float getArrivalTime(int index) {
        return arrivalTime.get(index);
    }

    ArrayList<Float> getArrivalTimeList() {
        return arrivalTime;
    }

    void addScheduledTime(float val) {
        scheduledTime.add(val);
    }

    float getScheduledTime(int index) {
        return scheduledTime.get(index);
    }

    ArrayList<Float> getScheduledTimeList() {
        return scheduledTime;
    }

    String getNextHop() {
        return nextHopName;
    }

    void setNextHop(String nextHop) {
        this.nextHopName = nextHop;
    }

    String getNodeName() {
        return nodeName;
    }

    void setNodeName(String nodeName) {
        this.nodeName = nodeName;
    }

    @Override
    int getNumOfPacketsSent() {
        return numOfPacketsSent;
    }

    @Override
    void setNumOfPacketsSent(int numOfPacketsSent) {
        this.numOfPacketsSent = numOfPacketsSent;
    }

    Flow getParent() {
        return parent;
    }

    void setParent(Flow parent) {
        this.parent = parent;
    }

    int getFragmentPriority() {
        return fragmentPriority;
    }

    void setFragmentPriority(int fragmentPriority) {
        this.fragmentPriority = fragmentPriority;
    }

    FlowFragment getPreviousFragment() {
        return previousFragment;
    }

    void setPreviousFragment(FlowFragment previousFragment) {
        this.previousFragment = previousFragment;
    }

    List<FlowFragment> getNextFragments() {
        return nextFragments;
    }

    void setNextFragments(List<FlowFragment> nextFragments) {
        this.nextFragments = nextFragments;
    }

    void addToNextFragments(FlowFragment frag) {
        this.nextFragments.add(frag);
    }

    PathNode getReferenceToNode() {
        return referenceToNode;
    }

    void setReferenceToNode(PathNode referenceToNode) {
        this.referenceToNode = referenceToNode;
    }

    Boolean getIsModifiedOrCreated() {
        return isModifiedOrCreated;
    }

    void setIsModifiedOrCreated(Boolean isModifiedOrCreated) {
        this.isModifiedOrCreated = isModifiedOrCreated;
    }

    Port getPort() {
        return this.port;
    }

    RealExpr getScheduledTimeZ3(int index) {
        return scheduledTimeZ3.get(index);
    }

    void setScheduledTimeZ3(RealExpr sTimeZ3, int index) {
        this.scheduledTimeZ3.set(index, sTimeZ3);
    }

    void addScheduledTimeZ3(RealExpr sTimeZ3) {
        this.scheduledTimeZ3.add(sTimeZ3);
    }

    void createNewScheduledTimeZ3List() {
        this.scheduledTimeZ3 = new ArrayList<RealExpr>();
    }

    void setPort(Port port) {
        this.port = port;
    }

};

}

#endif

