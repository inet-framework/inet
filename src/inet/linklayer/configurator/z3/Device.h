#ifndef __INET_Z3_DEVICE_H
#define __INET_Z3_DEVICE_H

// CLASS WHERE DEVICE PROPERTIES AND CONDITIONS ARE SPECIFIED


/**
 * [Class]: Device
 * [Usage]: Specifies properties of device nodes in the network.
 * They can be used as sending devices and receiving devices.
 * Their properties specify part of the core of a flow.
 */
public class Device implements Serializable {

	private static final long serialVersionUID = 1L;
	private String name;
    private float packetPeriodicity;
    private float firstT1Time;
    private float hardConstraintTime;
    private float softConstraintTime;
    private float packetSize;

    private static int indexCounter = -1;
    private transient RealExpr packetPeriodicityZ3;
	private transient RealExpr firstT1TimeZ3;
	private transient RealExpr hardConstraintTimeZ3;
	private transient RealExpr softConstraintTimeZ3;
    private transient RealExpr packetSizeZ3;
    private transient IntExpr flowPriority;

    /**
     * [Method]: Device
     * [Usage]: Default constructor method of a device.
     * Sets the newly created device name.
     */
    public Device() {
		this.name = "dev" + indexCounter++;
	}

    /**
     * [Method]: Device
     * [Usage]: Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * float values. These values will later be converted to
     * z3 values. Used for simplified configurations. Other
     * constructors either are deprecated or set parameters
     * that will be used in future works.
     *
     * @param packetPeriodicity     Periodicity of packet sending
     * @param hardConstraintTime    Maximum latency tolerated by this device
     */
    public Device(float packetPeriodicity,
                  float hardConstraintTime) {
        this.packetPeriodicity = packetPeriodicity;
        this.firstT1Time = 0;
        this.hardConstraintTime = hardConstraintTime;
        this.softConstraintTime = 0;
        this.packetSize = 0;
        this.name = "dev" + ++indexCounter;
    }

    /**
     * [Method]: Device
     * [Usage]: Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * float values. These values will later be converted to
     * z3 values.
     *
     * @param packetPeriodicity     Periodicity of packet sending
     * @param firstT1Time           Time where the first packet is sent
     * @param hardConstraintTime    Maximum latency tolerated by this device
     * @param softConstraintTime    Recommended latency for using this device
     * @param packetSize            Size of the packets sent by this device
     */
    public Device(float packetPeriodicity,
                  float firstT1Time,
                  float hardConstraintTime,
                  float softConstraintTime,
                  float packetSize) {
        this.packetPeriodicity = packetPeriodicity;
        this.firstT1Time = firstT1Time;
        this.hardConstraintTime = hardConstraintTime;
        this.softConstraintTime = softConstraintTime;
        this.packetSize = packetSize;
        this.name = "dev" + ++indexCounter;
    }

    /**
     * [Method]: Device
     * [Usage]: Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * float values. These values will later be converted to
     * z3 values.
     *
     * @param packetPeriodicity     Periodicity of packet sending
     * @param firstT1Time           Time where the first packet is sent
     * @param hardConstraintTime    Maximum latency tolerated by this device
     * @param packetSize            Size of the packets sent by this device
     */
    public Device(float packetPeriodicity,
                  float firstT1Time,
                  float hardConstraintTime,
                  float packetSize) {
        this.packetPeriodicity = packetPeriodicity;
        this.firstT1Time = firstT1Time;
        this.hardConstraintTime = hardConstraintTime;
        this.softConstraintTime = 0;
        this.packetSize = packetSize;
        this.name = "dev" + ++indexCounter;
    }


