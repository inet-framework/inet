#ifndef __INET_Z3_PORT_H
#define __INET_Z3_PORT_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

using namespace z3;

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

    bool useMicroCycles = false;
    bool useHyperCycle = true;

    std::vector<float> listOfPeriods;
    float definedHyperCycleSize = -1;
    float microCycleSize = -1;

    std::string name;
    std::string connectsTo;

    float bestEffortPercent = 0.5f;
    z3::expr bestEffortPercentZ3;

    Cycle cycle;
    std::vector<FlowFragment> flowFragments;
    int packetUpperBoundRange = Network.PACKETUPPERBOUNDRANGE; // Limits the applications of rules to the packets
    int cycleUpperBoundRange = Network.CYCLEUPPERBOUNDRANGE; // Limits the applications of rules to the cycles

    float gbSize;

    float maxPacketSize;
    float timeToTravel;
    float transmissionTime;
    float portSpeed;
    int portNum;

    z3::expr gbSizeZ3; // Size of the guardBand
    z3::expr maxPacketSizeZ3;
    z3::expr timeToTravelZ3;
    z3::expr transmissionTimeZ3;
    z3::expr portSpeedZ3;


    /**
     * [Method]: Port
     * [Usage]: Overloaded constructor of this class. Will start
     * the port with setting properties given as parameters.
     *
     * @param name                  Logical index of the port for z3
     * @param maxPacketSize         Maximum size of a packet that can be transmitted by this port
     * @param timeToTravel          Time to travel on the middle used by this port
     * @param transmissionTime      Time taken to process a packet in this port
     * @param portSpeed             Transmission speed of this port
     * @param gbSize                Size of the guard band
     * @param cycle                 Cycle used by the port
     */
    Port (std::string name,
            int portNum,
            std::string connectsTo,
            float maxPacketSize,
            float timeToTravel,
            float transmissionTime,
            float portSpeed,
            float gbSize,
            Cycle cycle) {
        this->name = name;
        this->portNum = portNum;
        this->connectsTo = connectsTo;
        this->maxPacketSize = maxPacketSize;
        this->timeToTravel = timeToTravel;
        this->transmissionTime = transmissionTime;
        this->portSpeed = portSpeed;
        this->gbSize = gbSize;
        this->cycle = cycle;
        this->flowFragments.clear();
        this->cycle.setPortName(this->name);
    }


    /**
     * [Method]: toZ3
     * [Usage]: After setting all the numeric input values of the class,
     * generates the z3 equivalent of these values and creates any extra
     * variable needed.
     *
     * @param ctx      context variable containing the z3 environment used
     */
    void toZ3(context ctx) {
        this->gbSizeZ3 = ctx.real_val(std::to_string(gbSize));
        this->maxPacketSizeZ3 = ctx.real_val(std::to_string(this->maxPacketSize));
        this->timeToTravelZ3 = ctx.real_val(std::to_string(this->timeToTravel));
        this->transmissionTimeZ3 = ctx.real_val(std::to_string(this->transmissionTime));
        this->portSpeedZ3 = ctx.real_val(std::to_string(portSpeed));
        this->bestEffortPercentZ3 = ctx.real_val(std::to_string(bestEffortPercent));

        if(this->cycle.getFirstCycleStartZ3() == null) {
            this->cycle.toZ3(ctx);
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
    void setUpCycleRules(solver solver, context ctx) {

        for(FlowFragment frag : this->flowFragments) {
            for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
                z3::expr flowPriority = frag.getFragmentPriorityZ3();
                z3::expr indexZ3 = ctx.mkInt(index);

                // A slot will be somewhere between 0 and the end of the cycle minus its duration (Slot in cycle constraint)
                solver.add(ctx.mkGe(cycle.slotStartZ3(ctx, flowPriority, indexZ3), ctx.mkInt(0)));
                solver.add(
                    ctx.mkLe(cycle.slotStartZ3(ctx, flowPriority, indexZ3),
                        ctx.mkSub(
                            cycle.getCycleDurationZ3(),
                            cycle.slotDurationZ3(ctx, flowPriority, indexZ3)
                        )
                    )
                );

                // Every slot duration is greater or equal 0 and lower or equal than the maximum (Slot duration constraint)
                solver.add(ctx.mkGe(cycle.slotDurationZ3(ctx, flowPriority, indexZ3), ctx.mkInt(0)));
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

                    z3::expr auxFlowPriority = auxFrag.getFragmentPriorityZ3();

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
                            cycle.slotStartZ3(ctx, flowPriority, ctx.mkInt(index + 1))
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
                        z3::expr auxIndexZ3 = ctx.mkInt(auxIndex);

                        if(auxFrag.equals(frag)) {
                            continue;
                        }

                        z3::expr auxFlowPriority = auxFrag.getFlowPriority();

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
    void setupTimeSlots(solver solver, context ctx, FlowFragment flowFrag) {
        z3::expr indexZ3;

        // If there is a flow assigned to the slot, slotDuration must be greater than transmission time
        for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
            solver.add(
                ctx.mkGe(
                    cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.mkInt(index+1)),
                    ctx.mkAdd(
                            cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.mkInt(index)),
                            cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.mkInt(index))
                    )
                )
            );
        }

        for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
            indexZ3 = ctx.mkInt(index);

            // solver.add(ctx.mkGe(cycle.slotDurationZ3(ctx, flowFrag.getFlowPriority(), indexZ3), this->transmissionTimeZ3));

            // Every flow must have a priority (Priority assignment constraint)
            solver.add(ctx.mkGe(flowFrag.getFragmentPriorityZ3(), ctx.mkInt(0)));
            solver.add(ctx.mkLt(flowFrag.getFragmentPriorityZ3(), ctx.mkInt(this->cycle.getNumOfPrts())));

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
    void setupDevPacketTimes(solver solver, context ctx, FlowFragment flowFrag) {

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

        z3::expr indexZ3 = null;
        Expr auxExp = null;
        Expr auxExp2 = ctx.mkTrue();
        Expr exp = null;

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
                    if (auxExp == null) {
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
                        indexZ3 = ctx.mkInt(index);

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
                                                    cycle.cycleStartZ3(ctx, ctx.mkInt(j))
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
                                                        cycle.slotStartZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.mkInt(index - 1)),
                                                        cycle.slotDurationZ3(ctx, flowFrag.getFragmentPriorityZ3(), ctx.mkInt(index - 1))
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
                                                    cycle.slotStartZ3(ctx, flowFrag.getFlowPriority(), ctx.mkInt(0)),
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


                if(exp == null) {
                    exp = auxExp;
                } else {
                    exp = ctx.mkAnd((BoolExpr) exp, (BoolExpr) auxExp);
                }
                auxExp = ctx.mkFalse();
                auxExp2 = ctx.mkTrue();
            }
        }

        solver.add((BoolExpr)exp);

        auxExp = null;
        exp = ctx.mkFalse();

        //Every packet must be transmitted inside a timeslot (transmit inside a time slot constraint)
        for(int i = 0; i < flowFrag.getNumOfPacketsSent(); i++) {
            for(int j = 0; j < this->cycleUpperBoundRange; j++) {
                for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
                    indexZ3 = ctx.mkInt(index);
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


    /**
     * [Method]: setupBestEffort
     * [Usage]: Use in order to enable the best effort traffic reservation
     * constraint.
     *
     * @param solver    solver object
     * @param ctx        context object for the solver
     */
    void setupBestEffort(solver solver, context ctx) {
        z3::expr []slotStart = new z3::expr[8];
        z3::expr []slotDuration = new z3::expr[8];
        // z3::expr guardBandTime = null;

        BoolExpr firstPartOfImplication = null;
        z3::expr sumOfPrtTime = null;

        for(int i = 0; i < 8; i++) {
            slotStart[i] = ctx.real_const((this->name + std::string("SlotStart") + i).c_str());
            slotDuration[i] = ctx.real_const((this->name + std::string("SlotDuration") + i).c_str());
        }

        for(FlowFragment f : this->flowFragments) {
            // z3::expr sumOfSlotsStart = ctx.real_val(0);
            z3::expr sumOfSlotsDuration = ctx.real_val(0);

            for(int i = 0; i < this->cycle.getNumOfSlots(); i++) {
                sumOfSlotsDuration = (z3::expr) ctx.mkAdd(cycle.slotDurationZ3(ctx, f.getFragmentPriorityZ3(), ctx.mkInt(i)));
            }

            /**/
            for(int i = 1; i <= 8; i++) {
                solver.add(
                    ctx.mkImplies(
                        ctx.mkEq(
                            f.getFragmentPriorityZ3(),
                            ctx.mkInt(i)
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
            firstPartOfImplication = null;

            for(FlowFragment f : this->flowFragments) {
                if(firstPartOfImplication == null) {
                    firstPartOfImplication = ctx.mkNot(ctx.mkEq(
                                                f.getFragmentPriorityZ3(),
                                                ctx.mkInt(i)
                                             ));
                } else {
                    firstPartOfImplication = ctx.mkAnd(firstPartOfImplication,
                                             ctx.mkNot(ctx.mkEq(
                                                 f.getFragmentPriorityZ3(),
                                                 ctx.mkInt(i)
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
            if(sumOfPrtTime == null) {
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

    /**
     * [Method]: gcd
     * [Usage]: Method used to obtain the greatest common
     * divisor of two values.
     *
     * @param a        First value
     * @param b        Second value
     * @return        Greatest common divisor or the two previous parameters
     */
    static float gcd(float a, float b) {
        if (a == 0) {
            return b;
        }

        return gcd(b % a, a);
    }


    /**
     * [Method]: findGCD
     * [Usage]: Retrieves the value of the greatest common divisor
     * of all the values in an array.
     *
     * @param arr    Array of float values
     * @return        Greatest common divisor of all values of arr
     */
    static float findGCD(std::vector<float> arr) {
        float gdc = arr.get(0);
        for (int i = 1; i < arr.size(); i++) {
            gdc = gcd(arr.get(i), gdc);
        }

        return gdc;
    }


    /**
     * [Method]: findLCM
     * [Usage]: Retrieves the least common multiple of all values in
     * an array.
     *
     * @param arr         Array of float values
     * @return            Least common multiple of all values of arr
     */
    static float findLCM(std::vector<float> arr) {

        float n = arr.size();

        float max_num = 0;
        for (int i = 0; i < n; i++) {
            if (max_num < arr.get(i)) {
                max_num = arr.get(i);
            }
        }

        float res = 1;

        float x = 2;
        while (x <= max_num) {
            Vector<int> indexes = new Vector<>();
            for (int j = 0; j < n; j++) {
                if (arr.get(j) % x == 0) {
                    indexes.add(indexes.size(), j);
                }
            }
            if (indexes.size() >= 2) {
                for (int j = 0; j < indexes.size(); j++) {
                    arr.set(indexes.get(j), arr.get(indexes.get(j)) / x);
                }

                res = res * x;
            } else {
                x++;
            }
        }

        for (int i = 0; i < n; i++) {
            res = res * arr.get(i);
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
    void setUpHyperCycle(solver solver, context ctx) {
        int numOfPacketsScheduled = 0;

        /*
        for(FlowFragment flowFrag : this->flowFragments) {
            System.out.println(flowFrag.getStartDevice());
            listOfPeriods.add(flowFrag.getStartDevice().getPacketPeriodicity());
        }
        for(float periodicity : this->listOfPeriods) {
            System.out.println(std::string("Periodicidade: ") + periodicity);
        }
        */

        float hyperCycleSize = findLCM((std::vector<float>) listOfPeriods.clone());

        this->definedHyperCycleSize = hyperCycleSize;
        this->cycle.setCycleDuration(hyperCycleSize);

        this->cycleUpperBoundRange = 1;

        /*
        for(FlowFragment flowFrag : this->flowFragments) {
            flowFrag.setNumOfPacketsSent((int) (hyperCycleSize/flowFrag.getStartDevice().getPacketPeriodicity()));
            System.out.println(std::string("Frag num packets: ") + flowFrag.getNumOfPacketsSent());
            numOfPacketsScheduled += (int) (hyperCycleSize/flowFrag.getStartDevice().getPacketPeriodicity());
        }
        */

        for(float periodicity : this->listOfPeriods) {
            numOfPacketsScheduled += (int) (hyperCycleSize/periodicity);
        }

        // System.out.println(std::string("Num of Cycles: ") + this->cycleUpperBoundRange);

        this->cycle.setNumOfSlots(numOfPacketsScheduled);

        // In order to use the value cycle time obtained, we must override the minimum and maximum cycle times
        this->cycle.setUpperBoundCycleTime(hyperCycleSize + 1);
        this->cycle.setLowerBoundCycleTime(hyperCycleSize - 1);

    }

    /**
     * [Method]: setUpMicroCycles
     * [Usage]: Set up the cycle duration and number of packets, cycles and slots
     * to be scheduled according to the micro cycle approach.
     *
     * @param solver    solver object
     * @param ctx        context object for the solver
     */
    void setUpMicroCycles(solver solver, context ctx) {

        /*
        for(FlowFragment flowFrag : this->flowFragments) {
            System.out.println(flowFrag.getStartDevice());
            listOfPeriods.add(flowFrag.getStartDevice().getPacketPeriodicity());
        }
        */

        this->microCycleSize = findGCD((std::vector<float>) listOfPeriods.clone());
        float hyperCycleSize = findLCM((std::vector<float>) listOfPeriods.clone());

        this->definedHyperCycleSize = hyperCycleSize;

        this->cycleUpperBoundRange = (int) (hyperCycleSize/microCycleSize);

        /*
        for(FlowFragment flowFrag : this->flowFragments) {
            flowFrag.setNumOfPacketsSent((int) (hyperCycleSize/flowFrag.getStartDevice().getPacketPeriodicity()));
            System.out.println(std::string("Frag num packets: ") + flowFrag.getNumOfPacketsSent());
        }
        System.out.println(std::string("Num of Cycles: ") + this->cycleUpperBoundRange);
          */

        // In order to use the value cycle time obtained, we must override the minimum and maximum cycle times
        this->cycle.setUpperBoundCycleTime(microCycleSize + 1);
        this->cycle.setLowerBoundCycleTime(microCycleSize - 1);
    }


    /**
     * [Method]: bindTimeSlots
     * [Usage]: IN DEVELOPMENT - Bind timeslots to a fixed name instead
     * of a variable.
     *
     * @param solver    solver object
     * @param ctx        context object for the solver
     */
    void bindTimeSlots(solver solver, context ctx) {

        // Ideia = se a prioridade de um flow e' igual a um numero,
        // ctx.mkeq nele com o slot the cycle (getSlotS/D(prt, slotnum))
        for(FlowFragment frag : this->flowFragments) {

            for(int i = 0; i < this->cycle.getNumOfPrts(); i++) {
                for(int j = 0; j < this->cycle.getNumOfSlots(); j++) {
                    /*
                    solver.add(
                        ctx.mkITE(
                            ctx.mkAnd(
                                ctx.mkEq(frag.getFlowPriority(), ctx.mkInt(i)),
                                ctx.mkEq(frag.get, ctx.mkInt(j))
                            )
                                ,
                                )
                    );
                    */
                }
            }
        }

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
    void setUpCycle(solver solver, context ctx) {

        this->cycle.toZ3(ctx);

        // System.out.println(std::string("On port: " + this->name + std::string(" with: ") + this->listOfPeriods.size() + " fragments"));

        if(this->listOfPeriods.size() < 1) {
            return;
        }


        if(useMicroCycles && this->listOfPeriods.size() > 0) {
            setUpMicroCycles(solver, ctx);

            solver.add(
                ctx.mkEq(this->cycle.getCycleDurationZ3(), ctx.real_val(std::to_string(this->microCycleSize)))
            );
        } else if (useHyperCycle && this->listOfPeriods.size() > 0) {
            setUpHyperCycle(solver, ctx);

            solver.add(
                ctx.mkEq(this->cycle.getCycleDurationZ3(), ctx.real_val(std::to_string(this->definedHyperCycleSize)))
            );
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
    void zeroOutNonUsedSlots(solver solver, context ctx) {
        BoolExpr exp1;
        BoolExpr exp2;
        z3::expr indexZ3;

        for(int prtIndex = 0; prtIndex < this->cycle.getNumOfPrts(); prtIndex++) {
            for(FlowFragment frag : this->flowFragments) {
                for(int slotIndex = 0; slotIndex < this->cycle.getNumOfSlots(); slotIndex++) {
                    solver.add(
                        ctx.mkImplies(
                            ctx.mkEq(frag.getFragmentPriorityZ3(), ctx.mkInt(prtIndex)),
                            ctx.mkAnd(
                                ctx.mkEq(
                                    cycle.slotStartZ3(ctx, frag.getFragmentPriorityZ3(), ctx.mkInt(slotIndex)),
                                    cycle.slotStartZ3(ctx, ctx.mkInt(prtIndex), ctx.mkInt(slotIndex))
                                ),
                                ctx.mkEq(
                                    cycle.slotDurationZ3(ctx, frag.getFragmentPriorityZ3(), ctx.mkInt(slotIndex)),
                                    cycle.slotDurationZ3(ctx, ctx.mkInt(prtIndex), ctx.mkInt(slotIndex))
                                )
                            )
                        )
                    );
                }
            }
        }


        for(int prtIndex = 0; prtIndex < this->cycle.getNumOfPrts(); prtIndex++) {
            for(int cycleNum = 0; cycleNum < this->cycleUpperBoundRange; cycleNum++) {
                for(int indexNum = 0; indexNum < this->cycle.getNumOfSlots(); indexNum++) {
                    indexZ3 = ctx.mkInt(indexNum);
                    exp1 = ctx.mkTrue();
                    for(FlowFragment frag : this->flowFragments) {
                        for(int packetNum = 0; packetNum < frag.getNumOfPacketsSent(); packetNum++) {
                            exp1 = ctx.mkAnd(
                                    exp1,
                                    ctx.mkAnd(
                                        ctx.mkNot(
                                            ctx.mkAnd(
                                                ctx.mkGe(
                                                    this->scheduledTime(ctx, packetNum, frag),
                                                    ctx.mkAdd(
                                                        cycle.slotStartZ3(ctx, ctx.mkInt(prtIndex), indexZ3),
                                                        cycle.cycleStartZ3(ctx, ctx.mkInt(cycleNum))
                                                    )
                                                ),
                                                ctx.mkLe(
                                                    this->scheduledTime(ctx, packetNum, frag),
                                                    ctx.mkAdd(
                                                        cycle.slotStartZ3(ctx, ctx.mkInt(prtIndex), indexZ3),
                                                        cycle.slotDurationZ3(ctx, ctx.mkInt(prtIndex), indexZ3),
                                                        cycle.cycleStartZ3(ctx, ctx.mkInt(cycleNum))
                                                    )
                                                )
                                            )
                                        ),
                                        ctx.mkEq(ctx.mkInt(prtIndex), frag.getFragmentPriorityZ3())
                                    )
                            );
                        }

                    }

                    solver.add(
                        ctx.mkImplies(
                            exp1,
                            ctx.mkEq(cycle.slotDurationZ3(ctx, ctx.mkInt(prtIndex), indexZ3), ctx.mkInt(0))
                        )
                    );
                }

            }

        }



    }


    /**
     * [Method]: setupSchedulingRules
     * [Usage]: Calls the set of functions that will set the z3 rules
     * regarding the cycles, time slots, priorities and timing of packets.
     *
     * @param solver        z3 solver object used to discover the variables' values
     * @param ctx           z3 context which specify the environment of constants, functions and variables
     */
    void setupSchedulingRules(solver solver, context ctx) {

        if (this->flowFragments.size() == 0) {
            solver.add(ctx.mkEq(
                ctx.real_val(std::to_string(0)),
                this->cycle.getCycleDurationZ3()
            ));
            //    solver.add(ctx.mkEq(
            //    ctx.real_val(std::to_string(0)),
            //    this->cycle.getFirstCycleStartZ3()
            // ));

            return;
        }

        /*
        if(useMicroCycles && this->flowFragments.size() > 0) {
            solver.add(ctx.mkEq(
                ctx.real_val(std::to_string(this->microCycleSize)),
                this->cycle.getCycleDurationZ3()
            ));
        } else if (useHyperCycle && this->flowFragments.size() > 0) {
            solver.add(ctx.mkEq(
                ctx.real_val(std::to_string(this->definedHyperCycleSize)),
                this->cycle.getCycleDurationZ3()
            ));
        } else {
            for(FlowFragment flowFrag : this->flowFragments) {
                flowFrag.setNumOfPacketsSent(this->packetUpperBoundRange);
            }
        }
        /**/


        setUpCycleRules(solver, ctx);
        zeroOutNonUsedSlots(solver, ctx);

        /*
         * Differently from setUpCycleRules, setupTimeSlots and setupDevPacketTimes
         * need to be called multiple times since there will be a completely new set
         * of rules for each flow fragment on this port.
         */


        for(FlowFragment flowFrag : this->flowFragments) {
            setupTimeSlots(solver, ctx, flowFrag);
            setupDevPacketTimes(solver, ctx, flowFrag);
        }

        /*
        if(flowFragments.size() > 0) {
            setupBestEffort(solver, ctx);
        }
        /**/

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

    z3::expr departureTime(context ctx, z3::expr index, FlowFragment flowFrag){

        // If the index is 0, then its the first departure time, else add index * periodicity
        return this->departureTime(ctx, (int.parseInt(index.toString())), flowFrag);

        /*
        return (z3::expr) ctx.mkITE(
               ctx.mkGe(index, ctx.mkInt(1)),
               ctx.mkAdd(
                       flowFrag.getDepartureTimeZ3(int.parseInt(index.toString())),
                       ctx.mkMul(flowFrag.getPacketPeriodicity(), index)
                       ),
               flowFrag.getDepartureTimeZ3(int.parseInt(index.toString())));
        /**/
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
    z3::expr departureTime(context ctx, int auxIndex, FlowFragment flowFrag){
        z3::expr index = null;
        z3::expr departureTime;
        int cycleNum = 0;


        if(auxIndex + 1 > flowFrag.getNumOfPacketsSent()) {
            cycleNum = (auxIndex - (auxIndex % flowFrag.getNumOfPacketsSent()))/flowFrag.getNumOfPacketsSent();

            auxIndex = (auxIndex % flowFrag.getNumOfPacketsSent());

            departureTime = (z3::expr)
                    ctx.mkAdd(
                        flowFrag.getDepartureTimeZ3(auxIndex),
                        ctx.mkMul(ctx.real_val(cycleNum), this->cycle.getCycleDurationZ3())
                    );


            return departureTime;
        }

        departureTime = flowFrag.getDepartureTimeZ3(auxIndex);

        return departureTime;

    }


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
    z3::expr arrivalTime(context ctx, z3::expr index, FlowFragment flowFrag){


       // The arrival time of this index from the given flow fragment is
       // equal to the its departure time + time to travel

       return (z3::expr) ctx.mkAdd(
                      departureTime(ctx, index, flowFrag),
                      timeToTravelZ3
                      );
    }
    /**/

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
    z3::expr arrivalTime(context ctx, int auxIndex, FlowFragment flowFrag){
        z3::expr index = ctx.mkInt(auxIndex);

        return (z3::expr) ctx.mkAdd( // Arrival time value constraint
                        departureTime(ctx, index, flowFrag),
                        timeToTravelZ3
                        );
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
    z3::expr scheduledTime(context ctx, z3::expr index, FlowFragment flowFrag){
        z3::expr devT3 = ctx.real_const((flowFrag.getName() + std::string("ScheduledTime") + index.toString()).c_str());

        return (z3::expr) devT3;
    }
    /**/

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
    z3::expr scheduledTime(context ctx, int auxIndex, FlowFragment flowFrag){
        z3::expr index = null;
        z3::expr scheduledTime;
        int cycleNum = 0;

        if(auxIndex + 1 > flowFrag.getNumOfPacketsSent()) {
            cycleNum = (auxIndex - (auxIndex % flowFrag.getNumOfPacketsSent()))/flowFrag.getNumOfPacketsSent();

            auxIndex = (auxIndex % flowFrag.getNumOfPacketsSent());
            index = ctx.mkInt(auxIndex);

            scheduledTime = (z3::expr)
                    ctx.mkAdd(
                        ctx.real_const(flowFrag.getName() + std::string("ScheduledTime") + index.toString()),
                        ctx.mkMul(ctx.real_val(cycleNum), this->cycle.getCycleDurationZ3())
                    );


            return scheduledTime;
        }

        index = ctx.mkInt(auxIndex);


        scheduledTime = ctx.real_const((flowFrag.getName() + std::string("ScheduledTime") + index.toString()).c_str());

        return (z3::expr) scheduledTime;
    }


    /**
     * [Method]: checkIfAutomatedApplicationPeriod
     * [Usage]: Returns true if the port uses an automated application period
     * methodology.
     *
     * @return boolean value. True if automated application period methodology is used, false elsewhise
     */
    bool checkIfAutomatedApplicationPeriod() {
        if(this->useHyperCycle || this->useMicroCycles)
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
    void loadZ3(context ctx, solver solver) {

        this->cycle.loadZ3(ctx, solver);

        for(FlowFragment frag : this->flowFragments) {

            frag.setFragmentPriorityZ3(
                ctx.mkInt(
                    frag.getFragmentPriority()
                )
            );

            /*
            solver.add(
                ctx.mkEq(
                    frag.getFragmentPriorityZ3(),
                    ctx.mkInt(frag.getFragmentPriority())
                )
            );
            */

            for(int index = 0; index < this->cycle.getNumOfSlots(); index++) {
                solver.add(
                    ctx.mkEq(
                        this->cycle.slotDurationZ3(ctx, frag.getFragmentPriorityZ3(), ctx.mkInt(index)),
                        ctx.real_val(
                            std::to_string(
                                this->cycle.getSlotDuration(frag.getFragmentPriority(), index)
                            )
                        )
                    )
                );

                solver.add(
                    ctx.mkEq(
                        this->cycle.slotStartZ3(ctx, frag.getFragmentPriorityZ3(), ctx.mkInt(index)),
                        ctx.real_val(
                            std::to_string(
                                this->cycle.getSlotStart(frag.getFragmentPriority(), index)
                            )
                        )
                    )
                );

            }

            for(int i = 0; i < frag.getNumOfPacketsSent(); i++) {
                /*
                solver.add(
                    ctx.mkEq(
                        this->departureTime(ctx, i, frag),
                        ctx.real_val(std::to_string(frag.getDepartureTime(i)))
                    )
                );
                */
                if (i > 0)
                    frag.addDepartureTimeZ3(ctx.real_val(std::to_string(frag.getDepartureTime(i))));

                solver.add(
                    ctx.mkEq(
                        this->arrivalTime(ctx, i, frag),
                        ctx.real_val(std::to_string(frag.getArrivalTime(i)))
                    )
                );

                solver.add(
                    ctx.mkEq(
                        this->scheduledTime(ctx, i, frag),
                        ctx.real_val(std::to_string(frag.getScheduledTime(i)))
                    )
                );

            }

        }


    }


    /*
     * GETTERS AND SETTERS:
     */

    Cycle getCycle() {
        return cycle;
    }

    void setCycle(Cycle cycle) {
        this->cycle = cycle;
    }

    std::vector<FlowFragment> getDeviceList() {
        return flowFragments;
    }

    void setDeviceList(std::vector<FlowFragment> flowFragments) {
        this->flowFragments = flowFragments;
    }

    void addToFragmentList(FlowFragment flowFrag) {
        this->flowFragments.add(flowFrag);
    }

    float getGbSize() {
        return gbSize;
    }

    void setGbSize(float gbSize) {
        this->gbSize = gbSize;
    }

    z3::expr getGbSizeZ3() {
        return gbSizeZ3;
    }

    void setGbSizeZ3(z3::expr gbSizeZ3) {
        this->gbSizeZ3 = gbSizeZ3;
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

    void addToListOfPeriods(float period) {
        this->listOfPeriods.add(period);
    }

    std::vector<float> getListOfPeriods() {
        return listOfPeriods;
    }

     void setListOfPeriods(std::vector<float> listOfPeriods) {
        this->listOfPeriods = listOfPeriods;
    }

    int getCycleUpperBoundRange() {
        return cycleUpperBoundRange;
    }

     void setCycleUpperBoundRange(int cycleUpperBoundRange) {
        this->cycleUpperBoundRange = cycleUpperBoundRange;
    }

     float getDefinedHyperCycleSize() {
        return definedHyperCycleSize;
    }

     void setDefinedHyperCycleSize(float definedHyperCycleSize) {
        this->definedHyperCycleSize = definedHyperCycleSize;
    }

     int getPortNum() {
        return portNum;
    }

     void setPortNum(int portNum) {
        this->portNum = portNum;
    }

    std::vector<FlowFragment> getFlowFragments() {
        return flowFragments;
    }

    void setFlowFragments(std::vector<FlowFragment> flowFragments) {
        this->flowFragments = flowFragments;
    }


    /***************************************************
     *
     * The methods bellow are not completely operational
     * and are currently not used in the project. Might be
     * useful in future iterations of TSNsched.
     *
     ***************************************************/

    z3::expr getCycleOfScheduledTime(context ctx, FlowFragment f, int index) {
        z3::expr cycleIndex = null;

        z3::expr relativeST = (z3::expr) ctx.mkSub(
                this->scheduledTime(ctx, index, f),
                this->cycle.getFirstCycleStartZ3()
        );

        cycleIndex = ctx.mkReal2Int(
                        (z3::expr) ctx.mkDiv(relativeST, this->cycle.getCycleDurationZ3())
                     );

        return cycleIndex;
    }


    z3::expr getCycleOfTime(context ctx, z3::expr time) {
        z3::expr cycleIndex = null;

        z3::expr relativeST = (z3::expr) ctx.mkSub(
                time,
                this->cycle.getFirstCycleStartZ3()
        );

        cycleIndex = ctx.mkReal2Int(
                        (z3::expr) ctx.mkDiv(relativeST, this->cycle.getCycleDurationZ3())
                     );

        return cycleIndex;
    }

    z3::expr getScheduledTimeOfPreviousPacket(context ctx, FlowFragment f, int index) {
        z3::expr prevPacketST = ctx.real_val(0);

        for(FlowFragment auxFrag : this->flowFragments) {

            for(int i = 0; i < f.getNumOfPacketsSent(); i++) {
                prevPacketST = (z3::expr)
                        ctx.mkITE(
                            ctx.mkAnd(
                                ctx.mkEq(auxFrag.getFragmentPriorityZ3(), f.getFragmentPriorityZ3()),
                                ctx.mkLt(
                                    this->scheduledTime(ctx, i, auxFrag),
                                    this->scheduledTime(ctx, index, f)
                                ),
                                ctx.mkGt(
                                    this->scheduledTime(ctx, i, auxFrag),
                                    prevPacketST
                                )
                            ),
                            this->scheduledTime(ctx, i, auxFrag),
                            prevPacketST

                        );

            }

        }

        return (z3::expr)
                ctx.mkITE(
                    ctx.mkEq(prevPacketST, ctx.real_val(0)),
                    this->scheduledTime(ctx, index, f),
                    prevPacketST
                );
    }

};

}

#endif

