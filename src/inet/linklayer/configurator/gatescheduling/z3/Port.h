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

#ifndef __INET_Z3_PORT_H
#define __INET_Z3_PORT_H

#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Cycle.h"

namespace inet {

using namespace z3;

class FlowFragment;

/**
 * [Class]: Port
 * [Usage]: This class is used to implement the logical role of a
 * port of a switch for the schedule. The core of the scheduling
 * process happens here. Simplifying the whole process, the other
 * classes in this project are used to create, manage and break
 * flows into smaller pieces. These pieces are given to the switches,
 * and from the switches they will be given to their respective ports.
 *
 * After this is done, each port now has an array of fragments of flows
 * that are going through them. This way, it is easier to schedule
 * the packets since all you have to focus are the flow fragments that
 * might conflict in this specific port. The type of flow, its path or
 * anything else does not matter at this point.
 */
class INET_API Port {
  public:
    bool useMicroCycles = false;
    bool useHyperCycle = true;

    std::vector<double> listOfPeriods;
    double definedHyperCycleSize = -1;
    double microCycleSize = -1;

    std::string name;
    std::string connectsTo;

    double bestEffortPercent = 0.5f;
    std::shared_ptr<expr> bestEffortPercentZ3;

    Cycle *cycle;
    std::vector<FlowFragment *> flowFragments;
    int cycleUpperBoundRange = 25; // Limits the applications of rules to the cycles

    double gbSize;

    double maxPacketSize;
    double timeToTravel;
    double portSpeed;
    int portNum;

    std::shared_ptr<expr> gbSizeZ3; // Size of the guardBand
    std::shared_ptr<expr> maxPacketSizeZ3;
    std::shared_ptr<expr> timeToTravelZ3;
    std::shared_ptr<expr> portSpeedZ3;

