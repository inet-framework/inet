#ifndef __INET_Z3_CYCLE_H
#define __INET_Z3_CYCLE_H

#include <algorithm>
#include <memory>
#include <z3++.h>

#include "inet/linklayer/configurator/z3/Defs.h"

namespace inet {

using namespace z3;

#define PACKETUPPERBOUNDRANGE 5 // Limits the applications of rules to the packets
#define CYCLEUPPERBOUNDRANGE 25 // Limits the applications of rules to the cycles

/**
 * [Class]: Cycle
 * [Usage]: Contains all properties of a TSN cycle.
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
    float upperBoundCycleTime;
    float lowerBoundCycleTime;
    float firstCycleStart;
    float maximumSlotDuration;

    float cycleDuration;
    float cycleStart;

    std::vector<std::vector<std::shared_ptr<expr>>> slotStartZ3_;
    std::vector<std::vector<std::shared_ptr<expr>>> slotDurationZ3_;

    std::vector<int> slotsUsed;
    std::vector<std::vector<float>> slotStart;
    std::vector<std::vector<float>> slotDuration;

    std::shared_ptr<expr> cycleDurationZ3;
    std::shared_ptr<expr> firstCycleStartZ3;
    std::shared_ptr<expr> maximumSlotDurationZ3;
    int numOfPrts = 8;

    int numOfSlots = 1;



    /**
     * [Method]: Cycle
     * [Usage]: Overloaded method of this class. Will create
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
    Cycle(float upperBoundCycleTime,
          float lowerBoundCycleTime,
          float maximumSlotDuration) {
        this->upperBoundCycleTime = upperBoundCycleTime;
        this->lowerBoundCycleTime = lowerBoundCycleTime;
        this->maximumSlotDuration = maximumSlotDuration;
        this->firstCycleStart = 0;
    }


    Cycle(float maximumSlotDuration) {
       this->maximumSlotDuration = maximumSlotDuration;
       this->firstCycleStart = 0;
    }


    /**
     * [Method]: Cycle
     * [Usage]: Overloaded method of this class. Will create
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
    Cycle(float upperBoundCycleTime,
          float lowerBoundCycleTime,
          float firstCycleStart,
          float maximumSlotDuration) {
        this->upperBoundCycleTime = upperBoundCycleTime;
        this->lowerBoundCycleTime = lowerBoundCycleTime;
        this->firstCycleStart = firstCycleStart;
        this->maximumSlotDuration = maximumSlotDuration;
    }

    /**
     * [Method]: Cycle
     * [Usage]: Overloaded method of this class. Will create
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
     * [Method]: toZ3
     * [Usage]: After setting all the numeric input values of the class,
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

        for(int i = 0; i < this->numOfPrts; i++) {
            this->slotStartZ3_.push_back(std::vector<std::shared_ptr<expr> >());
            this->slotDurationZ3_.push_back(std::vector<std::shared_ptr<expr> >());
            for(int j = 0; j < this->numOfSlots; j++) {
                expr v = ctx.real_const((std::string("cycleOfPort") + this->portName + std::string("prt") + std::to_string(i+1) + std::string("slot") + std::to_string(j+1)).c_str());
                this->slotStartZ3_.at(i).push_back(std::make_shared<expr>(v));
            }
        }

    }


    /**
     * [Method]: cycleStartZ3
     * [Usage]: Returns the time of the start of a cycle
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
     * [Method]: cycleStartZ3
     * [Usage]: Returns the time of the start of a cycle
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
     * [Method]: addSlotUsed
     * [Usage]: After generating the schedule, the z3 values are
     * converted to floats and integers. The used slots are now
     * placed on a arrayList, and so are the slot start and duration.
     *
     * @param prt           Priority of the slot to be added
     * @param sStart        Slot start of the slot to be added
     * @param sDuration     Slot duration of the slot to be added
     */
    void addSlotUsed(int prt, std::vector<float> sStart, std::vector<float> sDuration) {

        if(std::find(slotsUsed.begin(), slotsUsed.end(), prt) == slotsUsed.end()) {
            this->slotsUsed.push_back(prt);
            this->slotStart.push_back(sStart);
            this->slotDuration.push_back(sDuration);
        }

    }


