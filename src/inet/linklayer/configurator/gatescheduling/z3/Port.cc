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

#include "inet/linklayer/configurator/gatescheduling/z3/FlowFragment.h"
#include "inet/linklayer/configurator/gatescheduling/z3/Port.h"

namespace inet {

void Port::setUpCycleRules(solver& solver, context& ctx) {

    for (FlowFragment *frag : this->flowFragments) {
        for (int index = 0; index < this->cycle->getNumOfSlots(); index++) {
            std::shared_ptr<expr> flowPriority = frag->getFragmentPriorityZ3();
            expr indexZ3 = ctx.int_val(index);

            // A slot will be somewhere between 0 and the end of the cycle minus its duration (Slot in cycle constraint)
            addAssert(solver, mkGe(cycle->slotStartZ3(ctx, *flowPriority, indexZ3), ctx.int_val(0)));
            addAssert(solver,
                mkLe(cycle->slotStartZ3(ctx, *flowPriority, indexZ3),
                    mkSub(
                        cycle->getCycleDurationZ3(),
                        cycle->slotDurationZ3(ctx, *flowPriority, indexZ3))));

            // Every slot duration is greater or equal 0 and lower or equal than the maximum (Slot duration constraint)
            addAssert(solver, mkGe(cycle->slotDurationZ3(ctx, *flowPriority, indexZ3), ctx.int_val(0)));
            addAssert(solver, mkLe(cycle->slotDurationZ3(ctx, *flowPriority, indexZ3), cycle->getMaximumSlotDurationZ3()));

            //Every slot must fit inside a cycle
            addAssert(solver,
                mkGe(
                    cycle->getCycleDurationZ3(),
                    mkAdd(
                        cycle->slotStartZ3(ctx, *flowPriority, indexZ3),
                        cycle->slotDurationZ3(ctx, *flowPriority, indexZ3))));

            /*
             * If the priority of the fragments are the same, then the start and duration
             * of a slot is also the same (needs to be specified due to z3 variable naming
             * properties) (Same priority, same slot constraint)
             */

            for (FlowFragment *auxFrag : this->flowFragments) {
                addAssert(solver,
                    implies(
                        mkEq(frag->getFragmentPriorityZ3(), *auxFrag->getFragmentPriorityZ3()),
                        mkAnd(

                             mkEq(
                                     cycle->slotStartZ3(ctx, *frag->getFragmentPriorityZ3(), indexZ3),
                                     cycle->slotStartZ3(ctx, *auxFrag->getFragmentPriorityZ3(), indexZ3)),
                             mkEq(
                                     cycle->slotDurationZ3(ctx, *frag->getFragmentPriorityZ3(), indexZ3),
                                     cycle->slotDurationZ3(ctx, *auxFrag->getFragmentPriorityZ3(), indexZ3))
)));
            }

            // No two slots can overlap (No overlapping slots constraint)
            for (FlowFragment *auxFrag : this->flowFragments) {
                if (auxFrag == frag) {
                    continue;
                }

                std::shared_ptr<expr> auxFlowPriority = auxFrag->getFragmentPriorityZ3();

                addAssert(solver,
                    implies(
                        !mkEq(
                            flowPriority,
                            auxFlowPriority),
                        mkOr(
                            mkGe(
                                cycle->slotStartZ3(ctx, *flowPriority, indexZ3),
                                mkAdd(
                                    cycle->slotStartZ3(ctx, *auxFlowPriority, indexZ3),
                                    cycle->slotDurationZ3(ctx, *auxFlowPriority, indexZ3))),
                            mkLe(
                                mkAdd(
                                    cycle->slotStartZ3(ctx, *flowPriority, indexZ3),
                                    cycle->slotDurationZ3(ctx, *flowPriority, indexZ3)),
                                cycle->slotStartZ3(ctx, *auxFlowPriority, indexZ3)))));
            }


            if (index < this->cycle->getNumOfSlots() - 1) {
                addAssert(solver,
                    mkLt(
                        cycle->slotStartZ3(ctx, *flowPriority, indexZ3),
                        cycle->slotStartZ3(ctx, *flowPriority, ctx.int_val(index + 1))));
            }



            /*
             * If 2 slots are not consecutive, then there must be a space
             * of at least gbSize (the size of the guard band) between them
             * (guard band constraint).
             *
            for (FlowFragment auxFrag : this->flowFragments) {
                for (int auxIndex = 0; auxIndex < this->cycle->getNumOfSlots(); auxIndex++) {
                    std::shared_ptr<expr> auxIndexZ3 = ctx.int_val(auxIndex);

                    if (auxFrag->equals(frag)) {
                        continue;
                    }

                    std::shared_ptr<expr> auxFlowPriority = auxFrag->getFlowPriority();

                    addAssert(solver,
                        implies(
                            mkAnd(
                                mkNot(
                                    mkAnd(
                                        mkEq(flowPriority, auxFlowPriority),
                                        mkEq(indexZ3, auxIndexZ3))),
                                mkNot(
                                    mkEq(
                                        cycle->slotStartZ3(ctx, flowPriority, indexZ3),
                                        mkAdd(
                                            cycle->slotDurationZ3(ctx, auxFlowPriority, auxIndexZ3),
                                            cycle->slotStartZ3(ctx, auxFlowPriority, auxIndexZ3)))),
                                mkGt(
                                    cycle->slotStartZ3(ctx, flowPriority, indexZ3),
                                    cycle->slotStartZ3(ctx, auxFlowPriority, auxIndexZ3))),
                            mkGe(
                                cycle->slotStartZ3(ctx, flowPriority, indexZ3),
                                mkAdd(
                                    cycle->slotStartZ3(ctx, auxFlowPriority, auxIndexZ3),
                                    cycle->slotDurationZ3(ctx, auxFlowPriority, auxIndexZ3),
                                    gbSizeZ3))));
                }
            }
            */
        }

    }
}

void Port::setupTimeSlots(solver& solver, context& ctx, FlowFragment *flowFrag) {
    // If there is a flow assigned to the slot, slotDuration must be greater than transmission time
    for (int index = 0; index < this->cycle->getNumOfSlots(); index++) {
        addAssert(solver,
            mkGe(
                cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), ctx.int_val(index+1)),
                mkAdd(
                        cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), ctx.int_val(index)),
                        cycle->slotDurationZ3(ctx, *flowFrag->getFragmentPriorityZ3(), ctx.int_val(index)))));
    }

    for (int index = 0; index < this->cycle->getNumOfSlots(); index++) {
        expr indexZ3 = ctx.int_val(index);

        // addAssert(solver, mkGe(cycle->slotDurationZ3(ctx, flowFrag->getFlowPriority(), indexZ3), this->transmissionTimeZ3));

        // Every flow must have a priority (Priority assignment constraint)
        addAssert(solver, mkGe(flowFrag->getFragmentPriorityZ3(), ctx.int_val(0)));
        addAssert(solver, mkLt(flowFrag->getFragmentPriorityZ3(), ctx.int_val(this->cycle->getNumOfPrts())));

        // Slot start must be <= cycle time - slot duration
        addAssert(solver,
            mkLe(
                mkAdd(
                    cycle->slotDurationZ3(ctx, *flowFrag->getFragmentPriorityZ3(), indexZ3),
                    cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), indexZ3)),
                cycle->getCycleDurationZ3()));

    }
}

