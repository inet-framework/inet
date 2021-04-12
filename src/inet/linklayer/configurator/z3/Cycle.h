#ifndef __INET_Z3_CYCLE_H
#define __INET_Z3_CYCLE_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

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
    String portName = "";
	static int instanceCounter = 0;
    float upperBoundCycleTime;
    float lowerBoundCycleTime;
    float firstCycleStart;
    float maximumSlotDuration;

    float cycleDuration;
    float cycleStart;

    ArrayList<ArrayList<RealExpr>> slotStartZ3 = new ArrayList<ArrayList<RealExpr>>();
    ArrayList<ArrayList<RealExpr>> slotDurationZ3 = new ArrayList<ArrayList<RealExpr>>();

    ArrayList<Integer> slotsUsed = new ArrayList<Integer>();
    ArrayList<ArrayList<Float>> slotStart = new ArrayList<ArrayList<Float>>();
    ArrayList<ArrayList<Float>> slotDuration = new ArrayList<ArrayList<Float>>();

    RealExpr cycleDurationZ3;
    RealExpr firstCycleStartZ3;
    RealExpr maximumSlotDurationZ3;
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
    public Cycle(float upperBoundCycleTime,
                 float lowerBoundCycleTime,
                 float maximumSlotDuration) {
        this.upperBoundCycleTime = upperBoundCycleTime;
        this.lowerBoundCycleTime = lowerBoundCycleTime;
        this.maximumSlotDuration = maximumSlotDuration;
        this.firstCycleStart = 0;
    }


    public Cycle(float maximumSlotDuration) {
       this.maximumSlotDuration = maximumSlotDuration;
       this.firstCycleStart = 0;
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
    public Cycle(float upperBoundCycleTime,
                 float lowerBoundCycleTime,
                 float firstCycleStart,
                 float maximumSlotDuration) {
        this.upperBoundCycleTime = upperBoundCycleTime;
        this.lowerBoundCycleTime = lowerBoundCycleTime;
        this.firstCycleStart = firstCycleStart;
        this.maximumSlotDuration = maximumSlotDuration;
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
    public Cycle(RealExpr upperBoundCycleTimeZ3,
                 RealExpr lowerBoundCycleTimeZ3,
                 RealExpr firstCycleStartZ3,
                 RealExpr maximumSlotDurationZ3) {
        // this.upperBoundCycleTimeZ3 = upperBoundCycleTimeZ3;
        // this.lowerBoundCycleTimeZ3 = lowerBoundCycleTimeZ3;
        this.firstCycleStartZ3 = firstCycleStartZ3;
        //this.guardBandSizeZ3 = guardBandSize;
        this.maximumSlotDurationZ3 = maximumSlotDurationZ3;
    }

    /**
     * [Method]: toZ3
     * [Usage]: After setting all the numeric input values of the class,
     * generates the z3 equivalent of these values and creates any extra
     * variable needed.
     *
     * @param ctx      Context variable containing the z3 environment used
     */
    public void toZ3(Context ctx) {
        instanceCounter++;

        this.cycleDurationZ3 = ctx.mkRealConst("cycle" + Integer.toString(instanceCounter) + "Duration");
        this.firstCycleStartZ3 = ctx.mkRealConst("cycle" + Integer.toString(instanceCounter) + "Start");
        // this.firstCycleStartZ3 = ctx.mkReal(Float.toString(0));
        // this.firstCycleStartZ3 = ctx.mkReal(Float.toString(firstCycleStart));
        this.maximumSlotDurationZ3 = ctx.mkReal(Float.toString(maximumSlotDuration));

        this.slotStartZ3 = new ArrayList<ArrayList<RealExpr>>();
        this.slotDurationZ3 = new ArrayList<ArrayList<RealExpr>>();

        for(int i = 0; i < this.numOfPrts; i++) {
        	this.slotStartZ3.add(new ArrayList<RealExpr>());
        	this.slotDurationZ3.add(new ArrayList<RealExpr>());
        	for(int j = 0; j < this.numOfSlots; j++) {
        		this.slotStartZ3.get(i).add(ctx.mkRealConst("cycleOfPort" + this.portName + "prt" + (i+1) + "slot" + (j+1)));
        	}
        }

    }


    /**
     * [Method]: cycleStartZ3
     * [Usage]: Returns the time of the start of a cycle
     * specified by its index. The index is given as a z3
     * variable
     *
     * @param ctx       Context containing the z3 environment
     * @param index     Index of the desired cycle
     * @return          Z3 variable containing the cycle start time
     */
    public RealExpr cycleStartZ3(Context ctx, IntExpr index){
        return (RealExpr) ctx.mkITE(
                ctx.mkGe(index, ctx.mkInt(1)),
                ctx.mkAdd(
                        firstCycleStartZ3,
                        ctx.mkMul(cycleDurationZ3, index)
                        ),
                firstCycleStartZ3);

     }

    /**
     * [Method]: cycleStartZ3
     * [Usage]: Returns the time of the start of a cycle
     * specified by its index. The index is given as integer
     *
     * @param ctx       Context containing the z3 environment
     * @param index     Index of the desired cycle
     * @return          Z3 variable containing the cycle start time
     */
    public RealExpr cycleStartZ3(Context ctx, int auxIndex){
        IntExpr index = ctx.mkInt(auxIndex);

        return (RealExpr) ctx.mkITE(
                ctx.mkGe(index, ctx.mkInt(1)),
                ctx.mkAdd(
                        firstCycleStartZ3,
                        ctx.mkMul(cycleDurationZ3, index)
                        ),
                firstCycleStartZ3);

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
    public void addSlotUsed(int prt, ArrayList<Float> sStart, ArrayList<Float> sDuration) {

        if(!this.slotsUsed.contains(prt)) {
            this.slotsUsed.add(prt);
            this.slotStart.add(sStart);
            this.slotDuration.add(sDuration);
        }

    }


    /**
     * [Method]: loadZ3
     * [Usage]: From the loaded primitive values of the class
     * obtained in the deserialization process, initialize the
     * z3 variables.
     *
     * @param ctx		Context object of z3
     * @param solver	Solver object to add constraints
     */
    public void loadZ3(Context ctx, Solver solver) {
    	// maximumSlotDurationZ3 already started on toZ3;

    	solver.add(
			ctx.mkEq(
				this.cycleDurationZ3,
				ctx.mkReal(Float.toString(this.cycleDuration))
			)
		);

    	solver.add(
			ctx.mkEq(
				this.firstCycleStartZ3,
				ctx.mkReal(Float.toString(this.firstCycleStart))
			)
		);


    	for(int prt : this.getSlotsUsed()) {

    		// Where are the slot duration per priority instantiated? Must do it before loading

    		for(int slotIndex = 0; slotIndex < this.numOfSlots; slotIndex++) {
    			solver.add(
					ctx.mkEq(
						this.slotStartZ3.get(prt).get(slotIndex),
						ctx.mkReal(Float.toString(this.slotStart.get(this.slotsUsed.indexOf(prt)).get(slotIndex)))
					)
				);
    		}

    		/*
    		for(int slotIndex = 0; slotIndex < this.numOfSlots; slotIndex++) {
    			solver.add(
					ctx.mkEq(
						this.slotDurationZ3.get(prt).get(slotIndex),
						ctx.mkReal(Float.toString(this.slotDuration.get(prt).get(slotIndex)))
					)
				);
    		}
    		*/
    	}



    }

    /*
     *  GETTERS AND SETTERS
     */


    public float getUpperBoundCycleTime() {
        return upperBoundCycleTime;
    }

    public void setUpperBoundCycleTime(float upperBoundCycleTime) {
        this.upperBoundCycleTime = upperBoundCycleTime;
    }

    public float getLowerBoundCycleTime() {
        return lowerBoundCycleTime;
    }

    public void setLowerBoundCycleTime(float lowerBoundCycleTime) {
        this.lowerBoundCycleTime = lowerBoundCycleTime;
    }

    public float getCycleDuration() {
        return cycleDuration;
    }

    public float getCycleStart() {
        return cycleStart;
    }


    public void setCycleStart(float cycleStart) {
        this.cycleStart = cycleStart;
    }

    public void setCycleDuration(float cycleDuration) {
        this.cycleDuration = cycleDuration;
    }

    public float getFirstCycleStart() {
        return firstCycleStart;
    }

    public void setFirstCycleStart(float firstCycleStart) {
        this.firstCycleStart = firstCycleStart;
    }

    public float getMaximumSlotDuration() {
        return maximumSlotDuration;
    }

    public void setMaximumSlotDuration(float maximumSlotDuration) {
        this.maximumSlotDuration = maximumSlotDuration;
    }


    public RealExpr slotStartZ3(Context ctx, IntExpr prt, IntExpr index) {
        return ctx.mkRealConst("priority" + prt.toString() + "slot" + index.toString() + "Start");
    }

    public RealExpr slotStartZ3(Context ctx, int auxPrt, int auxIndex) {
        IntExpr index = ctx.mkInt(auxIndex);
        IntExpr prt = ctx.mkInt(auxPrt);
        return ctx.mkRealConst("priority" + prt.toString() + "slot" + index.toString() + "Start");
    }


    public RealExpr slotDurationZ3(Context ctx, IntExpr prt, IntExpr index) {
        return ctx.mkRealConst("priority" + prt.toString() + "slot" + index.toString() + "Duration");
    }

    public RealExpr slotDurationZ3(Context ctx, int auxPrt, int auxIndex) {
        IntExpr index = ctx.mkInt(auxIndex);
        IntExpr prt = ctx.mkInt(auxPrt);
        return ctx.mkRealConst("priority" + prt.toString() + "slot" + index.toString() + "Duration");
    }

    public RealExpr getCycleDurationZ3() {
        return cycleDurationZ3;
    }

    public void setCycleDurationZ3(RealExpr cycleDuration) {
        this.cycleDurationZ3 = cycleDuration;
    }

    public RealExpr getFirstCycleStartZ3() {
        return firstCycleStartZ3;
    }

    public void setFirstCycleStartZ3(RealExpr firstCycleStart) {
        this.firstCycleStartZ3 = firstCycleStart;
    }

    public int getNumOfPrts() {
        return this.numOfPrts;
    }

    public void setNumOfPrts(int numOfPrts) {
        this.numOfPrts = numOfPrts;
    }

    public int getNumOfSlots() {
		return numOfSlots;
	}

	public void setNumOfSlots(int numOfSlots) {
		this.numOfSlots = numOfSlots;
	}

    public RealExpr getMaximumSlotDurationZ3() {
        return maximumSlotDurationZ3;
    }

    public void setMaximumSlotDurationZ3(RealExpr maximumSlotDuration) {
        this.maximumSlotDurationZ3 = maximumSlotDuration;
    }

    public ArrayList<Integer> getSlotsUsed (){
        return this.slotsUsed;
    }

    public ArrayList<ArrayList<Float>> getSlotDuration() {
        return slotDuration;
    }

    public float getSlotStart(int prt, int index) {
        return this.slotStart.get(this.slotsUsed.indexOf(prt)).get(index);
    }

    public float getSlotDuration(int prt, int index) {
        return this.slotDuration.get(this.slotsUsed.indexOf(prt)).get(index);
    }

    public String getPortName() {
		return portName;
	}


	public void setPortName(String portName) {
		this.portName = portName;
	}

	public RealExpr getSlotStartZ3(int prt, int slotNum) {
		return this.slotStartZ3.get(prt).get(slotNum);
	}

	/*
	public RealExpr getSlotDurationZ3(int prt, int slotNum) {
		return this.slotDurationZ3.get(prt).get(slotNum);
	}
	*/
};

}

#endif

