//
// Copyright (C) 2015 Andras Varga
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

package inet.linklayer.ieee80211.newmac;

//
// TODO
//
simple BasicUpperMac like IUpperMac
{
    parameters:
        string macModule;
        string rxModule;
        string immediateTxModule;
        string firstContentionTxModule;

        string opMode @enum("a", "b", "g", "n", "p") = default("g");
        double bitrate @unit("bps");
        int rtsThresholdBytes @unit("B") = default(2346B); // longer frames will be sent using RTS/CTS
        int retryLimit = default(-1); // maximum number of retries per frame, -1 means default
        @display("i=block/layer");
}
