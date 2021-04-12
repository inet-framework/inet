#ifndef __INET_Z3_NETWORK_H
#define __INET_Z3_NETWORK_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

using namespace z3;

/**
 * [Class]: Network
 * [Usage]: Using this class, the user can specify the network
 * topology using switches and flows. The network will be given
 * to the scheduler generator so it can iterate over the network's
 * flows and switches setting up the scheduling rules.
 *
 */
class INET_API Network {

    std::string db_name;
    std::string file_id;

    //TODO: Remove debugging variables:
    expr *avgOfAllLatency;
    std::vector<z3::expr *> avgLatencyPerDev;


    std::vector<Switch> switches;
    std::vector<Flow> flows;
    float timeToTravel;
    std::vector<z3::expr *> allSumOfJitter;
    std::vector<int> numberOfNodes;

#define PACKETUPPERBOUNDRANGE 5 // Limits the applications of rules to the packets
#define CYCLEUPPERBOUNDRANGE 25 // Limits the applications of rules to the cycles

    float jitterUpperBoundRange = -1;
    expr *jitterUpperBoundRangeZ3;

    /**
     * [Method]: Network
     * [Usage]: Default constructor method of class Network. Creates
     * the std::vectors for the switches and flows. Sets up the default
     * time to travel for 2 (Needs revision).
     */
    Network (float jitterUpperBoundRange) {
        this->jitterUpperBoundRange = jitterUpperBoundRange;
        this->switches.clear();
        this->flows.clear();
        this->timeToTravel = 2;
    }


    /**
     * [Method]: Network
     * [Usage]: Default constructor method of class Network. Creates
     * the std::vectors for the switches and flows. Sets up the default
     * time to travel for 2 (Needs revision).
     */
    Network () {
        this->switches.clear();
        this->flows.clear();
        this->timeToTravel = 2;
    }

    /**
     * [Method]: Network
     * [Usage]: Overloaded constructor method of class Network. Will
     * create a network with the given std::vectors for switches, flows
     * and time to travel.
     *
     * @param switches      std::vector with the instances of switches of the network
     * @param flows         std::vector with the instances of flows of the network
     * @param timeToTravel  Value used as travel time from a node to another in the network
     */
    Network (std::vector<Switch> switches, std::vector<Flow> flows, float timeToTravel) {
        this->switches = switches;
        this->flows = flows;
        this->timeToTravel = timeToTravel;
    }

    /**
     * [Method]: SecureHC
     * [Usage]: Iterates over the flows of the network, assuring that
     * each flow will have its hard constraint established.
     *
     * @param solver    z3 solver object used to discover the variables' values
     * @param ctx       z3 context which specify the environment of constants, functions and variables
     */
    void secureHC(solver solver, context ctx) {

        if(jitterUpperBoundRange != -1) { // If there is a value on the upperBoundRange, it was set through the network
            this->setJitterUpperBoundRangeZ3(ctx, this->jitterUpperBoundRange);
        }

        Stack<z3::expr *> jitterList = new Stack<z3::expr *>();
        int totalNumOfLeaves = 0;
        expr *sumOfAllJitter;
        // On every switch, set up the constraints of the schedule
        //switch1.setupSchedulingRules(solver, ctx);


        for (Switch swt : this->getSwitches()) {
            ((TSNSwitch) swt).setupSchedulingRules(solver, ctx);;
        }

        /*
         *  Iterate over the flows. Get last and first fragment of the flow
         *  make sure last scheduled time minus first departure time is lesser
         *  than HC. For publish subscribe flows, make sure that every flow fragment
         *  of all fathers of all leaves have their scheduled time minus the
         *  departure time of the first child of the root lesser than the hard
         *  constraint
         */

        for(Flow flw : this->getFlows()) {
            flw.setNumberOfPacketsSent(flw.getPathTree().getRoot());

            flw.bindAllFragments(solver, ctx);

            solver.add( // No negative cycle values constraint
                ctx.mkGe(
                    flw.getStartDevice().getFirstT1TimeZ3(),
                    ctx.real_val(0)
                )
            );
            solver.add( // Maximum transmission offset constraint
                ctx.mkLe(
                    flw.getStartDevice().getFirstT1TimeZ3(),
                    flw.getStartDevice().getPacketPeriodicityZ3()
                )
            );



            if(flw.getType() == Flow.UNICAST) {

                std::vector<FlowFragment> currentFrags = flw.getFlowFragments();
                std::vector<Switch> path = flw.getPath();


                //Make sure that HC is respected
                for(int i = 0; i < flw.getNumOfPacketsSent(); i++) {
                    solver.add(
                            ctx.mkLe(
                                ctx.mkSub(
                                    ((TSNSwitch) path.get(path.size() - 1)).scheduledTime(ctx, i, currentFrags.get(currentFrags.size() - 1)),
                                    ((TSNSwitch) path.get(0)).departureTime(ctx, i, currentFrags.get(0))
                                ),
                                flw.getStartDevice().getHardConstraintTimeZ3()
                            )
                      );
                }

            } else if (flw.getType() == Flow.PUBLISH_SUBSCRIBE) {
                PathNode root = flw.getPathTree().getRoot();
                std::vector<PathNode> leaves = flw.getPathTree().getLeaves();
                std::vector<PathNode> parents;

                // Make list of parents of all leaves
                for(PathNode leaf : leaves) {

                    if(!parents.contains(leaf.getParent())){
                        parents.add(leaf.getParent());
                    }


                    // Set the maximum allowed jitter
                    for(int index = 0; index < flw.getNumOfPacketsSent(); index++) {
                        solver.add( // Maximum allowed jitter constraint
                            ctx.mkLe(
                                flw.getJitterZ3((Device) leaf.getNode(), solver, ctx, index),
                                this->jitterUpperBoundRangeZ3
                            )
                        );
                    }

                }

                // Iterate over the flows of each leaf parent, assert HC
                for(PathNode parent : parents) {
                    for(FlowFragment ffrag : parent.getFlowFragments()) {
                        for(int i = 0; i < flw.getNumOfPacketsSent(); i++) {
                            solver.add( // Maximum Allowed Latency constraint
                                ctx.mkLe(
                                    ctx.mkSub(
                                        ((TSNSwitch) parent.getNode()).scheduledTime(ctx, i, ffrag),
                                        ((TSNSwitch) root.getChildren().get(0).getNode()).departureTime(ctx, i,
                                            root.getChildren().get(0).getFlowFragments().get(0)
                                        )
                                    ),
                                    flw.getStartDevice().getHardConstraintTimeZ3()
                                )
                            );
                        }
                    }

                }

                /*

                // TODO: CHECK FAIRNESS CONSTRAINT (?)

                sumOfAllJitter = flw.getSumOfAllDevJitterZ3(solver, ctx, Network.PACKETUPPERBOUNDRANGE - 1);

                jitterList.push(sumOfAllJitter);
                totalNumOfLeaves += flw.getPathTree().getLeaves().size();

                // SET THE MAXIMUM JITTER FOR THE FLOW
                solver.add(
                    ctx.mkLe(
                        ctx.mkDiv(
                            sumOfAllJitter,
                            ctx.real_val(flw.getPathTree().getLeaves().size() * (PACKETUPPERBOUNDRANGE))
                        ),
                        jitterUpperBoundRangeZ3
                    )
                );
                */

                // TODO: Remove code for debugging
                avgOfAllLatency = flw.getAvgLatency(solver, ctx);
                for(PathNode node : flw.getPathTree().getLeaves()) {
                    Device endDev = (Device) node.getNode();

                    this->avgLatencyPerDev.add(
                        (z3::expr) ctx.mkDiv(
                            flw.getSumOfJitterZ3(endDev, solver, ctx, flw.getNumOfPacketsSent() - 1),
                            ctx.int_val(flw.getNumOfPacketsSent())
                        )
                    );



                }

                /**/
            }

        }

    }


