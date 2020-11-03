//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.linklayer.ieee8021d.relay;

import inet.linklayer.base.MacRelayUnitBase;
import inet.linklayer.contract.IMacRelayUnit;


//
// This modul forwards frames (~EtherFrame) based on their destination MAC addresses to appropriate ports.
//
// It can work in two mode:
// - STP unaware mode (If no module connected to its STP gate):
//     Looks up the frames destination address in the ~MACAddressTable, if it found the address, sends through
//	   that port, or otherwise broadcasts it.
// - STP aware mode (If an ~STP or ~RSTP module connected to its STP gate):
//     Handles BPDU frames.
//     If relaying a message also checks the port's status in the InterfaceTable.
//
// @see EtherFrame, MacAddressTable, Stp, Rstp
//
simple Ieee8021dRelay extends MacRelayUnitBase like IMacRelayUnit
{
    parameters:
        bool hasStp;
        @class(Ieee8021dRelay);
}