void Port::setupDevPacketTimes(solver& solver, context& ctx, FlowFragment *flowFrag) {

    // For the specified range of packets defined by [0, upperBoundRange],
    // apply the scheduling rules.

    //System.out.println(std::string("Setting up rules for " + flowFrag->getName() + std::string(" - Num of packets: ")) + flowFrag->getParent().getNumOfPacketsSent());
    for (int i = 0; i < flowFrag->getParent()->getNumOfPacketsSent(); i++) {
        // Make t3 > t2 + transmissionTime
        addAssert(solver,  // Time to Transmit constraint.
            mkGe(
                this->scheduledTime(ctx, i, flowFrag),
                mkAdd(this->arrivalTime(ctx, i, flowFrag), mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3))));

    }

    std::shared_ptr<expr> indexZ3 = nullptr;
    std::shared_ptr<expr> auxExp = nullptr;
    std::shared_ptr<expr> auxExp2 = std::make_shared<expr>(ctx.bool_val(true));
    std::shared_ptr<expr> exp = nullptr;

    for (FlowFragment *auxFragment : this->flowFragments) {

        /*
        System.out.println(std::string("Num de pacotes escalonados:") + auxFragment->getNumOfPacketsSent());

        for (int i = 0; i < auxFragment->getNumOfPacketsSent(); i++) {
            addAssert(solver,
                mkEq(
                    this->arrivalTime(ctx, i, auxFragment),
                    mkAdd(
                            this->departureTime(ctx, i, auxFragment),
                            this->timeToTravelZ3)));

            addAssert(solver,
                mkGe(
                    this->scheduledTime(ctx, i, auxFragment),
                    mkAdd(
                            this->arrivalTime(ctx, i, auxFragment),
                            this->transmissionTimeZ3)));
        }
        */


        for (int i = 0; i < flowFrag->getNumOfPacketsSent(); i++) {
            for (int j = 0; j < auxFragment->getNumOfPacketsSent(); j++) {
                if (auxExp == nullptr) {
                    auxExp = std::make_shared<expr>(ctx.bool_val(false));
                }

                if (auxFragment == flowFrag && i == j) {
                    continue;
                }


                /*****************************************************
                 *
                 * Packet A must be transfered after packet B or
                 * fit one of the three base cases.
                 *
                 *****************************************************/
                auxExp = std::make_shared<expr>(mkOr(auxExp,
                        mkAnd(
                            mkAnd(
                                mkEq(auxFragment->getFragmentPriorityZ3(), flowFrag->getFragmentPriorityZ3()),
                                mkLe(this->arrivalTime(ctx, i, flowFrag), this->arrivalTime(ctx, j, auxFragment))),
                            mkEq(
                                this->scheduledTime(ctx, j, auxFragment),
                                mkAdd(
                                    this->scheduledTime(ctx, i, flowFrag),
                                    mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3))))));



            }


            for (int j = 0; j < this->cycleUpperBoundRange; j++) {

                /*
                T2 IS INSIDE SLOT, HAS ENOUGH TIME TO TRANSMIT
                ; **************************************
                ; |------------------------------------|
                ; CS       S    t2-------t3    E       CE
                ;               transmission
                ; **************************************
                */

                for (int index = 0; index < this->cycle->getNumOfSlots(); index++) {
                    indexZ3 = std::make_shared<expr>(ctx.int_val(index));

                    auxExp2 = std::make_shared<expr>(mkAnd(auxExp2, // Arrived during a time slot predicate
                            implies(
                                mkAnd(
                                    mkLe(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        mkSub(
                                            mkAdd(
                                                cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                                mkAdd(
                                                    cycle->slotDurationZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                                    cycle->cycleStartZ3(ctx, ctx.int_val(j)))),
                                            mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3))),
                                    mkGe(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        mkAdd(
                                            cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                            cycle->cycleStartZ3(ctx, j)))),
                                mkEq(
                                    this->scheduledTime(ctx, i, flowFrag),
                                    mkAdd(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3))))));


                    /*
                    ; T2 IS BEFORE THE SLOT
                    ; **************************************
                    ; |------------------------------------|
                    ; CS     t2      S-------t3    E       CE
                    ;               transmission
                    ; **************************************
                    */

                    if (index == 0) {
                        auxExp2 = std::make_shared<expr>(mkAnd(auxExp2, // Arrived before slot start constraint
                                implies(
                                    mkAnd(
                                        mkLt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                                cycle->cycleStartZ3(ctx, j))),
                                        mkGe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            cycle->cycleStartZ3(ctx, j))),
                                    mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        mkAdd(
                                            mkAdd(
                                                cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                                cycle->cycleStartZ3(ctx, j)),
                                            mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3))
))));
                    } else if (index < this->cycle->getNumOfSlots()) {
                        auxExp2 = std::make_shared<expr>(mkAnd(auxExp2,
                                implies(
                                    mkAnd(
                                        mkLt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                                cycle->cycleStartZ3(ctx, j))),
                                        mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkSub(
                                                mkAdd(
                                                    cycle->cycleStartZ3(ctx, j),
                                                    mkAdd(
                                                        cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), ctx.int_val(index - 1)),
                                                        cycle->slotDurationZ3(ctx, *flowFrag->getFragmentPriorityZ3(), ctx.int_val(index - 1)))),
                                                mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3)))),
                                    mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        mkAdd(
                                            mkAdd(
                                                cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                                cycle->cycleStartZ3(ctx, j)),
                                            mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3))
))));
                    }

                    /*
                    ; T2 IS AFTER THE SLOT OR INSIDE WITHOUT ENOUGH TIME. The packet won't be trans-
                    ; mitted. This happens due to the usage of hyper and micro-cycles.
                    ; ****************************************************************************
                    ; |------------------------------------|------------------------------------|
                    ; CS        S        t2     E        CE/CS       S----------t3   E         CE
                    ;                                                transmission
                    ; ****************************************************************************
                    */

                    if (index == this->cycle->getNumOfSlots() - 1) {
                        auxExp2 = std::make_shared<expr>(mkAnd(auxExp2, // Arrived after slot end constraint
                                implies(
                                    mkAnd(
                                        mkGe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            cycle->cycleStartZ3(ctx, j)),
                                        mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle->getCycleDurationZ3(),
                                                cycle->cycleStartZ3(ctx, j)))),
                                    mkLe(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        mkAdd(
                                            cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                            mkAdd(
                                                cycle->slotDurationZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                                cycle->cycleStartZ3(ctx, j)))))));
                    }

                    /**
                    * THE CODE BELLOW HAS ISSUES REGARDING NOT COVERING ALL CASES (ALLOWS DELAY).
                    * REVIEW LATER.

                    if (j < this->cycleUpperBoundRange - 1 && index == this->cycle->getNumOfSlots() - 1) {
                        auxExp2 = mkAnd((expr) auxExp2, // Arrived after slot end constraint
                                implies(
                                    mkAnd(
                                        mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkSub(
                                                mkAdd(
                                                    cycle->slotStartZ3(ctx, flowFrag->getFlowPriority(), indexZ3),
                                                    cycle->slotDurationZ3(ctx, flowFrag->getFlowPriority(), indexZ3),
                                                    cycle->cycleStartZ3(ctx, j)),
                                                this->transmissionTimeZ3)),
                                        mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle->cycleStartZ3(ctx, j),
                                                cycle->getCycleDurationZ3()))),

                                    mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        mkAdd(
                                            mkAdd(
                                                cycle->slotStartZ3(ctx, flowFrag->getFlowPriority(), ctx.int_val(0)),
                                                cycle->cycleStartZ3(ctx, j + 1)),
                                            this->transmissionTimeZ3))));
                    } else if (j == this->cycleUpperBoundRange - 1 && index == this->cycle->getNumOfSlots() - 1) {
                        auxExp2 = mkAnd((expr) auxExp2,
                                implies(
                                    mkAnd(
                                        mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkSub(
                                                mkAdd(
                                                    cycle->slotStartZ3(ctx, flowFrag->getFlowPriority(), indexZ3),
                                                    cycle->slotDurationZ3(ctx, flowFrag->getFlowPriority(), indexZ3),
                                                    cycle->cycleStartZ3(ctx, j)),
                                                this->transmissionTimeZ3)),
                                        mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle->cycleStartZ3(ctx, j),
                                                cycle->getCycleDurationZ3()))),

                                    mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        this->arrivalTime(ctx, i, flowFrag))
));
                    }
                    */
                }
            }

            //auxExp = mkOr((expr)ctx.bool_val(false)(), (expr)auxExp2);
            auxExp = std::make_shared<expr>(mkOr(auxExp, auxExp2));


            if (exp == nullptr) {
                exp = auxExp;
            } else {
                exp = std::make_shared<expr>(mkAnd(exp, auxExp));
            }
            auxExp = std::make_shared<expr>(ctx.bool_val(false));
            auxExp2 = std::make_shared<expr>(ctx.bool_val(true));
        }
    }

    addAssert(solver, *exp);

    auxExp = nullptr;
    exp = std::make_shared<expr>(ctx.bool_val(false));

    //Every packet must be transmitted inside a timeslot (transmit inside a time slot constraint)
    for (int i = 0; i < flowFrag->getNumOfPacketsSent(); i++) {
        for (int j = 0; j < this->cycleUpperBoundRange; j++) {
            for (int index = 0; index < this->cycle->getNumOfSlots(); index++) {
                indexZ3 = std::make_shared<expr>(ctx.int_val(index));
                auxExp = std::make_shared<expr>(mkAnd(
                         mkGe(
                            this->scheduledTime(ctx, i, flowFrag),
                            mkAdd(
                                cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                mkAdd(
                                    cycle->cycleStartZ3(ctx, j),
                                    mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3)))),
                        mkLe(
                            this->scheduledTime(ctx, i, flowFrag),
                            mkAdd(
                                cycle->slotStartZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                mkAdd(
                                    cycle->slotDurationZ3(ctx, *flowFrag->getFragmentPriorityZ3(), *indexZ3),
                                    cycle->cycleStartZ3(ctx, j))))));

                exp = std::make_shared<expr>(mkOr(exp, auxExp));
            }
        }
        addAssert(solver, *exp);
        exp = std::make_shared<expr>(ctx.bool_val(false));
    }

    for (int i = 0; i < flowFrag->getNumOfPacketsSent() - 1; i++) {
        addAssert(solver,
            mkGe(
                this->scheduledTime(ctx, i + 1, flowFrag),
                mkAdd(
                        this->scheduledTime(ctx, i, flowFrag),
                        mkDiv(flowFrag->getPacketSizeZ3(), this->portSpeedZ3))));
    }

    for (int i = 0; i < flowFrag->getNumOfPacketsSent(); i++) {
        for (FlowFragment *auxFlowFrag : this->flowFragments) {
            for (int j = 0; j < auxFlowFrag->getNumOfPacketsSent(); j++) {

               if (auxFlowFrag == flowFrag) {
                   continue;
               }



               /*
                * Given that packets from two different flows have
                * the same priority in this switch, they must not
                * be transmitted at the same time
                *
                * OBS: This constraint might not be needed due to the
                * FIFO constraint.
                *
               addAssert(solver,
                   implies(
                       mkEq(
                           auxFlowFrag->getFlowPriority(),
                           flowFrag->getFlowPriority()),
                       mkOr(
                           mkLe(
                               this->scheduledTime(ctx, i, flowFrag),
                               mkSub(
                                   this->scheduledTime(ctx, j, auxFlowFrag),
                                   this->transmissionTimeZ3)),
                           mkGe(
                               this->scheduledTime(ctx, i, flowFrag),
                               mkAdd(
                                   this->scheduledTime(ctx, j, auxFlowFrag),
                                   this->transmissionTimeZ3)))));
               */

               /*
                * Frame isolation constraint as specified by:
                *   Craciunas, Silviu S., et al. "Scheduling real-time communication in IEEE
                *   802.1 Qbv time sensitive networks." Proceedings of the 24th International
                *   Conference on Real-Time Networks and Systems. ACM, 2016.
                *
                * Two packets from different flows cannot be in the same priority queue
                * at the same time
                *

               addAssert(solver,
                   implies(
                      mkEq(
                          flowFrag->getFlowPriority(),
                           auxFlowFrag->getFlowPriority()),
                       mkOr(
                           mkLt(
                               this->scheduledTime(ctx, i, flowFrag),
                               this->arrivalTime(ctx, j, auxFlowFrag)),
                           mkGt(
                               this->arrivalTime(ctx, i, flowFrag),
                               this->scheduledTime(ctx, j, auxFlowFrag)))));
               */

            }
        }
    }

    /*
     * If two packets are from the same priority, the first one to arrive
     * should be transmitted first (FIFO priority queue constraint)
     */
    for (int i = 0; i < flowFrag->getNumOfPacketsSent(); i++) {
        for (FlowFragment *auxFlowFrag : this->flowFragments) {
            for (int j = 0; j < auxFlowFrag->getNumOfPacketsSent(); j++) {

                if ((flowFrag == auxFlowFrag && i == j)) {
                    continue;
                }

                addAssert(solver,  // Packet transmission order constraint
                    implies(
                        mkAnd(
                            mkLe(
                                this->arrivalTime(ctx, i, flowFrag),
                                this->arrivalTime(ctx, j, auxFlowFrag)),
                            mkEq(
                                flowFrag->getFragmentPriorityZ3(),
                                auxFlowFrag->getFragmentPriorityZ3())),
                        mkLe(
                            this->scheduledTime(ctx, i, flowFrag),
                            mkSub(
                                this->scheduledTime(ctx, j, auxFlowFrag),
                                mkDiv(auxFlowFrag->getPacketSizeZ3(), this->portSpeedZ3)))));

                /*
                if (!(flowFrag->equals(auxFlowFrag) && i == j)) {
                    addAssert(solver,
                        mkNot(
                            mkEq(
                                this->arrivalTime(ctx, i, flowFrag),
                                this->arrivalTime(ctx, j, auxFlowFrag))));
                }
                */

            }
        }
    }

}

