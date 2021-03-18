//
// Copyright (C) 2012 OpenSim Ltd.
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

package inet.networklayer.common;

import inet.clock.contract.IClock;
import inet.common.packet.recorder.PcapRecorder;

//
// This module serves as the base module for all network interfaces.
//
module NetworkInterface
{
    parameters:
        string displayStringTextFormat = default("%a (%i)\n%m");
        bool recordPcap = default(false);
        int numPcapRecorders = default(recordPcap ? 1 : 0);
        pcapRecorder[*].pcapFile = default("results/" + fullPath() + ".pcap");
        **.clockModule = default(exists(clock) ? absPath(".clock") : "");
        @networkInterface();
        @class(NetworkInterface);
        @display("i=block/ifcard");
    submodules:
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                moduleNamePatterns = ".^";
                @display("p=100,100;is=s");
        }
        clock: <default("")> like IClock if typename != "" {
            parameters:
                @display("p=100,200;is=s");
        }
}

