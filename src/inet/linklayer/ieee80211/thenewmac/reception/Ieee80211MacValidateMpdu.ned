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
// This process receives an MPDU from the
// PHY while calculating and checking the
// FCS value. Frames with valid FCS, length
// and protocol version are sent for receive
// filtering, along with a snapshot of the WEP
// keys if dot11PrivacyOptionImplemented=true.
//
// This process also provides Channel_State
// with Difs/Eifs and Rts timeout signals,
// and maintains the mRxA remote variable.
//
simple Ieee80211MacValidateMpdu
{
    parameters:
        string macsortsPackage = default("^.^.macsorts");
        string macmibPackage = default("^.^.macmib");

	gates:
	    output ifsCtl;
}