    /**
     * [Method]: loadZ3
     * [Usage]: From the loaded primitive values of the class
     * obtained in the deserialization process, initialize the
     * z3 variables.
     *
     * @param ctx        context object of z3
     * @param solver    solver object to add constraints
     */
    void loadZ3(context& ctx, solver solver) {
        // maximumSlotDurationZ3 already started on toZ3;

        solver.add(
                *this->cycleDurationZ3 ==
                ctx.real_val(std::to_string(this->cycleDuration).c_str())
        );

        solver.add(
                *this->firstCycleStartZ3 ==
                ctx.real_val(std::to_string(this->firstCycleStart).c_str())
        );


        for(int prt : this->getSlotsUsed()) {

            // Where are the slot duration per priority instantiated? Must do it before loading

            for(int slotIndex = 0; slotIndex < this->numOfSlots; slotIndex++) {
                solver.add(
                        *this->slotStartZ3_.at(prt).at(slotIndex) ==
                        ctx.real_val(std::to_string(this->slotStart.at(std::find(slotsUsed.begin(), slotsUsed.end(), prt) - slotsUsed.begin()).at(slotIndex)).c_str())
                );
            }

            /*
            for(int slotIndex = 0; slotIndex < this->numOfSlots; slotIndex++) {
                solver.add(
                    mkEq(
                        this->slotDurationZ3_.get(prt).get(slotIndex),
                        ctx.real_val(std::to_string(this->slotDuration.get(prt).get(slotIndex)))
                    )
                );
            }
            */
        }



    }

    /*
     *  GETTERS AND SETTERS
     */


    float getUpperBoundCycleTime() {
        return upperBoundCycleTime;
    }

    void setUpperBoundCycleTime(float upperBoundCycleTime) {
        this->upperBoundCycleTime = upperBoundCycleTime;
    }

    float getLowerBoundCycleTime() {
        return lowerBoundCycleTime;
    }

    void setLowerBoundCycleTime(float lowerBoundCycleTime) {
        this->lowerBoundCycleTime = lowerBoundCycleTime;
    }

    float getCycleDuration() {
        return cycleDuration;
    }

    float getCycleStart() {
        return cycleStart;
    }


    void setCycleStart(float cycleStart) {
        this->cycleStart = cycleStart;
    }

    void setCycleDuration(float cycleDuration) {
        this->cycleDuration = cycleDuration;
    }

    float getFirstCycleStart() {
        return firstCycleStart;
    }

    void setFirstCycleStart(float firstCycleStart) {
        this->firstCycleStart = firstCycleStart;
    }

    float getMaximumSlotDuration() {
        return maximumSlotDuration;
    }

    void setMaximumSlotDuration(float maximumSlotDuration) {
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
        return this->numOfPrts;
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
        return this->slotsUsed;
    }

    std::vector<std::vector<float>> getSlotDuration() {
        return slotDuration;
    }

    float getSlotStart(int prt, int index) {
        return this->slotStart.at(std::find(slotsUsed.begin(), slotsUsed.end(), prt) - slotsUsed.begin()).at(index);
    }

    float getSlotDuration(int prt, int index) {
        return this->slotDuration.at(std::find(slotsUsed.begin(), slotsUsed.end(), prt) - slotsUsed.begin()).at(index);
    }

    std::string getPortName() {
        return portName;
    }


     void setPortName(std::string portName) {
        this->portName = portName;
    }

     std::shared_ptr<expr> getSlotStartZ3(int prt, int slotNum) {
        return this->slotStartZ3_.at(prt).at(slotNum);
    }

    /*
     std::shared_ptr<expr> getSlotDurationZ3(int prt, int slotNum) {
        return this->slotDurationZ3.get(prt).get(slotNum);
    }
    */
};

}

#endif

