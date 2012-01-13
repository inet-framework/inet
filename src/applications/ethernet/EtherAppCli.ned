//
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
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

package inet.applications.ethernet;

//
// A simple traffic generator for the Ethernet model and the 802.11 model,
// and generally for any L2 model that accepts Ieee80211Ctrl control info
// on the packets. It generates EtherAppReq packets. It should be connected
// directly to EtherLLC or a Ieee802NicXXX module.
//
simple EtherAppCli
{
    parameters:
        string destAddress = default("");  // destination MAC address, or module path name of destination station; empty means off
        double startTime @unit("s") = default(this.waitTime);  // time of sending the first request
        volatile double waitTime @unit("s") = default(uniform(0s,1s));  // interval between sending requests
        volatile int reqLength @unit(B) = default(100B);  // length of request packets
        volatile int respLength @unit(B) = default(1KiB);  // length of response packets
        bool registerSAP = default(false);   // whether to sent IEEE802CTRL_REGISTER_DSAP on startup
        @display("i=block/app");
    gates:
        input in @labels(Ieee802Ctrl/up);
        output out @labels(Ieee802Ctrl/down);
}
