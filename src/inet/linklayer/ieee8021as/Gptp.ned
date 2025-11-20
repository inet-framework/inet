//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Based on work by: Enkhtuvshin Janchivnyambuu, Henning Puttnies, Peter Danielis at University of Rostock, Germany
//

package inet.linklayer.ieee8021as;

import inet.clock.contract.IClockServo;
import inet.common.Module;
import inet.linklayer.contract.IGptp;

//
// Implements the generalized Precision Time Protocol (gPTP) as defined in the IEEE 802.1AS-2020 standard.
//
// This module performs periodic peer delay measurements to estimate the link delay to neighboring gPTP-capable nodes.
// The roles of connected ports (e.g., master or slave) define their function in the time synchronization hierarchy.
// Time synchronization is also performed periodically by exchanging Sync and Follow_Up messages, and the local clock
// module is adjusted accordingly.
//
// This module requires an associated clock module to operate. The default simulation time precision is 1 ps which is
// not accurate enough for peer delay measurements, it must be set to 1 fs or less instead.
//
// Typical system parameters:
//  - Simulation time precision: 1 fs (instead of default 1 ps)
//  - Propagation delay: ~ 50 ns/m (cable speed of light ~ 2×10⁸ m/s)
//  - Clock resolution: ~ 1 ns (less than 40 ns according to the standard)
//  - Clock drift: less than ±100 ppm according to the standard
//  - Clock drift variation: less than ±0.01 ppm/s according to the standard
//  - Peer delay measurement interval: 1 s (default parameter)
//  - Peer delay measurement accuracy: less than ~ 1 ns (typically)
//  - Time synchronization interval: 125 ms (default parameter)
//  - Time synchronization accuracy: less than ~ 1 us (typically)
//
module Gptp extends Module like IGptp
{
    parameters:
        string clockModule; // Relative path of a module that implements IClock
        string clockServoModule = default(".clockServo"); // Relative path of a module that implements IClockServo
        string interfaceTableModule; // Relative path of the interface table module

        string gptpNodeType; // @enum("GptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
        int domainNumber = default(0); // Specifies the time domain number used in gPTP messages

        string slavePort = default(""); // Port for receiving time (empty for MASTER_NODE)
        object masterPorts = default([]); // Ports for sending out time (empty for SLAVE_NODE)

        double syncInterval @unit(s) = default(0.125s); // Time interval between SYNC messages
        double syncInitialOffset @unit(s) = default(syncInterval); // Time of first SYNC message

        double pdelayInterval @unit(s) = default(1s); // Frequency of link delay measurements
        double pdelaySmoothingFactor = default(0.1); // Exponential averaging for mean peer delay, 1 means no averaging
        double pdelayInitialOffset @unit(s) = default(0s); // Time of first link delay measurement

        @class(Gptp);
        @display("i=block/timer");
        @signal[gmRateRatioChanged](type=double);
        @signal[neighborRateRatioChanged](type=double);
        @signal[pdelayChanged](type=simtime_t);
        @signal[packetDropped](type=inet::Packet);
        @statistic[gmRateRatio](record=vector; source=gmRateRatioChanged; interpolationmode=sample-hold);
        @statistic[neighborRateRatio](record=vector; source=neighborRateRatioChanged; interpolationmode=sample-hold);
        @statistic[pdelay](record=vector; source=pdelayChanged; interpolationmode=sample-hold; unit=s);
        @statistic[packetDropNotAddressedToUs](title="packet drop: not addressed to us"; source=packetDropReasonIsNotAddressedToUs(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
    gates:
        input socketIn;
        output socketOut;
    submodules:
        clockServo: <default("StepClockServo")> like IClockServo if typename != "" && gptpNodeType != "MASTER_NODE" {
            parameters:
                @display("p=100,100");
                clockModule = default(parent.clockModule);
        }
}

