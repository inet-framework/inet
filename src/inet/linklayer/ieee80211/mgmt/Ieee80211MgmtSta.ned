//
// Copyright (C) 2006 Andras Varga
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
// @author Andras Varga
//

package inet.linklayer.ieee80211.mgmt;

//
// Used in 802.11 infrastructure mode: handles management frames for a station (STA).
//
// Relies on the MAC layer (~Ieee80211Mac) for reception and transmission of frames.
//
simple Ieee80211MgmtSta like IIeee80211Mgmt
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        int numChannels = default(1);  // number of channels to scan
        string mibModule;
        string macModule;               // The path to the MAC module
        @display("i=block/cogwheel");
        @signal[l2Associated](type=inet::InterfaceEntry);
        @signal[l2BeaconLost](type=inet::InterfaceEntry);
    gates:
        input macIn @labels(Ieee80211MacHeader);
        output macOut @labels(Ieee80211MacHeader);
        input agentIn;   // used for attaching an agent algorithm
        output agentOut; // used for attaching an agent algorithm
}

