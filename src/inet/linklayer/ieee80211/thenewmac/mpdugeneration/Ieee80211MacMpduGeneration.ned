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

package inet.linklayer.ieee80211.thenewmac.mpdugeneration;

//
// Includes encryption, fragmentation, and power save queuing.
//
module Ieee80211MacMpduGeneration
{
    @display("bgb=125,198");
    gates:
        inout tpdu;
        inout mmTxMmpdu;
        inout mmTxPwrMgt;
        inout tsdu;

    submodules:
        prepareMpdu: Ieee80211MacPrepareMpdu {
            @display("p=67,55");
        }
        pmFilter: Ieee80211MacPmFilterSta {

            @display("p=67,125");
        }

    connections:
        prepareMpdu.fragMsdu <--> pmFilter.fragMsdu;

        // Outer links
        pmFilter.mpdu <--> tpdu;
        prepareMpdu.msdu <--> tsdu;
        prepareMpdu.mmpdu <--> mmTxMmpdu;
        pmFilter.pwrMgt <--> mmTxPwrMgt;
}