    /**
     * [Method]: loadNetwork
     * [Usage]: From the primitive values retrieved in the object
     * deserialization process, instantiate the z3 objects that represent
     * the same properties.
     *
     * @param ctx        context object for the solver
     * @param solver    solver object
     */
    void loadNetwork(context ctx, solver solver) {

        // TODO: Don't forget to load the values of this class

        // On all network flows: Data given by the user will be converted to z3 values
       for(Flow flw : this->flows) {
           // flw.toZ3(ctx);
           flw.flowPriority = ctx.int_valConst(flw.name + std::string("Priority"));
           ((Device) flw.getPathTree().getRoot().getNode()).toZ3(ctx);
       }

       // On all network switches: Data given by the user will be converted to z3 values
        for(Switch swt : this->switches) {
            if(swt instanceof TSNSwitch) {
                for(Port port : ((TSNSwitch) swt).getPorts()) {
                    for(FlowFragment frag : port.getFlowFragments()) {
                        frag.createNewDepartureTimeZ3List();
                        frag.addDepartureTimeZ3(ctx.real_val(std::to_string(frag.getDepartureTime(0))));
                    }
                }
                ((TSNSwitch) swt).toZ3(ctx, solver);
                ((TSNSwitch) swt).loadZ3(ctx, solver);
            }
        }

        /*
        for(Switch swt : this->getSwitches()) {
            if(swt instanceof TSNSwitch) {
                ((TSNSwitch) swt).loadZ3(ctx, solver);
            }
        }
        */

    }


    /*
     * GETTERS AND SETTERS
     */

    expr *getJitterUpperBoundRangeZ3() {
        return jitterUpperBoundRangeZ3;
    }

    void setJitterUpperBoundRangeZ3(z3::expr jitterUpperBoundRange) {
        this->jitterUpperBoundRangeZ3 = jitterUpperBoundRange;
    }

    void setJitterUpperBoundRangeZ3(context ctx, float auxJitterUpperBoundRange) {
        this->jitterUpperBoundRangeZ3 = ctx.real_val(std::string.valueOf(auxJitterUpperBoundRange));
    }

    std::vector<Switch> getSwitches() {
        return switches;
    }

    void setSwitches(std::vector<Switch> switches) {
        this->switches = switches;
    }

    std::vector<Flow> getFlows() {
        return flows;
    }

    void setFlows(std::vector<Flow> flows) {
        this->flows = flows;
    }

    void addFlow (Flow flw) {
        this->flows.add(flw);
    }

    void addSwitch (Switch swt) {
        this->switches.add(swt);
    }

};

}

#endif