void Port::setupBestEffort(solver& solver, context& ctx) {
    std::shared_ptr<expr> slotStart[8];
    std::shared_ptr<expr> slotDuration[8];
    // expr guardBandTime = nullptr;

    std::shared_ptr<expr> firstPartOfImplication = nullptr;
    std::shared_ptr<expr> sumOfPrtTime = nullptr;

    for (int i = 0; i < 8; i++) {
        slotStart[i] = std::make_shared<expr>(ctx.real_const((this->name + std::string("SlotStart") + std::to_string(i)).c_str()));
        slotDuration[i] = std::make_shared<expr>(ctx.real_const((this->name + std::string("SlotDuration") + std::to_string(i)).c_str()));
    }

    for (FlowFragment f : this->flowFragments) {
        // expr sumOfSlotsStart = ctx.real_val(0);
        std::shared_ptr<expr> sumOfSlotsDuration = std::make_shared<expr>(ctx.real_val(0));

        for (int i = 0; i < this->cycle->getNumOfSlots(); i++) {
            sumOfSlotsDuration = std::make_shared<expr>(mkAdd(sumOfSlotsDuration, cycle->slotDurationZ3(ctx, *f.getFragmentPriorityZ3(), ctx.int_val(i))));
        }

        for (int i = 1; i <= 8; i++) {
            addAssert(solver,
                implies(
                    mkEq(
                        f.getFragmentPriorityZ3(),
                        ctx.int_val(i)),
                    mkEq(
                        slotDuration[i-1],
                        sumOfSlotsDuration)
));
        }
    }

    for (int i = 1; i<=8; i++) {
        firstPartOfImplication = nullptr;

        for (FlowFragment f : this->flowFragments) {
            if (firstPartOfImplication == nullptr) {
                firstPartOfImplication = std::make_shared<expr>(!mkEq(
                                            *f.getFragmentPriorityZ3(),
                                            ctx.int_val(i)));
            } else {
                firstPartOfImplication = std::make_shared<expr>(mkAnd(firstPartOfImplication,
                                         !mkEq(
                                             *f.getFragmentPriorityZ3(),
                                             ctx.int_val(i))));
            }
        }

        addAssert(solver,  // Queue not used constraint
            implies(
                *firstPartOfImplication,
                mkAnd(
                    mkEq(slotStart[i-1], ctx.real_val(0)),
                    mkEq(slotDuration[i-1], ctx.real_val(0)))
));

    }

    for (std::shared_ptr<expr> slotDr : slotDuration) {
        if (sumOfPrtTime == nullptr) {
            sumOfPrtTime = slotDr;
        } else {
            sumOfPrtTime = std::make_shared<expr>(mkAdd(sumOfPrtTime, slotDr));
        }
    }


    addAssert(solver,  // Best-effort bandwidth reservation constraint
        mkLe(
            sumOfPrtTime,
            mkMul(
                mkSub(
                    ctx.real_val(1),
                    bestEffortPercentZ3),
                this->cycle->getCycleDurationZ3())));

    /*
    addAssert(solver,
            mkLe(
                sumOfPrtTime,
                mkMul(bestEffortPercentZ3, this->cycle->getCycleDurationZ3())));

    addAssert(solver,
        mkGe(
            bestEffortPercentZ3,
            mkDiv(sumOfPrtTime, this->cycle->getCycleDurationZ3())));
    */

}

