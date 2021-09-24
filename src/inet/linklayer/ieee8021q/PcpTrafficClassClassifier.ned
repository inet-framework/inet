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

package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

module PcpTrafficClassClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string mode @enum("req", "ind", "both") = default("both");
        // IEEE Standard for Local and Metropolitan Area Networks, Bridges and Bridged Networks
        // Table 8-5—Recommended priority to traffic class mappings
        object mapping = default([[0, 0, 0, 0, 0, 1, 1, 1],   // BE best effort
                                  [0, 0, 0, 0, 0, 0, 0, 0],   // BK background
                                  [0, 0, 0, 1, 1, 2, 2, 2],   // EE excellent effort
                                  [0, 0, 0, 1, 1, 2, 3, 3],   // CA critical applications
                                  [0, 1, 1, 2, 2, 3, 4, 4],   // VI video
                                  [0, 1, 1, 2, 2, 3, 4, 5],   // VO voice
                                  [0, 1, 2, 3, 3, 4, 5, 6],   // IC internetwork control
                                  [0, 1, 2, 3, 4, 5, 6, 7]]); // NC network control
        int defaultGateIndex = default(0); // default gate index if no PCP found
        @class(PcpTrafficClassClassifier);
}
