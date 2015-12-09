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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.ieee80211.mac;

simple BasicRateSelection like IRateSelection
{
    parameters:
        string modeSet @enum("a", "b", "g", "n", "p") = default("g");
        double dataBitrate @unit("bps") = default(-1bps); // unicast data and management frames
        double multicastBitrate @unit("bps") = default(-1bps); // multicast data and management frames
        double controlBitrate @unit("bps") = default(-1bps); // RTS, CTS, ACK, etc.
        @display("i=block/cogwheel");
}
