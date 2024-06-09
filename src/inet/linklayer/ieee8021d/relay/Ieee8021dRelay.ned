//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021d.relay;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;


//
// This module forwards frames (~EtherFrame) based on their destination MAC addresses to appropriate ports.
//
// It can work in two modes:
// - STP unaware mode (If no module is connected to its STP gate):
//     Looks up the frame's destination address in the ~MacForwardingTable. If it finds the address, it sends it through
//     that port; otherwise, it broadcasts it.
// - STP aware mode (If an ~Stp or ~Rstp module is connected to its STP gate):
//     Handles BPDU frames.
//     If relaying a message, it also checks the port's status in the `InterfaceTable`.
//
// @see ~EtherFrame, ~MacForwardingTable, ~Stp, ~Rstp
//
simple Ieee8021dRelay extends MacRelayUnitBase like IMacRelayUnit
{
    parameters:
        @class(Ieee8021dRelay);
}

