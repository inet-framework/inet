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


// Includes validate, decrypt, address & duplicate filter,
// defragment, channel state (physical and virtual carrier
// sense), and IFS & slot timing.
//
module Ieee80211MacReception
{
    parameters:

        @display("bgb=349,259");
    gates:
        output csToTx;
        input psFromSync;
        output psToPs;
        input rxFromCtl;
        output rxToRx;
        output rxIndAck;
        inout phyCca;
        
    submodules:
        channelState: Ieee80211MacChannelState {
            @display("p=84,75");
        }
        mpduFilter: Ieee80211MacMpduFilter {
            @display("p=279,75");
        }
        mpduValidator: Ieee80211MacMpduValidator {
            @display("p=84,187");
        }
        defragment: Ieee80211MacDefragment {
            @display("p=279,187");
        }

    connections:
        channelState.toTx --> csToTx;
        channelState.updNav <-- mpduFilter.updNav;
        channelState.ifsCtl <-- mpduValidator.ifsCtl;
        channelState.fromSync <-- psFromSync;
        channelState.fromCtl <-- rxFromCtl;
        channelState.phyCca <--> phyCca;
        
        defragment.toRx --> rxToRx;
        defragment.defrag <-- mpduFilter.defrag;

        mpduFilter.indAck --> rxIndAck;
        mpduFilter.toPs --> psToPs;
}
