#ifndef __INET_Z3_SWITCH_H
#define __INET_Z3_SWITCH_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * [Class]: Switch
 * [Usage]: Contains most of the properties of a normal
 * switch that are used to build the schedule. Since this
 * scheduler doesn't take in consideration scenarios where
 * normal switches and TSN switches interact, no z3 properties
 * had to be specified in this class.
 *
 * Can be used to further extend the usability of this
 * project in the future.
 *
 */
class INET_API Switch {

	protected String name;
    protected float maxPacketSize;
    protected float timeToTravel;
    protected float transmissionTime;
    protected float portSpeed;

    //protected RealExpr maxPacketSizeZ3;
    //protected RealExpr timeToTravelZ3;
    //protected RealExpr transmissionTimeZ3;
    //protected RealExpr portSpeedZ3;


    /**
     * [Method]: Switch
     * [Usage]: Default constructor of class Switch. Is used by
     * TSNSwitch to instantiate new children classes.
     */
    public Switch() {

    }

    /**
     * [Method]: Switch
     * [Usage]: Overloaded constructor of class Switch. Can
     * instantiate new switches using properties given as
     * parameters.
     *
     * @param name                  Name of the switch
     * @param maxPacketSize         Maximum size of a packet supported by this switch
     * @param timeToTravel          Time that a packet takes to leave its port and reach the destination
     * @param transmissionTime      Time taken to process the packet inside the switch
     * @param portSpeed             Transmission speed of the port
     */
    public Switch(String name,
                  float maxPacketSize,
                  float timeToTravel,
                  float transmissionTime,
                  float portSpeed) {
        this.name = name;
        this.maxPacketSize = maxPacketSize;
        this.timeToTravel = timeToTravel;
        this.transmissionTime = transmissionTime;
        this.portSpeed = portSpeed;
    }


    /*
     *  GETTERS AND SETTERS
     */


    public float getMaxPacketSize() {
        return maxPacketSize;
    }

    public void setMaxPacketSize(float maxPacketSize) {
        this.maxPacketSize = maxPacketSize;
    }

    public float getTimeToTravel() {
        return timeToTravel;
    }

    public void setTimeToTravel(float timeToTravel) {
        this.timeToTravel = timeToTravel;
    }

    public float getTransmissionTime() {
        return transmissionTime;
    }

    public void setTransmissionTime(float transmissionTime) {
        this.transmissionTime = transmissionTime;
    }

    public float getPortSpeed() {
        return portSpeed;
    }

    public void setPortSpeed(float portSpeed) {
        this.portSpeed = portSpeed;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return this.name;
    }
};

}

#endif

