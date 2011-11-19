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
simple InetSimpleBattery like IBattery
{
    parameters:
        double nominal;
        double capacity;
        double voltage;
        double resolution @unit(s);
        double publishDelta;
        double publishTime @unit(s);
        bool ConsumedVector = default(false);
}
