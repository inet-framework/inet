#include "inet/linklayer/configurator/z3/Port.h"

namespace inet {

void Port::setUpCycleRules(solver solver, context& ctx) {

    for(FlowFragment *frag : this->flowFragments) {
        for(int index = 0; index < this->cycle->getNumOfSlots(); index++) {
            std::shared_ptr<expr> flowPriority = frag.getFragmentPriorityZ3();
            std::shared_ptr<expr> indexZ3 = ctx.int_val(index);

            // A slot will be somewhere between 0 and the end of the cycle minus its duration (Slot in cycle constraint)
            solver.add(ctx.mkGe(cycle.slotStartZ3(ctx, flowPriority, indexZ3), ctx.int_val(0)));
            solver.add(
                ctx.mkLe(cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                    ctx.mkSub(
                        cycle.getCycleDurationZ3(),
                        cycle.slotDurationZ3(ctx, flowPriority, indexZ3)
                    )
                )
            );

            // Every slot duration is greater or equal 0 and lower or equal than the maximum (Slot duration constraint)
            solver.add(ctx.mkGe(cycle.slotDurationZ3(ctx, flowPriority, indexZ3), ctx.int_val(0)));
            solver.add(ctx.mkLe(cycle.slotDurationZ3(ctx, flowPriority, indexZ3), cycle.getMaximumSlotDurationZ3()));

            //Every slot must fit inside a cycle
            solver.add(
                ctx.mkGe(
                    cycle.getCycleDurationZ3(),
                    ctx.mkAdd(
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
                    ctx.mkImplies(
                        ctx.mkEq(frag.getFragmentPriorityZ3(), auxFrag.getFragmentPriorityZ3()),
                        ctx.mkAnd(

                             ctx.mkEq(
                                     cycle.slotStartZ3(ctx, frag.getFragmentPriorityZ3(), indexZ3),
                                     cycle.slotStartZ3(ctx, auxFrag.getFragmentPriorityZ3(), indexZ3)
                             ),
                             ctx.mkEq(
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
                    ctx.mkImplies(
                        ctx.mkNot(
                            ctx.mkEq(
                                flowPriority,
                                auxFlowPriority
                            )
                        ),
                        ctx.mkOr(
                            ctx.mkGe(
                                cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                ctx.mkAdd(
                                    cycle.slotStartZ3(ctx, auxFlowPriority, indexZ3),
                                    cycle.slotDurationZ3(ctx, auxFlowPriority, indexZ3)
                                )
                            ),
                            ctx.mkLe(
                                ctx.mkAdd(
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
                    ctx.mkLt(
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
                        ctx.mkImplies(
                            ctx.mkAnd(
                                ctx.mkNot(
                                    ctx.mkAnd(
                                        ctx.mkEq(flowPriority, auxFlowPriority),
                                        ctx.mkEq(indexZ3, auxIndexZ3)
                                    )
                                ),
                                ctx.mkNot(
                                    ctx.mkEq(
                                        cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                        ctx.mkAdd(
                                            cycle.slotDurationZ3(ctx, auxFlowPriority, auxIndexZ3),
                                            cycle.slotStartZ3(ctx, auxFlowPriority, auxIndexZ3)
                                        )
                                    )
                                ),
                                ctx.mkGt(
                                    cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                    cycle.slotStartZ3(ctx, auxFlowPriority, auxIndexZ3)
                                )
                            ),
                            ctx.mkGe(
                                cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                                ctx.mkAdd(
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
            ctx.mkGe(
                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index+1)),
                ctx.mkAdd(
                        cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index)),
                        cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index))
                )
            )
        );
    }

    for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
        indexZ3 = ctx.int_val(index);

        // solver.add(ctx.mkGe(cycle.slotDurationZ3(ctx, flowFrag.getFlowPriority(), indexZ3), this->transmissionTimeZ3));

        // Every flow must have a priority (Priority assignment constraint)
        solver.add(ctx.mkGe(flowFrag.getFragmentPriorityZ3(), ctx.int_val(0)));
        solver.add(ctx.mkLt(flowFrag.getFragmentPriorityZ3(), ctx.int_val(this->cycle.getNumOfPrts())));

        // Slot start must be <= cycle time - slot duration
        solver.add(
            ctx.mkLe(
                ctx.mkAdd(
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
            ctx.mkGe(
                this->scheduledTime(ctx, i, flowFrag),
                ctx.mkAdd(this->arrivalTime(ctx, i, flowFrag), ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3))
            )
        );

    }

    std::shared_ptr<expr> indexZ3 = nullptr;
    Expr auxExp = nullptr;
    Expr auxExp2 = ctx.mkTrue();
    Expr exp = nullptr;

    for(FlowFragment auxFragment : this->flowFragments) {

        /*
        System.out.println(std::string("Num de pacotes escalonados:") + auxFragment.getNumOfPacketsSent());

        for(int i = 0; i < auxFragment.getNumOfPacketsSent(); i++) {
            solver.add(
                ctx.mkEq(
                    this->arrivalTime(ctx, i, auxFragment),
                    ctx.mkAdd(
                            this->departureTime(ctx, i, auxFragment),
                            this->timeToTravelZ3
                    )
                )
            );

            solver.add(
                ctx.mkGe(
                    this->scheduledTime(ctx, i, auxFragment),
                    ctx.mkAdd(
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
                    auxExp = ctx.mkFalse();
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
                auxExp = ctx.mkOr((BoolExpr) auxExp,
                        ctx.mkAnd(
                            ctx.mkAnd(
                                ctx.mkEq(auxFragment.getFragmentPriorityZ3(), flowFrag.getFragmentPriorityZ3()),
                                ctx.mkLe(this->arrivalTime(ctx, i, flowFrag), this->arrivalTime(ctx, j, auxFragment))
                            ),
                            ctx.mkEq(
                                this->scheduledTime(ctx, j, auxFragment),
                                ctx.mkAdd(
                                    this->scheduledTime(ctx, i, flowFrag),
                                    ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
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
                    auxExp2 = ctx.mkAnd((BoolExpr) auxExp2, // Arrived during a time slot predicate
                            ctx.mkImplies(
                                ctx.mkAnd(
                                    ctx.mkLe(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        ctx.mkSub(
                                            ctx.mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, ctx.int_val(j))
                                            ),
                                            ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                        )
                                    ),
                                    ctx.mkGe(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        ctx.mkAdd(
                                            cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                            cycle.cycleStartZ3(ctx, j)
                                        )
                                    )
                                ),
                                ctx.mkEq(
                                    this->scheduledTime(ctx, i, flowFrag),
                                    ctx.mkAdd(
                                        this->arrivalTime(ctx, i, flowFrag),
                                        ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
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
                        auxExp2 = ctx.mkAnd((BoolExpr) auxExp2, // Arrived before slot start constraint
                                ctx.mkImplies(
                                    ctx.mkAnd(
                                        ctx.mkLt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            ctx.mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, j)
                                            )
                                        ),
                                        ctx.mkGe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            cycle.cycleStartZ3(ctx, j)
                                        )
                                    ),
                                    ctx.mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        ctx.mkAdd(
                                            ctx.mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, j)
                                            ),
                                            ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                        )

                                    )
                                )
                            );
                    } else if (index < this->cycle.getNumOfSlots()) {
                        auxExp2 = ctx.mkAnd((BoolExpr) auxExp2,
                                ctx.mkImplies(
                                    ctx.mkAnd(
                                        ctx.mkLt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            ctx.mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, j)
                                            )
                                        ),
                                        ctx.mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            ctx.mkSub(
                                                ctx.mkAdd(
                                                    cycle.cycleStartZ3(ctx, j),
                                                    cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index - 1)),
                                                    cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.int_val(index - 1))
                                                ),
                                                ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                                            )
                                        )
                                    ),
                                    ctx.mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        ctx.mkAdd(
                                            ctx.mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                                cycle.cycleStartZ3(ctx, j)
                                            ),
                                            ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
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
                        auxExp2 = ctx.mkAnd((BoolExpr) auxExp2, // Arrived after slot end constraint
                                ctx.mkImplies(
                                    ctx.mkAnd(
                                        ctx.mkGe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            cycle.cycleStartZ3(ctx, j)
                                        ),
                                        ctx.mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            ctx.mkAdd(
                                                cycle.getCycleDurationZ3(),
                                                cycle.cycleStartZ3(ctx, j)
                                            )
                                        )
                                    ),
                                    ctx.mkLe(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        ctx.mkAdd(
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
                        auxExp2 = ctx.mkAnd((BoolExpr) auxExp2, // Arrived after slot end constraint
                                ctx.mkImplies(
                                    ctx.mkAnd(
                                        ctx.mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            ctx.mkSub(
                                                ctx.mkAdd(
                                                    cycle.slotStartZ3(ctx, flowFrag.getFlowPriority(), indexZ3),
                                                    cycle.slotDurationZ3(ctx, flowFrag.getFlowPriority(), indexZ3),
                                                    cycle.cycleStartZ3(ctx, j)
                                                ),
                                                this->transmissionTimeZ3
                                            )
                                        ),
                                        ctx.mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            ctx.mkAdd(
                                                cycle.cycleStartZ3(ctx, j),
                                                cycle.getCycleDurationZ3()
                                            )
                                        )
                                    ),

                                    ctx.mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        ctx.mkAdd(
                                            ctx.mkAdd(
                                                cycle.slotStartZ3(ctx, flowFrag.getFlowPriority(), ctx.int_val(0)),
                                                cycle.cycleStartZ3(ctx, j + 1)
                                            ),
                                            this->transmissionTimeZ3
                                        )
                                    )
                                )
                        );
                    } else if (j == this->cycleUpperBoundRange - 1 && index == this->cycle.getNumOfSlots() - 1) {
                        auxExp2 = ctx.mkAnd((BoolExpr) auxExp2,
                                ctx.mkImplies(
                                    ctx.mkAnd(
                                        ctx.mkGt(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            ctx.mkSub(
                                                ctx.mkAdd(
                                                    cycle.slotStartZ3(ctx, flowFrag.getFlowPriority(), indexZ3),
                                                    cycle.slotDurationZ3(ctx, flowFrag.getFlowPriority(), indexZ3),
                                                    cycle.cycleStartZ3(ctx, j)
                                                ),
                                                this->transmissionTimeZ3
                                            )
                                        ),
                                        ctx.mkLe(
                                            this->arrivalTime(ctx, i, flowFrag),
                                            ctx.mkAdd(
                                                cycle.cycleStartZ3(ctx, j),
                                                cycle.getCycleDurationZ3()
                                            )
                                        )
                                    ),

                                    ctx.mkEq(
                                        this->scheduledTime(ctx, i, flowFrag),
                                        this->arrivalTime(ctx, i, flowFrag)
                                    )

                                )
                        );
                    }
                    /**/
                }
            }

            //auxExp = ctx.mkOr((BoolExpr)ctx.mkFalse(), (BoolExpr)auxExp2);
            auxExp = ctx.mkOr((BoolExpr)auxExp, (BoolExpr)auxExp2);


            if(exp == nullptr) {
                exp = auxExp;
            } else {
                exp = ctx.mkAnd((BoolExpr) exp, (BoolExpr) auxExp);
            }
            auxExp = ctx.mkFalse();
            auxExp2 = ctx.mkTrue();
        }
    }

    solver.add((BoolExpr)exp);

    auxExp = nullptr;
    exp = ctx.mkFalse();

    //Every packet must be transmitted inside a timeslot (transmit inside a time slot constraint)
    for(int i = 0; i < flowFrag.getNumOfPacketsSent(); i++) {
        for(int j = 0; j < this->cycleUpperBoundRange; j++) {
            for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
                indexZ3 = ctx.int_val(index);
                auxExp = ctx.mkAnd(
                         ctx.mkGe(
                            this->scheduledTime(ctx, i, flowFrag),
                            ctx.mkAdd(
                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                cycle.cycleStartZ3(ctx, j),
                                ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                            )
                          ),
                        ctx.mkLe(
                            this->scheduledTime(ctx, i, flowFrag),
                            ctx.mkAdd(
                                cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), indexZ3),
                                cycle.cycleStartZ3(ctx, j)
                            )
                        )
                );

                exp = ctx.mkOr((BoolExpr) exp, (BoolExpr) auxExp);
            }
        }
        solver.add((BoolExpr) exp);
        exp = ctx.mkFalse();
    }


    /**/
    for(int i = 0; i < flowFrag.getNumOfPacketsSent() - 1; i++) {
        solver.add(
            ctx.mkGe(
                this->scheduledTime(ctx, i + 1, flowFrag),
                ctx.mkAdd(
                        this->scheduledTime(ctx, i, flowFrag),
                        ctx.mkDiv(flowFrag.getPacketSizeZ3(), this->portSpeedZ3)
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
                   ctx.mkImplies(
                       ctx.mkEq(
                           auxFlowFrag.getFlowPriority(),
                           flowFrag.getFlowPriority()
                       ),
                       ctx.mkOr(
                           ctx.mkLe(
                               this->scheduledTime(ctx, i, flowFrag),
                               ctx.mkSub(
                                   this->scheduledTime(ctx, j, auxFlowFrag),
                                   this->transmissionTimeZ3
                               )
                           ),
                           ctx.mkGe(
                               this->scheduledTime(ctx, i, flowFrag),
                               ctx.mkAdd(
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
                   ctx.mkImplies(
                      ctx.mkEq(
                          flowFrag.getFlowPriority(),
                           auxFlowFrag.getFlowPriority()
                       ),
                       ctx.mkOr(
                           ctx.mkLt(
                               this->scheduledTime(ctx, i, flowFrag),
                               this->arrivalTime(ctx, j, auxFlowFrag)
                           ),
                           ctx.mkGt(
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
                    ctx.mkImplies(
                        ctx.mkAnd(
                            ctx.mkLe(
                                this->arrivalTime(ctx, i, flowFrag),
                                this->arrivalTime(ctx, j, auxFlowFrag)
                            ),
                            ctx.mkEq(
                                flowFrag.getFragmentPriorityZ3(),
                                auxFlowFrag.getFragmentPriorityZ3()
                            )
                        ),
                        ctx.mkLe(
                            this->scheduledTime(ctx, i, flowFrag),
                            ctx.mkSub(
                                this->scheduledTime(ctx, j, auxFlowFrag),
                                ctx.mkDiv(auxFlowFrag.getPacketSizeZ3(), this->portSpeedZ3)
                            )
                        )
                    )
                );

                /*
                if(!(flowFrag.equals(auxFlowFrag) && i == j)) {
                    solver.add(
                        ctx.mkNot(
                            ctx.mkEq(
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
            sumOfSlotsDuration = (z3::expr) ctx.mkAdd(cycle.slotDurationZ3(ctx, f.getFragmentPriorityZ3(), ctx.int_val(i)));
        }

        /**/
        for(int i = 1; i <= 8; i++) {
            solver.add(
                ctx.mkImplies(
                    ctx.mkEq(
                        f.getFragmentPriorityZ3(),
                        ctx.int_val(i)
                    ),
                    ctx.mkEq(
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
                firstPartOfImplication = ctx.mkNot(ctx.mkEq(
                                            f.getFragmentPriorityZ3(),
                                            ctx.int_val(i)
                                         ));
            } else {
                firstPartOfImplication = ctx.mkAnd(firstPartOfImplication,
                                         ctx.mkNot(ctx.mkEq(
                                             f.getFragmentPriorityZ3(),
                                             ctx.int_val(i)
                                         )));
            }
        }

        solver.add( // Queue not used constraint
            ctx.mkImplies(
                firstPartOfImplication,
                ctx.mkAnd(
                    ctx.mkEq(slotStart[i-1], ctx.real_val(0)),
                    ctx.mkEq(slotDuration[i-1], ctx.real_val(0))
                )

            )
        );

    }

    for(z3::expr slotDr : slotDuration) {
        if(sumOfPrtTime == nullptr) {
            sumOfPrtTime = slotDr;
        } else {
            sumOfPrtTime = (z3::expr) ctx.mkAdd(sumOfPrtTime, slotDr);
        }
    }


    solver.add( // Best-effort bandwidth reservation constraint
        ctx.mkLe(
            sumOfPrtTime,
            ctx.mkMul(
                ctx.mkSub(
                    ctx.real_val(1),
                    bestEffortPercentZ3
                ),
                this->cycle.getCycleDurationZ3()
            )
        )
    );

    /*
    solver.add(
            ctx.mkLe(
                sumOfPrtTime,
                ctx.mkMul(bestEffortPercentZ3, this->cycle.getCycleDurationZ3())
            )
        );

    solver.add(
        ctx.mkGe(
            bestEffortPercentZ3,
            ctx.mkDiv(sumOfPrtTime, this->cycle.getCycleDurationZ3())
        )
    );
    */

}

}
