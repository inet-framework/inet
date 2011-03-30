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

package inet.nodes.wireless;


import inet.nodes.inet.StandardHost;


//
// Models a host with one wireless (802.11b) card in infrastructure mode.
// This module is basically a ~StandardHost with an ~Ieee80211NicSTA added.
// It should be used in conjunction with ~WirelessAP, or any other AP model
// which contains ~Ieee80211NicAP.
//
// @see ~WirelessAP, ~WirelessAPSimplified, ~WirelessAPWithEth, ~WirelessAPWithEthSimplified
// @see ~WirelessHost, ~WirelessHostSimplified
// @see ~MobileHost, ~MFMobileHost
//
// @deprecated: use StandardHost with numRadios=1 instead
//
module WirelessHost extends StandardHost
{
    @display("i=device/wifilaptop");
    mobilityType = default("NullMobility");
    numRadios = 1;
}

