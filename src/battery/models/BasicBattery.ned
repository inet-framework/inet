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
// @short Implementation of a simple battery model
//      "real" battery models should subclass this!
//      The basic class publishes the remaining energy on the notification board,
//      but does not decrement the energy!
//      It can therefore be used for hosts having an infinite energy supply
//      i.e. a power plug
//
//
//      capacity (mA-H): battery capacity
//      nominal(mA-H): nominal battery capacity
//      voltage (V): nominal voltage
//      resolution (time): capacity is updated at least every resolution time
//      publishDelta(0..1): capacity is published to the BB each time it is
//              observed to have changed by publishDelta * nominal_capacity
//      publishTime: if > 0, capacity is published to the BB each publishTime
//              interval,
// Note that resolution should be much smaller than the expected time to
// first node failure.
//
simple BasicBattery like IBattery
{
}
