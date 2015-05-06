//
// Copyright (C) 2015 OpenSim Ltd.
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

package inet.linklayer.ieee80211.thenewmac.transmission;

//
// This process sends an Mpdu to the Phy while generating & appending the Fcs. 
// On beacons and probe responses inserts (TSF + Phy TxDelay) in the timestamp
// field at confirm of octet 23
//
simple Ieee80211MacDataPump
{
    parameters:
        string macsortsModule = default("^.^.macsorts");

    gates:
        input fromCs; // [Busy, Idle, Slot]
        output fwdCs; // [Busy, Idle, Slot]
        inout txRq[2]; // To TX : [TxConfirm], from TX: [TxRequest]
//        inout toPhy; unimplemented
}
