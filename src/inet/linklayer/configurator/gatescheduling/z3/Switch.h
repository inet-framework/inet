//
// Copyright (C) 2021 by original authors
//
// This file is copied from the following project with the explicit permission
// from the authors: https://github.com/ACassimiro/TSNsched
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#ifndef __INET_Z3_TSNSWITCH_H
#define __INET_Z3_TSNSWITCH_H

#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Device.h"
#include "inet/linklayer/configurator/gatescheduling/z3/Port.h"

namespace inet {

using namespace z3;

/**
 * This class contains the information needed to
 * specify a switch capable of complying with the TSN patterns
 * to the schedule. Aside from part of the z3 data used to
 * generate the schedule, objects created from this class
 * are able to organize a sequence of ports that connect the
 * switch to other nodes in the network.
 */
class INET_API Switch : public cObject {
  public:
    std::string name;

    std::vector<std::string> connectsTo;
    std::vector<Port *> ports;
    double cycleDurationUpperBound;
    double cycleDurationLowerBound;

    std::shared_ptr<expr> cycleDuration;
    std::shared_ptr<expr> cycleStart;
    std::shared_ptr<expr> cycleDurationUpperBoundZ3;
    std::shared_ptr<expr> cycleDurationLowerBoundZ3;
    int portNum = 0;

    static int indexCounter;

    /**
     * Overloaded constructor method of this class.
     * Creates a switch, giving it a name and creating a new
     * list of ports and labels of devices that it can reach.
     *
     * Can be adapted in the future to start the switch with
     * default properties.
     *
     * @param name      Name of the switch
     */
    Switch(std::string name) {
        this->name = name;
        ports.clear();
        connectsTo.clear();
    }

    /**
     * Overloaded constructor method of this class.
     * Instantiates a new Switch object setting up its properties
     * that are given as parameters. Used for simplified configurations.
     * Other constructors either are deprecated or set parameters
     * that will be used in future works.
     */
    Switch() {
        name = std::string("dev") + std::to_string(indexCounter++);
        ports.clear();
        connectsTo.clear();
    }

    /**
     * Overloaded constructor method of this class.
     * Instantiates a new Switch object setting up its properties
     * that are given as parameters.
     *
     * @param name                  Name of the switch
     */
    Switch(std::string name,
           double cycleDurationLowerBound,
           double cycleDurationUpperBound) {
        this->name = name;
        this->cycleDurationLowerBound = cycleDurationLowerBound;
        this->cycleDurationUpperBound = cycleDurationUpperBound;
        ports.clear();
        connectsTo.clear();
    }

