// 
// Author:  Alfonso Ariza
// Based in the mixim code Author:  Laura Marie Feeney
// 
// Copyright 2009 Malaga University.
// Copyright 2009 Swedish Institute of Computer Science.
//   
//  
// This software is provided `as is' and without any express or implied
// warranties, including, but not limited to, the implied warranties of
// merchantability and fitness for a particular purpose.
//
// Simple battery model for inetmanet framework

package inet.battery.models;

import inet.battery.IBattery;


// 
// a simple battery module
//
// Publish capacity to BatteryStats every publishTime (if > 0) and
// whenever capacity has changed by publishDelta (if < 100%).
//
simple SimpleBattery like IBattery
{
    parameters:
        double nominal @unit(mAh);   //nominal battery capacity
        double capacity @unit(mAh);  // initial capacity of battery
        double voltage @unit(V);     // nominal voltage
        double resolution @unit(s);  //  capacity is updated at least every resolution time
        double publishDelta;         // [0,1] capacity is published to the BB each time it is
                                     // observed to have changed by publishDelta * nominal_capacity
        double publishTime @unit(s); // if > 0, capacity is published to the BB each publishTime
                                     // interval, 
        @statistic[currCapacity](title="Current battery capacity [%]"; record=min,max,vector);
        @statistic[consumedEnergy](title="Consumed energy [V*mA*s]"; record=histogram,vector);
}
