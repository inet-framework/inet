//
// Copyright (C) 2016 OpenSim Ltd.
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

package inet.linklayer.newcsma;

simple Ieee80211Mac
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        string queueModule = default("");    // name of optional external queue module
        int maxQueueSize; // max queue length in frames; only used if queueModule==""
        double bitrate @unit("bps");
        int rtsThresholdBytes @unit("B") = default(2346B); // longer messages will be sent using RTS/CTS
        int retryLimit = default(-1); // maximum number of retries per message, -1 means default
        int cwMinData = default(-1); // contention window for normal data frames, -1 means default
        int cwMinBroadcast = default(-1); // contention window for broadcast messages, -1 means default
        int mtu = default(1500);
        @display("i=block/layer");
    gates:
        input uppergateIn;
        output uppergateOut;
        input lowergateIn;
        output lowergateOut;
}