    /**
     * [Method]: Device
     * [Usage]: Overloaded constructor method of a device.
     * Can create a device specifying its properties through
     * z3 values.
     *
     * @param packetPeriodicityZ3       Periodicity of packet sending
     * @param firstT1TimeZ3             Time where the first packet is sent
     * @param hardConstraintTimeZ3      Maximum latency tolerated by this device
     * @param softConstraintTimeZ3      Recommended latency for using this device
     * @param packetSizeZ3              Size of the packets sent by this device
     * @param flowPriority              Defines the priority queue in which this device packets belongs to (Not used yet)
     */
	public Device(RealExpr packetPeriodicityZ3,
				  RealExpr firstT1TimeZ3,
				  RealExpr hardConstraintTimeZ3,
				  RealExpr softConstraintTimeZ3,
				  RealExpr packetSizeZ3,
				  IntExpr flowPriority) {
		this.packetPeriodicityZ3 = packetPeriodicityZ3;
		this.firstT1TimeZ3 = firstT1TimeZ3;
		this.hardConstraintTimeZ3 = hardConstraintTimeZ3;
		this.softConstraintTimeZ3 = softConstraintTimeZ3;
		this.packetSizeZ3 = packetSizeZ3;
		this.flowPriority = flowPriority;
        this.name = "dev" + indexCounter++;
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
	    this.packetPeriodicityZ3 = ctx.mkReal(Float.toString(this.packetPeriodicity));
	    //this.firstT1TimeZ3 = ctx.mkReal(Float.toString(this.firstT1Time)); // In case of fixed firstT1Time
	    this.firstT1TimeZ3 = ctx.mkRealConst(this.name + "FirstT1Time");
	    this.hardConstraintTimeZ3 = ctx.mkReal(Float.toString(this.hardConstraintTime));
	    this.softConstraintTimeZ3 = ctx.mkReal(Float.toString(this.softConstraintTime));
	    this.packetSizeZ3 = ctx.mkReal(Float.toString(this.packetSize));
	}

    /*
     *  GETTERS AND SETTERS
     */

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

	public float getPacketPeriodicity() {
        return packetPeriodicity;
    }

    public void setPacketPeriodicity(float packetPeriodicity) {
        this.packetPeriodicity = packetPeriodicity;
    }

    public float getFirstT1Time() {
        return firstT1Time;
    }

    public void setFirstT1Time(float firstT1Time) {
        this.firstT1Time = firstT1Time;
    }

    public float getHardConstraintTime() {
        return hardConstraintTime;
    }

    public void setHardConstraintTime(float hardConstraintTime) {
        this.hardConstraintTime = hardConstraintTime;
    }

    public float getSoftConstraintTime() {
        return softConstraintTime;
    }

    public void setSoftConstraintTime(float softConstraintTime) {
        this.softConstraintTime = softConstraintTime;
    }

    public float getPacketSize() {
        return packetSize;
    }

    public void setPacketSize(float packetSize) {
        this.packetSize = packetSize;
    }

    /*
     *  Z3 GETTERS AND SETTERS
     */

	public RealExpr getPacketPeriodicityZ3() {
		return packetPeriodicityZ3;
	}

	public void setPacketPeriodicityZ3(RealExpr packetPeriodicity) {
		this.packetPeriodicityZ3 = packetPeriodicity;
	}

	public RealExpr getFirstT1TimeZ3() {
		return firstT1TimeZ3;
	}

	public void setFirstT1TimeZ3(RealExpr firstT1Time) {
		this.firstT1TimeZ3 = firstT1Time;
	}

	public RealExpr getHardConstraintTimeZ3() {
		return hardConstraintTimeZ3;
	}

	public void setHardConstraintTimeZ3(RealExpr hardConstraintTime) {
		this.hardConstraintTimeZ3 = hardConstraintTime;
	}

	public RealExpr getSoftConstraintTimeZ3() {
		return softConstraintTimeZ3;
	}

	public void setSoftConstraintTimeZ3(RealExpr softConstraintTime) {
		this.softConstraintTimeZ3 = softConstraintTime;
	}

    public RealExpr getPacketSizeZ3() {
        return packetSizeZ3;
    }

    public void setPacketSizeZ3(RealExpr packetSize) {
        this.packetSizeZ3 = packetSize;
    }

    public IntExpr getFlowPriority() {
        return flowPriority;
    }

    public void setFlowPriority(IntExpr flowPriority) {
        this.flowPriority = flowPriority;
    }



}

#endif

