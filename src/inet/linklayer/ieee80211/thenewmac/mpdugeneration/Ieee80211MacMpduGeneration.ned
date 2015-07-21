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
// This block converts outgoing Msdus and Mmpdus
// into Mpdus, fragmenting and encrypting as necessary.
// If the station is in a Bss, outgoing Msdus are directed
// via distribution service at the AP. The PM_Filter
// process queues frames needing announcement by Atim in
// an Ibss; or frames to be sent in the CF-period at a
// CF-pollable station in a Bss.
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