    /**
     * After setting all the numeric input values of the class,
     * generates the z3 equivalent of these values and creates any extra
     * variable needed.
     *
     * @param ctx      context variable containing the z3 environment used
     */
    void toZ3(context& ctx, solver& solver) {
        cycleDurationLowerBoundZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(cycleDurationLowerBound).c_str()));
        cycleDurationUpperBoundZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(cycleDurationUpperBound).c_str()));

        // Creating the cycle duration and start for this switch
        cycleDuration = std::make_shared<expr>(ctx.real_const((std::string("cycleOf") + name + std::string("Duration")).c_str()));
        cycleStart = std::make_shared<expr>(ctx.real_const((std::string("cycleOf") + name + std::string("Start")).c_str()));


        // Creating the cycle setting up the bounds for the duration (Cycle duration constraint)
        addAssert(solver, cycleDuration >= cycleDurationLowerBoundZ3);
        addAssert(solver, cycleDuration <= cycleDurationUpperBoundZ3);

        // A cycle must start on a point in time, so it must be greater than 0
        addAssert(solver, cycleStart >= ctx.int_val(0)); // No negative cycle values constraint

        for (Port *port : ports) {
            port->toZ3(ctx);

            for (FlowFragment *frag : port->getFlowFragments())
                addAssert(solver, port->getCycle()->getFirstCycleStartZ3() <= arrivalTime(ctx, 0, frag)); // Maximum cycle start constraint

            addAssert(solver, port->getCycle()->getFirstCycleStartZ3() >= ctx.int_val(0)); // No negative cycle values constraint

            // The cycle of every port must have the same starting point
            addAssert(solver, cycleStart == port->getCycle()->getFirstCycleStartZ3()); // Equal cycle constraints
        }

        addAssert(solver, cycleStart == ctx.int_val(0));
    }

    /**
     * Iterates over the ports of the switch, calling the
     * method responsible for setting up the rules for each individual port
     * on the switch.
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     */
    void setupSchedulingRules(solver& solver, context& ctx) {
        for (Port *port : ports)
            port->setupSchedulingRules(solver, ctx);
    }

    /**
     * Adds a port to the switch. A cycle to that port and
     * the device object that it connects to (since TSN ports connect to
     * individual nodes in the approach of this schedule) must be given
     * as parameters.
     *
     * @param destination       Destination of the port as Switch or Device
     * @param cycle             Cycle used by the port
     */
    void createPort(cObject *destination, Cycle *cycle, double maxPacketSize, double timeToTravel, double portSpeed, double gbSize) {
        if (dynamic_cast<Device *>(destination)) {
            connectsTo.push_back(((Device *)destination)->getName());
            ports.push_back(
                    new Port(name + std::string("Port") + std::to_string(portNum),
                            portNum,
                            ((Device *)destination)->getName(),
                            maxPacketSize,
                            timeToTravel,
                            portSpeed,
                            gbSize,
                            cycle));
        } else if (dynamic_cast<Switch *>(destination)) {
            connectsTo.push_back(((Switch *)destination)->getName());

            Port *newPort = new Port(name + std::string("Port") + std::to_string(portNum),
                    portNum,
                    ((Switch *)destination)->getName(),
                    maxPacketSize,
                    timeToTravel,
                    portSpeed,
                    gbSize,
                    cycle);

            newPort->setPortNum(portNum);

            ports.push_back(newPort);
        }
        else
            ; // [TODO]: THROW ERROR
        portNum++;
    }

    /**
     * Adds a port to the switch. A cycle to that port and
     * the device name that it connects to (since TSN ports connect to
     * individual nodes in the approach of this schedule) must be given
     * as parameters.
     *
     * @param destination       Name of the destination of the port
     * @param cycle             Cycle used by the port
     */
    void createPort(std::string destination, Cycle *cycle, double maxPacketSize, double timeToTravel, double portSpeed, double gbSize) {
        connectsTo.push_back(destination);
        ports.push_back(
                new Port(name + std::string("Port") + std::to_string(portNum),
                        portNum,
                        destination,
                        maxPacketSize,
                        timeToTravel,
                        portSpeed,
                        gbSize,
                        cycle));
        portNum++;
    }

    /**
     * Given a flow fragment, it finds the port that connects to
     * its destination and adds the it to the fragment list of that specific
     * port.
     *
     * @param flowFrag      Fragment of a flow to be added to a port
     */
    void addToFragmentList(FlowFragment *flowFrag);

    /**
     * Given a name of a node, returns the port that
     * can reach this node.
     *
     * @param name      Name of the node that the switch is connects to
     * @return          Port of the switch that connects to a given node
     */
    Port *getPortOf(std::string name) {
        int index = std::find(connectsTo.begin(), connectsTo.end(), name) - connectsTo.begin();
        return ports.at(index);
    }

    /**
     * Iterate over its ports. The ones using automated application
     * periods will calculate their cycle size.
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     */
    void setUpCycleSize(solver& solver, context& ctx) {
        for (Port *port : ports) {
            port->setUpCycle(solver, ctx);
        }
    }

    /**
     * Retrieves the arrival time of a packet from a flow fragment
     * specified by the index given as a parameter. The arrival time is the
     * time when a packet reaches this switch's port.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex      Index of the packet of the flow fragment as an integer
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     */
    std::shared_ptr<expr> arrivalTime(context& ctx, int auxIndex, FlowFragment *flowFrag);

    /**
     * Retrieves the departure time of a packet from a flow fragment
     * specified by the index given as a parameter. The departure time is the
     * time when a packet leaves its previous node with this switch as a destination.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param index         Index of the packet of the flow fragment as a z3 variable
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     */
    std::shared_ptr<expr> departureTime(context& ctx, z3::expr index, FlowFragment *flowFrag);

    /**
     * Retrieves the departure time of a packet from a flow fragment
     * specified by the index given as a parameter. The departure time is the
     * time when a packet leaves its previous node with this switch as a destination.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex         Index of the packet of the flow fragment as an integer
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     */
    std::shared_ptr<expr> departureTime(context& ctx, int auxIndex, FlowFragment *flowFrag);

    /**
     * Retrieves the scheduled time of a packet from a flow fragment
     * specified by the index given as a parameter. The scheduled time is the
     * time when a packet leaves this switch for its next destination.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex         Index of the packet of the flow fragment as an integer
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the scheduled time of the desired packet
     */
    std::shared_ptr<expr> scheduledTime(context& ctx, int auxIndex, FlowFragment *flowFrag);

    void loadZ3(context& ctx, solver& solver) {
        if (!ports.empty()) {
            for (Port *port : ports) {
                //System.out.println(port.getIsModifiedOrCreated());
                port->loadZ3(ctx, solver);
            }
        }

    }

    std::string getName() {
        return name;
    }

    void setName(std::string name) {
        this->name = name;
    }

    Cycle *getCycle(int index) {
        return ports.at(index)->getCycle();
    }

    void setCycle(Cycle *cycle, int index) {
        ports.at(index)->setCycle(cycle);
    }

    std::vector<Port *> getPorts() {
        return ports;
    }

    void setPorts(std::vector<Port *> ports) {
        this->ports = ports;
    }

    void addPort(Port *port, std::string name) {
        ports.push_back(port);
        connectsTo.push_back(name);
    }

    std::shared_ptr<expr> getCycleDuration() {
        return cycleDuration;
    }

    void setCycleDuration(z3::expr cycleDuration) {
        this->cycleDuration = std::make_shared<expr>(cycleDuration);
    }

    std::shared_ptr<expr> getCycleStart() {
        return cycleStart;
    }

    void setCycleStart(z3::expr cycleStart) {
        this->cycleStart = std::make_shared<expr>(cycleStart);
    }

    std::vector<std::string> getConnectsTo(){
        return connectsTo;
    }
};

}

#endif