void Port::zeroOutNonUsedSlots(solver& solver, context& ctx)
{
    std::shared_ptr<expr> exp1;
    std::shared_ptr<expr> exp2;
    std::shared_ptr<expr> indexZ3;

    for (int prtIndex = 0; prtIndex < this->cycle->getNumOfPrts(); prtIndex++) {
        for (FlowFragment *frag : this->flowFragments) {
            for (int slotIndex = 0; slotIndex < this->cycle->getNumOfSlots(); slotIndex++) {
                addAssert(solver,
                    implies(
                        mkEq(frag->getFragmentPriorityZ3(), ctx.int_val(prtIndex)),
                        mkAnd(
                            mkEq(
                                cycle->slotStartZ3(ctx, *frag->getFragmentPriorityZ3(), ctx.int_val(slotIndex)),
                                cycle->slotStartZ3(ctx, ctx.int_val(prtIndex), ctx.int_val(slotIndex))),
                            mkEq(
                                cycle->slotDurationZ3(ctx, *frag->getFragmentPriorityZ3(), ctx.int_val(slotIndex)),
                                cycle->slotDurationZ3(ctx, ctx.int_val(prtIndex), ctx.int_val(slotIndex))))));
            }
        }
    }


    for (int prtIndex = 0; prtIndex < this->cycle->getNumOfPrts(); prtIndex++) {
        for (int cycleNum = 0; cycleNum < this->cycleUpperBoundRange; cycleNum++) {
            for (int indexNum = 0; indexNum < this->cycle->getNumOfSlots(); indexNum++) {
                indexZ3 = std::make_shared<expr>(ctx.int_val(indexNum));
                exp1 = std::make_shared<expr>(ctx.bool_val(true));
                for (FlowFragment *frag : this->flowFragments) {
                    for (int packetNum = 0; packetNum < frag->getNumOfPacketsSent(); packetNum++) {
                        exp1 = std::make_shared<expr>(mkAnd(
                                exp1,
                                mkAnd(
                                    !mkAnd(
                                        mkGe(
                                            this->scheduledTime(ctx, packetNum, frag),
                                            mkAdd(
                                                cycle->slotStartZ3(ctx, ctx.int_val(prtIndex), *indexZ3),
                                                cycle->cycleStartZ3(ctx, ctx.int_val(cycleNum)))),
                                        mkLe(
                                            this->scheduledTime(ctx, packetNum, frag),
                                            mkAdd(
                                                cycle->slotStartZ3(ctx, ctx.int_val(prtIndex), *indexZ3),
                                                mkAdd(
                                                        cycle->slotDurationZ3(ctx, ctx.int_val(prtIndex), *indexZ3),
                                                        cycle->cycleStartZ3(ctx, ctx.int_val(cycleNum)))))),
                                    mkEq(ctx.int_val(prtIndex), frag->getFragmentPriorityZ3()))));
                    }

                }

                addAssert(solver,
                    implies(
                        *exp1,
                        mkEq(cycle->slotDurationZ3(ctx, ctx.int_val(prtIndex), *indexZ3), ctx.int_val(0))));
            }

        }

    }
}

