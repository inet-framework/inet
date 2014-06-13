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

package inet.node.inet;

//
// Models a host with (default) one wireless (802.11) card in infrastructure mode.
// This module is basically a ~StandardHost with an ~Ieee80211Nic with
// mgmtType = ~Ieee80211MgmtSTA added.
// It should be used in conjunction with ~AccessPoint, or any other AP model
// which contains ~Ieee80211Nic with mgmtType = ~Ieee80211MgmtAP.
//
module WirelessHost extends StandardHost
{
    @display("i=device/wifilaptop");
    numRadios = default(1);
}
