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

#ifndef __INET_Z3_CYCLE_H
#define __INET_Z3_CYCLE_H

#include <algorithm>
#include <memory>
#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Defs.h"

namespace inet {

using namespace z3;

/**
 * Contains all properties of a TSN cycle.
 * After the specification of its properties through
 * user input, the toZ3 method can be used to convert
 * the values to z3 variables and query the unknown
 * values.
 *
 * There is no direct reference from a cycle to
 * its time slots. The user must use a priority from a flow
 * to reference the time window of a cycle. This happens
 * because of the generation of z3 variables.
 *
 * For example, if I want to know the duration of the time slot
 * reserved for the priority 3, it most likely will return a value
 * different from the actual time slot that a flow is making use.
 * This happens due to the way that z3 variables are generated. A flow
 * fragment can have a priority 3 on this cycle, but its variable
 * name will be "flowNfragmentMpriority". Even if z3 says that this
 * variable 3, the reference to the cycle duration will be called
 * "cycleXSlotflowNfragmentMpriorityDuration", which is clearly different
 * from "cycleXSlot3Duration".
 *
 * To make this work, every flow that has the same time window has the
 * same priority value. And this value is limited to a maximum value
 * (numOfSlots). So, to access the slot start and duration of a certain
 * priority, a flow fragment from that priority must be retrieved.
 *
 * This also deals with the problem of having unused priorities,
 * which can end up causing problems due to constraints of guard band
 * and such.
 *
 */
class INET_API Cycle {
  public:
    std::string portName = "";
    static int instanceCounter;
    double upperBoundCycleTime;
    double lowerBoundCycleTime;
    double firstCycleStart;
    double maximumSlotDuration;

    double cycleDuration;
    double cycleStart;

    std::vector<std::vector<std::shared_ptr<expr>>> slotStartZ3_;
    std::vector<std::vector<std::shared_ptr<expr>>> slotDurationZ3_;

    std::vector<int> slotsUsed;
    std::vector<std::vector<double>> slotStart;
    std::vector<std::vector<double>> slotDuration;

    std::shared_ptr<expr> cycleDurationZ3;
    std::shared_ptr<expr> firstCycleStartZ3;
    std::shared_ptr<expr> maximumSlotDurationZ3;

    int numOfPrts = 8;
    int numOfSlots = 1;

    /**
     * Overloaded method of this class. Will create
     * an object setting up the minimum and maximum cycle time,
     * the first cycle start and the maximum duration of a
     * priority slot. Other constructors either are deprecated
     * or set parameters that will be used in future works.
     *
     *
     * @param upperBoundCycleTime   Maximum size of the cycle
     * @param lowerBoundCycleTime   Minimum size of the cycle
     * @param maximumSlotDuration   Every priority slot should have up this time units
     */
    Cycle(double upperBoundCycleTime,
          double lowerBoundCycleTime,
          double maximumSlotDuration) {
        this->upperBoundCycleTime = upperBoundCycleTime;
        this->lowerBoundCycleTime = lowerBoundCycleTime;
        this->maximumSlotDuration = maximumSlotDuration;
        this->firstCycleStart = 0;
    }

    Cycle(double maximumSlotDuration) {
       this->maximumSlotDuration = maximumSlotDuration;
       this->firstCycleStart = 0;
    }

    /**
     * Overloaded method of this class. Will create
     * an object setting up the minimum and maximum cycle time,
     * the first cycle start and the maximum duration of a
     * priority slot.
     *
     *
     * @param upperBoundCycleTime   Maximum size of the cycle
     * @param lowerBoundCycleTime   Minimum size of the cycle
     * @param firstCycleStart       Where the first cycle should start
     * @param maximumSlotDuration   Every priority slot should have up this time units
     */
    Cycle(double upperBoundCycleTime,
          double lowerBoundCycleTime,
          double firstCycleStart,
          double maximumSlotDuration) {
        this->upperBoundCycleTime = upperBoundCycleTime;
        this->lowerBoundCycleTime = lowerBoundCycleTime;
        this->firstCycleStart = firstCycleStart;
        this->maximumSlotDuration = maximumSlotDuration;
    }

    /**
     * Overloaded method of this class. Will create
     * an object setting up the minimum and maximum cycle time,
     * the first cycle start and the maximum duration of a
     * priority slot. These properties must be given as z3
     * variables.
     *
     *
     * @param upperBoundCycleTimeZ3   Maximum size of the cycle
     * @param lowerBoundCycleTimeZ3   Minimum size of the cycle
     * @param firstCycleStartZ3       Where the first cycle should start
     * @param maximumSlotDurationZ3   Every priority slot should have up this time units
     */
    Cycle(z3::expr upperBoundCycleTimeZ3,
          std::shared_ptr<expr> lowerBoundCycleTimeZ3,
          std::shared_ptr<expr> firstCycleStartZ3,
          std::shared_ptr<expr> maximumSlotDurationZ3) {
        // this->upperBoundCycleTimeZ3 = upperBoundCycleTimeZ3;
        // this->lowerBoundCycleTimeZ3 = lowerBoundCycleTimeZ3;
        this->firstCycleStartZ3 = firstCycleStartZ3;
        //this->guardBandSizeZ3 = guardBandSize;
        this->maximumSlotDurationZ3 = maximumSlotDurationZ3;
    }

