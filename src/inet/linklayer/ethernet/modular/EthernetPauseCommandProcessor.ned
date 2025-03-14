//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

//
// Processes Ethernet PAUSE commands and generates IEEE 802.3x PAUSE frames for
// flow control in Ethernet networks. When a PAUSE command is received, creates
// and sends a PAUSE frame with the specified duration to the destination MAC
// address, typically the multicast PAUSE address (01-80-C2-00-00-01).
//
// PAUSE frames are a flow control mechanism in full-duplex Ethernet that allow
// a congested receiver to request that the sender temporarily stop transmitting
// data for a specified period. This helps prevent buffer overflow and packet loss
// in switches and other network devices during periods of congestion.
//
// The module emits a signal for each PAUSE frame sent, recording the pause duration
// in units of 512 bit times. This allows for monitoring and statistics collection
// on the flow control behavior of the network.
//
// @see ~EthernetMacLayer, ~EthernetLayer
//
simple EthernetPauseCommandProcessor
{
    parameters:
        @class(EthernetPauseCommandProcessor);
        @display("i=block/inserter");
        @signal[pauseSent](type=long); // Pause units
        @statistic[pauseSent](title="pause sent";record=count,sum,vector);
    gates:
        input in @labels(send);
        output out @labels(send);
}
