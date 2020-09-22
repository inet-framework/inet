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

package inet.common.misc;

//
// This channels adds support for thruput metering to the datarate channel.
// A cDatarateChannel extended with throughput calculation. Values
// are displayed on the link, using the connection's "t=" or "tt=" display
// string tag.
//
// The display can be customized with the "thruputDisplayFormat" parameter.
// In the format string, the following characters will get expanded:
//   - 'N': number of packets
//   - 'V': volume (in bytes)
//   - 'p': current packet/sec
//   - 'b': current bandwidth
//   - 'u': current channel utilization (%)
//   - 'P': average packet/sec on [0,now)
//   - 'B': average bandwidth on [0,now)
//   - 'U': average channel utilization (%) on [0,now)
//
// Other characters are copied verbatim.
//
// "Current" actually means the last measurement interval, which is
// 10 packets or 0.1s, whichever comes first.
//
channel ThruputMeteringChannel extends ned.DatarateChannel
{
    @class(ThruputMeteringChannel);
    bool displayAsTooltip = default(false);
    string thruputDisplayFormat = default("");  // the format string for display on the link. Leave empty to disable display
}