std::shared_ptr<expr> Port::departureTime(context& ctx, int auxIndex, FlowFragment *flowFrag)
{
    std::shared_ptr<expr> index = nullptr;
    std::shared_ptr<expr> departureTime;
    int cycleNum = 0;


    if (auxIndex + 1 > flowFrag->getNumOfPacketsSent()) {
        cycleNum = (auxIndex - (auxIndex % flowFrag->getNumOfPacketsSent()))/flowFrag->getNumOfPacketsSent();

        auxIndex = (auxIndex % flowFrag->getNumOfPacketsSent());

        departureTime = std::make_shared<expr>(
                mkAdd(
                        flowFrag->getDepartureTimeZ3(auxIndex),
                    mkMul(ctx.real_val(cycleNum), this->cycle->getCycleDurationZ3())));


        return departureTime;
    }

    departureTime = flowFrag->getDepartureTimeZ3(auxIndex);

    return departureTime;

}

std::shared_ptr<expr> Port::scheduledTime(context& ctx, int auxIndex, FlowFragment *flowFrag)
{
    std::shared_ptr<expr> index = nullptr;
    std::shared_ptr<expr> scheduledTime;
    int cycleNum = 0;

    if (auxIndex + 1 > flowFrag->getNumOfPacketsSent()) {
        cycleNum = (auxIndex - (auxIndex % flowFrag->getNumOfPacketsSent()))/flowFrag->getNumOfPacketsSent();

        auxIndex = (auxIndex % flowFrag->getNumOfPacketsSent());
        index = std::make_shared<expr>(ctx.int_val(auxIndex));

        scheduledTime = std::make_shared<expr>(
                mkAdd(
                    ctx.real_const((flowFrag->getName() + std::string("ScheduledTime") + index->to_string()).c_str()),
                    mkMul(ctx.real_val(cycleNum), this->cycle->getCycleDurationZ3())));

        return scheduledTime;
    }

    index = std::make_shared<expr>(ctx.int_val(auxIndex));


    scheduledTime = std::make_shared<expr>(ctx.real_const((flowFrag->getName() + std::string("ScheduledTime") + index->to_string()).c_str()));

    return scheduledTime;
}