    /**
     * [Method]: Port
     * [Usage]: Overloaded constructor of this class. Will start
     * the port with setting properties given as parameters.
     *
     * @param name                  Logical index of the port for z3
     * @param maxPacketSize         Maximum size of a packet that can be transmitted by this port
     * @param timeToTravel          Time to travel on the middle used by this port
     * @param portSpeed             Transmission speed of this port
     * @param gbSize                Size of the guard band
     * @param cycle                 Cycle used by the port
     */
    Port (std::string name,
          int portNum,
          std::string connectsTo,
          double maxPacketSize,
          double timeToTravel,
          double portSpeed,
          double gbSize,
          Cycle *cycle) {
        this->name = name;
        this->portNum = portNum;
        this->connectsTo = connectsTo;
        this->maxPacketSize = maxPacketSize;
        this->timeToTravel = timeToTravel;
        this->portSpeed = portSpeed;
        this->gbSize = gbSize;
        this->cycle = cycle;
        this->flowFragments.clear();
        this->cycle->setPortName(this->name);
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
        this->gbSizeZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(gbSize).c_str()));
        this->maxPacketSizeZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(this->maxPacketSize).c_str()));
        this->timeToTravelZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(this->timeToTravel).c_str()));
        this->portSpeedZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(portSpeed).c_str()));
        this->bestEffortPercentZ3 = std::make_shared<expr>(ctx.real_val(std::to_string(bestEffortPercent).c_str()));

        if (this->cycle->getFirstCycleStartZ3() == nullptr) {
            this->cycle->toZ3(ctx);
        }
    }

    /**
     * [Method]: setUpCycleRules
     * [Usage]: This method is responsible for setting up the scheduling rules related
     * to the cycle of this port. Assertions about how the time slots are supposed to be
     * are also specified here.
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     */
    void setUpCycleRules(solver& solver, context& ctx);

    /**
     * [Method]: setupTimeSlots
     * [Usage]: Given a single flow fragment, establish the scheduling rules
     * regarding its proper slot, referencing it using the fragment's priority.
     *
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param flowFrag      A fragment of a flow that goes through this port
     */
    void setupTimeSlots(solver& solver, context& ctx, FlowFragment *flowFrag);

    /**
     * [Method]: setupDevPacketTimes
     * [Usage]: Sets the core scheduling rules for a certain number of
     * packets of a flow fragment. The number of packets is specified
     * by the packetUpperBound range variable. The scheduler will attempt
     * to fit these packets within a certain number of cycles, specified
     * by the cycleUpperBoundRange variable.
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param flowFrag      A fragment of a flow that goes through this port
     */
    void setupDevPacketTimes(solver& solver, context& ctx, FlowFragment *flowFrag);

    /**
     * [Method]: setupBestEffort
     * [Usage]: Use in order to enable the best effort traffic reservation
     * constraint.
     *
     * @param solver    solver object
     * @param ctx        context object for the solver
     */
    void setupBestEffort(solver& solver, context& ctx);

    /**
     * [Method]: gcd
     * [Usage]: Method used to obtain the greatest common
     * divisor of two values.
     *
     * @param a        First value
     * @param b        Second value
     * @return        Greatest common divisor or the two previous parameters
     */
    static double gcd(double a, double b) {
        if (a == 0) {
            return b;
        }

        return gcd(fmod(b, a), a);
    }

    /**
     * [Method]: findGCD
     * [Usage]: Retrieves the value of the greatest common divisor
     * of all the values in an array.
     *
     * @param arr    Array of double values
     * @return        Greatest common divisor of all values of arr
     */
    static double findGCD(std::vector<double> arr) {
        double gdc = arr.at(0);
        for (int i = 1; i < arr.size(); i++) {
            gdc = gcd(arr.at(i), gdc);
        }

        return gdc;
    }

    /**
     * [Method]: findLCM
     * [Usage]: Retrieves the least common multiple of all values in
     * an array.
     *
     * @param arr         Array of double values
     * @return            Least common multiple of all values of arr
     */
    static double findLCM(std::vector<double> arr) {

        double n = arr.size();

        double max_num = 0;
        for (int i = 0; i < n; i++) {
            if (max_num < arr.at(i)) {
                max_num = arr.at(i);
            }
        }

        double res = 1;

        double x = 2;
        while (x <= max_num) {
            std::vector<int> indexes;
            for (int j = 0; j < n; j++) {
                if (fmod(arr.at(j), x) == 0) {
                    indexes.push_back(j);
                }
            }
            if (indexes.size() >= 2) {
                for (int j = 0; j < indexes.size(); j++) {
                    arr[indexes.at(j)] = arr.at(indexes.at(j)) / x;
                }

                res = res * x;
            } else {
                x++;
            }
        }

        for (int i = 0; i < n; i++) {
            res = res * arr.at(i);
        }

        return res;
    }

    /**
     * [Method]: setUpHyperCycle
     * [Usage]: Set up the cycle duration and number of packets and slots
     * to be scheduled according to the hyper cycle approach.
     *
     * @param solver    solver object
     * @param ctx        context object for the solver
     */
    void setUpHyperCycle(solver& solver, context& ctx) {
        int numOfPacketsScheduled = 0;

        /*
        for (FlowFragment *flowFrag : this->flowFragments) {
            System.out.println(flowFrag->getStartDevice());
            listOfPeriods.add(flowFrag->getStartDevice().getPacketPeriodicity());
        }
        for (double periodicity : this->listOfPeriods) {
            System.out.println(std::string("Periodicidade: ") + periodicity);
        }
        */

        double hyperCycleSize = findLCM(listOfPeriods);

        this->definedHyperCycleSize = hyperCycleSize;
        this->cycle->setCycleDuration(hyperCycleSize);

        this->cycleUpperBoundRange = 1;

        /*
        for (FlowFragment *flowFrag : this->flowFragments) {
            flowFrag->setNumOfPacketsSent((int) (hyperCycleSize/flowFrag->getStartDevice().getPacketPeriodicity()));
            System.out.println(std::string("Frag num packets: ") + flowFrag->getNumOfPacketsSent());
            numOfPacketsScheduled += (int) (hyperCycleSize/flowFrag->getStartDevice().getPacketPeriodicity());
        }
        */

        for (double periodicity : this->listOfPeriods) {
            numOfPacketsScheduled += (int) (hyperCycleSize/periodicity);
        }

        // System.out.println(std::string("Num of Cycles: ") + this->cycleUpperBoundRange);

        this->cycle->setNumOfSlots(numOfPacketsScheduled);

        // In order to use the value cycle time obtained, we must override the minimum and maximum cycle times
        this->cycle->setUpperBoundCycleTime(hyperCycleSize + 1);
        this->cycle->setLowerBoundCycleTime(hyperCycleSize - 1);

    }

    /**
     * [Method]: setUpMicroCycles
     * [Usage]: Set up the cycle duration and number of packets, cycles and slots
     * to be scheduled according to the micro cycle approach.
     *
     * @param solver    solver object
     * @param ctx        context object for the solver
     */
    void setUpMicroCycles(solver& solver, context& ctx) {

        /*
        for (FlowFragment *flowFrag : this->flowFragments) {
            System.out.println(flowFrag->getStartDevice());
            listOfPeriods.add(flowFrag->getStartDevice().getPacketPeriodicity());
        }
        */

        this->microCycleSize = findGCD(listOfPeriods);
        double hyperCycleSize = findLCM(listOfPeriods);

        this->definedHyperCycleSize = hyperCycleSize;

        this->cycleUpperBoundRange = (int) (hyperCycleSize/microCycleSize);

        /*
        for (FlowFragment *flowFrag : this->flowFragments) {
            flowFrag->setNumOfPacketsSent((int) (hyperCycleSize/flowFrag->getStartDevice().getPacketPeriodicity()));
            System.out.println(std::string("Frag num packets: ") + flowFrag->getNumOfPacketsSent());
        }
        System.out.println(std::string("Num of Cycles: ") + this->cycleUpperBoundRange);
          */

        // In order to use the value cycle time obtained, we must override the minimum and maximum cycle times
        this->cycle->setUpperBoundCycleTime(microCycleSize + 1);
        this->cycle->setLowerBoundCycleTime(microCycleSize - 1);
    }

    /**
     * [Method]: bindTimeSlots
     * [Usage]: IN DEVELOPMENT - Bind timeslots to a fixed name instead
     * of a variable.
     *
     * @param solver    solver object
     * @param ctx        context object for the solver
     */
    void bindTimeSlots(solver& solver, context& ctx) {

        // Ideia = se a prioridade de um flow e' igual a um numero,
        // mkEq nele com o slot the cycle (getSlotS/D(prt, slotnum))
        /*
        for (FlowFragment *frag : this->flowFragments) {

            for (int i = 0; i < this->cycle->getNumOfPrts(); i++) {
                for (int j = 0; j < this->cycle->getNumOfSlots(); j++) {
                    addAssert(solver,
                        mkITE(
                            mkAnd(
                                mkEq(frag->getFlowPriority(), ctx.int_val(i)),
                                mkEq(frag->get, ctx.int_val(j)))
                                ,));
                }
            }
        }
        */
    }

    /**
     * [Method]: setUpCycle
     * [Usage]: If the port is configured to use a specific automated
     * application period methodology, it will configure its cycle size.
     * Also calls the toZ3 method for the cycle
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     */
    void setUpCycle(solver& solver, context& ctx) {

        this->cycle->toZ3(ctx);

        // System.out.println(std::string("On port: " + this->name + std::string(" with: ") + this->listOfPeriods.size() + " fragments"));

        if (this->listOfPeriods.size() < 1) {
            return;
        }


        if (useMicroCycles && this->listOfPeriods.size() > 0) {
            setUpMicroCycles(solver, ctx);

            addAssert(solver,
                *this->cycle->getCycleDurationZ3() == ctx.real_val(std::to_string(this->microCycleSize).c_str()));
        } else if (useHyperCycle && this->listOfPeriods.size() > 0) {
            setUpHyperCycle(solver, ctx);

            addAssert(solver,
                *this->cycle->getCycleDurationZ3() == ctx.real_val(std::to_string(this->definedHyperCycleSize).c_str()));
        }
    }

    /**
     * [Method]: zeroOutNonUsedSlots
     * [Usage]: Iterates over the slots adding a constraint that states that
     * if no packet its transmitted inside it, its size must be 0. Can be used
     * to filter out non-used slots and avoid losing bandwidth.
     *
     * @param solver
     * @param ctx
     */
    void zeroOutNonUsedSlots(solver& solver, context& ctx);

    /**
     * [Method]: setupSchedulingRules
     * [Usage]: Calls the set of functions that will set the z3 rules
     * regarding the cycles, time slots, priorities and timing of packets.
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     */
    void setupSchedulingRules(solver& solver, context& ctx) {

        if (this->flowFragments.size() == 0) {
            addAssert(solver,
                ctx.real_val(std::to_string(0).c_str()) ==
                *this->cycle->getCycleDurationZ3());
            //    addAssert(solver, mkEq(
            //    ctx.real_val(std::to_string(0)),
            //    this->cycle->getFirstCycleStartZ3()
            // ));

            return;
        }

        /*
        if (useMicroCycles && this->flowFragments.size() > 0) {
            addAssert(solver, mkEq(
                ctx.real_val(std::to_string(this->microCycleSize)),
                this->cycle->getCycleDurationZ3()));
        } else if (useHyperCycle && this->flowFragments.size() > 0) {
            addAssert(solver, mkEq(
                ctx.real_val(std::to_string(this->definedHyperCycleSize)),
                this->cycle->getCycleDurationZ3()));
        } else {
            for (FlowFragment flowFrag : this->flowFragments) {
                flowFrag->setNumOfPacketsSent(this->packetUpperBoundRange);
            }
        }
        */


        setUpCycleRules(solver, ctx);
        zeroOutNonUsedSlots(solver, ctx);

        /*
         * Differently from setUpCycleRules, setupTimeSlots and setupDevPacketTimes
         * need to be called multiple times since there will be a completely new set
         * of rules for each flow fragment on this port.
         */


        for (FlowFragment *flowFrag : this->flowFragments) {
            setupTimeSlots(solver, ctx, flowFrag);
            setupDevPacketTimes(solver, ctx, flowFrag);
        }

        /*
        if (flowFragments.size() > 0) {
            setupBestEffort(solver, ctx);
        }
        */

    }

    /**
     * [Method]: departureTime
     * [Usage]: Retrieves the departure time of a packet from a flow fragment
     * specified by the index given as a parameter. The departure time is the
     * time when a packet leaves its previous node with this switch as a destination.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param index         Index of the packet of the flow fragment as a z3 variable
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     */

    std::shared_ptr<expr> departureTime(context& ctx, z3::expr index, FlowFragment *flowFrag){

        // If the index is 0, then its the first departure time, else add index * periodicity
        return this->departureTime(ctx, atoi(index.to_string().c_str()), flowFrag);

        /*
        return (z3::expr) mkITE(
               mkGe(index, ctx.int_val(1)),
               mkAdd(
                       flowFrag->getDepartureTimeZ3(int.parseInt(index.toString())),
                       mkMul(flowFrag->getPacketPeriodicity(), index)),
               flowFrag->getDepartureTimeZ3(int.parseInt(index.toString())));
        */
    }

    /**
     * [Method]: departureTime
     * [Usage]: Retrieves the departure time of a packet from a flow fragment
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
     * [Method]: arrivalTime
     * [Usage]: Retrieves the arrival time of a packet from a flow fragment
     * specified by the index given as a parameter. The arrival time is the
     * time when a packet reaches this switch's port.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex      Index of the packet of the flow fragment as a z3 variable
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     *
    std::shared_ptr<expr> arrivalTime(context& ctx, z3::expr index, FlowFragment flowFrag){


       // The arrival time of this index from the given flow fragment is
       // equal to the its departure time + time to travel

       return (z3::expr) mkAdd(
                      departureTime(ctx, index, flowFrag),
                      timeToTravelZ3);
    }
    */

    /**
     * [Method]: arrivalTime
     * [Usage]: Retrieves the arrival time of a packet from a flow fragment
     * specified by the index given as a parameter. The arrival time is the
     * time when a packet reaches this switch's port.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex      Index of the packet of the flow fragment as an integer
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the arrival time of the desired packet
     */
    std::shared_ptr<expr> arrivalTime(context& ctx, int auxIndex, FlowFragment *flowFrag) {
        expr index = ctx.int_val(auxIndex);

        return std::make_shared<expr>(mkAdd( // Arrival time value constraint
                        departureTime(ctx, index, flowFrag),
                        timeToTravelZ3));
    }

    /**
     * [Method]: scheduledTime
     * [Usage]: Retrieves the scheduled time of a packet from a flow fragment
     * specified by the index given as a parameter. The scheduled time is the
     * time when a packet leaves this switch for its next destination.
     *
     * Since the scheduled time is an unknown value, it won't be specified as an
     * if and else situation or an equation like the departure or arrival time.
     * Instead, given a flow fragment and an index, this function will return the
     * name of the z3 variable that will be the queried to the solver.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param index         Index of the packet of the flow fragment as a z3 variable
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the scheduled time of the desired packet
     *
    std::shared_ptr<expr> scheduledTime(context& ctx, z3::expr index, FlowFragment *flowFrag){
        std::shared_ptr<expr> devT3 = ctx.real_const((flowFrag->getName() + std::string("ScheduledTime") + index.toString()).c_str());

        return (z3::expr) devT3;
    }
    */

    /**
     * [Method]: scheduledTime
     * [Usage]: Retrieves the scheduled time of a packet from a flow fragment
     * specified by the index given as a parameter. The scheduled time is the
     * time when a packet leaves this switch for its next destination.
     *
     * Since the scheduled time is an unknown value, it won't be specified as an
     * if and else situation or an equation like the departure or arrival time.
     * Instead, given a flow fragment and an index, this function will return the
     * name of the z3 variable that will be the queried to the solver.
     *
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     * @param auxIndex         Index of the packet of the flow fragment as an integer
     * @param flowFrag      Flow fragment that the packets belong to
     * @return              Returns the z3 variable for the scheduled time of the desired packet
     */
    std::shared_ptr<expr> scheduledTime(context& ctx, int auxIndex, FlowFragment *flowFrag);

    /**
     * [Method]: checkIfAutomatedApplicationPeriod
     * [Usage]: Returns true if the port uses an automated application period
     * methodology.
     *
     * @return bool value. True if automated application period methodology is used, false elsewhise
     */
    bool checkIfAutomatedApplicationPeriod() {
        if (this->useHyperCycle || this->useMicroCycles)
            return true;
        return false;
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
    void loadZ3(context& ctx, solver& solver);

    /*
     * GETTERS AND SETTERS:
     */

    Cycle *getCycle() {
        return cycle;
    }

    void setCycle(Cycle *cycle) {
        this->cycle = cycle;
    }

    std::vector<FlowFragment *> getDeviceList() {
        return flowFragments;
    }

    void setDeviceList(std::vector<FlowFragment *> flowFragments) {
        this->flowFragments = flowFragments;
    }

    void addToFragmentList(FlowFragment *flowFrag) {
        this->flowFragments.push_back(flowFrag);
    }

    double getGbSize() {
        return gbSize;
    }

    void setGbSize(double gbSize) {
        this->gbSize = gbSize;
    }

    std::shared_ptr<expr> getGbSizeZ3() {
        return gbSizeZ3;
    }

    void setGbSizeZ3(z3::expr gbSizeZ3) {
        this->gbSizeZ3 = std::make_shared<expr>(gbSizeZ3);
    }

    std::string getName() {
        return name;
    }

    void setName(std::string name) {
        this->name = name;
    }

    std::string getConnectsTo() {
        return connectsTo;
    }

    void setConnectsTo(std::string connectsTo) {
        this->connectsTo = connectsTo;
    }

    void addToListOfPeriods(double period) {
        this->listOfPeriods.push_back(period);
    }

    std::vector<double> getListOfPeriods() {
        return listOfPeriods;
    }

    void setListOfPeriods(std::vector<double> listOfPeriods) {
        this->listOfPeriods = listOfPeriods;
    }

    int getCycleUpperBoundRange() {
        return cycleUpperBoundRange;
    }

    void setCycleUpperBoundRange(int cycleUpperBoundRange) {
        this->cycleUpperBoundRange = cycleUpperBoundRange;
    }

    double getDefinedHyperCycleSize() {
        return definedHyperCycleSize;
    }

    void setDefinedHyperCycleSize(double definedHyperCycleSize) {
        this->definedHyperCycleSize = definedHyperCycleSize;
    }

    int getPortNum() {
        return portNum;
    }

    void setPortNum(int portNum) {
        this->portNum = portNum;
    }

    std::vector<FlowFragment *> getFlowFragments() {
        return flowFragments;
    }

    void setFlowFragments(std::vector<FlowFragment *> flowFragments) {
        this->flowFragments = flowFragments;
    }

    /***************************************************
     *
     * The methods bellow are not completely operational
     * and are currently not used in the project. Might be
     * useful in future iterations of TSNsched.
     *
     ***************************************************/

    std::shared_ptr<expr> getCycleOfScheduledTime(context& ctx, FlowFragment *f, int index) {
        std::shared_ptr<expr> cycleIndex = nullptr;

        expr relativeST = mkSub(
                this->scheduledTime(ctx, index, f),
                this->cycle->getFirstCycleStartZ3());

        cycleIndex = std::make_shared<expr>(mkReal2Int(
                        (z3::expr) mkDiv(relativeST, this->cycle->getCycleDurationZ3())));

        return cycleIndex;
    }


    std::shared_ptr<expr> getCycleOfTime(context& ctx, z3::expr time) {
        std::shared_ptr<expr> cycleIndex = nullptr;

        expr relativeST = mkSub(
                time,
                this->cycle->getFirstCycleStartZ3());

        cycleIndex = std::make_shared<expr>(mkReal2Int(
                        (z3::expr) mkDiv(relativeST, this->cycle->getCycleDurationZ3())));

        return cycleIndex;
    }

    std::shared_ptr<expr> getScheduledTimeOfPreviousPacket(context& ctx, FlowFragment *f, int index);

};

}

#endif