    /**
     * After setting all the numeric input values of the class,
     * generates the z3 equivalent of these values and creates any extra
     * variable needed.
     *
     * @param ctx      context variable containing the z3 environment used
     */
    void toZ3(context& ctx) {
        instanceCounter++;

        this->cycleDurationZ3 = std::make_shared<expr>(ctx.real_const((std::string("cycle") + std::to_string(instanceCounter) + std::string("Duration")).c_str()));
        this->firstCycleStartZ3 = std::make_shared<expr>(ctx.real_const((std::string("cycle") + std::to_string(instanceCounter) + std::string("Start")).c_str()));
        // this->firstCycleStartZ3 = std::make_shared<expr>(ctx.real_val((std::to_string(0)).c_str()));
        // this->firstCycleStartZ3 = std::make_shared<expr>(ctx.real_val((std::to_string(firstCycleStart)).c_str()));
        this->maximumSlotDurationZ3 = std::make_shared<expr>(ctx.real_val((std::to_string(maximumSlotDuration)).c_str()));

        this->slotStartZ3_.clear();
        this->slotDurationZ3_.clear();

        for (int i = 0; i < numOfPrts; i++) {
            slotStartZ3_.push_back(std::vector<std::shared_ptr<expr> >());
            slotDurationZ3_.push_back(std::vector<std::shared_ptr<expr> >());
            for (int j = 0; j < numOfSlots; j++) {
                expr v = ctx.real_const((std::string("cycleOfPort") + portName + std::string("prt") + std::to_string(i+1) + std::string("slot") + std::to_string(j+1)).c_str());
                slotStartZ3_.at(i).push_back(std::make_shared<expr>(v));
            }
        }
    }


    /**
     * Returns the time of the start of a cycle
     * specified by its index. The index is given as a z3
     * variable
     *
     * @param ctx       context containing the z3 environment
     * @param index     Index of the desired cycle
     * @return          Z3 variable containing the cycle start time
     */
    std::shared_ptr<expr> cycleStartZ3(context& ctx, z3::expr index){
        return std::make_shared<expr>(ite(index >= ctx.int_val(1),
                            *firstCycleStartZ3 + *cycleDurationZ3 * index,
                            *firstCycleStartZ3));
    }

    /**
     * Returns the time of the start of a cycle
     * specified by its index. The index is given as integer
     *
     * @param ctx       context containing the z3 environment
     * @param index     Index of the desired cycle
     * @return          Z3 variable containing the cycle start time
     */
    std::shared_ptr<expr> cycleStartZ3(context& ctx, int auxIndex){
        expr index = ctx.int_val(auxIndex);
        return std::make_shared<expr>(ite(index >= ctx.int_val(1),
                            *firstCycleStartZ3 + *cycleDurationZ3 * index,
                            *firstCycleStartZ3));

    }


    /**
     * After generating the schedule, the z3 values are
     * converted to doubles and integers. The used slots are now
     * placed on a arrayList, and so are the slot start and duration.
     *
     * @param prt           Priority of the slot to be added
     * @param sStart        Slot start of the slot to be added
     * @param sDuration     Slot duration of the slot to be added
     */
    void addSlotUsed(int prt, std::vector<double> sStart, std::vector<double> sDuration) {
        if (std::find(slotsUsed.begin(), slotsUsed.end(), prt) == slotsUsed.end()) {
            slotsUsed.push_back(prt);
            slotStart.push_back(sStart);
            slotDuration.push_back(sDuration);
        }
    }

    /**
     * From the loaded primitive values of the class
     * obtained in the deserialization process, initialize the
     * z3 variables.
     *
     * @param ctx        context object of z3
     * @param solver    solver object to add constraints
     */
    void loadZ3(context& ctx, solver& solver) {
        // maximumSlotDurationZ3 already started on toZ3;

        addAssert(solver, cycleDurationZ3 == ctx.real_val(std::to_string(cycleDuration).c_str()));
        addAssert(solver, firstCycleStartZ3 == ctx.real_val(std::to_string(firstCycleStart).c_str()));

        for (int prt : getSlotsUsed()) {
            // Where are the slot duration per priority instantiated? Must do it before loading
            for (int slotIndex = 0; slotIndex < numOfSlots; slotIndex++) {
                addAssert(solver,
                    slotStartZ3_.at(prt).at(slotIndex) ==
                    ctx.real_val(std::to_string(slotStart.at(std::find(slotsUsed.begin(), slotsUsed.end(), prt) - slotsUsed.begin()).at(slotIndex)).c_str()));
            }
        }
    }