void Port::loadZ3(context& ctx, solver& solver)
{

    this->cycle->loadZ3(ctx, solver);

    for (FlowFragment *frag : this->flowFragments) {

        frag->setFragmentPriorityZ3(
            ctx.int_val(
                    frag->getFragmentPriority()));

        /*
        addAssert(solver,
            mkEq(
                frag->getFragmentPriorityZ3(),
                ctx.int_val(frag->getFragmentPriority())));
        */

        for (int index = 0; index < this->cycle->getNumOfSlots(); index++) {
            addAssert(solver,
                mkEq(
                    this->cycle->slotDurationZ3(ctx, *frag->getFragmentPriorityZ3(), ctx.int_val(index)),
                    ctx.real_val(
                        std::to_string(
                            this->cycle->getSlotDuration(frag->getFragmentPriority(), index)).c_str())));

            addAssert(solver,
                mkEq(
                    this->cycle->slotStartZ3(ctx, *frag->getFragmentPriorityZ3(), ctx.int_val(index)),
                    ctx.real_val(
                        std::to_string(
                            this->cycle->getSlotStart(frag->getFragmentPriority(), index)).c_str())));

        }

        for (int i = 0; i < frag->getNumOfPacketsSent(); i++) {
            /*
            addAssert(solver,
                mkEq(
                    this->departureTime(ctx, i, frag),
                    ctx.real_val(std::to_string(frag->getDepartureTime(i)))));
            */
            if (i > 0)
                frag->addDepartureTimeZ3(ctx.real_val(std::to_string(frag->getDepartureTime(i)).c_str()));

            addAssert(solver,
                mkEq(
                    this->arrivalTime(ctx, i, frag),
                    ctx.real_val(std::to_string(frag->getArrivalTime(i)).c_str())));

            addAssert(solver,
                mkEq(
                    this->scheduledTime(ctx, i, frag),
                    ctx.real_val(std::to_string(frag->getScheduledTime(i)).c_str())));

        }

    }
}

std::shared_ptr<expr> Port::getScheduledTimeOfPreviousPacket(context& ctx, FlowFragment *f, int index)
{
    std::shared_ptr<expr> prevPacketST = std::make_shared<expr>(ctx.real_val(0));

    for (FlowFragment *auxFrag : this->flowFragments) {

        for (int i = 0; i < f->getNumOfPacketsSent(); i++) {
            prevPacketST = std::make_shared<expr>(
                        mkAnd(
                            mkEq(auxFrag->getFragmentPriorityZ3(), f->getFragmentPriorityZ3()),
                            mkAnd(
                                mkLt(
                                    this->scheduledTime(ctx, i, auxFrag),
                                    this->scheduledTime(ctx, index, f)),
                                mkGt(
                                    this->scheduledTime(ctx, i, auxFrag),
                                    prevPacketST))) ?
                        *this->scheduledTime(ctx, i, auxFrag) :
                        *prevPacketST);

        }

    }

    return std::make_shared<expr>(
                mkEq(prevPacketST, ctx.real_val(0)) ?
                *this->scheduledTime(ctx, index, f) :
                *prevPacketST);
}

}
