#include "inet/linklayer/configurator/z3/Port.h"

namespace inet {

void Port::setUpCycleRules(solver solver, context& ctx) {

    for(FlowFragment *frag : this->flowFragments) {
        for(int index = 0; index < this->cycle->getNumOfSlots(); index++) {
            std::shared_ptr<expr> flowPriority = frag.getFragmentPriorityZ3();
            std::shared_ptr<expr> indexZ3 = ctx.int_val(index);

            // A slot will be somewhere between 0 and the end of the cycle minus its duration (Slot in cycle constraint)
            solver.add(mkGe(cycle.slotStartZ3(ctx, flowPriority, indexZ3), ctx.int_val(0)));
            solver.add(
                mkLe(cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                    mkSub(
                        cycle.getCycleDurationZ3(),
                        cycle.slotDurationZ3(ctx, flowPriority, indexZ3)
                    )
                )
            );

            // Every slot duration is greater or equal 0 and lower or equal than the maximum (Slot duration constraint)
            solver.add(mkGe(cycle.slotDurationZ3(ctx, flowPriority, indexZ3), ctx.int_val(0)));
            solver.add(mkLe(cycle.slotDurationZ3(ctx, flowPriority, indexZ3), cycle.getMaximumSlotDurationZ3()));

            //Every slot must fit inside a cycle
            solver.add(
                mkGe(
                    cycle.getCycleDurationZ3(),
                    mkAdd(
                        cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                        cycle.slotDurationZ3(ctx, flowPriority, indexZ3)
                    )
                )
            );

            /*
             * If the priority of the fragments are the same, then the start and duration
             * of a slot is also the same (needs to be specified due to z3 variable naming
             * properties) (Same priority, same slot constraint)
             */

            for (FlowFragment auxFrag : this->flowFragments) {
                solver.add(
                    mkImplies(
                        mkEq(frag.getFragmentPriorityZ3(), auxFrag.getFragmentPriorityZ3()),
                        mkAnd(

                             mkEq(
                                     cycle.slotStartZ3(ctx, frag.getFragmentPriorityZ3(), indexZ3),
                                     cycle.slotStartZ3(ctx, auxFrag.getFragmentPriorityZ3(), indexZ3)
                             ),
                             mkEq(
                                     cycle.slotDurationZ3(ctx, frag.getFragmentPriorityZ3(), indexZ3),
                                     cycle.slotDurationZ3(ctx, auxFrag.getFragmentPriorityZ3(), indexZ3)
                             )

                        )
                    )
                );
            }

            // No two slots can overlap (No overlapping slots constraint)
            for (FlowFragment auxFrag : this->flowFragments) {
                if(auxFrag.equals(frag)) {
                    continue;
                }

                std::shared_ptr<expr> auxFlowPriority = auxFrag.getFragmentPriorityZ3();

                solver.add(
                    mkImplies(
                        mkNot(
                            mkEq(
                                flowPriority,
                                auxFlowPriority
                            )
                        ),
                        mkOr(
                            mkGe(
                                cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                mkAdd(
                                    cycle.slotStartZ3(ctx, auxFlowPriority, indexZ3),
                                    cycle.slotDurationZ3(ctx, auxFlowPriority, indexZ3)
                                )
                            ),
                            mkLe(
                                mkAdd(
                                    cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                    cycle.slotDurationZ3(ctx, flowPriority, indexZ3)
                                ),
                                cycle.slotStartZ3(ctx, auxFlowPriority, indexZ3)
                            )
                        )
                    )
                );
            }


            if(index < this->cycle.getNumOfSlots() - 1) {
                solver.add(
                    mkLt(
                        cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                        cycle.slotStartZ3(ctx, flowPriority, ctx.int_val(index + 1))
                    )
                );
            }



            /*
             * If 2 slots are not consecutive, then there must be a space
             * of at least gbSize (the size of the guard band) between them
             * (guard band constraint).
             *
            for (FlowFragment auxFrag : this->flowFragments) {
                for(int auxIndex = 0; auxIndex < this->cycle.getNumOfSlots(); auxIndex++) {
                    std::shared_ptr<expr> auxIndexZ3 = ctx.int_val(auxIndex);

                    if(auxFrag.equals(frag)) {
                        continue;
                    }

                    std::shared_ptr<expr> auxFlowPriority = auxFrag.getFlowPriority();

                    solver.add(
                        mkImplies(
                            mkAnd(
                                mkNot(
                                    mkAnd(
                                        mkEq(flowPriority, auxFlowPriority),
                                        mkEq(indexZ3, auxIndexZ3)
                                    )
                                ),
                                mkNot(
                                    mkEq(
                                        cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                        mkAdd(
                                            cycle.slotDurationZ3(ctx, auxFlowPriority, auxIndexZ3),
                                            cycle.slotStartZ3(ctx, auxFlowPriority, auxIndexZ3)
                                        )
                                    )
                                ),
                                mkGt(
                                    cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                    cycle.slotStartZ3(ctx, auxFlowPriority, auxIndexZ3)
                                )
                            ),
                            mkGe(
                                cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                mkAdd(
                                    cycle.slotStartZ3(ctx, auxFlowPriority, auxIndexZ3),
                                    cycle.slotDurationZ3(ctx, auxFlowPriority, auxIndexZ3),
                                    gbSizeZ3
                                )
                            )
                        )
                    );
                }
            }
            /**/
        }

    }
}

void Port::setupTimeSlots(solver solver, context& ctx, FlowFragment *flowFrag) {
    std::shared_ptr<expr> indexZ3;

    // If there is a flow assigned to the slot, slotDuration must be greater than transmission time
    for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
        solver.add(
            mkGe(
                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index+1)),
                mkAdd(
                        cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index)),
                        cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index))
                )
            )
        );
    }

    for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
        indexZ3 = ctx.int_val(index);

        // solver.add(mkGe(cycle.slotDurationZ3(ctx, flowFrag.getFlowPriority(), indexZ3), this->transmissionTimeZ3));

        // Every flow must have a priority (Priority assignment constraint)
        solver.add(mkGe(flowFrag.getFragmentPriorityZ3(), ctx.int_val(0)));
        solver.add(mkLt(flowFrag.getFragmentPriorityZ3(), ctx.int_val(this->cycle.getNumOfPrts())));

        // Slot start must be <= cycle time - slot duration
        solver.add(
            mkLe(
                mkAdd(
                    cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                    cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3)
                ),
                cycle.getCycleDurationZ3()
            )
        );

    }
}

