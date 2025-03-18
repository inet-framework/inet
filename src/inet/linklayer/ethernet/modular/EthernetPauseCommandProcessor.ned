//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.common.SimpleModule;

//
// Processes Ethernet PAUSE commands for flow control. Intercepts
// Ieee802PauseCommand control messages and creates EthernetPauseFrame packets
// with the specified pause time. Sends these frames to the destination address
// (or the multicast pause address if unspecified) and tracks statistics about
// sent PAUSE commands. Passes through all other messages unchanged.
//
simple EthernetPauseCommandProcessor extends SimpleModule
{
    parameters:
        @class(EthernetPauseCommandProcessor);
        //@class(EthernetPauseCommandProcessor);
        @display("i=block/inserter");
        @signal[pauseSent](type=long); // Pause units
        @statistic[pauseSent](title="pause sent";record=count,sum,vector);
    gates:
        input in @labels(send);
        output out @labels(send);
}