    double getUpperBoundCycleTime() {
        return upperBoundCycleTime;
    }

    void setUpperBoundCycleTime(double upperBoundCycleTime) {
        this->upperBoundCycleTime = upperBoundCycleTime;
    }

    double getLowerBoundCycleTime() {
        return lowerBoundCycleTime;
    }

    void setLowerBoundCycleTime(double lowerBoundCycleTime) {
        this->lowerBoundCycleTime = lowerBoundCycleTime;
    }

    double getCycleDuration() {
        return cycleDuration;
    }

    double getCycleStart() {
        return cycleStart;
    }


    void setCycleStart(double cycleStart) {
        this->cycleStart = cycleStart;
    }

    void setCycleDuration(double cycleDuration) {
        this->cycleDuration = cycleDuration;
    }

    double getFirstCycleStart() {
        return firstCycleStart;
    }

    void setFirstCycleStart(double firstCycleStart) {
        this->firstCycleStart = firstCycleStart;
    }

    double getMaximumSlotDuration() {
        return maximumSlotDuration;
    }

    void setMaximumSlotDuration(double maximumSlotDuration) {
        this->maximumSlotDuration = maximumSlotDuration;
    }

    std::shared_ptr<expr> slotStartZ3(context& ctx, z3::expr prt, z3::expr index) {
        return std::make_shared<expr>(ctx.real_const((std::string("priority") + prt.to_string() + std::string("slot") + index.to_string() + "Start").c_str()));
    }

    std::shared_ptr<expr> slotStartZ3(context& ctx, int auxPrt, int auxIndex) {
        expr index = ctx.int_val(auxIndex);
        expr prt = ctx.int_val(auxPrt);
        return std::make_shared<expr>(ctx.real_const((std::string("priority") + prt.to_string() + std::string("slot") + index.to_string() + "Start").c_str()));
    }

    std::shared_ptr<expr> slotDurationZ3(context& ctx, z3::expr prt, z3::expr index) {
        return std::make_shared<expr>(ctx.real_const((std::string("priority") + prt.to_string() + std::string("slot") + index.to_string() + "Duration").c_str()));
    }

    std::shared_ptr<expr> slotDurationZ3(context& ctx, int auxPrt, int auxIndex) {
        expr index = ctx.int_val(auxIndex);
        expr prt = ctx.int_val(auxPrt);
        return std::make_shared<expr>(ctx.real_const((std::string("priority") + prt.to_string() + std::string("slot") + index.to_string() + "Duration").c_str()));
    }

    std::shared_ptr<expr> getCycleDurationZ3() {
        return cycleDurationZ3;
    }

    void setCycleDurationZ3(z3::expr cycleDuration) {
        this->cycleDurationZ3 = std::make_shared<expr>(cycleDuration);
    }

    std::shared_ptr<expr> getFirstCycleStartZ3() {
        return firstCycleStartZ3;
    }

    void setFirstCycleStartZ3(z3::expr firstCycleStart) {
        this->firstCycleStartZ3 = std::make_shared<expr>(firstCycleStart);
    }

    int getNumOfPrts() {
        return numOfPrts;
    }

    void setNumOfPrts(int numOfPrts) {
        this->numOfPrts = numOfPrts;
    }

    int getNumOfSlots() {
        return numOfSlots;
    }

    void setNumOfSlots(int numOfSlots) {
        this->numOfSlots = numOfSlots;
    }

    std::shared_ptr<expr> getMaximumSlotDurationZ3() {
        return maximumSlotDurationZ3;
    }

    void setMaximumSlotDurationZ3(z3::expr maximumSlotDuration) {
        this->maximumSlotDurationZ3 = std::make_shared<expr>(maximumSlotDuration);
    }

    std::vector<int> getSlotsUsed (){
        return slotsUsed;
    }

    std::vector<std::vector<double>> getSlotDuration() {
        return slotDuration;
    }

    double getSlotStart(int prt, int index) {
        return slotStart.at(std::find(slotsUsed.begin(), slotsUsed.end(), prt) - slotsUsed.begin()).at(index);
    }

    double getSlotDuration(int prt, int index) {
        return slotDuration.at(std::find(slotsUsed.begin(), slotsUsed.end(), prt) - slotsUsed.begin()).at(index);
    }

    std::string getPortName() {
        return portName;
    }

    void setPortName(std::string portName) {
        this->portName = portName;
    }

    std::shared_ptr<expr> getSlotStartZ3(int prt, int slotNum) {
        return slotStartZ3_.at(prt).at(slotNum);
    }
};

}

#endif

