//
// Copyright (C) 2012 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.networklayer.common;

import inet.common.packet.recorder.PcapRecorder;

module InterfaceEntry
{
    parameters:
        bool recordPcap = default(false);
        int numPcapRecorders = default(recordPcap ? 1 : 0);
        string displayStringTextFormat = default("%a (%i)\n%m");
        pcapRecorder[*].pcapFile = default("results/" + fullPath() + ".pcap");
    submodules:
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                moduleNamePatterns = ".^";
                @display("p=100,100;is=s");
        }
}

