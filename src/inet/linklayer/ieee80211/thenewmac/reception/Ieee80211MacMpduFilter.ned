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

package inet.linklayer.ieee80211.thenewmac.reception;

//
// This process filters valid received frames by destination address, and
// BssId for group destination addresses. This process also maintains received
// pdu counters and the tuple cache for detecting duplicated unicast frames.
//
// Data and management frames which need acknowledgment cause a
// NeedAck signal to Protocol_Control as well as an RxMpdu to Defragment.
// Piggybacked CfAcks cause RxCfack signals, and CfPolls cause RxCfpoll
// signals to Protocol_Control. If an RxCfPoll is sent for a Data+CfPoll
// or Data+CfPoll+CfAck, the NeedAck has to reach TxCoord during the Sifs.
// (The data frame report cannot serve this purpose because the payload may
// be a nonfinal fragment.)
//
// Duration and Cfp duration remaining are reported to Channel_State, and
// power save mode is reported to Mlme.
//
simple Ieee80211MacMpduFilter
{
    gates:
		output updNav; // [SetNav, ClearNav]
		output defrag; // [RxMpdu]
		output indAck; // [NeedAck, RxCfAck, RxCfPoll]
		output toPs; // [PsIndicate]
}
