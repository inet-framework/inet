//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021d.relay;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;


//
// This modul forwards frames (~EtherFrame) based on their destination MAC addresses to appropriate ports.
//
// It can work in two mode:
// - STP unaware mode (If no module connected to its STP gate):
//     Looks up the frames destination address in the ~MacForwardingTable, if it found the address, sends through
//     that port, or otherwise broadcasts it.
// - STP aware mode (If an ~STP or ~RSTP module connected to its STP gate):
//     Handles BPDU frames.
//     If relaying a message also checks the port's status in the InterfaceTable.
//
// @see EtherFrame, MacForwardingTable, Stp, Rstp
//
simple Ieee8021dRelay extends MacRelayUnitBase like IMacRelayUnit
{
    parameters:
        @class(Ieee8021dRelay);
}

