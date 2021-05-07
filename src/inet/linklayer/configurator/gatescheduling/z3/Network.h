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

#ifndef __INET_Z3_NETWORK_H
#define __INET_Z3_NETWORK_H

#include <stack>
#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Flow.h"
#include "inet/linklayer/configurator/gatescheduling/z3/Switch.h"

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
  public:
    //TODO: Remove debugging variables:
    std::shared_ptr<expr> avgOfAllLatency;
    std::vector<std::shared_ptr<expr>> avgLatencyPerDev;

    std::vector<Switch *> switches;
    std::vector<Flow *> flows;
    double timeToTravel;
    std::vector<std::shared_ptr<expr> > allSumOfJitter;
    std::vector<int> numberOfNodes;

    double jitterUpperBoundRange = -1;
    std::shared_ptr<expr> jitterUpperBoundRangeZ3;

    /**
     * [Usage]: Default constructor method of class Network. Creates
     * the std::vectors for the switches and flows. Sets up the default
     * time to travel for 2 (Needs revision).
     */
    Network (double jitterUpperBoundRange) {
        this->jitterUpperBoundRange = jitterUpperBoundRange;
        this->switches.clear();
        this->flows.clear();
        this->timeToTravel = 2;
    }

    /**
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
     * [Usage]: Overloaded constructor method of class Network. Will
     * create a network with the given std::vectors for switches, flows
     * and time to travel.
     *
     * @param switches      std::vector with the instances of switches of the network
     * @param flows         std::vector with the instances of flows of the network
     * @param timeToTravel  Value used as travel time from a node to another in the network
     */
    Network (std::vector<Switch *> switches, std::vector<Flow *> flows, double timeToTravel) {
        this->switches = switches;
        this->flows = flows;
        this->timeToTravel = timeToTravel;
    }

    /**
     * [Usage]: Iterates over the flows of the network, assuring that
     * each flow will have its hard constraint established.
     *
     * @param solver    z3 solver object used to discover the variables' values
     * @param ctx       z3 context which specify the environment of constants, functions and variables
     */
    void secureHC(solver& solver, context& ctx) {
        if (jitterUpperBoundRange != -1) { // If there is a value on the upperBoundRange, it was set through the network
            setJitterUpperBoundRangeZ3(ctx, jitterUpperBoundRange);
        }

        std::stack<std::shared_ptr<expr>> jitterList;
        // int totalNumOfLeaves = 0;
        std::shared_ptr<expr> sumOfAllJitter;
        // On every switch, set up the constraints of the schedule
        //switch1.setupSchedulingRules(solver, ctx);


        for (Switch *swt : getSwitches()) {
            swt->setupSchedulingRules(solver, ctx);
        }

        /*
         *  Iterate over the flows. Get last and first fragment of the flow
         *  make sure last scheduled time minus first departure time is lesser
         *  than HC. For publish subscribe flows, make sure that every flow fragment
         *  of all fathers of all leaves have their scheduled time minus the
         *  departure time of the first child of the root lesser than the hard
         *  constraint
         */

        for (Flow *flw : getFlows()) {
            flw->setNumberOfPacketsSent(flw->getPathTree()->getRoot());

            flw->bindAllFragments(solver, ctx);

            // No negative cycle values constraint
            addAssert(solver, flw->getStartDevice()->getFirstT1TimeZ3() >= ctx.real_val(0));
            // Maximum transmission offset constraint
            addAssert(solver, flw->getStartDevice()->getFirstT1TimeZ3() <= flw->getStartDevice()->getPacketPeriodicityZ3());

            if (flw->getType() == Flow::UNICAST) {

                std::vector<FlowFragment *> currentFrags = flw->getFlowFragments();
                std::vector<Switch *> path = flw->getPath();


                //Make sure that HC is respected
                for (int i = 0; i < flw->getNumOfPacketsSent(); i++) {
                    addAssert(solver, path.back()->scheduledTime(ctx, i, currentFrags.at(currentFrags.size() - 1)) -
                                      path.front()->departureTime(ctx, i, currentFrags.at(0)) <=
                                      flw->getStartDevice()->getHardConstraintTimeZ3());
                }

            } else if (flw->getType() == Flow::PUBLISH_SUBSCRIBE) {
                PathNode *root = flw->getPathTree()->getRoot();
                std::vector<PathNode *> *leaves = flw->getPathTree()->getLeaves();
                std::vector<PathNode *> parents;

                // Make list of parents of all leaves
                for (PathNode *leaf : *leaves) {

                    if (std::find(parents.begin(), parents.end(), leaf->getParent()) == parents.end()){
                        parents.push_back(leaf->getParent());
                    }


                    // Set the maximum allowed jitter
                    for (int index = 0; index < flw->getNumOfPacketsSent(); index++) {
                        // Maximum allowed jitter constraint
                        addAssert(solver, flw->getJitterZ3((Device *)leaf->getNode(), solver, ctx, index) <= jitterUpperBoundRangeZ3);
                    }

                }

                // Iterate over the flows of each leaf parent, assert HC
                for (PathNode *parent : parents) {
                    for (FlowFragment *ffrag : parent->getFlowFragments()) {
                        for (int i = 0; i < flw->getNumOfPacketsSent(); i++) {
                            addAssert(solver,  // Maximum Allowed Latency constraint
                                ((Switch *) parent->getNode())->scheduledTime(ctx, i, ffrag) -
                                ((Switch *) root->getChildren().at(0)->getNode())->departureTime(ctx, i,
                                        root->getChildren().at(0)->getFlowFragments().at(0)) <=
                                flw->getStartDevice()->getHardConstraintTimeZ3());
                        }
                    }

                }

                // TODO: Remove code for debugging
                avgOfAllLatency = flw->getAvgLatency(solver, ctx);
                for (PathNode *node : *flw->getPathTree()->getLeaves()) {
                    Device *endDev = (Device *) node->getNode();

                    avgLatencyPerDev.push_back(std::make_shared<expr>(
                        flw->getSumOfJitterZ3(endDev, solver, ctx, flw->getNumOfPacketsSent() - 1) /
                        ctx.int_val(flw->getNumOfPacketsSent())));
                }
            }

        }
    }

    /**
     * [Usage]: From the primitive values retrieved in the object
     * deserialization process, instantiate the z3 objects that represent
     * the same properties.
     *
     * @param ctx        context object for the solver
     * @param solver    solver object
     */
    void loadNetwork(context& ctx, solver& solver) {
        // TODO: Don't forget to load the values of this class

        // On all network flows: Data given by the user will be converted to z3 values
        for (Flow *flw : flows) {
            // flw->toZ3(ctx);
            flw->flowPriority = std::make_shared<expr>(ctx.int_val((flw->name + std::string("Priority")).c_str()));
            ((Device *) flw->getPathTree()->getRoot()->getNode())->toZ3(ctx);
        }

        // On all network switches: Data given by the user will be converted to z3 values
        for (Switch *swt : switches) {
            for (Port *port : swt->getPorts()) {
                for (FlowFragment *frag : port->getFlowFragments()) {
                    frag->createNewDepartureTimeZ3List();
                    frag->addDepartureTimeZ3(ctx.real_val(std::to_string(frag->getDepartureTime(0)).c_str()));
                }
            }
            swt->toZ3(ctx, solver);
            swt->loadZ3(ctx, solver);
        }

        /*
        for (Switch swt : getSwitches()) {
            if (dynamic_cast<Switch *>(swt)) {
                ((Switch *) swt).loadZ3(ctx, solver);
            }
        }
        */

    }


    /*
     * GETTERS AND SETTERS
     */

    std::shared_ptr<expr> getJitterUpperBoundRangeZ3() {
        return jitterUpperBoundRangeZ3;
    }

    void setJitterUpperBoundRangeZ3(z3::expr jitterUpperBoundRange) {
        this->jitterUpperBoundRangeZ3 = std::make_shared<expr>(jitterUpperBoundRange);
    }

    void setJitterUpperBoundRangeZ3(context& ctx, double auxJitterUpperBoundRange) {
        this->jitterUpperBoundRangeZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(auxJitterUpperBoundRange).c_str()));
    }

    std::vector<Switch *> getSwitches() {
        return switches;
    }

    void setSwitches(std::vector<Switch *> switches) {
        this->switches = switches;
    }

    std::vector<Flow *> getFlows() {
        return flows;
    }

    void setFlows(std::vector<Flow *> flows) {
        this->flows = flows;
    }

    void addFlow (Flow *flw) {
        this->flows.push_back(flw);
    }

    void addSwitch (Switch *swt) {
        this->switches.push_back(swt);
    }
};

}

#endif