void Port::setupDevPacketTimes(solver solver, context& ctx, FlowFragment *flowFrag) {

    // For the specified range of packets defined by [0, upperBoundRange],
    // apply the scheduling rules.

    //System.out.println(std::string("Setting up rules for " + flowFrag.getName() + std::string(" - Num of packets: ")) + flowFrag.getParent().getNumOfPacketsSent());
    for(int i = 0; i < flowFrag.getParent().getNumOfPacketsSent(); i++) {
        // Make t3 > t2 + transmissionTime
        solver.add( // Time to Transmit constraint.
            mkGe(
                this->scheduledTime(ctx, i, flowFrag),
                mkAdd(this->arrivalTime(ctx, i, flowFrag), mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3))
            )
        );

    }

    std::shared_ptr<expr> indexZ3 = nullptr;
    Expr auxExp = nullptr;
    Expr auxExp2 = ctx.bool_val(true)();
    Expr exp = nullptr;

    for(FlowFragment auxFragment : this->flowFragments) {

        /*
        System.out.println(std::string("Num de pacotes escalonados:") + auxFragment.getNumOfPacketsSent());

        for(int i = 0; i < auxFragment.getNumOfPacketsSent(); i++) {
            solver.add(
                mkEq(
                    this->arrivalTime(ctx, i, auxFragment),
                    mkAdd(
                            this->departureTime(ctx, i, auxFragment),
                            this->timeToTravelZ3
                    )
                )
            );

            solver.add(
                mkGe(
                    this->scheduledTime(ctx, i, auxFragment),
                    mkAdd(
                            this->arrivalTime(ctx, i, auxFragment),
                            this->transmissionTimeZ3
                    )
                )
            );
        }
        /**/


        for(int i = 0; i < flowFrag.getNumOfPacketsSent(); i++) {
            for(int j = 0; j < auxFragment.getNumOfPacketsSent(); j++) {
                if (auxExp == nullptr) {
                    auxExp = ctx.bool_val(false);
                }

                if(auxFragment == flowFrag && i == j) {
                    continue;
                }


                /*****************************************************
                 *
                 * Packet A must be transfered after packet B or
                 * fit one of the three base cases.
                 *
                 *****************************************************/
                auxExp = mkOr((BoolExpr) auxExp,
                        mkAnd(
                            mkAnd(
                                mkEq(auxFragment.getFragmentPriorityZ3(), flowFrag.getFragmentPriorityZ3()),
                                mkLe(this->arrivalTime(ctx, i, flowFrag), this->arrivalTime(ctx, j, auxFragment))
                            ),
                            mkEq(
                                this->scheduledTime(ctx, j, auxFragment),
                                mkAdd(
                                    this->scheduledTime(ctx, i, flowFrag),
                                    mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                )
                            )
                        )
                );



            }


            for(int j = 0; j < this->cycleUpperBoundRange; j++) {

                /*
                T2 IS INSIDE SLOT, HAS ENOUGH TIME TO TRANSMIT
                ; **************************************
                ; |------------------------------------|
                ; CS       S    t2-------t3    E       CE
                ;               transmission
                ; **************************************
                */

                for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
                    indexZ3 = ctx.int_val(index);

                    /**/
                    auxExp2 = mkAnd((BoolExpr) auxExp2, // Arrived during a time slot predicate
                            mkImplies(
                                mkAnd(
                                    mkLe(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        mkSub(
                                            mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, ctx.int_val(j))
                                            ),
                                            mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                        )
                                    ),
                                    mkGe(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        mkAdd(
                                            cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                            cycle.cycleStartZ3(ctx, j)
                                        )
                                    )
                                ),
                                mkEq(
                                    this->scheduledTime(ctx, i, flowFrag),
                                    mkAdd(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                    )
                                )
                            )
                    );
                    /**/


                    /*
                    ; T2 IS BEFORE THE SLOT
                    ; **************************************
                    ; |------------------------------------|
                    ; CS     t2      S-------t3    E       CE
                    ;               transmission
                    ; **************************************
                    */

                    if(index == 0) {
                        auxExp2 = mkAnd((BoolExpr) auxExp2, // Arrived before slot start constraint
                                mkImplies(
                                    mkAnd(
                                        mkLt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, j)
                                            )
                                        ),
                                        mkGe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            cycle.cycleStartZ3(ctx, j)
                                        )
                                    ),
                                    mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        mkAdd(
                                            mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, j)
                                            ),
                                            mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                        )

                                    )
                                )
                            );
                    } else if (index < this->cycle.getNumOfSlots()) {
                        auxExp2 = mkAnd((BoolExpr) auxExp2,
                                mkImplies(
                                    mkAnd(
                                        mkLt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, j)
                                            )
                                        ),
                                        mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkSub(
                                                mkAdd(
                                                    cycle.cycleStartZ3(ctx, j),
                                                    cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index - 1)),
                                                    cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index - 1))
                                                ),
                                                mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                            )
                                        )
                                    ),
                                    mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        mkAdd(
                                            mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, j)
                                            ),
                                            mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                        )

                                    )
                                )
                            );
                    }
                    /**/



                    /*
                    ; T2 IS AFTER THE SLOT OR INSIDE WITHOUT ENOUGH TIME. The packet won't be trans-
                    ; mitted. This happens due to the usage of hyper and micro-cycles.
                    ; ****************************************************************************
                    ; |------------------------------------|------------------------------------|
                    ; CS        S        t2     E        CE/CS       S----------t3   E         CE
                    ;                                                transmission
                    ; ****************************************************************************
                    */

                    if(index == this->cycle.getNumOfSlots() - 1) {
                        auxExp2 = mkAnd((BoolExpr) auxExp2, // Arrived after slot end constraint
                                mkImplies(
                                    mkAnd(
                                        mkGe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            cycle.cycleStartZ3(ctx, j)
                                        ),
                                        mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle.getCycleDurationZ3(),
                                                cycle.cycleStartZ3(ctx, j)
                                            )
                                        )
                                    ),
                                    mkLe(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        mkAdd(
                                              cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                            cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                            cycle.cycleStartZ3(ctx, j)
                                        )
                                    )
                                )
                        );
                    }

                    /**
                    * THE CODE BELLOW HAS ISSUES REGARDING NOT COVERING ALL CASES (ALLOWS DELAY).
                    * REVIEW LATER.

                    if(j < this->cycleUpperBoundRange - 1 && index == this->cycle.getNumOfSlots() - 1) {
                        auxExp2 = mkAnd((BoolExpr) auxExp2, // Arrived after slot end constraint
                                mkImplies(
                                    mkAnd(
                                        mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkSub(
                                                mkAdd(
                                                    cycle.slotStartZ3(ctx, flowFrag.getFlowPriority(), indexZ3),
                                                    cycle.slotDurationZ3(ctx, flowFrag.getFlowPriority(), indexZ3),
                                                    cycle.cycleStartZ3(ctx, j)
                                                ),
                                                this->transmissionTimeZ3
                                            )
                                        ),
                                        mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle.cycleStartZ3(ctx, j),
                                                cycle.getCycleDurationZ3()
                                            )
                                        )
                                    ),

                                    mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        mkAdd(
                                            mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFlowPriority(), ctx.int_val(0)),
                                                cycle.cycleStartZ3(ctx, j + 1)
                                            ),
                                            this->transmissionTimeZ3
                                        )
                                    )
                                )
                        );
                    } else if (j == this->cycleUpperBoundRange - 1 && index == this->cycle.getNumOfSlots() - 1) {
                        auxExp2 = mkAnd((BoolExpr) auxExp2,
                                mkImplies(
                                    mkAnd(
                                        mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkSub(
                                                mkAdd(
                                                    cycle.slotStartZ3(ctx, flowFrag.getFlowPriority(), indexZ3),
                                                    cycle.slotDurationZ3(ctx, flowFrag.getFlowPriority(), indexZ3),
                                                    cycle.cycleStartZ3(ctx, j)
                                                ),
                                                this->transmissionTimeZ3
                                            )
                                        ),
                                        mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            mkAdd(
                                                cycle.cycleStartZ3(ctx, j),
                                                cycle.getCycleDurationZ3()
                                            )
                                        )
                                    ),

                                    mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        this->arrivalTime(ctx, i, flowFrag)
                                    )

                                )
                        );
                    }
                    /**/
                }
            }

            //auxExp = mkOr((BoolExpr)ctx.bool_val(false)(), (BoolExpr)auxExp2);
            auxExp = mkOr((BoolExpr)auxExp, (BoolExpr)auxExp2);


            if(exp == nullptr) {
                exp = auxExp;
            } else {
                exp = mkAnd((BoolExpr) exp, (BoolExpr) auxExp);
            }
            auxExp = ctx.bool_val(false)();
            auxExp2 = ctx.bool_val(true)();
        }
    }

    solver.add((BoolExpr)exp);

    auxExp = nullptr;
    exp = ctx.bool_val(false)();

    //Every packet must be transmitted inside a timeslot (transmit inside a time slot constraint)
    for(int i = 0; i < flowFrag.getNumOfPacketsSent(); i++) {
        for(int j = 0; j < this->cycleUpperBoundRange; j++) {
            for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
                indexZ3 = ctx.int_val(index);
                auxExp = mkAnd(
                         mkGe(
                            this->scheduledTime(ctx, i, flowFrag),
                            mkAdd(
                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                cycle.cycleStartZ3(ctx, j),
                                mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                            )
                          ),
                        mkLe(
                            this->scheduledTime(ctx, i, flowFrag),
                            mkAdd(
                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                cycle.cycleStartZ3(ctx, j)
                            )
                        )
                );

                exp = mkOr((BoolExpr) exp, (BoolExpr) auxExp);
            }
        }
        solver.add((BoolExpr) exp);
        exp = ctx.bool_val(false)();
    }


    /**/
    for(int i = 0; i < flowFrag.getNumOfPacketsSent() - 1; i++) {
        solver.add(
            mkGe(
                this->scheduledTime(ctx, i + 1, flowFrag),
                mkAdd(
                        this->scheduledTime(ctx, i, flowFrag),
                        mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                )
            )
        );
    }
    /**/



    for(int i = 0; i < flowFrag.getNumOfPacketsSent(); i++) {
        for(FlowFragment auxFlowFrag : this->flowFragments) {
            for(int j = 0; j < auxFlowFrag.getNumOfPacketsSent(); j++) {

               if(auxFlowFrag.equals(flowFrag)) {
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
               solver.add(
                   mkImplies(
                       mkEq(
                           auxFlowFrag.getFlowPriority(),
                           flowFrag.getFlowPriority()
                       ),
                       mkOr(
                           mkLe(
                               this->scheduledTime(ctx, i, flowFrag),
                               mkSub(
                                   this->scheduledTime(ctx, j, auxFlowFrag),
                                   this->transmissionTimeZ3
                               )
                           ),
                           mkGe(
                               this->scheduledTime(ctx, i, flowFrag),
                               mkAdd(
                                   this->scheduledTime(ctx, j, auxFlowFrag),
                                   this->transmissionTimeZ3
                               )
                           )
                       )
                   )
               );

               /*
                * Frame isolation constraint as specified by:
                *   Craciunas, Silviu S., et al. "Scheduling real-time communication in IEEE
                *   802.1 Qbv time sensitive networks." Proceedings of the 24th International
                *   Conference on Real-Time Networks and Systems. ACM, 2016.
                *
                * Two packets from different flows cannot be in the same priority queue
                * at the same time
                *

               solver.add(
                   mkImplies(
                      mkEq(
                          flowFrag.getFlowPriority(),
                           auxFlowFrag.getFlowPriority()
                       ),
                       mkOr(
                           mkLt(
                               this->scheduledTime(ctx, i, flowFrag),
                               this->arrivalTime(ctx, j, auxFlowFrag)
                           ),
                           mkGt(
                               this->arrivalTime(ctx, i, flowFrag),
                               this->scheduledTime(ctx, j, auxFlowFrag)
                           )
                       )
                   )
               );
               /**/

            }
        }
    }

    /*
     * If two packets are from the same priority, the first one to arrive
     * should be transmitted first (FIFO priority queue constraint)
     */
    for(int i = 0; i < flowFrag.getNumOfPacketsSent(); i++) {
        for(FlowFragment auxFlowFrag : this->flowFragments) {
            for(int j = 0; j < auxFlowFrag.getNumOfPacketsSent(); j++) {

                if((flowFrag.equals(auxFlowFrag) && i == j)) {
                    continue;
                }

                solver.add( // Packet transmission order constraint
                    mkImplies(
                        mkAnd(
                            mkLe(
                                this->arrivalTime(ctx, i, flowFrag),
                                this->arrivalTime(ctx, j, auxFlowFrag)
                            ),
                            mkEq(
                                flowFrag.getFragmentPriorityZ3(),
                                auxFlowFrag.getFragmentPriorityZ3()
                            )
                        ),
                        mkLe(
                            this->scheduledTime(ctx, i, flowFrag),
                            mkSub(
                                this->scheduledTime(ctx, j, auxFlowFrag),
                                mkDiv(auxFlowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                            )
                        )
                    )
                );

                /*
                if(!(flowFrag.equals(auxFlowFrag) && i == j)) {
                    solver.add(
                        mkNot(
                            mkEq(
                                this->arrivalTime(ctx, i, flowFrag),
                                this->arrivalTime(ctx, j, auxFlowFrag)
                            )
                        )
                    );
                }
                /**/

            }
        }
    }

}

void Port::setupBestEffort(solver solver, context& ctx) {
    std::shared_ptr<expr> []slotStart = new z3::expr[8];
    std::shared_ptr<expr> []slotDuration = new z3::expr[8];
    // z3::expr guardBandTime = nullptr;

    BoolExpr firstPartOfImplication = nullptr;
    std::shared_ptr<expr> sumOfPrtTime = nullptr;

    for(int i = 0; i < 8; i++) {
        slotStart[i] = ctx.real_const((this->name + std::string("SlotStart") + i).c_str());
        slotDuration[i] = ctx.real_const((this->name + std::string("SlotDuration") + i).c_str());
    }

    for(FlowFragment f : this->flowFragments) {
        // z3::expr sumOfSlotsStart = ctx.real_val(0);
        std::shared_ptr<expr> sumOfSlotsDuration = ctx.real_val(0);

        for(int i = 0; i < this->cycle.getNumOfSlots(); i++) {
            sumOfSlotsDuration = (z3::expr) mkAdd(cycle.slotDurationZ3(ctx, f.getFragmentPriorityZ3(), ctx.int_val(i)));
        }

        /**/
        for(int i = 1; i <= 8; i++) {
            solver.add(
                mkImplies(
                    mkEq(
                        f.getFragmentPriorityZ3(),
                        ctx.int_val(i)
                    ),
                    mkEq(
                        slotDuration[i-1],
                        sumOfSlotsDuration
                    )

                )
            );
        }
        /**/

    }

    for(int i = 1; i<=8; i++) {
        firstPartOfImplication = nullptr;

        for(FlowFragment f : this->flowFragments) {
            if(firstPartOfImplication == nullptr) {
                firstPartOfImplication = mkNot(mkEq(
                                            f.getFragmentPriorityZ3(),
                                            ctx.int_val(i)
                                         ));
            } else {
                firstPartOfImplication = mkAnd(firstPartOfImplication,
                                         mkNot(mkEq(
                                             f.getFragmentPriorityZ3(),
                                             ctx.int_val(i)
                                         )));
            }
        }

        solver.add( // Queue not used constraint
            mkImplies(
                firstPartOfImplication,
                mkAnd(
                    mkEq(slotStart[i-1], ctx.real_val(0)),
                    mkEq(slotDuration[i-1], ctx.real_val(0))
                )

            )
        );

    }

    for(z3::expr slotDr : slotDuration) {
        if(sumOfPrtTime == nullptr) {
            sumOfPrtTime = slotDr;
        } else {
            sumOfPrtTime = (z3::expr) mkAdd(sumOfPrtTime, slotDr);
        }
    }


    solver.add( // Best-effort bandwidth reservation constraint
        mkLe(
            sumOfPrtTime,
            mkMul(
                mkSub(
                    ctx.real_val(1),
                    bestEffortPercentZ3
                ),
                this->cycle.getCycleDurationZ3()
            )
        )
    );

    /*
    solver.add(
            mkLe(
                sumOfPrtTime,
                mkMul(bestEffortPercentZ3, this->cycle.getCycleDurationZ3())
            )
        );

    solver.add(
        mkGe(
            bestEffortPercentZ3,
            mkDiv(sumOfPrtTime, this->cycle.getCycleDurationZ3())
        )
    );
    */

}

}
