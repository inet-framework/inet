//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.nodes.adhoc;



//
// Models a mobile host with a wireless (802.11b) card in ad-hoc mode.
// This model contains the new IEEE 802.11 implementation ~Ieee80211Nic,
// and IP, TCP and UDP protocols. The mobility model can be
// dynamically specified with the mobilityType parameter.
//
// (deprecated - use AdhocHost instead with Ieee80211NicAdhoc as the NIC parameter)
//
// @see ~MFMobileHost, ~WirelessHost
//
module MobileHost extends AdhocHost
{
    @display("i=device/pocketpc_s");
    IPForward = default(false);
}
